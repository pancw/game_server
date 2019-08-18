
UNIX_CLIENT = {}

setmetatable( UNIX_CLIENT, {
	__index = function (t, method_name)
		local function caller( ... )
			local args = { ... }
			local port = args[1]
			table.remove(args, 1)
			local msg = cmsgpack.pack({method_name, args})
			if not UnixClient.checkHasConnected(port) then
				NETWORK.pushBackMsg(port, msg)
				return	
			end
			assert(UnixClient.send(port, msg))
		end 
		return caller
	end 
})

function UnixClientConnectSuc(port)
	NETWORK.unixClientConnectSuc(port)
end
