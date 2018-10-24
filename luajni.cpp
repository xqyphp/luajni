#include "lua.hpp"
#include "jni.h"
#include "LuaGlobals.h"
#include "JavaData.h"
#include "JavaCaller.h"

#include <string>
#include "luajni.h"


#define LUA_JNI "luajni"

int ExceptionCheckAndClear(JNIEnv *env);

int lua_FindClass(lua_State *L);
int lua_NewObject(lua_State *L);

int lua_CallMethod(lua_State *L);
int lua_CallStaticMethod(lua_State *L);

int lua_GetField(lua_State *L);
int lua_SetField(lua_State *L);

int lua_GetStaticField(lua_State *L);
int lua_SetStaticField(lua_State *L);


int lua_GetJavaObject(lua_State *L);

int lua_PushLocalFrame(lua_State *L);

int lua_PopLocalFrame(lua_State *L);

int lua_DeleteLocalRef(lua_State *L);

void luajni_open(lua_State *L, JNIEnv *env)
{
	lua_newtable(L);

	lua_setglobal(L, LUA_JNI);

	lua_getglobal(L, LUA_JNI);


	lua_pushstring(L, "findClass");
	lua_pushcfunction(L, &lua_FindClass);
	lua_settable(L, -3);

	lua_pushstring(L, "newObject");
	lua_pushcfunction(L, &lua_NewObject);
	lua_settable(L, -3);

	lua_pushstring(L, "callMethod");
	lua_pushcfunction(L, &lua_CallMethod);
	lua_settable(L, -3);

	lua_pushstring(L, "callStaticMethod");
	lua_pushcfunction(L, &lua_CallStaticMethod);
	lua_settable(L, -3);

	lua_pushstring(L, "getField");
	lua_pushcfunction(L, &lua_GetField);
	lua_settable(L, -3);

	lua_pushstring(L, "setField");
	lua_pushcfunction(L, &lua_SetField);
	lua_settable(L, -3);

	lua_pushstring(L, "getStaticField");
	lua_pushcfunction(L, &lua_GetStaticField);
	lua_settable(L, -3);

	lua_pushstring(L, "setStaticField");
	lua_pushcfunction(L, &lua_SetStaticField);
	lua_settable(L, -3);

	lua_pushstring(L, "getJavaObject");
	lua_pushcfunction(L, &lua_GetJavaObject);
	lua_settable(L, -3);

    lua_pushstring(L, "pushLocalFrame");
    lua_pushcfunction(L, &lua_PushLocalFrame);
    lua_settable(L, -3);

    lua_pushstring(L, "popLocalFrame");
    lua_pushcfunction(L, &lua_PopLocalFrame);
    lua_settable(L, -3);

    lua_pushstring(L, "deleteLocalRef");
    lua_pushcfunction(L, &lua_DeleteLocalRef);
    lua_settable(L, -3);

	lua_pop(L, 1);

	LuaGlobals luaGlobals(L);
	luaGlobals.pushJniEnv(env);
}

void luajni_push_object(lua_State * L, const char * key, jobject obj)
{
	LuaGlobals globals(L);
	globals.pushJavaObject(key, obj);
}


int ExceptionCheckAndClear(JNIEnv * env)
{
	if (env->ExceptionOccurred()) {
		env->ExceptionClear();
		return -1;
	}
	return 0;
}

int lua_FindClass(lua_State * L)
{
	LuaGlobals globals(L);
	JNIEnv *env = globals.getJniEnv();
	
	if (!lua_isstring(L, 1)) {
		lua_pushnil(L);
		return 1;
	}

	const char* clzName = lua_tostring(L, 1);
	jclass clz = env->FindClass(clzName);


	if (0 != ExceptionCheckAndClear(env)) {
		lua_pushnil(L);
		return 1;
	}

	JavaData javaData(L);
	return javaData.pushJavaClass(clz, clzName);
}

int lua_NewObject(lua_State * L)
{
	LuaGlobals globals(L);
	JNIEnv *env = globals.getJniEnv();
	jclass javaClz = NULL;
	const char* clzName = NULL;
	if (lua_isstring(L, 1)) {
		clzName = lua_tostring(L, 1);
		javaClz = env->FindClass(clzName);
	}
	else if(lua_isuserdata(L,1)){
		JavaData javaData(L);
		if (javaData.GetDataType(1) != JavaTypeClass) {
			lua_pushnil(L);
			return 1;
		}
		javaClz = javaData.GetJavaClass(1);
	}
	
	if (0 != ExceptionCheckAndClear(env) || javaClz == NULL) {
		lua_pushnil(L);
		return 1;
	}

	if (!lua_isstring(L, 2)) {
		lua_pushnil(L);
		return 1;
	}

	const char *descriptor = lua_tostring(L, 2);
	JavaCaller caller(env, L);
	if (caller.CallConstruct(javaClz, descriptor, 3)) {
		return caller.PushReturn();
	}

	return 0;
}


int lua_CallMethod(lua_State * L)
{
	LuaGlobals globals(L);
	JNIEnv *env = globals.getJniEnv();

	jobject javaObject;
	JavaData javaData(L);
	if (javaData.GetDataType(1) != JavaTypeObject) {
		lua_pushnil(L);
		return 1;
	}

	javaObject = javaData.GetJavaObject(1);
	if (javaObject == NULL) {
		lua_pushnil(L);
		return 1;
	}

	if (!lua_isstring(L, 2)) {
		lua_pushnil(L);
		return 1;
	}
	const char* methodName = lua_tostring(L, 2);
	if (!lua_isstring(L, 3)) {
		lua_pushnil(L);
		return 1;
	}
	const char *descriptor = lua_tostring(L, 3);
	JavaCaller caller(env, L);
	if (caller.CallMethod(javaObject, methodName, descriptor, 4)) {
		return caller.PushReturn();
	}

	return 0;
}

int lua_CallStaticMethod(lua_State * L)
{
	LuaGlobals globals(L);
	JNIEnv *env = globals.getJniEnv();
	jclass javaClz = NULL;
	if (lua_isstring(L, 1)) {
		const char* clzName = lua_tostring(L, 1);
		javaClz = env->FindClass(clzName);
	}
	else if (lua_isuserdata(L, 1)) {
		JavaData javaData(L);
		if (javaData.GetDataType(1) != JavaTypeClass) {
			lua_pushnil(L);
			return 1;
		}
		javaClz = javaData.GetJavaClass(1);
	}

	if (0 != ExceptionCheckAndClear(env) || javaClz == NULL) {
		lua_pushnil(L);
		return 1;
	}

	if (!lua_isstring(L, 2)) {
		lua_pushnil(L);
		return 1;
	}
	const char* methodName = lua_tostring(L, 2);
	if (!lua_isstring(L, 3)) {
		lua_pushnil(L);
		return 1;
	}
	const char *descriptor = lua_tostring(L, 3);
	JavaCaller caller(env, L);
	if (caller.CallStaticMethod(javaClz,  methodName, descriptor, 4)) {
		return caller.PushReturn();
	}

	return 0;
}

int lua_GetField(lua_State * L)
{
	LuaGlobals globals(L);
	JNIEnv *env = globals.getJniEnv();

	jobject javaObject;
	JavaData javaData(L);
	if (javaData.GetDataType(1) != JavaTypeObject) {
		lua_pushnil(L);
		return 1;
	}

	javaObject = javaData.GetJavaObject(1);
	if (javaObject == NULL) {
		lua_pushnil(L);
		return 1;
	}

	if (!lua_isstring(L, 2)) {
		lua_pushnil(L);
		return 1;
	}
	const char* fieldName = lua_tostring(L, 2);
	if (!lua_isstring(L, 3)) {
		lua_pushnil(L);
		return 1;
	}
	const char *descriptor = lua_tostring(L, 3);
	JavaCaller caller(env, L);
	if (caller.GetField(javaObject, fieldName, descriptor)) {
		return caller.PushReturn();
	}

	return 0;
}

int lua_SetField(lua_State * L)
{
	LuaGlobals globals(L);
	JNIEnv *env = globals.getJniEnv();

	jobject javaObject;
	JavaData javaData(L);
	if (javaData.GetDataType(1) != JavaTypeObject) {
		lua_pushnil(L);
		return 1;
	}

	javaObject = javaData.GetJavaObject(1);
	if (javaObject == NULL) {
		lua_pushnil(L);
		return 1;
	}

	if (!lua_isstring(L, 2)) {
		lua_pushnil(L);
		return 1;
	}
	const char* fieldName = lua_tostring(L, 2);
	if (!lua_isstring(L, 3)) {
		lua_pushnil(L);
		return 1;
	}
	const char *descriptor = lua_tostring(L, 3);
	JavaCaller caller(env, L);
	if (caller.SetField(javaObject, fieldName, descriptor,4)) {
		lua_pushboolean(L, true);
	}
	lua_pushboolean(L, false);
	return 1;
}

int lua_GetStaticField(lua_State * L)
{
	LuaGlobals globals(L);
	JNIEnv *env = globals.getJniEnv();
	jclass javaClz = NULL;
	if (lua_isstring(L, 1)) {
		const char* clzName = lua_tostring(L, 1);
		javaClz = env->FindClass(clzName);
	}
	else if (lua_isuserdata(L, 1)) {
		JavaData javaData(L);
		if (javaData.GetDataType(1) != JavaTypeClass) {
			lua_pushnil(L);
			return 1;
		}
		javaClz = javaData.GetJavaClass(1);
	}
	
	if (0 != ExceptionCheckAndClear(env) || javaClz == NULL) {
		lua_pushnil(L);
		return 1;
	}

	if (!lua_isstring(L, 2)) {
		lua_pushnil(L);
		return 1;
	}
	const char* fieldName = lua_tostring(L, 2);
	if (!lua_isstring(L, 3)) {
		lua_pushnil(L);
		return 1;
	}
	const char *descriptor = lua_tostring(L, 3);
	JavaCaller caller(env, L);
	if (caller.GetStaticField(javaClz, fieldName, descriptor)) {
		return caller.PushReturn();
	}

	return 0;
}

int lua_SetStaticField(lua_State * L)
{
	LuaGlobals globals(L);
	JNIEnv *env = globals.getJniEnv();
	jclass javaClz = NULL;
	if (lua_isstring(L, 1)) {
		const char* clzName = lua_tostring(L, 1);
		javaClz = env->FindClass(clzName);
	}
	else if (lua_isuserdata(L, 1)) {
		JavaData javaData(L);
		if (javaData.GetDataType(1) != JavaTypeClass) {
			lua_pushnil(L);
			return 1;
		}
		javaClz = javaData.GetJavaClass(1);
	}

	if (0 != ExceptionCheckAndClear(env) || javaClz == NULL) {
		lua_pushnil(L);
		return 1;
	}

	if (!lua_isstring(L, 2)) {
		lua_pushnil(L);
		return 1;
	}
	const char* fieldName = lua_tostring(L, 2);
	if (!lua_isstring(L, 3)) {
		lua_pushnil(L);
		return 1;
	}
	const char *descriptor = lua_tostring(L, 3);
	JavaCaller caller(env, L);
	if (caller.SetStaticField(javaClz, fieldName, descriptor, 4)) {
		lua_pushboolean(L, true);
	}
	lua_pushboolean(L, false);
	return 1;
}


int lua_GetJavaObject(lua_State * L)
{
	if (!lua_isstring(L, 1)) {
		lua_pushnil(L);
		return 1;
	}

	const char* key = lua_tostring(L, 1);
	LuaGlobals globals(L);
	jobject obj = globals.getJavaObject(key);
	if (obj != NULL) {
		return JavaData::pushJavaObject(L,obj, NULL);
	}
	lua_pushnil(L);
	return 1;
}

int lua_PushLocalFrame(lua_State *L) {
    LuaGlobals globals(L);
    JNIEnv* env = globals.getJniEnv();
    int frame_cnt = 16;
    if(lua_isinteger(L,1)){
       frame_cnt = lua_tointeger(L,1);
    }
    env->PushLocalFrame(frame_cnt);
    return 0;
}

int lua_PopLocalFrame(lua_State *L) {
    LuaGlobals globals(L);
    JNIEnv* env = globals.getJniEnv();
    env->PopLocalFrame(NULL);
    return 0;
}

int lua_DeleteLocalRef(lua_State *L) {
    LuaGlobals globals(L);
    JNIEnv* env = globals.getJniEnv();
    JavaData javaData(L);
    switch (javaData.GetDataType(1)){
        case JavaTypeClass:
            env->DeleteLocalRef(javaData.GetJavaClass(1));
            break;
        case JavaTypeObject:
            env->DeleteLocalRef(javaData.GetJavaObject(1));
            break;
        case JavaTypeArray:
            env->DeleteLocalRef(javaData.GetJavaArray(1));
            break;
        default:
            break;
    }
    return 0;
}
