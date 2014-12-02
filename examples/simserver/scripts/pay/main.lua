--
-- main.lua
--

require "script/conf"
require "script/common"

g_variables = 
{
	workerid		=	-1,
	logger_fd		=	-1,
	listening_fd	=	-1,
	db				=	-1,
}

function lua_protocol_start(workerid)
	if workerid == 0 then
		-- init logger file
		g_variables.logger_fd = cc.open_logger(string.format("%s/pay.%s", g_configure.logs_dir, cc.timestamp("%04Y.%02m.%02d")))
		if g_variables.logger_fd == nil or g_variables.logger_fd < 0 then
			cc.error_log("open logger file failure\n")
			cc.halt()
			return
		end
		if cc.daemon() then
			cc.dup(g_variables.logger_fd, 1)
			cc.dup(g_variables.logger_fd, 2)
			cc.close_logger(g_variables.logger_fd)
		end

		-- create server
		g_variables.listening_fd = cc.create_server(g_configure.pay.address, g_configure.pay.port)
		if g_variables.listening_fd == nil or g_variables.listening_fd < 0 then
			cc.error_log(string.format("create server:%s:%d failure\n", g_configure.pay.address, g_configure.pay.port))
			cc.halt()
			return
		end
	end

	-- open database
	g_variables.db = cc.open_database(g_configure.pay.db.host, g_configure.pay.db.user, g_configure.pay.db.passwd, g_configure.pay.db.db, g_configure.pay.db.port)
	if not g_variables.db then
		cc.error_log(string.format("fail to open database:%s,%s,%s,%s,%d\n", g_configure.pay.db.host, g_configure.pay.db.user, g_configure.pay.db.passwd, g_configure.pay.db.db, g_configure.pay.db.port))
		cc.halt()
		return
	end

	g_variables.workerid = workerid
end

function lua_protocol_stop()
end

function lua_protocol_connection_close(fd)
	cc.alarm_log(string.format("lost connection:%d\n", fd))
end

function lua_protocol_timetick(milliseconds)
end

function lua_protocol_msg_parser(fd, userid, cmd, o)
end

function lua_protocol_http_request(httpid, reqid, uri)
end

