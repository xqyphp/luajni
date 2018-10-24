#pragma once
#include "lua.hpp"
#include <jni.h>

void luajni_open(lua_State *L, JNIEnv *env);
void luajni_push_object(lua_State *L, const char* key, jobject obj);