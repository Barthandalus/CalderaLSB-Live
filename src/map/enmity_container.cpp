/*
===========================================================================

Copyright (c) 2010-2015 Darkstar Dev Teams

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see http://www.gnu.org/licenses/

===========================================================================
*/

#include "../common/logging.h"
#include "../common/utils.h"

#include "ai/ai_container.h"
#include "alliance.h"
#include "enmity_container.h"
#include "entities/battleentity.h"
#include "entities/charentity.h"
#include "entities/mobentity.h"
#include "notoriety_container.h"
#include "packets/entity_update.h"
#include "status_effect_container.h"
#include "utils/battleutils.h"
#include "utils/charutils.h"
#include "utils/zoneutils.h"

/************************************************************************
 *                                                                       *
 *                                                                       *
 *                                                                       *
 ************************************************************************/

CEnmityContainer::CEnmityContainer(CMobEntity* holder)
: m_EnmityHolder(holder)
{
}

CEnmityContainer::~CEnmityContainer()
{
    Clear();
}

/************************************************************************
 *                                                                       *
 *  Clear Enmity List                                                    *
 *                                                                       *
 ************************************************************************/

void CEnmityContainer::Clear(uint32 EntityID)
{
    TracyZoneScoped;
    if (EntityID == 0)
    {
        // Iterate over all all entries and remove the relevant entry from their notoriety list
        for (const auto& listEntry : m_EnmityList)
        {
            if (const auto& maybeEntityObj = m_EnmityList.find(listEntry.first); maybeEntityObj != m_EnmityList.end())
            {
                auto entry = maybeEntityObj->second;
                if (entry.PEnmityOwner && m_EnmityHolder)
                {
                    entry.PEnmityOwner->PNotorietyContainer->remove(m_EnmityHolder);
                }
            }
        }
        m_EnmityList.clear();
        return;
    }
    else
    {
        if (const auto& maybeEntityObj = m_EnmityList.find(EntityID); maybeEntityObj != m_EnmityList.end())
        {
            auto entry = maybeEntityObj->second;
            if (entry.PEnmityOwner && m_EnmityHolder)
            {
                entry.PEnmityOwner->PNotorietyContainer->remove(m_EnmityHolder);
            }
        }
        m_EnmityList.erase(EntityID);
    }
    m_tameable = true;
}

void CEnmityContainer::LogoutReset(uint32 EntityID)
{
    if (const auto& enmity_obj = m_EnmityList.find(EntityID); enmity_obj != m_EnmityList.end())
    {
        enmity_obj->second.PEnmityOwner = nullptr;
        enmity_obj->second.active       = false;
    }
}

/************************************************************************
 *                                                                       *
 *  Минимальное (базовое) значение ненависти                             *
 *                                                                       *
 ************************************************************************/

void CEnmityContainer::AddBaseEnmity(CBattleEntity* PChar)
{
    TracyZoneScoped;
    m_EnmityList.emplace(PChar->id, EnmityObject_t{ PChar, 0, 0, false, 0 });
    PChar->PNotorietyContainer->add(m_EnmityHolder);
}

/************************************************************************
 *                                                                       *
 *  Calculate Enmity Bonus
 *                                                                       *
 ************************************************************************/

float CEnmityContainer::CalculateEnmityBonus(CBattleEntity* PEntity)
{
    TracyZoneScoped;
    int enmityBonus = PEntity->getMod(Mod::ENMITY);

    if (auto* PChar = dynamic_cast<CCharEntity*>(PEntity))
    {
        enmityBonus += PChar->PMeritPoints->GetMeritValue(MERIT_ENMITY_INCREASE, PChar) - PChar->PMeritPoints->GetMeritValue(MERIT_ENMITY_DECREASE, PChar);

        // DRK
        if (PChar->StatusEffectContainer->HasStatusEffect(EFFECT_SOULEATER))
        {
            enmityBonus -= PChar->PMeritPoints->GetMeritValue(MERIT_MUTED_SOUL, PChar);
        }

        // RNG
        if (PChar->StatusEffectContainer->HasStatusEffect(EFFECT_CAMOUFLAGE))
        {
            enmityBonus -= PChar->StatusEffectContainer->GetStatusEffect(EFFECT_CAMOUFLAGE)->GetPower();
        }
        if (PChar->StatusEffectContainer->HasStatusEffect(EFFECT_DOUBLE_SHOT) && PChar->getMod(Mod::DOUBLE_SHOT_AMMO) > 0)
        {
            enmityBonus -= PEntity->getMod(Mod::ENH_DOUBLE_SHOT);
        }
        if (PChar->StatusEffectContainer->HasStatusEffect(EFFECT_FLASHY_SHOT))
        {
            enmityBonus += 25;
        }
        if (PChar->StatusEffectContainer->HasStatusEffect(EFFECT_STEALTH_SHOT))
        {
            int16 merit = (100 + PChar->PMeritPoints->GetMeritValue(MERIT_STEALTH_SHOT, PChar)) / 100;
            merit = enmityBonus * merit;
            enmityBonus -= merit;
        }

        // SCH
        if (PChar->StatusEffectContainer->HasStatusEffect(EFFECT_LIGHT_ARTS) && PChar->getMod(Mod::ENH_ADDENDUM_WHITE) > 0)
        {
            enmityBonus -= PChar->getMod(Mod::ENH_ADDENDUM_WHITE);
        }
        if (PChar->StatusEffectContainer->HasStatusEffect(EFFECT_DARK_ARTS) && PChar->getMod(Mod::ENH_ADDENDUM_BLACK) > 0)
        {
            enmityBonus -= PChar->getMod(Mod::ENH_ADDENDUM_BLACK);
        }
    }

    float bonus = (100.f + std::clamp(enmityBonus, -50, 200)) / 100.f;

    return bonus;
}

/************************************************************************
 *                                                                       *
 *  Add entity to hate list                                              *
 *                                                                       *
 ************************************************************************/

void CEnmityContainer::UpdateEnmity(CBattleEntity* PEntity, int32 CE, int32 VE, bool withMaster, bool tameable)
{
    TracyZoneScoped;

    if (m_EnmityHolder->objtype != ENTITYTYPE::TYPE_MOB) // pets and trusts dont have enmity.
    {
        return;
    }

    // you're too far away so i'm ignoring you
    if (!IsWithinEnmityRange(PEntity))
    {
        CE = 0;
        VE = 0;
    }

    auto enmity_obj = m_EnmityList.find(PEntity->id);

    if (enmity_obj != m_EnmityList.end())
    {
        if (enmity_obj->second.PEnmityOwner == nullptr)
        {
            enmity_obj->second.PEnmityOwner = PEntity;
        }
        float bonus = CalculateEnmityBonus(PEntity);

        int32 newCE = (int32)(enmity_obj->second.CE + (CE > 0 ? CE * bonus : CE));
        int32 newVE = (int32)(enmity_obj->second.VE + (VE > 0 ? VE * bonus : VE));

        // Check for cap limit
        enmity_obj->second.CE     = std::clamp(newCE, 0, EnmityCap);
        enmity_obj->second.VE     = std::clamp(newVE, 0, EnmityCap);
        enmity_obj->second.active = true;

        if (CE + VE > 0 && PEntity->getMod(Mod::TREASURE_HUNTER) > enmity_obj->second.maxTH)
        {
            enmity_obj->second.maxTH = PEntity->getMod(Mod::TREASURE_HUNTER);
        }
    }
    else if (CE >= 0 && VE >= 0)
    {
        bool initial = true;
        for (auto&& enmityObject : m_EnmityList)
        {
            if (enmityObject.second.active)
            {
                initial = false;
                break;
            }
        }

        int16 maxTH = CE + VE > 0 ? PEntity->getMod(Mod::TREASURE_HUNTER) : 0;

        if (initial)
        {
            CE += 200;
        }

        float bonus = CalculateEnmityBonus(PEntity);

        CE = std::clamp((int32)(CE * bonus), 0, EnmityCap);
        VE = std::clamp((int32)(VE * bonus), 0, EnmityCap);

        m_EnmityList.emplace(PEntity->id, EnmityObject_t{ PEntity, CE, VE, true, maxTH });
        PEntity->PNotorietyContainer->add(m_EnmityHolder);

        if (withMaster && PEntity->PMaster != nullptr)
        {
            // add master to the enmity list (pet and charmed mob)
            if (PEntity->objtype == TYPE_PET || (PEntity->objtype == TYPE_MOB && PEntity->PMaster != nullptr && PEntity->PMaster->objtype == TYPE_PC))
            {
                AddBaseEnmity(PEntity->PMaster);
            }
        }
    }

    if (!tameable)
    {
        m_tameable = false;
    }
}

bool CEnmityContainer::HasID(uint32 TargetID)
{
    return std::find_if(m_EnmityList.begin(), m_EnmityList.end(), [TargetID](auto elem)
                        { return elem.first == TargetID && elem.second.active; }) !=
           m_EnmityList.end();
}

/************************************************************************
 *                                                                       *
 *                                                                       *
 *                                                                       *
 ************************************************************************/

void CEnmityContainer::UpdateEnmityFromCure(CBattleEntity* PEntity, uint8 level, int32 CureAmount, bool isCureV)
{
    TracyZoneScoped;
    if (!IsWithinEnmityRange(PEntity))
    {
        return;
    }

    int32 CE                     = 0;
    int32 VE                     = 0;
    float bonus                  = CalculateEnmityBonus(PEntity);
    float tranquilHeartReduction = 1.f - battleutils::HandleTranquilHeart(PEntity);

    if (isCureV)
    {
        CE = (int32)(400.f * bonus * tranquilHeartReduction);
        VE = (int32)(800.f * bonus * tranquilHeartReduction);
    }
    else
    {
        CureAmount = (CureAmount < 1 ? 1 : CureAmount);

        CE = (int32)(40.f / battleutils::GetEnmityModCure(level) * CureAmount * bonus * tranquilHeartReduction);
        VE = (int32)(240.f / battleutils::GetEnmityModCure(level) * CureAmount * bonus * tranquilHeartReduction);
    }

    auto enmity_obj = m_EnmityList.find(PEntity->id);

    if (enmity_obj != m_EnmityList.end())
    {
        enmity_obj->second.CE     = std::clamp(enmity_obj->second.CE + CE, 0, EnmityCap);
        enmity_obj->second.VE     = std::clamp(enmity_obj->second.VE + VE, 0, EnmityCap);
        enmity_obj->second.active = true;
    }
    else
    {
        m_EnmityList.emplace(PEntity->id, EnmityObject_t{ PEntity, std::clamp(CE, 0, EnmityCap), std::clamp(VE, 0, EnmityCap), true, 0 });
        PEntity->PNotorietyContainer->add(m_EnmityHolder);
    }
}

/************************************************************************
 *                                                                       *
 *    Lower enmity by percent %                                          *
 *                                                                       *
 ************************************************************************/

void CEnmityContainer::LowerEnmityByPercent(CBattleEntity* PEntity, uint8 percent, CBattleEntity* HateReceiver)
{
    TracyZoneScoped;
    auto enmity_obj = m_EnmityList.find(PEntity->id);

    if (enmity_obj != m_EnmityList.end())
    {
        float mod = ((float)(percent) / 100.0f);

        auto CEValue = (int16)(enmity_obj->second.CE * mod);
        enmity_obj->second.CE -= (CEValue < 0 ? 0 : CEValue);

        auto VEValue = (int16)(enmity_obj->second.VE * mod);
        enmity_obj->second.VE -= (VEValue < 0 ? 0 : VEValue);

        // transfer hate if HateReceiver not nullptr
        if (HateReceiver != nullptr)
        {
            UpdateEnmity(HateReceiver, CEValue, VEValue);
        }
    }
}

/************************************************************************
 *                                                                       *
 *    Returns the CE or VE for the current entity                        *
 *                                                                       *
 ************************************************************************/

int32 CEnmityContainer::GetCE(CBattleEntity* PEntity) const
{
    auto PEnmity = m_EnmityList.find(PEntity->id);
    return PEnmity != m_EnmityList.end() ? PEnmity->second.CE : 0;
}

int32 CEnmityContainer::GetVE(CBattleEntity* PEntity) const
{
    auto PEnmity = m_EnmityList.find(PEntity->id);
    return PEnmity != m_EnmityList.end() ? PEnmity->second.VE : 0;
}

/************************************************************************
 *                                                                       *
 *    Sets the CE or VE for the current entity                           *
 *                                                                       *
 ************************************************************************/

void CEnmityContainer::SetCE(CBattleEntity* PEntity, const int32 amount)
{
    auto PEnmity = m_EnmityList.find(PEntity->id);
    if (PEnmity != m_EnmityList.end())
    {
        PEnmity->second.CE = std::min(amount, EnmityCap);
    }
    else
    {
        AddBaseEnmity(PEntity);
        SetCE(PEntity, amount);
    }
}

void CEnmityContainer::SetVE(CBattleEntity* PEntity, const int32 amount)
{
    auto PEnmity = m_EnmityList.find(PEntity->id);
    if (PEnmity != m_EnmityList.end())
    {
        PEnmity->second.VE = std::min(amount, EnmityCap);
    }
    else
    {
        AddBaseEnmity(PEntity);
        SetVE(PEntity, amount);
    }
}

/************************************************************************
 *                                                                       *
 *                                                                       *
 *                                                                       *
 ************************************************************************/

void CEnmityContainer::UpdateEnmityFromDamage(CBattleEntity* PEntity, int32 Damage)
{
    TracyZoneScoped;
    Damage          = (Damage < 1 ? 1 : Damage);
    int16 level     = m_EnmityHolder->GetMLevel();
    int16 damageMod = battleutils::GetEnmityModDamage(level);
    double lvlScalingFactor = 1;

    if (level >= 51 && level <= 99)
    {
        lvlScalingFactor = 1 - ((level - 49) * 0.014);
    }
    else if (level > 99 && level <= 120)
    {
        lvlScalingFactor = 0.3;
    }
    else if (level > 120 && level <= 135)
    {
        lvlScalingFactor = 0.2;
    }
    else if (level > 135)
    {
        lvlScalingFactor = 0.15;
    }

    if (charutils::GetCharVar((CCharEntity*)PEntity, "AuditEnmity") == 1)
    {
        lvlScalingFactor += (charutils::GetCharVar((CCharEntity*)PEntity, "EnmityMult") / 10);
    }

    int32 CE = (int32)(((80.f / damageMod) * Damage) * lvlScalingFactor);
    int32 VE = (int32)(((240.f / damageMod) * Damage) * lvlScalingFactor);

    if (charutils::GetCharVar((CCharEntity*)PEntity, "AuditEnmity") == 1)
    {
        printf("enmity_container.cpp UpdateEnmityFromDamage  PLAYER: [%s]  DMG: [%i]  DMG MOD: [%i]  MOB LVL: [%i]  SCALING FACTOR: [%f]  CE: [%i]  VE: [%i]\n", PEntity->GetName(), Damage, damageMod, level, lvlScalingFactor, CE, VE);
    }

    UpdateEnmity(PEntity, CE, VE);

    if (m_EnmityHolder && m_EnmityHolder->m_HiPCLvl < PEntity->GetMLevel())
    {
        m_EnmityHolder->m_HiPCLvl = PEntity->GetMLevel();
    }
}

/************************************************************************
 *                                                                       *
 *                                                                       *
 *                                                                       *
 ************************************************************************/

void CEnmityContainer::UpdateEnmityFromAttack(CBattleEntity* PEntity, int32 Damage)
{
    TracyZoneScoped;
    if (auto enmity_obj = m_EnmityList.find(PEntity->id); enmity_obj != m_EnmityList.end())
    {
        float reduction = (100.f - std::min<int16>(PEntity->getMod(Mod::ENMITY_LOSS_REDUCTION), 100)) / 100.f;
        int32 CE        = (int32)(-1800.f * Damage / PEntity->GetMaxHP() * reduction);

        enmity_obj->second.CE = std::clamp(enmity_obj->second.CE + CE, 0, EnmityCap);
    }
}

/************************************************************************
 *                                                                       *
 *  Decay Enmity, Get Entity with the highest enmity                     *
 *                                                                       *
 ************************************************************************/

CBattleEntity* CEnmityContainer::GetHighestEnmity()
{
    TracyZoneScoped;
    if (m_EnmityList.empty())
    {
        return nullptr;
    }
    uint32 HighestEnmity = 0;
    auto   highest       = m_EnmityList.end();
    bool   active        = false;

    for (auto it = m_EnmityList.begin(); it != m_EnmityList.end(); ++it)
    {
        const EnmityObject_t& PEnmityObject = it->second;
        uint32                Enmity        = PEnmityObject.CE + PEnmityObject.VE;

        if (Enmity >= HighestEnmity && ((PEnmityObject.active == active) || (PEnmityObject.active && !active)))
        {
            auto* POwner = PEnmityObject.PEnmityOwner;
            if (!POwner || (POwner->allegiance != m_EnmityHolder->allegiance))
            {
                active        = PEnmityObject.active;
                HighestEnmity = Enmity;
                highest       = it;
            }
        }
    }
    CBattleEntity* PEntity = nullptr;
    if (highest != m_EnmityList.end())
    {
        PEntity = highest->second.PEnmityOwner;
        if (!PEntity)
        {
            PEntity = zoneutils::GetChar(highest->first);
        }

        if (!PEntity || PEntity->getZone() != m_EnmityHolder->getZone() || PEntity->PInstance != m_EnmityHolder->PInstance)
        {
            m_EnmityList.erase(highest);
            PEntity = GetHighestEnmity();
        }
    }
    return PEntity;
}

void CEnmityContainer::DecayEnmity()
{
    CBattleEntity* PEntity = nullptr;
    int16 playerLevel = 0;
    int16 playerItemLevel = 0;
    int16 decayVE = 1;
    int16 decayCE = 1;

    for (auto& it : m_EnmityList)
    {
        // printf("enmity_container.cpp DecayEnmity FOR LOOP\n");
        EnmityObject_t& PEnmityObject = it.second;
        PEntity = PEnmityObject.PEnmityOwner;

        if (PEntity != nullptr && PEnmityObject.CE > 1)
        {
            if (PEntity->objtype == TYPE_PC)
            {
                CCharEntity* PChar = (CCharEntity*)PEntity;
                playerItemLevel = (int16)(charutils::getItemLevelDifference(PChar));
            }

            playerLevel = PEntity->GetMLevel() + playerItemLevel;
            // printf("enmity_container.cpp DecayEnmity PLAYER LEVEL: [%i]\n", playerLevel);

            if (playerLevel <= 50)
            {
                decayVE = 100;
                // printf("enmity_container.cpp DecayEnmity PLAYER LEVEL < 50\n");
            }
            else if (playerLevel >= 51 && playerLevel < 99)
            {
                decayVE = (int16)(playerLevel * 2.03);
                decayCE = (int16)(playerLevel * 0.3);
                // printf("enmity_container.cpp DecayEnmity PLAYER LEVEL < 99\n");

                if (PEntity->GetMJob() == JOB_PLD || PEntity->GetMJob() == JOB_NIN && !PEntity->StatusEffectContainer->HasStatusEffect(EFFECT_INNIN) || PEntity->GetMJob() == JOB_RUN)
                {
                    decayVE = 90;
                    decayCE = 20;
                }
            }
            else if (playerLevel >= 99)
            {
                decayVE = 200;
                decayCE = 30;

                if (PEntity->GetMJob() == JOB_PLD || PEntity->GetMJob() == JOB_NIN && !PEntity->StatusEffectContainer->HasStatusEffect(EFFECT_INNIN) || PEntity->GetMJob() == JOB_RUN)
                {
                    decayVE = 120;
                    decayCE = 30;
                }
                // printf("enmity_container.cpp DecayEnmity PLAYER LEVEL = 99\n");
            }
            else if (playerLevel >= 109)
            {
                decayVE = 300;
                decayCE = 50;

                if (PEntity->GetMJob() == JOB_PLD || PEntity->GetMJob() == JOB_NIN && !PEntity->StatusEffectContainer->HasStatusEffect(EFFECT_INNIN) || PEntity->GetMJob() == JOB_RUN)
                {
                    decayVE = 150;
                    decayCE = 35;
                }
                // printf("enmity_container.cpp DecayEnmity PLAYER LEVEL = 109\n");
            }
            else if (playerLevel >= 115)
            {
                decayVE = 500;
                decayCE = 75;

                if (PEntity->GetMJob() == JOB_PLD || PEntity->GetMJob() == JOB_NIN && !PEntity->StatusEffectContainer->HasStatusEffect(EFFECT_INNIN) || PEntity->GetMJob() == JOB_RUN)
                {
                    decayVE = 250;
                    decayCE = 45;
                }
                // printf("enmity_container.cpp DecayEnmity PLAYER LEVEL = 115\n");
            }

            int32 ve_decay_amount = (int)(decayVE / server_tick_rate); //constexpr int decay_amount = (int)(60 / server_tick_rate); // server_tick_rate = 2.5s
            int32 ce_decay_amount = (int)(decayCE / server_tick_rate); // constexpr int ce_decay_amount = (int)(60 / server_tick_rate);

            if (PEnmityObject.CE - ce_decay_amount > 1)
            {
                PEnmityObject.VE -= PEnmityObject.VE > ve_decay_amount ? ve_decay_amount : PEnmityObject.VE;
                PEnmityObject.CE -= PEnmityObject.CE > ce_decay_amount ? ce_decay_amount : PEnmityObject.CE;
                // ShowDebug("%d: active: %d CE: %d VE: %d\n", it->first, PEnmityObject.active, PEnmityObject.CE, PEnmityObject.VE);
            }
            else
            {
                PEnmityObject.VE -= PEnmityObject.VE > ve_decay_amount ? ve_decay_amount : PEnmityObject.VE;
                PEnmityObject.CE = 1;
            }
        }
        else
        {
            PEnmityObject.CE = 1;
        }
    }
}

bool CEnmityContainer::IsWithinEnmityRange(CBattleEntity* PEntity) const
{
    float maxRange = square(m_EnmityHolder->m_Type == MOBTYPE_NOTORIOUS ? 28.f : 25.f);
    return distanceSquared(m_EnmityHolder->loc.p, PEntity->loc.p) <= maxRange;
}

int16 CEnmityContainer::GetHighestTH() const
{
    CBattleEntity* PEntity = nullptr;
    int16          THLvl   = 0;

    for (const auto& it : m_EnmityList)
    {
        const EnmityObject_t& PEnmityObject = it.second;
        PEntity                             = PEnmityObject.PEnmityOwner;

        if (PEntity != nullptr && !PEntity->isDead() && PEnmityObject.maxTH > THLvl)
        {
            THLvl = PEnmityObject.maxTH;
        }
    }

    return THLvl;
}

EnmityList_t* CEnmityContainer::GetEnmityList()
{
    return &m_EnmityList;
}

bool CEnmityContainer::IsTameable() const
{
    return m_tameable;
}

void CEnmityContainer::UpdateEnmityFromCover(CBattleEntity* PCoverAbilityTarget, CBattleEntity* PCoverAbilityUser)
{
    TracyZoneScoped;
    // Update Enmity if cover ability target and cover ability user are not nullptr
    if (PCoverAbilityTarget != nullptr && PCoverAbilityUser != nullptr)
    {
        int32 currentCE = GetCE(PCoverAbilityUser);
        SetCE(PCoverAbilityUser, currentCE + 200);
        LowerEnmityByPercent(PCoverAbilityTarget, 10, nullptr);
    }
}
