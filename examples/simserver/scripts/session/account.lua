--
-- 账号系统
--

g_account = {}
g_account_cache = {}

local ACCT_TABLE_NAME = "acct"

-- 注册新帐号
function g_account.register(fd, acctid, name, password)
	local retcode = false
	if g_account_cache[acctid] == nil then
		local account = {name=name, password=password}
		retcode = cc.create_record(g_variables.db, ACCT_TABLE_NAME, acctid, cc.json_encode(account))
		if retcode then
			g_account_cache[acctid] = account
		end
	end
	cc.sendmsg(fd, acctid, 203, {retcode=retcode, name=name})
end

-- 绑定角色到账号上
function g_account.bind(fd, acctid, userid)
	local retcode = false
	local account = g_account_cache[acctid]
	if account == nil then
		local o = cc.select_record(g_variables.db, ACCT_TABLE_NAME, acctid)
		if o ~= nil then
			account = cc.json_decode(o)
			g_account_cache[acctid] = account
		end
	end
	if account ~= nil then
		account.userid = tostring(userid)
		retcode = cc.update_record(g_variables.db, ACCT_TABLE_NAME, acctid, cc.json_encode(account))
	end
	cc.sendmsg(fd, acctid, 205, {retcode=retcode})
end

-- 账号登录
function g_account.login(fd, acctid, name, password)
	local retcode = false
	local userid = acctid
	local account = g_account_cache[acctid]
	if account == nil then
		local o = cc.select_record(g_variables.db, ACCT_TABLE_NAME, acctid)
		if o ~= nil then
			account = cc.json_decode(o)
			g_account_cache[acctid] = account
		end
	end
	if account ~= nil then
		if account.name == name and account.password == password then
			retcode = true
			userid = account.userid
		end
	end
	cc.sendmsg(fd, acctid, 201, {retcode=retcode, userid=userid, name=name})
end

