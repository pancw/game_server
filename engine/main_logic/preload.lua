
for_maker = {} 
for_caller = {} 

rfc_iml = {}

--------------------------------------- global func for engine
function __G__TRACKBACK__(msg)
	print("server lua error", msg)
	local errInfo, btInfo = SafeY1Except(msg)
	print(btInfo)
	LOG.mainError(btInfo)
end

function BeforeShutDown()
	GAME_MAIN.saveAllData()

	MONGO_SL.releaseMongo()
end

function getServerConfig()
	return SRV_GAME_CONFIG.getServerConfig()
end

function getCompressPto()
	return SRV_GAME_CONFIG.getCompressPto()
end

function onUserCloseConnect(vfd)
end

function __ProtocolMessageDispatch(ptoName, vfd, ...)
	if not for_maker[ptoName] then
		print("no pto iml ", ptoName)
		return
	end
	for_maker[ptoName](vfd, ...)
end

function onRFCSrvConnectStatus(globalHostId, status)
end

function onHttpRequest(reqInfo, paramTbl, postData, reqIdx)
end

------------------------------------

local DOFILELIST = 
{
	"common/common_class.lua",
	"srv_common/base/class.lua",
	"srv_common/base/import.lua",
	"srv_common/base/extend.lua",
	"srv_common/base/linecache.lua",
	"srv_common/base/traceback.lua",
	"srv_common/base/ldb.lua",
	"main_logic/base/global.lua",
	"main_logic/base/initnetwork.lua",
}

function startGame()
	os.exit = nil 

	sys = sys or {}
	sys.path = sys.path or {}
	table.insert(sys.path,posix.getcwd())

	local random_num = os.time()
	math.randomseed(random_num)

	for _,file in ipairs(DOFILELIST) do
		dofile(file)
	end

	MONGO_SL.initMongo()

	GAME_MAIN.initGame()
	GAME_MAIN.main()
end

function compilePto()
	startGame()
	CreateNetWorkConf()
end

