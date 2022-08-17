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

#include "attack.h"
#include "ai/ai_container.h"
#include "attackround.h"
#include "entities/battleentity.h"
#include "items/item_weapon.h"
#include "job_points.h"
#include "status_effect_container.h"
#include "utils/charutils.h"
#include "utils/puppetutils.h"

#include <cmath>

/************************************************************************
 *                                                                      *
 *  Constructor.                                                            *
 *                                                                      *
 ************************************************************************/
CAttack::CAttack(CBattleEntity* attacker, CBattleEntity* defender, PHYSICAL_ATTACK_TYPE type, PHYSICAL_ATTACK_DIRECTION direction, CAttackRound* attackRound)
: m_attacker(attacker)
, m_victim(defender)
, m_attackRound(attackRound)
, m_attackType(type)
, m_attackDirection(direction)
{
}

/************************************************************************
 *                                                                      *
 *  Returns the attack direction.                                       *
 *                                                                      *
 ************************************************************************/
PHYSICAL_ATTACK_DIRECTION CAttack::GetAttackDirection()
{
    return m_attackDirection;
}

/************************************************************************
 *                                                                      *
 *  Returns the attack type.                                                *
 *                                                                      *
 ************************************************************************/
PHYSICAL_ATTACK_TYPE CAttack::GetAttackType()
{
    return m_attackType;
}

/************************************************************************
 *                                                                      *
 *  Sets the attack type.                                               *
 *                                                                      *
 ************************************************************************/
void CAttack::SetAttackType(PHYSICAL_ATTACK_TYPE type)
{
    m_attackType = type;
}

/************************************************************************
 *                                                                      *
 *  Returns the isCritical flag.                                            *
 *                                                                      *
 ************************************************************************/
bool CAttack::IsCritical() const
{
    return m_isCritical;
}

/************************************************************************
 *                                                                      *
 *  Sets the critical flag.                                             *
 *                                                                      *
 ************************************************************************/
void CAttack::SetCritical(bool value)
{
    m_isCritical = value;

    if (m_attackType == PHYSICAL_ATTACK_TYPE::DAKEN)
    {
        m_damageRatio = battleutils::GetRangedDamageRatio(m_attacker, m_victim, m_isCritical);
    }
    else
    {
        float attBonus = 0.f;
        if (m_attackType == PHYSICAL_ATTACK_TYPE::KICK)
        {
            if (CStatusEffect* footworkEffect = m_attacker->StatusEffectContainer->GetStatusEffect(EFFECT_FOOTWORK))
            {
                attBonus = footworkEffect->GetSubPower() / 256.f; // Mod is out of 256
            }
        }

        m_damageRatio = battleutils::GetDamageRatio(m_attacker, m_victim, m_isCritical, attBonus);
    }
}

/************************************************************************
 *                                                                      *
 *  Sets the guarded flag.                                              *
 *                                                                      *
 ************************************************************************/
void CAttack::SetGuarded(bool isGuarded)
{
    m_isGuarded = isGuarded;
}

/************************************************************************
 *                                                                      *
 *  Gets the guarded flag.                                              *
 *                                                                      *
 ************************************************************************/
bool CAttack::IsGuarded()
{
    m_isGuarded = attackutils::IsGuarded(m_attacker, m_victim);
    if (m_isGuarded)
    {
        if (m_damageRatio > 1.0f)
        {
            m_damageRatio -= 1.0f;
        }
        else
        {
            m_damageRatio = 0;
        }
    }
    return m_isGuarded;
}

/************************************************************************
 *                                                                      *
 *  Gets the evaded flag.                                               *
 *                                                                      *
 ************************************************************************/
bool CAttack::IsEvaded() const
{
    return m_isEvaded;
}

/************************************************************************
 *                                                                      *
 *  Sets the evaded flag.                                               *
 *                                                                      *
 ************************************************************************/
void CAttack::SetEvaded(bool value)
{
    m_isEvaded = value;
}

/************************************************************************
 *                                                                      *
 *  Gets the blocked flag.                                              *
 *                                                                      *
 ************************************************************************/
bool CAttack::IsBlocked() const
{
    return m_isBlocked;
}

bool CAttack::IsParried()
{
    if (m_attackType != PHYSICAL_ATTACK_TYPE::DAKEN)
    {
        return attackutils::IsParried(m_attacker, m_victim);
    }
    return false;
}

bool CAttack::IsAnticipated() const
{
    return m_anticipated;
}

/************************************************************************
 *                                                                      *
 *  Returns the isFirstSwing flag.                                      *
 *                                                                      *
 ************************************************************************/
bool CAttack::IsFirstSwing() const
{
    return m_isFirstSwing;
}

/************************************************************************
 *                                                                      *
 *  Sets this swing as the first.                                       *
 *                                                                      *
 ************************************************************************/
void CAttack::SetAsFirstSwing(bool isFirst)
{
    m_isFirstSwing = isFirst;
}

/************************************************************************
 *                                                                      *
 *  Gets the damage ratio.                                              *
 *                                                                      *
 ************************************************************************/
float CAttack::GetDamageRatio() const
{
    return m_damageRatio;
}

/************************************************************************
 *                                                                      *
 *  Sets the attack type.                                               *
 *                                                                      *
 ************************************************************************/
uint8 CAttack::GetWeaponSlot()
{
    if (m_attackRound->IsH2H())
    {
        return SLOT_MAIN;
    }
    if (m_attackType == PHYSICAL_ATTACK_TYPE::DAKEN)
    {
        return SLOT_AMMO;
    }
    return m_attackDirection == RIGHTATTACK ? SLOT_MAIN : SLOT_SUB;
}

/************************************************************************
 *                                                                      *
 *  Returns the animation ID.                                           *
 *                                                                      *
 ************************************************************************/
uint16 CAttack::GetAnimationID()
{
    AttackAnimation animation;

    // Try normal kick attacks (without footwork)
    if (this->m_attackType == PHYSICAL_ATTACK_TYPE::KICK)
    {
        animation = this->m_attackDirection == RIGHTATTACK ? AttackAnimation::RIGHTKICK : AttackAnimation::LEFTKICK;
    }

    else if (this->m_attackType == PHYSICAL_ATTACK_TYPE::DAKEN)
    {
        animation = AttackAnimation::THROW;
    }

    // Normal attack
    else
    {
        animation = this->m_attackDirection == RIGHTATTACK ? AttackAnimation::RIGHTATTACK : AttackAnimation::LEFTATTACK;
    }

    return (uint16)animation;
}

/************************************************************************
 *                                                                      *
 *  Returns the hitrate for this swing.                                 *
 *                                                                      *
 ************************************************************************/
uint8 CAttack::GetHitRate()
{
    if (m_attackType == PHYSICAL_ATTACK_TYPE::KICK)
    {
        m_hitRate = battleutils::GetHitRate(m_attacker, m_victim, 2);
    }
    else if (m_attackType == PHYSICAL_ATTACK_TYPE::DAKEN)
    {
        m_hitRate = battleutils::GetRangedHitRate(m_attacker, m_victim, false, 100);
    }
    else if (m_attackDirection == RIGHTATTACK)
    {
        if (m_attackType == PHYSICAL_ATTACK_TYPE::ZANSHIN)
        {
            m_hitRate = battleutils::GetHitRate(m_attacker, m_victim, 0, (uint8)35);
        }
        else
        {
            m_hitRate = battleutils::GetHitRate(m_attacker, m_victim, 0);
        }

        // Deciding this here because SA/TA wears on attack, before the 2nd+ hits go off.
        if (m_hitRate == 100)
        {
            m_attackRound->SetSATA(true);
        }
    }
    else if (m_attackDirection == LEFTATTACK)
    {
        if (m_attackType == PHYSICAL_ATTACK_TYPE::ZANSHIN)
        {
            m_hitRate = battleutils::GetHitRate(m_attacker, m_victim, 1, (uint8)35);
        }
        else
        {
            m_hitRate = battleutils::GetHitRate(m_attacker, m_victim, 1);
        }
    }
    return m_hitRate;
}

/************************************************************************
 *                                                                      *
 *  Returns the damage for this swing.                                  *
 *                                                                      *
 ************************************************************************/
int32 CAttack::GetDamage() const
{
    return m_damage;
}

/************************************************************************
 *                                                                      *
 *  Sets the damage for this swing.                                     *
 *                                                                      *
 ************************************************************************/
void CAttack::SetDamage(int32 value)
{
    m_damage = value;
}

bool CAttack::CheckAnticipated()
{
    if (m_attackType == PHYSICAL_ATTACK_TYPE::DAKEN)
    {
        return false;
    }

    if (m_victim->objtype == TYPE_PC)
    {
        bool hasSeigan          = m_victim->StatusEffectContainer->HasStatusEffect(EFFECT_SEIGAN, 0);
        bool hasThirdEye        = m_victim->StatusEffectContainer->HasStatusEffect(EFFECT_THIRD_EYE, 0);
        CStatusEffect* thirdEye = m_victim->StatusEffectContainer->GetStatusEffect(EFFECT_THIRD_EYE, 0);

        if (hasSeigan && !hasThirdEye && xirand::GetRandomNumber(100) < m_victim->getMod(Mod::ENH_SEIGAN))
        {
            if (m_victim->PAI->IsEngaged())
            {
                m_isCountered = true;
                m_isCritical = (xirand::GetRandomNumber(100) < battleutils::GetCritHitRate(m_victim, m_attacker, false));
            }
            m_anticipated = true;
            return true;
        }

        if (thirdEye == nullptr)
        {
            return false;
        }

        //power stores how many times this effect has anticipated
        auto pastAnticipations = thirdEye->GetPower();

        if (pastAnticipations > 7)
        {
            //max 7 anticipates!
            m_victim->StatusEffectContainer->DelStatusEffect(EFFECT_THIRD_EYE);
            return false;
        }

        if (!hasSeigan && pastAnticipations == 0)
        {
            m_victim->StatusEffectContainer->DelStatusEffect(EFFECT_THIRD_EYE);
            m_anticipated = true;
            return true;
        }
        else if (!hasSeigan)
        {
            m_victim->StatusEffectContainer->DelStatusEffect(EFFECT_THIRD_EYE);
            return false;
        }
        else
        { //do have seigan, decay anticipations correctly (guesstimated)
            //5-6 anticipates is a 'lucky' streak, going to assume 15% decay per proc, with a 100% base w/ Seigan
            if (xirand::GetRandomNumber(100) < (100 - (pastAnticipations * 15) + m_victim->getMod(Mod::THIRD_EYE_ANTICIPATE_RATE)))
            {
                //increment power and don't remove
                thirdEye->SetPower(thirdEye->GetPower() + 1);
                //chance to counter - 25% base
                if (xirand::GetRandomNumber(100) < 25 + m_victim->getMod(Mod::THIRD_EYE_COUNTER_RATE))
                {
                    if (m_victim->PAI->IsEngaged())
                    {
                        m_isCountered = true;
                        m_isCritical = (xirand::GetRandomNumber(100) < battleutils::GetCritHitRate(m_victim, m_attacker, false));
                    }
                }
                m_anticipated = true;
                return true;
            }
            m_victim->StatusEffectContainer->DelStatusEffect(EFFECT_THIRD_EYE);
            return false;
        }
    }
    return false;
}

bool CAttack::IsCountered() const
{
    return m_isCountered;
}

bool CAttack::CheckCounter()
{
    if (m_attackType == PHYSICAL_ATTACK_TYPE::DAKEN)
    {
        return false;
    }

    if (!m_victim->PAI->IsEngaged())
    {
        m_isCountered = false;
        return m_isCountered;
    }

    uint8 meritCounter = 0;
    if (m_victim->objtype == TYPE_PC && charutils::hasTrait((CCharEntity*)m_victim, TRAIT_COUNTER))
    {
        if (m_victim->GetMJob() == JOB_MNK || m_victim->GetMJob() == JOB_PUP)
        {
            meritCounter = ((CCharEntity*)m_victim)->PMeritPoints->GetMeritValue(MERIT_COUNTER_RATE, (CCharEntity*)m_victim);
        }
    }

    // counter check (rate AND your hit rate makes it land, else its just a regular hit)
    // having seigan active gives chance to counter at 25% of the zanshin proc rate
    uint16 seiganChance = 0;
    if (m_victim->objtype == TYPE_PC && m_victim->StatusEffectContainer->HasStatusEffect(EFFECT_SEIGAN))
    {
        seiganChance = m_victim->getMod(Mod::ZANSHIN) + ((CCharEntity*)m_victim)->PMeritPoints->GetMeritValue(MERIT_ZASHIN_ATTACK_RATE, (CCharEntity*)m_victim);
        seiganChance = std::clamp<uint16>(seiganChance, 0, 100);
        seiganChance /= 4;
    }
    // Yonin based Counter when wearing Augments "Yonin" effect gear
    if (m_victim->objtype == TYPE_PC && m_victim->StatusEffectContainer->HasStatusEffect(EFFECT_YONIN) && facing(m_victim->loc.p, m_attacker->loc.p, 64) &&
        xirand::GetRandomNumber(100) < m_victim->getMod(Mod::AUGMENT_YONIN))
    {
        m_isCountered = true;
        m_isCritical  = (xirand::GetRandomNumber(100) < battleutils::GetCritHitRate(m_victim, m_attacker, false));
    }
    // Normal Counter
    if ((xirand::GetRandomNumber(100) < std::clamp<uint16>(m_victim->getMod(Mod::COUNTER) + meritCounter, 0, 80) ||
         xirand::GetRandomNumber(100) < seiganChance) &&
        facing(m_victim->loc.p, m_attacker->loc.p, 64) && xirand::GetRandomNumber(100) < battleutils::GetHitRate(m_victim, m_attacker))
    {
        m_isCountered = true;
        m_isCritical  = (xirand::GetRandomNumber(100) < battleutils::GetCritHitRate(m_victim, m_attacker, false));
    }
    else if (m_victim->objtype == TYPE_PC && m_victim->StatusEffectContainer->HasStatusEffect(EFFECT_PERFECT_COUNTER))
    // Perfect Counter based Counter
    { // Perfect Counter only counters hits that normal counter misses, always critical, can counter 1-7 times before wearing
        m_isCountered         = true;
        m_isCritical          = true;
        CStatusEffect* effect = m_victim->StatusEffectContainer->GetStatusEffect(EFFECT_PERFECT_COUNTER, 0);
        uint8 vitMod          = (m_victim->getMod(Mod::VIT) / 10) + dynamic_cast<CCharEntity*>(m_victim)->PJobPoints->GetJobPointValue(JP_PERFECT_COUNTER_EFFECT);

        //Determine chance to not wear off using assumed formula VIT / 10 (ex. 250 VIT = 25% chance to not wear off)
        if (xirand::GetRandomNumber(100) < vitMod && effect->GetPower() < 7) // Maximum 7 counters! Similar to Third Eye
        {
            //Increment power and don't remove
            effect->SetPower(effect->GetPower() + 1);
        }
        else
        {
            m_victim->StatusEffectContainer->DelStatusEffect(EFFECT_PERFECT_COUNTER);
        }

        m_victim->StatusEffectContainer->DelStatusEffect(EFFECT_PERFECT_COUNTER);
    }
    // Inner Strength based Counter
    if (m_victim->objtype == TYPE_PC && m_victim->StatusEffectContainer->HasStatusEffect(EFFECT_INNER_STRENGTH))
    { // Inner Strength has a 100% Perfect Counter rate that does not wear off for Inner Strength's duration
        m_isCountered = true;
        m_isCritical = true;
    }

    return m_isCountered;
}

bool CAttack::IsCovered() const
{
    return m_isCovered;
}

bool CAttack::CheckCover()
{
    CBattleEntity* PCoverAbilityUser = m_attackRound->GetCoverAbilityUserEntity();
    if (PCoverAbilityUser != nullptr && PCoverAbilityUser->isAlive())
    {
        m_isCovered = true;
        m_victim    = PCoverAbilityUser;
    }
    else
    {
        m_isCovered = false;
    }

    return m_isCovered;
}

/************************************************************************
 *                                                                      *
 *  Processes the damage for this swing.                                    *
 *                                                                      *
 ************************************************************************/
void CAttack::ProcessDamage()
{
    // Sneak attack.
    if (m_attacker->GetMJob() == JOB_THF && m_isFirstSwing && m_attacker->StatusEffectContainer->HasStatusEffect(EFFECT_SNEAK_ATTACK) &&
       ((abs(m_victim->loc.p.rotation - m_attacker->loc.p.rotation) < 23) || m_attacker->StatusEffectContainer->HasStatusEffect(EFFECT_HIDE) ||
        m_victim->StatusEffectContainer->HasStatusEffect(EFFECT_DOUBT)))
    {
        m_trickAttackDamage += (int32)(m_attacker->DEX() * (1.125f + m_attacker->getMod(Mod::SNEAK_ATK_DEX) / 100.0f));

        if (m_attacker->objtype == TYPE_PC)
        {
            dynamic_cast<CCharEntity*>(m_attacker)->SetLocalVar("SneakAttack_Active", 1);
        }
    }

    // Trick attack.
    if (m_attacker->GetMJob() == JOB_THF && m_isFirstSwing && m_attackRound->GetTAEntity() != nullptr)
    {
        m_trickAttackDamage += (int32)(m_attacker->AGI() * (1.125f + m_attacker->getMod(Mod::TRICK_ATK_AGI) / 100.0f));

        if (m_attacker->objtype == TYPE_PC)
        {
            dynamic_cast<CCharEntity*>(m_attacker)->SetLocalVar("TrickAttack_Active", 1);
        }
    }

    SLOTTYPE slot   = (SLOTTYPE)GetWeaponSlot();
    auto mainWeapon = dynamic_cast<CItemWeapon*>(m_attacker->m_Weapons[SLOT_MAIN]);
    auto subWeapon  = dynamic_cast<CItemWeapon*>(m_attacker->m_Weapons[SLOT_SUB]);

    if (m_attackRound->IsH2H())
    {
        m_naturalH2hDamage = (int32)(m_attacker->GetSkill(SKILL_HAND_TO_HAND) * 0.11f) + 3;
        m_baseDamage       = m_attacker->GetMainWeaponDmg();
        m_damage           = (uint32)(((m_baseDamage + m_naturalH2hDamage + m_trickAttackDamage + battleutils::GetFSTR(m_attacker, m_victim, slot)) * m_damageRatio));
    }
    else if (slot == SLOT_MAIN)
    {
        m_damage = (uint32)(((m_attacker->GetMainWeaponDmg() + m_trickAttackDamage + battleutils::GetFSTR(m_attacker, m_victim, slot)) * m_damageRatio));
    }
    else if (slot == SLOT_SUB)
    {
        m_damage = (uint32)(((m_attacker->GetSubWeaponDmg() + m_trickAttackDamage + battleutils::GetFSTR(m_attacker, m_victim, slot)) * m_damageRatio));
    }
    else if (slot == SLOT_AMMO)
    {
        m_damage = (uint32)((m_attacker->GetRangedWeaponDmg() + battleutils::GetFSTR(m_attacker, m_victim, slot)) * m_damageRatio);
    }

    // Apply "Double Attack" damage and "Triple Attack" damage mods
    if (m_attackType == PHYSICAL_ATTACK_TYPE::DOUBLE && m_attacker->objtype == TYPE_PC)
    {
        if (m_attacker->objtype == TYPE_PC && charutils::GetCharVar(dynamic_cast<CCharEntity*>(m_attacker), "AuditMultiHit") == 1)
        {
            printf("attack.cpp ProcessDamage  1  DOUBLE ATTACK DAMAGE: [%i]  MULTIPLIER: [%1.2f]\n", m_damage, (100.0f + m_attacker->getMod(Mod::DOUBLE_ATTACK_DMG)) / 100.0f);
        }

        m_damage = (int32)(m_damage * ((100.0f + m_attacker->getMod(Mod::DOUBLE_ATTACK_DMG)) / 100.0f));

        if (m_attacker->objtype == TYPE_PC && charutils::GetCharVar(dynamic_cast<CCharEntity*>(m_attacker), "AuditMultiHit") == 1)
        {
            printf("attack.cpp ProcessDamage  2  DOUBLE ATTACK DAMAGE: [%i]\n", m_damage);
        }
    }
    else if (m_attackType == PHYSICAL_ATTACK_TYPE::TRIPLE && m_attacker->objtype == TYPE_PC)
    {
        if (m_attacker->objtype == TYPE_PC && charutils::GetCharVar(dynamic_cast<CCharEntity*>(m_attacker), "AuditMultiHit") == 1)
        {
            printf("attack.cpp ProcessDamage  1  TRIPLE ATTACK DAMAGE: [%i]  MULTIPLIER: [%1.2f]\n", m_damage, (100.0f + m_attacker->getMod(Mod::TRIPLE_ATTACK_DMG)) / 100.0f);
        }

        m_damage = (int32)(m_damage * ((100.0f + m_attacker->getMod(Mod::TRIPLE_ATTACK_DMG)) / 100.0f));

        if (m_attacker->objtype == TYPE_PC && charutils::GetCharVar(dynamic_cast<CCharEntity*>(m_attacker), "AuditMultiHit") == 1)
        {
            printf("attack.cpp ProcessDamage  2  TRIPLE ATTACK DAMAGE: [%i]\n", m_damage);
        }
    }

    // Caldera H2H base damage adjustment
    if (m_attacker->objtype == TYPE_PC && (mainWeapon != nullptr && mainWeapon->getSkillType() == SKILL_HAND_TO_HAND))
    {
        m_damage = (uint32)(m_damage * 1.50f);
    }

    // Caldera Dagger base damage adjustment
    if (m_attacker->objtype == TYPE_PC && (mainWeapon != nullptr && mainWeapon->getSkillType() == SKILL_DAGGER || subWeapon != nullptr && subWeapon->getSkillType() == SKILL_DAGGER))
    {
        m_damage = (uint32)(m_damage * 1.125f);
    }

    // Caldera Katana base damage adjustment
    if (m_attacker->objtype == TYPE_PC && (mainWeapon != nullptr && mainWeapon->getSkillType() == SKILL_KATANA || subWeapon != nullptr && subWeapon->getSkillType() == SKILL_KATANA))
    {
        m_damage = (uint32)(m_damage * 1.35f);
    }

    // Soul eater.
    if (m_attacker->objtype == TYPE_PC)
    {
        m_damage = battleutils::doSoulEaterEffect((CCharEntity*)m_attacker, m_damage);
    }

    // Consume mana
    if (m_attacker->objtype == TYPE_PC)
    {
        m_damage = battleutils::doConsumeManaEffect((CCharEntity*)m_attacker, m_damage);
    }

    // Set attack type to Samba if the attack type is normal.  Don't overwrite other types.  Used for Samba double damage.
    if (m_attackType == PHYSICAL_ATTACK_TYPE::NORMAL && (m_attacker->StatusEffectContainer->HasStatusEffect(EFFECT_DRAIN_SAMBA) ||
                                                         m_attacker->StatusEffectContainer->HasStatusEffect(EFFECT_ASPIR_SAMBA) ||
                                                         m_attacker->StatusEffectContainer->HasStatusEffect(EFFECT_HASTE_SAMBA)))
    {
        SetAttackType(PHYSICAL_ATTACK_TYPE::SAMBA);
    }

    // Check for Monster Correlation and Empyrean armor Killer Effects bonus
    if (m_attacker->objtype == TYPE_MOB || m_attacker->objtype == TYPE_PET || m_attacker->getMod(Mod::AUGMENT_KILLER_EFFECTS) > 0)
    {
        int16 KillerEffect = 0;

        switch (m_victim->m_EcoSystem)
        {
            case ECOSYSTEM::AMORPH:     KillerEffect = m_attacker->getMod(Mod::AMORPH_KILLER);   break;
            case ECOSYSTEM::AQUAN:      KillerEffect = m_attacker->getMod(Mod::AQUAN_KILLER);    break;
            case ECOSYSTEM::ARCANA:     KillerEffect = m_attacker->getMod(Mod::ARCANA_KILLER);   break;
            case ECOSYSTEM::BEAST:      KillerEffect = m_attacker->getMod(Mod::BEAST_KILLER);    break;
            case ECOSYSTEM::BIRD:       KillerEffect = m_attacker->getMod(Mod::BIRD_KILLER);     break;
            case ECOSYSTEM::DEMON:      KillerEffect = m_attacker->getMod(Mod::DEMON_KILLER);    break;
            case ECOSYSTEM::DRAGON:     KillerEffect = m_attacker->getMod(Mod::DRAGON_KILLER);   break;
            case ECOSYSTEM::EMPTY:      KillerEffect = m_attacker->getMod(Mod::EMPTY_KILLER);    break;
            case ECOSYSTEM::HUMANOID:   KillerEffect = m_attacker->getMod(Mod::HUMANOID_KILLER); break;
            case ECOSYSTEM::LIZARD:     KillerEffect = m_attacker->getMod(Mod::LIZARD_KILLER);   break;
            case ECOSYSTEM::LUMINION:   KillerEffect = m_attacker->getMod(Mod::LUMINION_KILLER); break;
            case ECOSYSTEM::LUMORIAN:   KillerEffect = m_attacker->getMod(Mod::LUMORIAN_KILLER); break;
            case ECOSYSTEM::PLANTOID:   KillerEffect = m_attacker->getMod(Mod::PLANTOID_KILLER); break;
            case ECOSYSTEM::UNDEAD:     KillerEffect = m_attacker->getMod(Mod::UNDEAD_KILLER);   break;
            case ECOSYSTEM::VERMIN:     KillerEffect = m_attacker->getMod(Mod::VERMIN_KILLER);   break;
            default: break;
        }

        float bonus = 0.0f;
        if (m_attacker->objtype == TYPE_PC)
        {
            KillerEffect += ((CCharEntity*)m_attacker)->PMeritPoints->GetMeritValue(MERIT_KILLER_EFFECTS, (CCharEntity*)m_attacker) + m_attacker->getMod(Mod::ALL_KILLER_EFFECTS);
            bonus = 1.0f + (((float)KillerEffect / 2.0f) / 100.0f);
        }
        else
        {
            bonus = 1.0f + ((float)KillerEffect / 100.0f);
        }

        // printf("attack.cpp ProcessDamage DAMAGE: [%i]\n", m_damage);
        m_damage = (uint32)(m_damage * bonus);
        // printf("attack.cpp ProcessDamage KILLER EFFECT: [%i]  BONUS: [%1.3f]  DAMAGE: [%i]\n", KillerEffect, bonus, m_damage);
    }

    // Get damage multipliers.
    m_damage =
        attackutils::CheckForDamageMultiplier((CCharEntity*)m_attacker, dynamic_cast<CItemWeapon*>(m_attacker->m_Weapons[slot]), m_damage, m_attackType, slot);

    // Get critical bonus mods.
    if (m_isCritical && slot != SLOT_AMMO)
    {        
        m_damage += (int32)(m_damage * (m_attacker->getMod(Mod::CRIT_DMG_INCREASE) - m_victim->getMod(Mod::CRIT_DEF_BONUS)) / 100.0f);
    }
    else if (m_isCritical && slot == SLOT_AMMO)
    {
        m_damage += (int32)(m_damage * ((m_attacker->getMod(Mod::CRIT_DMG_INCREASE) + m_attacker->getMod(Mod::RANGED_CRIT_DMG_INCREASE)) - m_victim->getMod(Mod::CRIT_DEF_BONUS)) / 100.0f);
    }

    // Apply Sneak Attack Augment Mod
    if (m_attacker->getMod(Mod::AUGMENTS_SA) > 0 && m_trickAttackDamage > 0 && m_attacker->StatusEffectContainer->HasStatusEffect(EFFECT_SNEAK_ATTACK))
    {
        m_damage += (int32)(m_damage * ((100 + (m_attacker->getMod(Mod::AUGMENTS_SA))) / 100.0f));
    }

    // Apply Trick Attack Augment Mod
    if (m_attacker->getMod(Mod::AUGMENTS_TA) > 0 && m_trickAttackDamage > 0 && m_attacker->StatusEffectContainer->HasStatusEffect(EFFECT_TRICK_ATTACK))
    {
        m_damage += (int32)(m_damage * ((100 + (m_attacker->getMod(Mod::AUGMENTS_TA))) / 100.0f));
    }

    // Apply Climactic, Striking, and Ternary Flourishes based off of player CHR
    if (m_isFirstSwing &&
       (m_attacker->StatusEffectContainer->HasStatusEffect(EFFECT_CLIMACTIC_FLOURISH) ||
        m_attacker->StatusEffectContainer->HasStatusEffect(EFFECT_STRIKING_FLOURISH) ||
        m_attacker->StatusEffectContainer->HasStatusEffect(EFFECT_TERNARY_FLOURISH)))
    {
        int16 flourishBonus = m_attacker->stats.CHR + m_attacker->getMod(Mod::CHR);

        if (m_attacker->StatusEffectContainer->HasStatusEffect(EFFECT_CLIMACTIC_FLOURISH))
        {
            CStatusEffect* effect = m_attacker->StatusEffectContainer->GetStatusEffect(EFFECT_CLIMACTIC_FLOURISH);
            int8 crits            = effect->GetPower();
            flourishBonus         = (int16)(flourishBonus / 2.0f);

            if (crits == 0)
            {
                m_attacker->StatusEffectContainer->DelStatusEffect(EFFECT_CLIMACTIC_FLOURISH);
            }
        }

        m_damage = (int32)((m_damage + flourishBonus) * (1.0f + ((float)m_attacker->getMod(Mod::ENH_CLIMACTIC_FLOURISH) / 100.0f)));
        m_attacker->StatusEffectContainer->DelStatusEffectSilent(EFFECT_STRIKING_FLOURISH);
        m_attacker->StatusEffectContainer->DelStatusEffectSilent(EFFECT_TERNARY_FLOURISH);
    }

    // Try skill up.
    if (m_damage > 0)
    {
        if (m_attacker->objtype == TYPE_PC)
        {
            if (m_attackType == PHYSICAL_ATTACK_TYPE::DAKEN)
            {
                charutils::TrySkillUP((CCharEntity*)m_attacker, SKILLTYPE::SKILL_THROWING, m_victim->GetMLevel());
            }
            else if (auto* weapon = dynamic_cast<CItemWeapon*>(m_attacker->m_Weapons[slot]))
            {
                charutils::TrySkillUP((CCharEntity*)m_attacker, (SKILLTYPE)weapon->getSkillType(), m_victim->GetMLevel());
            }
        }
        else if (m_attacker->objtype == TYPE_PET && m_attacker->PMaster && m_attacker->PMaster->objtype == TYPE_PC &&
                 static_cast<CPetEntity*>(m_attacker)->getPetType() == PET_TYPE::AUTOMATON)
        {
            puppetutils::TrySkillUP((CAutomatonEntity*)m_attacker, SKILL_AUTOMATON_MELEE, m_victim->GetMLevel());
        }
    }
    m_isBlocked = attackutils::IsBlocked(m_attacker, m_victim);

    // Apply Restraint Weaponskill Damage Modifier
    // Effect power tracks the total bonus
    // Effect sub power tracks remainder left over from whole percentage flooring
    if (m_isFirstSwing && m_attacker->StatusEffectContainer->HasStatusEffect(EFFECT_RESTRAINT))
    {
        CStatusEffect* effect = m_attacker->StatusEffectContainer->GetStatusEffect(EFFECT_RESTRAINT);

        if (effect->GetPower() < 30)
        {
            uint8 jpBonus = 0;

            if (m_attacker->objtype == TYPE_PC)
            {
                jpBonus = static_cast<CCharEntity*>(m_attacker)->PJobPoints->GetJobPointValue(JP_RESTRAINT_EFFECT) * 2;
            }

            // Convert weapon delay and divide
            // Pull remainder of previous hit's value from Effect sub Power
            float boostPerRound = ((m_attacker->GetWeaponDelay(false) / 1000.0f) * 60.0f) / 385.0f;
            float remainder     = effect->GetSubPower() / 100.0f;

            // Cap floor at 1 WSD per hit
            // Calculate bonuses from Enhances Restraint, Job Point upgrades, and remainder from previous hit
            boostPerRound = std::clamp<float>(boostPerRound, 1, boostPerRound);
            boostPerRound = (boostPerRound * (1 + m_attacker->getMod(Mod::ENHANCES_RESTRAINT) / 100.0f) * (1 + jpBonus / 100.0f)) + remainder;

            // Calculate new remainder and multiply by 100 so significant digits aren't lost
            // Floor Boost per Round
            remainder     = (1 - (std::ceil(boostPerRound) - boostPerRound)) * 100;
            boostPerRound = std::floor(boostPerRound);

            // Cap total power to +30% WSD
            if (effect->GetPower() + boostPerRound > 30)
            {
                boostPerRound = 30 - effect->GetPower();
            }

            effect->SetPower(effect->GetPower() + boostPerRound);
            effect->SetSubPower(remainder);
            m_attacker->addModifier(Mod::ALL_WSDMG_FIRST_HIT, boostPerRound);
        }
    }

    // Apply Impetus Attack and Critical Hit Rate Buffs
    // Effect power tracks the bonus per hit to be added to the modifiers
    // Sub Effect tracks if Augment "Impetus" is active
    if (m_attacker->objtype == TYPE_PC && m_attacker->StatusEffectContainer->HasStatusEffect(EFFECT_IMPETUS))
    {
        CStatusEffect* effect = m_attacker->StatusEffectContainer->GetStatusEffect(EFFECT_IMPETUS, 0);
        if (effect->GetPower() > 0 && effect->GetPower() < 50)
        {
            if (effect->GetSubPower() > 0)
            {
                m_attacker->StatusEffectContainer->DelStatusEffect(EFFECT_ATTACK_BOOST, effect->GetPower());
                effect->SetPower(effect->GetPower() + 1);
                m_attacker->addModifier(Mod::ATT, 2);
                m_attacker->addModifier(Mod::ACC, 2);
                m_attacker->addModifier(Mod::CRITHITRATE, 1);
                m_attacker->addModifier(Mod::CRIT_DMG_INCREASE, 1);
            }
            else
            {
                m_attacker->StatusEffectContainer->DelStatusEffect(EFFECT_ATTACK_BOOST, effect->GetPower());
                effect->SetPower(effect->GetPower() + 1);
                m_attacker->addModifier(Mod::ATT, 2);
                m_attacker->addModifier(Mod::CRITHITRATE, 1);
            }
        }
    }
}
