
package.path = package.path ..';..\/common\/?.lua';

function GetConfig()
	return CONFIG.GetLogUnixPort()	
end

function __G__TRACKBACK__(msg)
	print("server lua error", msg)
	local errInfo, btInfo = SafeY1Except(msg)
	print(btInfo)
	--LOG.mainError(btInfo)
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

local DOFILELIST = 
{
	"../common/base/common_class.lua",
	"../common/base/class.lua",
	"../common/base/import.lua",
	"../common/base/extend.lua",
	"../common/base/linecache.lua",
	"../common/base/traceback.lua",
	"../common/base/ldb.lua",
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
