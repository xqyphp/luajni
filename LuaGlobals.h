#pragma once
#include "lua.hpp"
#include "jni.h"

class LuaGlobals
{
public:
	LuaGlobals(lua_State *L);
	~LuaGlobals();
	
	void pushJniEnv(JNIEnv *env);
	JNIEnv *getJniEnv();
	//不会转换为全局引用，调用前最好转换
	void pushJavaObject(const char* key,jobject obj);
	jobject getJavaObject(const char *key);

	void pushData(const char* key, void* userdata, size_t size);
	void *getData(const char* key);
private:
	lua_State *mL;
};

