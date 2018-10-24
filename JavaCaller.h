#pragma once
#include <jni.h>
#include <lua.hpp>
#include "JavaSignature.h"
#include <vector>


class JavaCaller
{
public:
	JavaCaller(JNIEnv *env,lua_State *L);
	~JavaCaller();

	bool CallConstruct(jclass javaClz, const char* sig, int idx);
	bool CallMethod(jobject javaObject,const char* methodName,const char* sig,int idx);
	bool CallStaticMethod(jclass javaClz, const char* methodName, const char* sig, int idx);

	bool GetField(jobject javaObject, const char* fieldName,const char* sig);
	bool GetStaticField(jclass javaClz, const char* fieldName, const char* sig);

	bool SetField(jobject javaObject, const char* fieldName, const char* sig,int idx);
	bool SetStaticField(jclass javaClz, const char* fieldName, const char* sig,int idx);

	int PushReturn();

protected:
	bool InitCallArgs(const JavaSignature *pSig, int idx, std::vector<jvalue> *pArgsOut);
	//字符串有转化，需要释放
	bool ReleaseCallArgs(const JavaSignature *pSig,std::vector<jvalue> *pArgsOut);
	bool InitFieldSetArgs(const JavaSignature *pSig, int idx, jvalue *pArgOut);
	bool ReleaseFieldSetArgs(const JavaSignature *pSig, jvalue *pArgOut);
	static int ExceptionCheckAndClear(JNIEnv * env)
	{
		if (env->ExceptionOccurred()) {
			env->ExceptionClear();
			return -1;
		}
		return 0;
	}

private:
	JNIEnv *mEnv;
	lua_State *mLuaState;
	jvalue mRetValue;
	JavaSignature mJavaSig;
	bool mConstruct;
};

