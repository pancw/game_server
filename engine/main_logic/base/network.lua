
local function tryConnect(unixPort)
	if not UnixClient.checkHasConnected(unixPort) then
		UnixClient.connectUnixSrv(unixPort)	
	end
end

function tryConnectSrv()
	tryConnect(CONFIG.GetGateUnixPort())
	tryConnect(CONFIG.GetLogUnixPort())
end
