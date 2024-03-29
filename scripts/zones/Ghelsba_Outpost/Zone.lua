-----------------------------------
-- Zone: Ghelsba_Outpost (140)
-----------------------------------
local ID = require("scripts/zones/Ghelsba_Outpost/IDs")
require("scripts/globals/conquest")
require("scripts/globals/helm")
-----------------------------------
local zone_object = {}

zone_object.onInitialize = function(zone)
    xi.helm.initZone(zone, xi.helm.type.LOGGING)
end

zone_object.onZoneIn = function(player, prevZone)
    local cs = -1
    if (player:getXPos() == 0 and player:getYPos() == 0 and player:getZPos() == 0) then
        player:setPos(125.852, -22.097, 180.403, 128)
    end
    return cs
end

zone_object.onConquestUpdate = function(zone, updatetype)
    xi.conq.onConquestUpdate(zone, updatetype)
end

zone_object.onRegionEnter = function(player, region)
end

zone_object.onEventUpdate = function(player, csid, option)
end

zone_object.onEventFinish = function(player, csid, option)
end

return zone_object
