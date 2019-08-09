
package.path = package.path ..';..\/common\/?.lua';

require "config"

function GetConfig()
	return GetLogUnixPort()	
end

function BeforShutdown()

end

function BeforDispatch()

end

local cnt = 0
function Tick()
	cnt = cnt + 1
	Logger.writeLog("../test.log", string.format("[test %d]\n", cnt))
end
