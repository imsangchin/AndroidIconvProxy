#include <stdlib.h>
#include <jni.h>

JNIEnv* gEnv;

jclass class_Charset;
jmethodID method_Charset_forName;

void Java_iconv_Proxy_init(JNIEnv* env, jclass clazz)
{
    gEnv = env;
    
    class_Charset = (*env)->FindClass(env, "java/nio/charset/Charset");
    method_Charset_forName = (*env)->GetStaticMethodID(env, class_Charset, "forName", "(Ljava/lang/String;)Ljava/nio/charset/Charset;");
}

void* iconv_open(const char* tocode, const char* fromcode)
{
    jobjectArray a = (*gEnv)->NewObjectArray(gEnv, 2, class_Charset, NULL);

    jstring fromcodeString = (*gEnv)->NewStringUTF(gEnv, fromcode);
    jobject fromCharset = (*gEnv)->CallStaticObjectMethod(gEnv, class_Charset, method_Charset_forName, fromcodeString);
    (*gEnv)->DeleteLocalRef(gEnv, fromcodeString);
    
    
    
    jstring tocodeString = (*gEnv)->NewStringUTF(gEnv, tocode);
    jobject toCharset = (*gEnv)->CallStaticObjectMethod(gEnv, class_Carset, method_Charset_forName, tocodeString);
    (*gEnv)->DeleteLocalRef(gEnv, tocodeString);
}
