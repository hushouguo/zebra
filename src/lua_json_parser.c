/*
 * \file: lua_json_parser.c
 * \brief: Created by hushouguo at Nov 06 2014 18:20:14
 */

#include "zebra.h"

#define UTF8_ERROR 	-2

typedef enum
{
    JPS_NONE = 0,
    JPS_IN_ARRAY,
    JPS_IN_TABLE
} json_paser_state;

typedef struct lua_json_parser_ctx
{
	lua_State*					L;
    int                     	key_lvl;
    json_paser_state        	cur_state;
	byte_buffer_t*				state_stack;
    int							array_idx;
	byte_buffer_t*				array_stack;
} lua_json_parser_ctx_t;

lua_json_parser_ctx_t* lua_json_parser_ctx_create(lua_State* _L)
{
	lua_json_parser_ctx_t* ctx = (lua_json_parser_ctx_t*)malloc(sizeof(lua_json_parser_ctx_t));
	ctx->L = _L;
	ctx->key_lvl = 0;
	ctx->cur_state = JPS_NONE;
	ctx->state_stack = byte_buffer_create(256);
	ctx->array_idx = 1;
	ctx->array_stack = byte_buffer_create(256);
	return ctx;
}

void lua_json_parser_ctx_delete(lua_json_parser_ctx_t* ctx)
{
	byte_buffer_delete(ctx->state_stack);
	byte_buffer_delete(ctx->array_stack);
	SAFE_FREE(ctx);
}

int _json_parser_cb(void* ctx, int type, const JSON_value* value)
{
    int ret = 1;
    lua_json_parser_ctx_t* jpctx = (lua_json_parser_ctx_t*)ctx;

    switch(type)
    {
		case JSON_T_ARRAY_BEGIN:
			if (jpctx->cur_state == JPS_IN_ARRAY)
			{
				lua_pushinteger(jpctx->L, jpctx->array_idx++);
			}
		
			lua_newtable(jpctx->L);
			if (jpctx->state_stack->current == 0)
			{
				/* first array, root object */
			}
			byte_buffer_push_char(jpctx->state_stack, jpctx->cur_state);
			jpctx->cur_state = JPS_IN_ARRAY;

			byte_buffer_push_char(jpctx->array_stack, jpctx->array_idx);
			jpctx->array_idx = 1;			
			break;
		case JSON_T_ARRAY_END:
			jpctx->cur_state = byte_buffer_pop_char(jpctx->state_stack);
			jpctx->array_idx = byte_buffer_pop_char(jpctx->array_stack);
			if (jpctx->cur_state == JPS_IN_TABLE)
			{
				if (jpctx->key_lvl > 0)
				{
					lua_settable(jpctx->L, -3);
					--jpctx->key_lvl;
				}
				else
				{
					ret = 0;
					error_log("json object value without key\n");
				}
			}
			else if (jpctx->cur_state == JPS_IN_ARRAY)
			{
				lua_settable(jpctx->L, -3);
			}
			else
			{
				/* lua_pop(jpctx->L, 1); */
				/* root object, stay in stack, don't pop */
			}
			break;
		case JSON_T_OBJECT_BEGIN:
			if (jpctx->cur_state == JPS_IN_ARRAY)
			{
				lua_pushinteger(jpctx->L, jpctx->array_idx++);
			}
			
			lua_newtable(jpctx->L);
			if (jpctx->state_stack->current == 0)
			{
				/* first table, root object */
			}
			byte_buffer_push_char(jpctx->state_stack, jpctx->cur_state);
			jpctx->cur_state = JPS_IN_TABLE;
			break;
		case JSON_T_OBJECT_END:
			jpctx->cur_state = byte_buffer_pop_char(jpctx->state_stack);
			if (jpctx->cur_state == JPS_IN_TABLE)
			{
				if(jpctx->key_lvl > 0)
				{
					lua_settable(jpctx->L, -3);
					--jpctx->key_lvl;
				}
				else
				{
					ret = 0;
					error_log("json object value without key\n");
				}
			}
			else if (jpctx->cur_state == JPS_IN_ARRAY)
			{
				lua_settable(jpctx->L, -3);
			}
			else if (jpctx->cur_state == JPS_NONE)
			{
				/* root object, stay in stack */
			}
			else
			{
				/* lua_pop(jpctx->L, 1); */
			}
			break;
		case JSON_T_INTEGER:
			if(jpctx->cur_state == JPS_IN_TABLE)
			{
				if (jpctx->key_lvl > 0)
				{
					lua_pushinteger(jpctx->L, value->vu.integer_value);
					lua_settable(jpctx->L, -3);
					--jpctx->key_lvl;
				}
				else
				{
					ret = 0;
					error_log("json object value without key\n");
				}
			}
			else if (jpctx->cur_state == JPS_IN_ARRAY)
			{
				lua_pushinteger(jpctx->L, jpctx->array_idx++);
				lua_pushinteger(jpctx->L, value->vu.integer_value);
				lua_settable(jpctx->L, -3);
			}
			else
			{
				ret = 0;
				error_log("json no root object:%d\n", __LINE__);
			}
			break;
		case JSON_T_FLOAT:
			if (jpctx->cur_state == JPS_IN_TABLE)
			{
				if (jpctx->key_lvl > 0)
				{
					lua_pushnumber(jpctx->L, value->vu.float_value);
					lua_settable(jpctx->L, -3);
					--jpctx->key_lvl;
				}
				else
				{
					ret = 0;
					error_log("json object value without key\n");
				}
			}
			else if (jpctx->cur_state == JPS_IN_ARRAY)
			{
				lua_pushinteger(jpctx->L, jpctx->array_idx++);
				lua_pushnumber(jpctx->L, value->vu.float_value);
				lua_settable(jpctx->L, -3);
			}
			else
			{
				ret = 0;
				error_log("json no root object:%d\n", __LINE__);
			}
			break;
		case JSON_T_NULL:
			if (jpctx->cur_state == JPS_IN_TABLE)
			{
				if (jpctx->key_lvl > 0)
				{
					lua_pushnil(jpctx->L);
					lua_settable(jpctx->L, -3);
					--jpctx->key_lvl;
				}
				else
				{
					ret = 0;
					error_log("json object value without key\n");
				}
			}
			else if (jpctx->cur_state == JPS_IN_ARRAY)
			{
				lua_pushinteger(jpctx->L, jpctx->array_idx++);
				lua_pushnil(jpctx->L);
				lua_settable(jpctx->L, -3);
			}
			else
			{
				ret = 0;
				error_log("json no root object:%d\n", __LINE__);
			}
			break;
		case JSON_T_TRUE:
			if (jpctx->cur_state == JPS_IN_TABLE)
			{
				if (jpctx->key_lvl > 0)
				{
					lua_pushboolean(jpctx->L, true);
					lua_settable(jpctx->L, -3);
					--jpctx->key_lvl;
				}
				else
				{
					ret = 0;
					error_log("json object value without key\n");
				}
			}
			else if (jpctx->cur_state == JPS_IN_ARRAY)
			{
				lua_pushinteger(jpctx->L, jpctx->array_idx++);
				lua_pushboolean(jpctx->L, true);
				lua_settable(jpctx->L, -3);
			}
			else
			{
				ret = 0;
				error_log("json no root object:%d\n", __LINE__);
			}
			break;
		case JSON_T_FALSE:
			if (jpctx->cur_state == JPS_IN_TABLE)
			{
				if (jpctx->key_lvl > 0)
				{
					lua_pushboolean(jpctx->L, false);
					lua_settable(jpctx->L, -3);
					--jpctx->key_lvl;
				}
				else
				{
					ret = 0;
					error_log("json object value without key\n");
				}
			}
			else if (jpctx->cur_state == JPS_IN_ARRAY)
			{
				lua_pushinteger(jpctx->L, jpctx->array_idx++);
				lua_pushboolean(jpctx->L, false);
				lua_settable(jpctx->L, -3);
			}
			else
			{
				ret = 0;
				error_log("json no root object:%d\n", __LINE__);
			}
			break;
		case JSON_T_STRING:
			if (jpctx->cur_state == JPS_IN_TABLE)
			{
				if (jpctx->key_lvl > 0)
				{
					lua_pushlstring(jpctx->L, value->vu.str.value, value->vu.str.length);
					lua_settable(jpctx->L, -3);
					--jpctx->key_lvl;
				}
				else
				{
					ret = 0;
					error_log("json object value without key\n");
				}
			}
			else if (jpctx->cur_state == JPS_IN_ARRAY)
			{
				lua_pushinteger(jpctx->L, jpctx->array_idx++);
				lua_pushlstring(jpctx->L, value->vu.str.value, value->vu.str.length);
				lua_settable(jpctx->L, -3);
			}
			else
			{
				ret = 0;
				error_log("json no root object:%d\n", __LINE__);
			}
			break;
		case JSON_T_KEY:
			lua_pushlstring(jpctx->L, value->vu.str.value, value->vu.str.length);
			++jpctx->key_lvl;
			break;
    }
    return ret;
}
bool lua_json_parser_decode(lua_State* L, const char* jsonstr, unsigned int len)
{
	unsigned int i;
    int oldtop = lua_gettop(L);
	lua_json_parser_ctx_t* ctx = lua_json_parser_ctx_create(L);
    JSON_config config;
	struct JSON_parser_struct* jc = NULL;

    init_JSON_config(&config);

    config.depth                  = -1;
    config.callback               = &_json_parser_cb;
    config.callback_ctx           = ctx;
    config.allow_comments         = 0;
    config.handle_floats_manually = 0;

    /* setlocale(LC_ALL, argv[1]) */

    jc = new_JSON_parser(&config);
    for(i = 0; i < len; ++i)
    {
        if(jsonstr[i] <= 0)
        {
            break;
        }

        if (!JSON_parser_char(jc, jsonstr[i])) 
		{
			error_log("JSON_parser_char: syntax error, byte: %d, %s\n", i, jsonstr);
            delete_JSON_parser(jc);
			lua_json_parser_ctx_delete(ctx);
            lua_settop(L, oldtop);
            return false;
        }
    }

    if (!JSON_parser_done(jc)) 
	{
		error_log("JSON_parser_end: syntax error:%s\n", jsonstr);
        delete_JSON_parser(jc);
		lua_json_parser_ctx_delete(ctx);
        lua_settop(L, oldtop);
        return false;
    }

    delete_JSON_parser(jc);
	lua_json_parser_ctx_delete(ctx);

    return true;
}

int json_utf8_get(const char* buf, int idx)
{
    return buf[idx] & 0xFF;
}
/*
 *	Get the 6-bit payload of the next continuation byte.
 *	Return UTF8_ERROR if it is not a continuation byte.
 */
int json_utf8_cont(const char* buf, int idx)
{
    int c = json_utf8_get(buf, idx);
    return ((c & 0xC0) == 0x80) ? (c & 0x3F) : UTF8_ERROR;
}
bool json_utf8_decode_single(const char* buf, int buflen_left, unsigned int* o_char, int* o_len)
{
	int c;
    if(buflen_left < 1)
    {
        return false;
    }
    c = json_utf8_get(buf, 0);  /* the first byte of the character */

    /* Zero continuation (0 to 127) */
    if ((c & 0x80) == 0) 
	{
        *o_char = c;
        *o_len = 1;
        return true;
    }
    /* One continuation (128 to 2047) */
    if ((c & 0xE0) == 0xC0) 
	{
		int c1;
        if(buflen_left < 2)
        {
            return false;
        }
        c1 = json_utf8_cont(buf, 1);
        if (c1 < 0) 
		{
            return false;
        }
        *o_char = ((c & 0x1F) << 6) | c1;
        *o_len = 2;
        return *o_char >= 128 ? true : false;
    }
    /* Two continuation (2048 to 55295 and 57344 to 65535) */
    if ((c & 0xF0) == 0xE0) 
	{
		int c1, c2;
        if(buflen_left < 3)
        {
            return false;
        }
        c1 = json_utf8_cont(buf, 1);
        c2 = json_utf8_cont(buf, 2);
        if (c1 < 0 || c2 < 0) 
		{
            return false;
        }
        *o_char = ((c & 0x0F) << 12) | (c1 << 6) | c2;
        *o_len = 3;
        return *o_char >= 2048 && (*o_char < 55296 || *o_char > 57343) ? true : false;
    }
    /* Three continuation (65536 to 1114111) */
    if ((c & 0xF8) == 0xF0) 
	{
		int c1, c2, c3;
        if(buflen_left < 4)
        {
            return false;
        }
        c1 = json_utf8_cont(buf, 1);
        c2 = json_utf8_cont(buf, 2);
        c3 = json_utf8_cont(buf, 3);
        if (c1 < 0 || c2 < 0 || c3 < 0) 
		{
            return false;
        }
        *o_char = ((c & 0x0F) << 18) | (c1 << 12) | (c2 << 6) | c3;
        *o_len = 4;
        return *o_char >= 65536 && *o_char <= 1114111 ? true : false;
    }
    return false;
}
void create_json_escapes(const char* s, byte_buffer_t* bb, int srclen)
{
	const char* b = s;
	int srclen_left = srclen < 0 ? strlen(s) : srclen;
	byte_buffer_push_char(bb, '"');
    while (b && *b)
    {
		/*
         * This assumes an ASCII superset. But so does everything in PTree.
         * We escape everything outside ASCII, because this code can't
         * handle high unicode characters.
		 */
        if (
				*b == 0x20 
					|| 
				*b == 0x21 
					|| 
				(*b >= 0x23 && *b <= 0x2E) 
					||
            	(*b >= 0x30 && *b <= 0x5B) 
					|| 
				(*b >= 0x5D) /* && *b <= 0xFF */
			)
		{
			byte_buffer_push_char(bb, *b);
		}
        else if (*b == '\b') { byte_buffer_push_char(bb, '\\'); byte_buffer_push_char(bb, 'b'); }
        else if (*b == '\f') { byte_buffer_push_char(bb, '\\'); byte_buffer_push_char(bb, 'f'); }
        else if (*b == '\n') { byte_buffer_push_char(bb, '\\'); byte_buffer_push_char(bb, 'n'); }
        else if (*b == '\r') { byte_buffer_push_char(bb, '\\'); byte_buffer_push_char(bb, 'r'); }
        else if (*b == '/')  { byte_buffer_push_char(bb, '\\'); byte_buffer_push_char(bb, '/'); }
        else if (*b == '"')  { byte_buffer_push_char(bb, '\\'); byte_buffer_push_char(bb, '"'); }
        else if (*b == '\\') { byte_buffer_push_char(bb, '\\'); byte_buffer_push_char(bb, '\\');}
        else
        {
            unsigned int o_char = 0;
            int o_len = 0;
			unsigned int u = *b;
            unsigned int u_n = 0;
			const char* hexdigits = NULL;

            if(json_utf8_decode_single(b, srclen_left, &o_char, &o_len))
            {
                u = o_char & 0x0000ffff;
                u_n = o_char >> 16;

                b += (o_len-1);
                srclen_left -= (o_len-1);
            }

            hexdigits = "0123456789ABCDEF";
            if(u_n != 0)
            {
                int d1 = u_n / 4096; u_n %= 4096;
                int d2 = u_n / 256; u_n %= 256;
                int d3 = u_n / 16; u_n %= 16;
                int d4 = u_n;
				byte_buffer_push_char(bb, '\\'); byte_buffer_push_char(bb, 'u');
				byte_buffer_push_char(bb, hexdigits[d1]); byte_buffer_push_char(bb, hexdigits[d2]);
				byte_buffer_push_char(bb, hexdigits[d3]); byte_buffer_push_char(bb, hexdigits[d4]);
            }

			{
				int d1 = u / 4096; u %= 4096;
				int d2 = u / 256; u %= 256;
				int d3 = u / 16; u %= 16;
				int d4 = u;
				byte_buffer_push_char(bb, '\\'); byte_buffer_push_char(bb, 'u');
				byte_buffer_push_char(bb, hexdigits[d1]); byte_buffer_push_char(bb, hexdigits[d2]);
				byte_buffer_push_char(bb, hexdigits[d3]); byte_buffer_push_char(bb, hexdigits[d4]);
			}
        }
        ++b;
        --srclen_left;
    }
	byte_buffer_push_char(bb, '"');
	/* byte_buffer_push_char(bb, '\0'); */
}
bool lua_json_parser_dump_object(lua_State* L, byte_buffer_t* bb)
{
	switch (lua_type(L, -1))
    {
		case LUA_TNIL:
			byte_buffer_push_data(bb, "null", 4);
			break;
		case LUA_TNUMBER:
			{
				/*
				 *char temp[256];
				 *lua_Number f = lua_tonumber(L, -1);
				 *int nsize = snprintf(temp, 256, "%lf", f);
				 *json_str.push_back(temp, nsize);
				 */
				size_t len = 0;
				const char* s = lua_tolstring(L, -1, &len);/* compatible lua_Integer & lua_Number */
				byte_buffer_push_data(bb, (char*)s, len);
			}
			break;
		case LUA_TBOOLEAN:
			{
				int b = lua_toboolean(L, -1);
				if (b)
				{
					byte_buffer_push_data(bb, "true", 4);
				}
				else
				{
					byte_buffer_push_data(bb, "false", 5);
				}
			}
			break;
		case LUA_TSTRING:
			{
				size_t len = 0;
				const char* s = lua_tolstring(L, -1, &len);
				create_json_escapes(s, bb, len);
			}
			break;
		case LUA_TTABLE:
			{
				int idx = 0;
				byte_buffer_push_char(bb, '{');

				lua_pushnil(L);
				while (lua_next(L, -2))
				{
					size_t len = 0;
					const char* key = NULL;

					/* -1 is the value and -2 is the key */
					if (idx++ > 0)
					{
						byte_buffer_push_char(bb, ',');
					}

					/* fetch key, don't lua_tostring(L, -1); */
					lua_pushvalue(L, -2);
					key = lua_tolstring(L, -1, &len);
					lua_pop(L, 1);

					create_json_escapes(key, bb, len);

					byte_buffer_push_char(bb, ':');

					if (!lua_json_parser_dump_object(L, bb)) { return false; }

					lua_pop(L, 1); /* removes 'value'; keeps 'key' for next iteration */
				}
				byte_buffer_push_char(bb, '}');
			}
			break;
		case LUA_TFUNCTION:
		case LUA_TUSERDATA:
		case LUA_TTHREAD:
		case LUA_TLIGHTUSERDATA:
		default: 
			error_log("Unable dump object type:%s\n", lua_typename(L, lua_type(L, -1)));
			return false;
    }
    return true;
}
const char* lua_json_parser_encode(lua_State* L)
{
	int oldtop = lua_gettop(L);
	byte_buffer_t* bb = byte_buffer_create(KB);
    bool retcode = lua_json_parser_dump_object(L, bb);
	lua_settop(L, oldtop);
	if (retcode)
	{
		byte_buffer_push_char(bb, '\0');
		/*debug_log("lua json parser encode:%s,%ld\n", bb->buf, bb->current);*/
	}
    return retcode ? bb->buf : NULL;
}

