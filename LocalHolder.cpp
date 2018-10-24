//
// Created by likai_m on 2018/8/31.
//

#include "LocalHolder.h"



LocalHolder::LocalHolder(JNIEnv *env):mJniEnv(env) {
}


LocalHolder::~LocalHolder() {
    for(auto it = objs.begin(); it != objs.end(); it++){
        mJniEnv->DeleteLocalRef(*it);
    }
}

void LocalHolder::pushLocal(jobject obj) {
    if(obj != nullptr){
        objs.push_back(obj);
    }
}




