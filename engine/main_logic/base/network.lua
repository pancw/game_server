
UNIX_CLIENT = {}
UnixClientQueue = {
	--[[
	[port] = {
		msg,
	}
	--]]
}
function pushBackMsg(port, msg)
	if not UnixClientQueue[port] then
		UnixClientQueue[port] = {}
	end
	table.insert(UnixClientQueue[port], msg)
	print(string.format("push back msg to port:%d. sum:%d", port, #UnixClientQueue[port]))
end

function UnixClientConnectSuc(port)
	if not UnixClientQueue[port] then
		return
	end
	local msgCnt = #UnixClientQueue[port]
	for cnt=1, msgCnt do
		assert(UnixClient.send(port, UnixClientQueue[port][1]))
		table.remove(UnixClientQueue[port], 1)
	end
	UnixClientQueue[port] = nil
	print("resend msg to", port, msgCnt)
end

local function tryConnect(unixPort)
	if not UnixClient.checkHasConnected(unixPort) then
		UnixClient.connectUnixSrv(unixPort)	
	end
end

local b = 1000
local function tryConnectSrv()
	tryConnect(CONFIG.GetGateUnixPort())
	tryConnect(CONFIG.GetLogUnixPort())
end

function TickNetwork()
	local function all()
		tryConnectSrv()
	end
	xpcall(all, __G__TRACKBACK__)
end

setmetatable( UNIX_CLIENT, {
	__index = function (t, method_name)
		local function caller( ... )
			local args = { ... }
			local port = args[1]
			table.remove(args, 1)
			local msg = cmsgpack.pack({method_name, args})
			if not UnixClient.checkHasConnected(port) then
				pushBackMsg(port, msg)
				return	
			end
			assert(UnixClient.send(port, msg))
		end 
		return caller
	end 
})
