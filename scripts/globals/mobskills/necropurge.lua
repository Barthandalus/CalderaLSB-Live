-----------------------------------
-- Necropurge
--
-----------------------------------
require("scripts/settings/main")
require("scripts/globals/status")
require("scripts/globals/mobskills")
-----------------------------------
local mobskill_object = {}

mobskill_object.onMobSkillCheck = function(target, mob, skill)
  if(mob:getFamily() == 316) then
    local mobSkin = mob:getModelId()

    if (mobSkin == 1840) then
        return 0
    else
        return 1
    end
  end

  if(mob:getFamily() == 91) then
    local mobSkin = mob:getModelId()

    if (mobSkin == 1839) then
        return 0
    else
        return 1
    end
  end
  return 0
end

mobskill_object.onMobWeaponSkill = function(target, mob, skill)
    local numhits = 1
    local accmod = 10
    local dmgmod = 2
    local info = xi.mobskills.mobPhysicalMove(mob, target, skill, numhits, accmod, dmgmod, xi.mobskills.magicalTpBonus.NO_EFFECT)
    local dmg = xi.mobskills.mobFinalAdjustments(info.dmg, mob, skill, target, xi.attackType.PHYSICAL, xi.damageType.BLUNT, info.hitslanded)

    xi.mobskills.mobStatusEffectMove(mob, target, xi.effect.CURSE_I, 1, 0, 60)
    target:takeDamage(dmg, mob, xi.attackType.PHYSICAL, xi.damageType.BLUNT)

    return dmg
end

return mobskill_object
