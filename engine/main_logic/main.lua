
package.path = package.path ..';..\/common\/?.lua';

require "config"

function GetConfig()
	return GetMainConfig()	
end

function BeforShutdown()

end

function BeforDispatch()

end

local function tryConnect(unixPort)
	if not UnixClient.checkHasConnected(unixPort) then
		UnixClient.connectUnixSrv(unixPort)	
	end
end

function Tick()
	tryConnect(GetGateUnixPort())
	tryConnect(GetLogUnixPort())
end
