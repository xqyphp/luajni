#ifndef JAVA_SWITCH_LIST_H
#define JAVA_SWITCH_LIST_H

#define JAVA_DO_LIST \
	JAVA_DO_IT(Boolean,Boolean,z,lua_isboolean,lua_pushboolean,lua_toboolean) \
	JAVA_DO_IT(Byte,Byte,b,lua_isinteger,lua_pushinteger,lua_tointeger) \
	JAVA_DO_IT(Char,Char,c,lua_isinteger,lua_pushinteger,lua_tointeger) \
	JAVA_DO_IT(Short,Short,s,lua_isinteger,lua_pushinteger,lua_tointeger) \
	JAVA_DO_IT(Integer,Int,i,lua_isinteger,lua_pushinteger,lua_tointeger) \
	JAVA_DO_IT(Long,Long,j,lua_isinteger,lua_pushinteger,lua_tointeger) \
	JAVA_DO_IT(Float,Float,f,lua_isnumber,lua_pushnumber,lua_tonumber) \
	JAVA_DO_IT(Double,Double,d,lua_isnumber,lua_pushnumber,lua_tonumber) \
	JAVA_DO_IT(String,Object,l,lua_isstring,my_lua_pushjavastring,my_lua_tojavastring) \
	JAVA_DO_IT(Object,Object,l,lua_isuserdata,my_lua_pushjavaobject,my_lua_tojavaobject) \

#endif