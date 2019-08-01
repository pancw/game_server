
package.path = package.path ..';..\/..\/common\/?.lua';

require "config"

function GetConfig()
	return GetGateConfig()	
end

function BeforShutdown()
end

function BeforDispatch()
end

