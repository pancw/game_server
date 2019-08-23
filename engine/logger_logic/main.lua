
package.path = package.path ..';..\/common\/?.lua';

function GetConfig()
	return CONFIG.GetLogUnixPort()	
end

function __G__TRACKBACK__(msg)
	print("server lua error", msg)
	local errInfo, btInfo = SafeY1Except(msg)
	print(btInfo)
	--LOG.sendError(btInfo)
end

function BeforShutdown()

end

function BeforDispatch()

end

function Tick()
end

local DOFILELIST = 
{
	"../common/base/common_class.lua",
	"../common/base/class.lua",
	"../common/base/import.lua",
	"../common/base/extend.lua",
	"../common/base/linecache.lua",
	"../common/base/traceback.lua",
	"../common/base/ldb.lua",
	"base/interface.lua",
	"base/global.lua",
}

function startGame()
	os.exit = nil 
	sys = sys or {}
	sys.path = sys.path or {}

	math.randomseed(os.time())
	for _,file in ipairs(DOFILELIST) do
		dofile(file)
	end
end
startGame()

