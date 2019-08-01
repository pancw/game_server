
local GAME_SID = 2

local beginId = 1000 + GAME_SID * 100
local function fetchPort()
	beginId = beginId + 1	
	return beginId
end

local gameClientPort = fetchPort()
local gateUnixServerPort = fetchPort()

function GetGateConfig()
	return gateUnixServerPort, gameClientPort
end

