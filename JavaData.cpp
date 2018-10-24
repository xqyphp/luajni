#include "JavaData.h"
#include "LuaGlobals.h"

#define LUA_JAVA_DATA_TYPE "__lua_java_data_type"

#define LUA_JAVA_CLASS_NAME "__lua_java_class_name"

/* Garbage collector metamethod name */
#define LUA_GC_METAMETHOD_TAG    "__gc"

static jobject addRef(JNIEnv* env,jobject obj)
{
	return obj;
}

static void defRef(JNIEnv* env,jobject obj)
{
	//env->DeleteLocalRef(obj);
}

JavaData::JavaData(lua_State * L)
	:mL(L)
{
}

JavaData::~JavaData()
{
}

int JavaData::pushJavaObject(jobject javaObject, const char * clzName)
{
	return pushJavaObject(mL, javaObject, clzName);
}

int JavaData::pushJavaClass(jclass clz, const char * clzName)
{
	return pushJavaClass(mL, clz, clzName);
}

int JavaData::pushJavaArray(jarray array)
{
	return pushJavaArray(mL, array);
}

int JavaData::pushJavaObject(lua_State * L, jobject javaObject, const char * clzName)
{
	LuaGlobals luaGlobals(L);
	jobject *userData, globalRef;

	/* Gets the JNI Environment */
	JNIEnv *javaEnv = luaGlobals.getJniEnv();
	if (javaEnv == NULL) {
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}

	globalRef = addRef(javaEnv,javaObject);
	userData = (jobject *)lua_newuserdata(L, sizeof(jobject));
	*userData = globalRef;

	/* Creates metatable */
	lua_newtable(L);

	lua_pushstring(L, LUA_JAVA_DATA_TYPE);
	lua_pushinteger(L, JavaTypeObject);
	lua_rawset(L, -3);

	/* pushes the __gc metamethod */
	lua_pushstring(L, LUA_GC_METAMETHOD_TAG);
	lua_pushcfunction(L, &gc);
	lua_rawset(L, -3);


	if (clzName != NULL) {
		lua_pushstring(L, LUA_JAVA_CLASS_NAME);
		lua_pushstring(L, clzName);
		lua_rawset(L, -3);
	}

	if (lua_setmetatable(L, -2) == 0) {
		defRef(javaEnv,globalRef);
		lua_pushstring(L, "Cannot create proxy to java object.");
		lua_error(L);
	}

	return 1;
}

int JavaData::pushJavaString(lua_State * L, jobject obj)
{
	LuaGlobals luaGlobals(L);
	JNIEnv *env = luaGlobals.getJniEnv();

	jstring jStr = static_cast<jstring>(obj);
	const char* text = env->GetStringUTFChars(jStr, NULL);
	if (text == NULL) {
		lua_pushnil(L);
	}
	lua_pushstring(L, text);
	env->ReleaseStringUTFChars(jStr, text);
    env->DeleteLocalRef(obj);
	return 1;
}

int JavaData::pushJavaClass(lua_State * L, jclass clz, const char * clzName)
{
	LuaGlobals luaGlobals(L);
	jobject *userData, globalRef;

	/* Gets the JNI Environment */
	JNIEnv *javaEnv = luaGlobals.getJniEnv();
	if (javaEnv == NULL) {
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}


	globalRef = addRef(javaEnv,clz);
	userData = (jobject *)lua_newuserdata(L, sizeof(jobject));
	*userData = globalRef;

	/* Creates metatable */
	lua_newtable(L);

	lua_pushstring(L, LUA_JAVA_DATA_TYPE);
	lua_pushinteger(L, JavaTypeClass);
	lua_rawset(L, -3);

	/* pushes the __gc metamethod */
	lua_pushstring(L, LUA_GC_METAMETHOD_TAG);
	lua_pushcfunction(L, &gc);
	lua_rawset(L, -3);

	if (clzName != NULL) {
		lua_pushstring(L, LUA_JAVA_CLASS_NAME);
		lua_pushstring(L, clzName);
		lua_rawset(L, -3);
	}


	if (lua_setmetatable(L, -2) == 0) {
		defRef(javaEnv,globalRef);
		lua_pushstring(L, "Cannot create proxy to java object.");
		lua_error(L);
	}

	return 1;
}

int JavaData::pushJavaArray(lua_State * L, jarray array)
{
	LuaGlobals luaGlobals(L);
	jobject *userData, globalRef;

	/* Gets the JNI Environment */
	JNIEnv *javaEnv = luaGlobals.getJniEnv();
	if (javaEnv == NULL) {
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}

	globalRef = addRef(javaEnv,array);
	userData = (jobject *)lua_newuserdata(L, sizeof(jobject));
	*userData = globalRef;

	/* Creates metatable */
	lua_newtable(L);

	lua_pushstring(L, LUA_JAVA_DATA_TYPE);
	lua_pushinteger(L, JavaTypeArray);
	lua_rawset(L, -3);

	/* pushes the __gc metamethod */
	lua_pushstring(L, LUA_GC_METAMETHOD_TAG);
	lua_pushcfunction(L, &gc);
	lua_rawset(L, -3);


	if (lua_setmetatable(L, -2) == 0) {
		defRef(javaEnv,globalRef);
		lua_pushstring(L, "Cannot create proxy to java object.");
		lua_error(L);
	}

	return 1;
}

jobject JavaData::GetJavaObject(int idx)
{
	void* ud =  GetUserData(idx);
	if (ud == NULL) {
		return NULL;
	}

	return *(jobject*)ud;
}

jclass JavaData::GetJavaClass(int idx)
{
	void* ud = GetUserData(idx);
	if (ud == NULL) {
		return NULL;
	}

	return *(jclass*)ud;
}

jarray JavaData::GetJavaArray(int idx)
{
	void* ud = GetUserData(idx);
	if (ud == NULL) {
		return NULL;
	}

	return *(jarray*)ud;
}

JavaDataType JavaData::GetDataType(int idx)
{
	return GetDataType(mL, idx);
}

void * JavaData::GetUserData(int idx)
{
	lua_State *L = mL;
	if (!lua_isuserdata(L,idx)) {
		return nullptr;
	}
	return lua_touserdata(L, 1);
}

JavaDataType JavaData::GetDataType(lua_State * L, int idx)
{
	if (!lua_isuserdata(L, idx))
		return JavaTypeInvalid;

	if (lua_getmetatable(L, idx) == 0)
		return JavaTypeInvalid;

	lua_pushstring(L, LUA_JAVA_DATA_TYPE);
	lua_rawget(L, -2);

	if (!lua_isinteger(L, -1)) {
		lua_pop(L, 2);
		return JavaTypeInvalid;
	}

	JavaDataType ret = (JavaDataType)lua_tointeger(L, -1);

	lua_pop(L, 2);
	return ret;
}

int JavaData::gc(lua_State * L)
{
	jobject *pObj;
	JNIEnv *javaEnv;

	if (GetDataType(L, 1) == JavaTypeInvalid) {
		return 0;
	}

	pObj = (jobject *)lua_touserdata(L, 1);
	LuaGlobals luaGlobals(L);
	/* Gets the JNI Environment */
	javaEnv = luaGlobals.getJniEnv();
	if (javaEnv == NULL) {
		lua_pushstring(L, "Invalid JNI Environment.");
		lua_error(L);
	}
	defRef(javaEnv,*pObj);

	return 0;
}
