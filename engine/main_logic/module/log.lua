
local function sendLog(logIdx, err)
	UNIX_CLIENT.errorMsg(CONFIG.GetLogUnixPort(), logIdx, err)
end

function sendError(err)
	sendLog(1, err)
end
