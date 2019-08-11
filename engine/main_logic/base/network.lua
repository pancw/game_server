
local function tryConnect(unixPort)
	if not UnixClient.checkHasConnected(unixPort) then
		UnixClient.connectUnixSrv(unixPort)	
	end
end

function tryConnectSrv()
	tryConnect(CONFIG.GetGateUnixPort())
	tryConnect(CONFIG.GetLogUnixPort())
end

function tickAll()
	local function all()
		tryConnectSrv()
	end
	xpcall(all, __G__TRACKBACK__)
end
