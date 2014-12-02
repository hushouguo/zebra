function dump(o, indent)
	if indent == nil then indent = "" end

	print(string.format("%s{", indent))
	for k, v in pairs(o) do
		if type(v) == "table" then 
			print(string.format("%s\t%s:(%s) = ", indent, k, type(k)))
			dump(v, string.format("%s\t", indent))
		else
			print(string.format("%s\t%s:(%s) = %s:(%s)", indent, k, type(k), v, type(v)))
		end
	end
	print(string.format("%s}", indent))
end

function check_standard_functions()
	print("check standard functions...")
	print(string.format("\tcheck `cc.now()` function:%d", cc.now()))
	print(string.format("\tnow:%d", cc.now()))
	print(string.format("\tcheck `cc.sleep(1)` function"))
	cc.sleep(1)
	print(string.format("\tnow:%d", cc.now()))
	print(string.format("\tcheck `cc.usleep(1000000)` function"))
	cc.usleep(1000000)
	print(string.format("\tnow:%d", cc.now()))
	print(string.format("\tcheck `cc.clocktime()` function:%d,%d", cc.clocktime()))
	print(string.format("\tcheck `cc.timestamp()` function:%s", cc.timestamp()))
	print(string.format("\tcheck `cc.timestamp(seconds)` function:%s", cc.timestamp(cc.now()+3600)))
	print(string.format("\tcheck `cc.timestamp(seconds, format)` function:%s", cc.timestamp(cc.now(), "%02y-%02m-%02d %02H:%02M:%02S")))
	print(string.format("\tcheck `cc.year()` function:%d", cc.year()))
	print(string.format("\tcheck `cc.year(seconds)` function:%d", cc.year(cc.now()+366*24*3600)))
	print(string.format("\tcheck `cc.month()` function:%d", cc.month()))
	print(string.format("\tcheck `cc.month(seconds)` function:%d", cc.month(cc.now()+31*24*3600)))
	print(string.format("\tcheck `cc.day()` function:%d", cc.day()))
	print(string.format("\tcheck `cc.day(seconds)` function:%d", cc.day(cc.now()+24*3600)))
	print(string.format("\tcheck `cc.hour()` function:%d", cc.hour()))
	print(string.format("\tcheck `cc.hour(seconds)` function:%d", cc.hour(cc.now()+3600)))
	print(string.format("\tcheck `cc.minute()` function:%d", cc.minute()))
	print(string.format("\tcheck `cc.minute(seconds)` function:%d", cc.minute(cc.now()+60)))
	print(string.format("\tcheck `cc.second()` function:%d", cc.second()))
	print(string.format("\tcheck `cc.second(seconds)` function:%d", cc.second(cc.now()+1)))
	print(string.format("\tcheck `cc.rand()` function:%d", cc.rand()))
	print(string.format("\tcheck `cc.rand(seed)` function:%d", cc.rand(0)))
	print(string.format("\tcheck `cc.random_between()` function:%d", cc.random_between(-10,10)))
	print(string.format("\tcheck `cc.random_between()` function:%d", cc.random_between(0,1)))
	--print(string.format("\tcheck `cc.xml_decode()` function"))
	--local xmlstr = '<?xml version="1.0"?><root><shard id="1" version="0.0.1" APPID="1" APPNAME="zebra"/></root>'
	--local o = cc.xml_decode(xmlstr)
	--dump(o)
	print(string.format("\tcheck `cc.json_encode()` function"))
	o = 
	{
		id=1,
		name="这是一个测试用例",
		array=
		{
			number=10,
			price=1.2,
			sell=true,
		}
	}
	local jsonstr = cc.json_encode(o)
	print(string.format("\tresult:%s", jsonstr))

	print(string.format("\tcheck `cc.json_decode()` function"))
	o = cc.json_decode(jsonstr)
	dump(o)

	print(string.format("\tcheck `cc.hash_string()` function:%d", cc.hash_string("hushouguo")))
end

function check_record_functions()
	print("check standard functions...")
	local db = cc.open_database("127.0.0.1", "root", "", "zebra", 3306)
	if not db then
		print("\tcheck `cc.open_database` function failure")
	else
		print("\tcheck `cc.open_database` function ok")
	end
	local id = cc.now()
	local table_name = string.format("test%d", id)
	if not cc.create_table(db, table_name) then
		print("\tcheck `cc.create_table` function failure")
	else
		print("\tcheck `cc.create_table` function ok")
	end
	if not cc.create_record(db, table_name, cc.now(), "this is record test routine") then
		print("\tcheck `cc.create_record` function failure")
	else
		print("\tcheck `cc.create_record` function ok")
	end
	local o = cc.select_record(db, table_name, id)
	if not o then
		print("\tcheck `cc.select_record` function failure")
	else
		print("\tcheck `cc.select_record` function:%s", o)
	end
	o = string.format("%s!!", o)
	if not cc.update_record(db, table_name, id, o) then
		print("\tcheck `cc.update_record` function failure")
	else
		print("\tcheck `cc.update_record` function ok")
	end
	local newo = cc.select_record(db, table_name, id)
	if not newo then
		print("\tcheck `cc.select_record` function failure")
	else
		print("\tcheck `cc.select_record` function:%s", newo)
	end
	if true then return end
	if not cc.delete_record(db, table_name, id) then
		print("\tcheck `cc.delete_record` function failure")
	else
		print("\tcheck `cc.delete_record` function ok")
	end
	if not cc.delete_table(db, table_name) then
		print("\tcheck `cc.delete_table` function failure")
	else
		print("\tcheck `cc.delete_table` function ok")
	end
end

function check_network_functions()
	local fd = cc.create_server("0.0.0.0", 12306)
	if not fd then
		print("\tcheck `cc.create_server` function failure")
	else
		print("\tcheck `cc.create_server` function ok:%d", fd)
	end
	--local client = cc.create_client("127.0.0.1", 12306)
	--if not client then
	--	print("\tcheck `cc.create_client` function failure")
	--else
	--	print("\tcheck `cc.create_client` function ok:%d", client)
	--end
	--cc.sendmsg(client, 1000, {platform=1})
	--cc.close(client)
end

function lua_protocol_start(id)
	--check_standard_functions()
	--check_record_functions()
	check_network_functions()
	--print(string.format("\tcheck `cc.halt()` function"))
	--cc.halt()
end
function lua_protocol_stop()
end
function lua_protocol_connection_close(fd)
	print(string.format("lost connection:%d", fd))
end
function lua_protocol_msg_parser(fd, cmd, o)
	print(string.format("receive msg:%d, cmd:%d", fd, cmd))
	--cc.close(fd)
end
function lua_protocol_timetick(milliseconds)
end
function lua_protocol_http_request(httpid, reqid, uri)
end
