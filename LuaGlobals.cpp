#include "LuaGlobals.h"
#include <string.h>
#include <string>

#define LUA_JNI_ENV_GLOBAL_KEY "__jniEnv_lua"
#define LUA_JNI_JAVA_OBJECT_KEY "__javaObject_lua_"

LuaGlobals::LuaGlobals(lua_State * L)
	:mL(L)
{
}

LuaGlobals::~LuaGlobals()
{
}

void LuaGlobals::pushJniEnv(JNIEnv *env)
{
	pushData(LUA_JNI_ENV_GLOBAL_KEY, &env, sizeof(JNIEnv *));
}

JNIEnv * LuaGlobals::getJniEnv()
{
	return *(JNIEnv **)getData(LUA_JNI_ENV_GLOBAL_KEY);
}

void LuaGlobals::pushJavaObject(const char * key, jobject obj)
{
	std::string rkey(LUA_JNI_JAVA_OBJECT_KEY);
	rkey += key;
	pushData(rkey.c_str(), &obj, sizeof(obj));
}

jobject LuaGlobals::getJavaObject(const char * key)
{
	std::string rkey(LUA_JNI_JAVA_OBJECT_KEY);
	rkey += key;
	jobject * val = (jobject *)getData(rkey.c_str());
	if (val != NULL) {
		return *val;
	}
	return NULL;
}

void LuaGlobals::pushData(const char* key,void* data,size_t size)
{
	lua_State * L = mL;
	void * userdata;
	

	lua_pushstring(L, key);
	lua_rawget(L, LUA_REGISTRYINDEX);

	if (!lua_isnil(L, -1)) {
		userdata = lua_touserdata(L, -1);
		memcpy(userdata, data, size);
		lua_pop(L, 1);
	}
	else {
		lua_pop(L, 1);
		userdata = lua_newuserdata(L, size);
		memcpy(userdata, data, size);

		lua_pushstring(L, key);
		lua_insert(L, -2);
		lua_rawset(L, LUA_REGISTRYINDEX);
	}
}

void *LuaGlobals::getData(const char * key)
{
	lua_State * L = mL;
	void * userdata;

	lua_pushstring(L, key);
	lua_rawget(L, LUA_REGISTRYINDEX);

	if (!lua_isuserdata(L, -1)) {
		lua_pop(L, 1);
		return NULL;
	}

	userdata = lua_touserdata(L, -1);

	lua_pop(L, 1);
	return userdata;
}
