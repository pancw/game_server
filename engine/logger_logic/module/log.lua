local idxToLogName = {
	[1] = "server_error.log",	
}

local function getDir(logIdx)
	local fileName = idxToLogName[logIdx]
	assert(fileName)
	return "../log/" .. fileName	
end

local function doLog(dir, txt)
	local date = os.date("*t", os.time())
	local y = date.year
	local m = date.month
	local d = date.day
	local hour = date.hour
	local min = date.min
	local sec = date.sec
	local str = string.format("[%s-%02d-%02d %02d:%02d:%02d]:%s\n", y, m, d, hour, min, sec, txt)
	Logger.writeLog(dir, str)
end

local function onErrorMsg(ufd, logIdx, err)
	doLog(getDir(logIdx), err)
end

function __init__()
	RFC.errorMsg = onErrorMsg
end
