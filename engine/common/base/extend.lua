function isbson(T)
	return isbsontbl(T) or isbsonlist(T)
end
function isbsontbl(T)
	return type(T) == 'userdata' and
		T.gettype and
		T:gettype() == "bsontbl"
end
function isbsonlist(T)
	return type(T) == 'userdata' and
		T.gettype and
		T:gettype() == "bsonlist"
end
function bpairs(T)
	if isbson(T) then
		return T:pairs()
	else
		return pairs(T)
	end
end
function bipairs(T)
	if isbson(T) then
		if isbsonlist(T) then
			return T:ipairs()
		else
			return ipairs({})
		end
	else
		return ipairs(T)
	end
end

local function PairsKeyCompare(k1, k2)
	return k1 < k2
end

function PairsOrderly(tbl, comp)
	local keys = {}
	table.foreach(tbl, function (k,v) table.insert(keys, k) end)
	table.sort(keys, comp or PairsKeyCompare)
	local keys_count = #keys
	local index = 0

	local next_orderly = function(tbl)
		index = index + 1
		if index > keys_count then
			return
		end
		return keys[index], tbl[keys[index]]
	end

	return next_orderly, tbl
end

local function _normalize(value)
	local retval = ''
	if type(value) == 'function' then
		retval = '<' .. tostring(value) .. '>'
	elseif type(value) == 'table' then
		retval = '<' .. tostring(value) .. '>'
	elseif type(value) == 'string' then
		retval = string.format('%q',value)
	else
		retval = tostring(value)
	end
	return retval
end

function sys.repr(value)
	local rettbl = {}
	local function enum_content(rettbl, value)
		local visited = {}
		for i, v in bipairs(value) do
			table.insert(rettbl, _normalize(v))
			table.insert(rettbl, ',')
			visited[i] = 1
		end
		for k, v in bpairs(value) do
			if not visited[k] then
				table.insert(rettbl, '[')
				table.insert(rettbl, _normalize(k))
				table.insert(rettbl, '] = ')
				table.insert(rettbl, _normalize(v))
				table.insert(rettbl, ', ')
			end
		end
	end
	if type(value) == 'table' then
		table.insert(rettbl, '{')
		enum_content(rettbl, value)
		table.insert(rettbl, '}')
	elseif isbson(T) then
		table.insert(rettbl, '{# ')
		enum_content(rettbl, value)
		table.insert(rettbl, ' #}')
	else
		table.insert(rettbl, _normalize(value))
	end
	return table.concat(rettbl)
end

local function dodump (value, c)
	local rettbl = {}
	if type(value) == 'table' or isbson(value) then
		c = (c or 0) + 1
		if c >= 100 then
			error("sys.dump too deep:")
			error(table.concat(rettbl))
		end

		if type(value) == 'table' then
			table.insert(rettbl, '{')
		else
			table.insert(rettbl, '{# ')
		end
		for k, v in bpairs(value) do
			table.insert(rettbl, '[')
			table.insert(rettbl, dodump(k,c))
			table.insert(rettbl, '] = ')
			table.insert(rettbl, dodump(v,c))
			table.insert(rettbl, ', ')
		end
		if type(value) == 'table' then
			table.insert(rettbl, '}')
		else
			table.insert(rettbl, ' #}')
		end
	else
		table.insert(rettbl, _normalize(value))
	end
	return table.concat(rettbl)
end
--为了防止死循环，不让它遍历超过100个结点。谨慎使用。
function sys.dump (value)
	local ni, ret = pcall (dodump, value)
	return ret
end

local REPR_INDENT = '  '
function bdump(value, forprint, deep)
	local linebr
	if forprint then
		linebr = '\n'
	else
		linebr = '#r'
	end
	local ret = ''
	if type(value) == 'table' or isbson(value) then
		deep = deep or 0
		if deep >= 100 then error("sys.dump too deep:"..ret) end
		local indent = string.rep(REPR_INDENT, deep) 
		if type(value) == 'table' then
			ret = ret .. '{' .. linebr
		else
			ret = ret .. '{# ' .. linebr
		end
		for k, v in bpairs(value) do
			local krepr
			if type(k)=='string' and string.match(k, '[_%a][_%a%d]*')==k then
				krepr = k
			else
				krepr = '[' .. bdump(k, forprint, deep + 1) .. ']'
			end
			ret = ret .. indent .. REPR_INDENT .. krepr .. ' = ' .. bdump(v, forprint, deep + 1) .. ',' .. linebr
		end
		if type(value) == 'table' then
			ret = ret .. indent .. '}'
		else
			ret = ret .. indent .. ' #}'
		end
		return ret 
	else
		return _normalize(value)
	end
end

-- 美化版的sys.dump
function sys.bdump(obj, forprint)
	local ni, ret = pcall(bdump, obj, forprint)
	return ret
end

function table.copy(src)
	local tbl = {}
	for k,v in pairs(src) do
		tbl[k] = v
	end	
	return tbl
end

function table.find(tb, value)
	for k, v in pairs(tb) do
		if value == v then
			return k, v
		end
	end
end

function table.lightcopy2(to, from)
	for k,v in pairs(from) do
		to[k] = v
	end	
	return to
end

--
-- 将表t1的内容合并到t中
--
function table.merge(t, t1, pairs_func)
	pairs_func = pairs_func or pairs
	for k, v1 in pairs_func(t1) do
		local vt1 = type(v1)
		if not t[k] then
			t[k] = table.deepcopy(v1)
		else
			local v = t[k]
			local vt = type(v)
			if vt == vt1 then
				if vt == 'table' then
					table.merge(t[k], t1[k])
				else
					t[k] = t1[k]
				end
			end
		end
	end
	return t
end

--
-- 以加的方式将表t1合并到表t中。
--
function table.addmerge(t, t1, pairs_func)
	pairs_func = pairs_func or pairs
	for k, v1 in pairs_func(t1) do
		local vt1 = type(v1)
		if not t[k] then
			t[k] = UTIL.DeepCopy(v1, true)
		else
			local v = t[k]
			local vt = type(v)
			if vt == vt1 then
				if vt == 'table' then
					table.addmerge(t[k], t1[k])
				elseif vt == 'number' then
					t[k] = t[k] + t1[k]
				elseif vt == 'string' then
					t[k] = t[k] .. t1[k]
				end
			end
		end
	end
end

function string.split( line, sep, maxsplit ) 
	if string.len(line) == 0 then
		return {}
	end
	sep = sep or ' '
	maxsplit = maxsplit or 0
	local retval = {}
	local pos = 1   
	local step = 0
	local run_cnt = 1
	while true do   
		run_cnt = run_cnt + 1
		if run_cnt >= 1000 then
			print("crit error !! string slit over 1000")
			return
		end
		local from, to = string.find(line, sep, pos, true)
		step = step + 1
		if (maxsplit ~= 0 and step > maxsplit) or from == nil then
			local item = string.sub(line, pos)
			table.insert( retval, item )
			break
		else
			local item = string.sub(line, pos, from-1)
			table.insert( retval, item )
			pos = to + 1
		end
	end     
	return retval
end

local magic_words = {"(",")",".","%","+","-","*","?","[","]","^","$",}
function string.convert(str)
	local split_str, tmp_str
	local out_str = ""

	if str == nil then
		return
	else
		if str == "" then
			return ""
		else
			for i = 1, string.len(str) do
				split_str = string.sub(str,i,i)
				for j = 1, #magic_words do
					if split_str == magic_words[j] then
						tmp_str = "%" .. split_str
						break
					else
						tmp_str = split_str
					end
				end
				out_str = out_str .. tmp_str
			end
			return out_str
		end
	end
end

function table.size(Table)
	if Table then
		local Ret = 0
		for _,_ in pairs(Table) do
			Ret = Ret + 1
		end
		return Ret
	else
		return 0
	end
end

function math.div(up, down)
	assert(down ~= 0)
	return up / down
end

function math.mod(up, down)
	assert(down ~= 0)
	return up % down
end

function table.random_values(Table, n)
	local list = {}
	for k, v in pairs(Table) do
		table.insert(list, v)
	end

	local ret = {}
	for i = 1, n do
		local len = table.maxn(list)
		local idx = math.random(1, len)
		table.insert(ret, list[idx])
		table.remove(list, idx)
	end

	return ret
end

function WARNING(Fromat, ...)
	_RUNTIME_ERROR(string.format(Fromat, ...))	
end

function GetUsecTime()
        local s,u = engine.RealTime()
        return s*1000000 + u
end

function GetPackSpeed()
	local DataSize, SecDiff, uSecDiff = network.GetPackSpeed()
	return DataSize/(SecDiff + uSecDiff/1000000)
end

function GetEatNetdDataSpeed()
	local DataSize, SecDiff, uSecDiff = network.GetEatNetdDataSpeed()
	return DataSize/(SecDiff + uSecDiff/1000000)
end

function PtoTester(...)
	local Arg = {...}
	for Pos, Val in ipairs(Arg) do
		if IsTable(Arg) then
			print(Pos, sys.repr(Val))
		else
			print(Pos, Val)
		end
	end
end

local __TermColorMap = {
	black = 30, 
	red   =  31, 
	green =  32, 
	yellow =  33, 
	blue  =  34, 
	magenta = 35, 
	cyan =  36, 
	white =  37, 
}

function TermColor(s, color)
	local c = string.char(0x1b)
	local cc = __TermColorMap[color] or 37
	return string.format('%s[0;%d;1m%s%s[0m', c, cc, s, c)
end

function AuxPrintToTerm(color, s)
	print(s)
end

function PrintToTerm(color, fmt, ...)
	local s = string.format(fmt, ...)
	AuxPrintToTerm(color, s)
end
