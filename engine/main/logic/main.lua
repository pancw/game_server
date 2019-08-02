
package.path = package.path ..';..\/..\/common\/?.lua';

require "config"

function GetConfig()
	return GetMainConfig()	
end

function BeforShutdown()

end

function BeforDispatch()

end

function Tick()
	local gateUnixPort = GetGateUnixPort()
	if not UnixClient.checkHasConnected(gateUnixPort) then
		UnixClient.connectUnixSrv(gateUnixPort)	
	end
end
