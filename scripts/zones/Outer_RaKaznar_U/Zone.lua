-----------------------------------
--
-- Zone: Outer Ra’Kanzar [U] (275)
--
-----------------------------------
local ID = require("scripts/zones/Outer_RaKaznar_U/IDs")
-----------------------------------
local zone_object = {}

zone_object.onInitialize = function(zone)
end

zone_object.onZoneIn = function(player, prevZone)
    local cs = -1
    if (player:getXPos() == 0 and player:getYPos() == 0 and player:getZPos() == 0) then
        player:setPos(-40, -180, -20, 128)
    end
    return cs
end

zone_object.onInstanceZoneIn = function(player, instance)
    local cs = -1

    local pos = player:getPos()
    if pos.x == 0 and pos.y == 0 and pos.z == 0 then
        local entrypos = instance:getEntryPos()
        player:setPos(entrypos.x, entrypos.y, entrypos.z, entrypos.rot)
    end

    return cs
end

zone_object.onRegionEnter = function(player, region)
end

zone_object.onEventUpdate = function(player, csid, option)
end

zone_object.onEventFinish = function(player, csid, option)
end

zone_object.onInstanceLoadFailed = function()
    return 72
end

return zone_object
