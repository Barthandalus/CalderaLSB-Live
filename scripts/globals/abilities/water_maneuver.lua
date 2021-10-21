-----------------------------------
-- Ability: Water Maneuver
-- Enhances the effect of water attachments. Must have animator equipped.
-- Obtained: Puppetmaster level 1
-- Recast Time: 10 seconds (shared with all maneuvers)
-- Duration: 1 minute
-----------------------------------
require("scripts/globals/automaton")
-----------------------------------
local ability_object = {}

ability_object.onAbilityCheck = function(player, target, ability)
    return xi.automaton.onManeuverCheck(player, target, ability)
end

ability_object.onUseAbility = function(player, target, ability, action)
    return xi.automaton.onUseManeuver(player, target, ability, action)
end

return ability_object
