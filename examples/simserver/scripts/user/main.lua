--
-- main.lua
--

require "script/conf"
require "script/common"

g_variables = 
{
	workerid			=	-1,
	logger_fd			=	-1,
	session_client_fd	=	-1,
	pay_client_fd		=	-1,
}

function lua_protocol_start(workerid)
	if workerid == 0 then
		-- init logger file
		g_variables.logger_fd = cc.open_logger(string.format("%s/user.%s", g_configure.logs_dir, cc.timestamp("%04Y.%02m.%02d")))
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
		g_variables.listening_fd = cc.create_server(g_configure.user.address, g_configure.user.port)
		if g_variables.listening_fd == nil or g_variables.listening_fd < 0 then
			cc.error_log(string.format("create server:%s:%d failure\n", g_configure.user.address, g_configure.user.port))
			cc.halt()
			return
		end
	end

	-- connect session server
	g_variables.session_client_fd = cc.create_client(g_configure.user.connect_session_address, g_configure.user.connect_session_port)
	if g_variables.session_client_fd == nil or g_variables.session_client_fd < 0 then
		cc.error_log(string.format("create session client:%s:%d failure\n", g_configure.user.connect_session_address, g_configure.user.connect_session_port))
		cc.halt()
		return
	end

	-- connect pay server
	g_variables.pay_client_fd = cc.create_client(g_configure.user.connect_pay_address, g_configure.user.connect_pay_port)
	if g_variables.pay_client_fd == nil or g_variables.pay_client_fd < 0 then
		cc.error_log(string.format("create pay client:%s:%d failure\n", g_configure.user.connect_pay_address, g_configure.user.connect_pay_port))
		cc.halt()
		return
	end


	g_variables.workerid = workerid
end

function lua_protocol_stop()
end

function lua_protocol_connection_close(fd)
	if fd == g_variables.session_client_fd or fd == g_variables.pay_client_fd then
		cc.error_log(string.format("lost server connection, fd:%d, session:%d, pay:%d\n", fd, g_variables.session_client_fd, g_variables.pay_client_fd))
		cc.halt()
	else
		cc.alarm_log(string.format("lost connection:%d\n", fd))
	end
end

function lua_protocol_timetick(milliseconds)
end

function lua_protocol_msg_parser(fd, userid, cmd, o)
end

function lua_protocol_http_request(httpid, reqid, uri)
end

