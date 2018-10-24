# luajni
### 一、调用JNI的lua扩展，目前没有实现数组的操作，待以后实现。
### 二、c++接口 

 1. `void luajni_open(lua_State *L, JNIEnv *env);`
    注册扩展，会注册全局变量luajni，lua中可以直接调用里面的方法
                
 2. `void luajni_push_object(lua_State *L, const char* key, jobject obj);`
    存储JAVA对象到lua中，不会自动转换成全局引用，因此obj需要生成全局引用再调用,key为查询的索引，供lua层查找
### 三、数据类型说明：
1. java中string 对应 lua中string,会自动进行转换。
2. javaobject 和 javaclass 为自定义类型，jni局部引用，因为垃圾回收时机不确定，因此jni层的引用需要手动控制
3. 全局引用还没有实现，实现后会采用自动回收
### 四、lua接口 全部为 luajni下的属性方法：
1. `javaclass findClass(className:string);`
          成功返回 class局部引用，否则返回nil.
2. `javaobject newObject(class:javaclass|className:string,sig,...);`
          创建java对象，第一个参数可以传递javaclass或类名，第二个参数为签名，后面为传递的参数。失败返回nil.
3. `any callMethod(obj:javaobject,methodName:string,sig:string,...);`
          调用对象的方法
4. `any callStaticMethod(class:javaclass|className:string,methodName:string,sig:string,...);`
          调用类的方法
5. `any getField(obj:javaobject,fieldName:string,sig:string);`
          获取对象的属性
6. `bool setField(obj:javaobject,fieldName:string,sig:string,arg:any);`
          设置对象属性值，成功返回true
7. `any  getStaticField(class:javaclass|className:string,sig:string);`
          获取类的属性
8. `bool setStaticField(obj:javaobject,fieldName:string,sig:string,arg:any);`
          设置类属性值，成功返回true
9. `javaobjcet getJavaObject(key:string);`
          获取c++层调用luajni_push_object存储的对象
10. `void pushLocalFrame(size:int?);
    void popLocalFrame();
    void deleteLocalRef(ref:javaobject|javaclass);`
          同jni对局部变量的操作。
