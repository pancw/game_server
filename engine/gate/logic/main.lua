
package.path = package.path ..';..\/..\/common\/?.lua';

require "config"

function GetConfig()
	return GetGateConfig()	
end

function BeforShutdown()
end

function BeforDispatch()
end

function HandleTcpEvent(vfd, msg)
	print("HandleTcpEvent", vfd, msg)
end

function HandleUnixEvent(ufd, msg)
	print("HandleUnixEvent", ufd, msg)
end

function Tick()
end
