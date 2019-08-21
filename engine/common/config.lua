
local GAME_SID = 2

local beginId = 1000 + GAME_SID * 100
local function fetchPort()
	beginId = beginId + 1	
	return beginId
end

local gameClientPort = fetchPort()

local gateUnixServerPort = fetchPort()
local mainUnixServerPort = fetchPort()
local logUnixServerPort = fetchPort()
local dbUnixServerPort = fetchPort()

local DB_URL = "mongodb://127.0.0.1:27017"
function getMongoConfig()
	return DB_URL
end

function GetGateConfig()
	return gateUnixServerPort, gameClientPort
end

function GetMainConfig()
	return gateUnixServerPort, mainUnixServerPort, logUnixServerPort, dbUnixServerPort
end

function GetGateUnixPort()
	return gateUnixServerPort
end

function GetMainUnixPort()
	return mainUnixServerPort	
end

function GetLogUnixPort()
	return logUnixServerPort
end

