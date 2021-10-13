-----------------------------------
-- Difusion Ray
-- Description: Deals damage to enemies within a fan-shaped area originating from the caster.
-- Type: Magical Light (Element)
-----------------------------------
require("scripts/globals/mobskills")
require("scripts/globals/status")
-----------------------------------
local mobskill_object = {}

mobskill_object.onMobSkillCheck = function(target, mob, skill)
    return 0
end

mobskill_object.onMobWeaponSkill = function(target, mob, skill)
    local dmgmod = xi.mobskills.mobBreathMove(mob, target, 0.2, 0.65, xi.magic.ele.LIGHT, 500)
    local dmg = xi.mobskills.mobFinalAdjustments(dmgmod, mob, skill, target, xi.damageType.BREATH, xi.attackType.LIGHT, xi.mobskills.shadowBehavior.IGNORE_SHADOWS)
    target:takeDamage(dmg, mob, xi.attackType.BREATH, xi.damageType.LIGHT)

    return dmg
end

return mobskill_object
