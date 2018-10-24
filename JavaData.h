#pragma once
#include "lua.hpp"
#include "jni.h"

typedef enum {
	JavaTypeClass,
	JavaTypeObject,
	JavaTypeArray,
	JavaTypeInvalid
}JavaDataType;

class JavaData
{
public:
	JavaData(lua_State *L);
	~JavaData();

	int pushJavaObject(jobject obj,const char* clzName);
	int pushJavaClass(jclass clz,const char* clzName);
	int pushJavaArray(jarray array);

	static int pushJavaObject(lua_State *L,jobject obj, const char* clzName);
	//转化后会删除 obj 引用
	static int pushJavaString(lua_State *L, jobject obj);
	static int pushJavaClass(lua_State *L, jclass clz, const char* clzName);
	static int pushJavaArray(lua_State *L, jarray array);
	

	jobject GetJavaObject(int idx);
	jclass  GetJavaClass(int idx);
	jarray  GetJavaArray(int idx);

	JavaDataType GetDataType(int idx);
	static JavaDataType GetDataType(lua_State* L, int idx);
	static int gc(lua_State *L);
protected:
	
	void *GetUserData(int idx);
private:
	lua_State *mL;
};

