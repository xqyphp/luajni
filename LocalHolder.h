//
// Created by likai_m on 2018/8/31.
//

#ifndef CYAP_ALL_LOCAL_HOLDER_H
#define CYAP_ALL_LOCAL_HOLDER_H

#include <vector>
#include <jni.h>

class LocalHolder {

public:
    LocalHolder(JNIEnv *env);
    void pushLocal(jobject obj);
    ~LocalHolder();
private:
    JNIEnv *mJniEnv;
    LocalHolder(const LocalHolder&);
    void operator=(const LocalHolder&);
    std::vector<jobject> objs;
};


#endif //CYAP_ALL_JAVAHOLDER_H
