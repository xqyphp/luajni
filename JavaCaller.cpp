#include "JavaCaller.h"
#include "JavaData.h"
#include "LuaGlobals.h"
#include"java_switch_list.h"
#include "LocalHolder.h"

JavaCaller::JavaCaller(JNIEnv * env, lua_State * L)
		:mEnv(env), mLuaState(L),mConstruct(false)
{
}

JavaCaller::~JavaCaller()
{
}

int my_lua_pushjavastring(lua_State *L, jobject obj)
{
    int nRet = JavaData::pushJavaString(L, obj);
    return nRet;
}
int my_lua_pushjavaobject(lua_State *L, jobject obj)
{
	return JavaData::pushJavaObject(L, obj, NULL);
}

jobject my_lua_tojavastring(lua_State *L, int idx)
{
	LuaGlobals luaGlobals(L);
	JNIEnv *env = luaGlobals.getJniEnv();
	const char* name = lua_tostring(L, idx);
	return  env->NewStringUTF(name);
}

jobject my_lua_tojavaobject(lua_State *L, int idx)
{
	if (lua_isstring(L, idx)) {
		return  my_lua_tojavastring(L, idx);
	}
	jobject *obj = (jobject *)lua_touserdata(L, idx);
	return  *obj;
}

bool JavaCaller::CallConstruct(jclass javaClz, const char * sig, int idx)
{
	JNIEnv *env = mEnv;

	if (!mJavaSig.ParseSignature(sig)) {
		return false;
	}

	jmethodID methodId = env->GetMethodID(javaClz, "<init>", sig);
	if (ExceptionCheckAndClear(env) || methodId == NULL) {
		return false;
	}

	std::vector<jvalue> args;
	if (!InitCallArgs(&mJavaSig, idx, &args)) {
		ReleaseCallArgs(&mJavaSig,&args);
		return false;
	}

	if (mJavaSig.GetReturnType() != TypeVoid) {
		ReleaseCallArgs(&mJavaSig,&args);
		return false;
	}

	jvalue* jargs = args.size() == 0 ? NULL : &args[0];
	mRetValue.l = env->NewObjectA(javaClz, methodId, jargs);
	if (ExceptionCheckAndClear(env)) {
		ReleaseCallArgs(&mJavaSig,&args);
		return false;
	}
	ReleaseCallArgs(&mJavaSig,&args);
	mConstruct = true;
	return true;
}

bool JavaCaller::CallMethod(jobject javaObject, const char* methodName, const char * sig, int idx)
{
	JNIEnv *env = mEnv;

	if (!mJavaSig.ParseSignature(sig)) {
		return false;
	}

	LocalHolder localHolder(env);
	jclass javaClz = env->GetObjectClass(javaObject);
	localHolder.pushLocal(javaClz);

	if (ExceptionCheckAndClear(env) || javaClz == NULL) {
		return false;
	}
	jmethodID methodId = env->GetMethodID(javaClz, methodName, sig);
	if (ExceptionCheckAndClear(env) || methodId == NULL) {
		return false;
	}
	std::vector<jvalue> args;
	if (!InitCallArgs(&mJavaSig, idx, &args)) {
		ReleaseCallArgs(&mJavaSig,&args);
		return false;
	}

	jvalue* jargs = args.size() == 0 ? NULL :&args[0];

	switch (mJavaSig.GetReturnType())
	{
		case TypeVoid:
			env->CallVoidMethodA(javaObject, methodId, jargs);
			break;

#define JAVA_DO_IT(t,c,v,i,p,g) \
		case Type##t:   \
        mRetValue.v= env->Call##c##MethodA(javaObject, methodId, jargs);\
        break;


		JAVA_DO_LIST
#undef JAVA_DO_IT

		default:
			ReleaseCallArgs(&mJavaSig,&args);
			return false;
	}
	if (ExceptionCheckAndClear(env)) {
		ReleaseCallArgs(&mJavaSig,&args);
		return false;
	}
	ReleaseCallArgs(&mJavaSig,&args);
	return true;
}

bool JavaCaller::CallStaticMethod(jclass javaClz, const char * methodName, const char * sig, int idx)
{
	JNIEnv *env = mEnv;

	if (!mJavaSig.ParseSignature(sig)) {
		return false;
	}

	jmethodID methodId = env->GetStaticMethodID(javaClz, methodName, sig);

	if (ExceptionCheckAndClear(env) || methodId == NULL) {
		return false;
	}

	std::vector<jvalue> args;
	if (!InitCallArgs(&mJavaSig, idx, &args)) {
		ReleaseCallArgs(&mJavaSig,&args);
		return false;
	}

	jvalue* jargs = args.size() == 0 ? NULL : &args[0];

	switch (mJavaSig.GetReturnType())
	{
		case TypeVoid:
			env->CallStaticVoidMethodA(javaClz, methodId,jargs);


#define JAVA_DO_IT(t,c,v,i,p,g) \
		case Type##t:   \
		mRetValue.v = env->CallStatic##c##MethodA(javaClz, methodId, jargs);\
		break;

		JAVA_DO_LIST
#undef JAVA_DO_IT


		default:
			ReleaseCallArgs(&mJavaSig,&args);
			return false;
	}

	if (ExceptionCheckAndClear(env)) {
		ReleaseCallArgs(&mJavaSig,&args);
		return false;
	}
	ReleaseCallArgs(&mJavaSig,&args);
	return true;
}

bool JavaCaller::GetField(jobject javaObject, const char * fieldName, const char* sig)
{
	JNIEnv *env = mEnv;

	LocalHolder localHolder(env);

	if (!mJavaSig.ParseSignature(sig) || !mJavaSig.IsField()) {
		return false;
	}

	jclass javaClz = env->GetObjectClass(javaObject);
	localHolder.pushLocal(javaClz);
	if (ExceptionCheckAndClear(env) || javaClz == NULL) {
		return false;
	}


	jfieldID fieldId = env->GetFieldID(javaClz, fieldName, sig);

	if (ExceptionCheckAndClear(env) || fieldId == NULL) {
		return false;
	}

	switch (mJavaSig.GetReturnType())
	{

#define JAVA_DO_IT(t,c,v,i,p,g) \
		case Type##t:   \
		mRetValue.v = env->Get##c##Field(javaObject, fieldId);\
		break;

		JAVA_DO_LIST

#undef JAVA_DO_IT

		default:
			return false;
	}
	if (ExceptionCheckAndClear(env)) {
		return false;
	}
	return true;
}

bool JavaCaller::GetStaticField(jclass javaClz, const char * fieldName, const char * sig)
{

	JNIEnv *env = mEnv;



	if (!mJavaSig.ParseSignature(sig) || !mJavaSig.IsField()) {
		return false;
	}

	LocalHolder localHolder(env);

	if (ExceptionCheckAndClear(env) || javaClz == NULL) {
		return false;
	}
	localHolder.pushLocal(javaClz);

	jfieldID fieldId = env->GetStaticFieldID(javaClz, fieldName, sig);

	if (ExceptionCheckAndClear(env) || fieldId == NULL) {
		return false;
	}

	switch (mJavaSig.GetReturnType())
	{

#define JAVA_DO_IT(t,c,v,i,p,g) \
		case Type##t:   \
		mRetValue.v = env->GetStatic##c##Field(javaClz, fieldId);\
		break;

		JAVA_DO_LIST

#undef JAVA_DO_IT

		default:
			return false;
	}
	if (ExceptionCheckAndClear(env)) {
		return false;
	}
	return true;
}


bool JavaCaller::SetField(jobject javaObject, const char * fieldName, const char * sig, int idx)
{
	JNIEnv *env = mEnv;


	if (!mJavaSig.ParseSignature(sig)) {
		return false;
	}

	LocalHolder localHolder(env);

	jclass javaClz = env->GetObjectClass(javaObject);
	localHolder.pushLocal(javaClz);

	if (ExceptionCheckAndClear(env) || javaClz == NULL) {
		return false;
	}

	jfieldID fieldId = env->GetFieldID(javaClz, fieldName, sig);
	if (ExceptionCheckAndClear(env) || fieldId == NULL) {
		return false;
	}

	jvalue farg;

	if (!InitFieldSetArgs(&mJavaSig, idx, &farg)) {
		ReleaseFieldSetArgs(&mJavaSig,&farg);
		return false;
	}

	switch (mJavaSig.GetReturnType())
	{

#define JAVA_DO_IT(t,c,v,i,p,g) \
		case Type##t:   \
		env->Set##c##Field(javaObject, fieldId,farg.v);\
		break;

		JAVA_DO_LIST

#undef JAVA_DO_IT

		default:
			ReleaseFieldSetArgs(&mJavaSig,&farg);
			return false;
	}
	if (ExceptionCheckAndClear(env)) {
		ReleaseFieldSetArgs(&mJavaSig,&farg);
		return false;
	}
	ReleaseFieldSetArgs(&mJavaSig,&farg);
	return true;
}

bool JavaCaller::SetStaticField(jclass javaClz, const char * fieldName, const char * sig, int idx)
{
	JNIEnv *env = mEnv;
	//lua_State *L = mLuaState;


	if (!mJavaSig.ParseSignature(sig)) {
		return false;
	}


	jfieldID fieldId = env->GetFieldID(javaClz, fieldName, sig);
	if (ExceptionCheckAndClear(env) || fieldId == NULL) {
		return false;
	}

	jvalue farg;

	if (!InitFieldSetArgs(&mJavaSig, idx, &farg)) {
		ReleaseFieldSetArgs(&mJavaSig,&farg);
		return false;
	}

	switch (mJavaSig.GetReturnType())
	{

#define JAVA_DO_IT(t,c,v,i,p,g) \
		case Type##t:   \
		env->SetStatic##c##Field(javaClz, fieldId,farg.v);\
		break;

		JAVA_DO_LIST

#undef JAVA_DO_IT

		default:
			ReleaseFieldSetArgs(&mJavaSig,&farg);
			return false;
	}
	if (ExceptionCheckAndClear(env)) {
		ReleaseFieldSetArgs(&mJavaSig,&farg);
		return false;
	}
	ReleaseFieldSetArgs(&mJavaSig,&farg);
	return true;
}

int JavaCaller::PushReturn()
{
	lua_State * L = mLuaState;

	if (mConstruct) {
		return JavaData::pushJavaObject(L,mRetValue.l, NULL);
	}

	switch (mJavaSig.GetLuaReturnType())
	{
		case TypeVoid:
			return 0;

#define JAVA_DO_IT(t,c,v,i,p,g) \
		case Type##t:   \
		p(L, mRetValue.v);\
		return 1;

		JAVA_DO_LIST

#undef JAVA_DO_IT


		default:
			break;
	}

	lua_pushnil(L);
	return 1;
}

bool JavaCaller::InitCallArgs(const JavaSignature * pSig, int idx, std::vector<jvalue>* pArgsOut)
{
	//JNIEnv *env = mEnv;
	lua_State *L = mLuaState;
	int n = lua_gettop(L);
	size_t nArgs = pSig->GetArgCnt();

	//检测参数个数是否一致
	if (nArgs != n + 1 - idx) {
		return false;
	}

	jvalue tmpValue;
	//填充参数 TODO 类型检测
	for (int index = 0; index < nArgs; index++) {
		//int lua_index = index + idx;
		switch (pSig->GetArgTypes()[index]) {

#define JAVA_DO_IT(t,c,v,i,p,g) \
		case Type##t:   \
		tmpValue.v = g(L,idx);\
		break;

        JAVA_DO_LIST

#undef JAVA_DO_IT

			default:
				break;
		}

		pArgsOut->push_back(tmpValue);
	}

	lua_settop(L, n);
	return pArgsOut->size() == nArgs;
}

bool JavaCaller::InitFieldSetArgs(const JavaSignature * pSig, int idx, jvalue * pArgOut)
{
	//JNIEnv *env = mEnv;
	lua_State *L = mLuaState;
	if (!pSig->IsField()) {
		return false;
	}
	int n = lua_gettop(L);

	//检测参数个数是否一致
	if (1 != n + 1 - idx) {
		return false;
	}

	switch (pSig->GetReturnType())
	{

#define JAVA_DO_IT(t,c,v,i,p,g) \
		case Type##t:   \
		pArgOut->v = g(L,idx);\
		return 1;

		JAVA_DO_LIST
#undef JAVA_DO_IT

		default:
			return false;
	}
}

bool JavaCaller::ReleaseCallArgs(const JavaSignature *pSig, std::vector<jvalue> *pArgsOut) {

	if(!pSig->IsValid() || pSig->IsField() ){
		return false;
	}

	size_t count = pSig->GetArgCnt();
	if(count != pArgsOut->size()){
		return false;
	}
	std::vector<JavaValueType> types = pSig->GetArgTypes();
	for(size_t i = 0; i < count;i++){
		if(types[i] == TypeString){
			mEnv->DeleteLocalRef((*pArgsOut)[i].l);
		}
	}

	return false;
}

bool JavaCaller::ReleaseFieldSetArgs(const JavaSignature *pSig, jvalue *pArgOut) {
	if(!pSig->IsValid() || !pSig->IsField() || pArgOut == nullptr){
		return false;
	}

	if(pSig->GetReturnType() == TypeString){
		mEnv->DeleteLocalRef(pArgOut->l);
	}
	return false;
}
