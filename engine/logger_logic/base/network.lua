RFC = {}

function HandleUnixEvent(ufd, msg)
	local msgTbl = cmsgpack.unpack(msg)
	local method = msgTbl[1]
	local args = msgTbl[2]
	RFC[method](ufd, unpack(args))
end
