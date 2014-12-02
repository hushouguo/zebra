--
-- conf.lua
--

g_configure = 
{
	version = "0.0.1",
	APPID = 1,
	APPNAME = "zebra",
	logs_dir = ".logs",
	session = 
	{
		address = "0.0.0.0",
		port = 31000,
		db = 
		{
			host = "127.0.0.1",
			user = "root",
			passwd = "",
			db = "zebra",
			port = 3306
		},
	},
	pay = 
	{
		address = "0.0.0.0",
		port = 32000,
		db = 
		{
			host = "127.0.0.1",
			user = "root",
			passwd = "",
			db = "zebra",
			port = 3306
		},
		-- android 支付验证地址
		http_address = "0.0.0.0",
		http_port = 80,
	},
	user = 
	{
		address = "0.0.0.0",
		port = 33000,
		connect_session_address = "127.0.0.1",
		connect_session_port = 31000,
		connect_pay_address = "127.0.0.1",
		connect_pay_port = 32000,
		-- OSS、监控地址
		http_address = "0.0.0.0",
		http_port = 8000,
	},
}
