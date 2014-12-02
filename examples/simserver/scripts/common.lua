--
-- common.lua
--

function dump(o, indent)
	if indent == nil then indent = "" end

	cc.debug_log(string.format("%s{", indent))
	for k, v in pairs(o) do
		if k ~= o then
			if type(v) == "table" then 
				cc.debug_log(string.format("%s  %s:(%s) = ", indent, k, type(k)))
				dump(v, string.format("%s  ", indent))
			else
				cc.debug_log(string.format("%s  %s:(%s) = %s:(%s)", indent, k, type(k), v, type(v)))
			end
		end
	end
	cc.debug_log(string.format("%s}", indent))
end

