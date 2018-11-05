-----------------------------------
-- Area: Dynamis Buburimu
--  MOB: Vanguard_Oracle
-----------------------------------
mixins =
{
    require("scripts/mixins/dynamis_beastmen"),
    require("scripts/mixins/job_special")
}
require("scripts/globals/status")
-----------------------------------

function onMobSpawn(mob)
    mob:setLocalVar("mainSpec", dsp.jsa.ASTRAL_FLOW_MAAT)
    mob:setLocalVar("dynamis_currency", 1449)
end

function onMobDeath(mob, player, isKiller)
end