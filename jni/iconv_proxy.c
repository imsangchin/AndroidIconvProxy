#include <stdlib.h>
#include <errno.h>
#include <jni.h>

#define min(a, b) (a < b ? a : b)

typedef void* iconv_t;

JNIEnv* gEnv;

jclass class_Charset;
jclass class_ByteBuffer;

jmethodID method_Charset_forName;
jmethodID method_Charset_encode;
jmethodID method_Charset_decode;
jmethodID method_ByteBuffer_wrap;
jmethodID method_ByteBuffer_array;
jmethodID method_ByteBuffer_limit;

void Java_iconv_Proxy_init(JNIEnv* env, jclass clazz)
{
    gEnv = env;
    
    class_Charset = (*env)->FindClass(env, "java/nio/charset/Charset");
    class_ByteBuffer = (*env)->FindClass(env, "java/nio/ByteBuffer");

    method_Charset_forName = (*env)->GetStaticMethodID(env, class_Charset, "forName", "(Ljava/lang/String;)Ljava/nio/charset/Charset;");
    method_Charset_encode = (*env)->GetMethodID(env, class_Charset, "encode", "(Ljava/nio/CharBuffer;)Ljava/nio/ByteBuffer;");
    method_Charset_encode = (*env)->GetMethodID(env, class_Charset, "decode", "(Ljava/nio/ByteBuffer;)Ljava/nio/CharBuffer;");

    method_ByteBuffer_wrap = (*env)->GetStaticMethodID(env, class_ByteBuffer, "wrap", "([B)Ljava/nio/ByteBuffer;");
    method_ByteBuffer_array = (*env)->GetMethodID(env, class_ByteBuffer, "array", "()[B");
    method_ByteBuffer_limit = (*env)->GetMethodID(env, class_ByteBuffer, "array", "()I");
}

iconv_t libiconv_open(const char* tocode, const char* fromcode)
{
    jobjectArray a = (*gEnv)->NewObjectArray(gEnv, 2, class_Charset, NULL);

    jstring fromcodeString = (*gEnv)->NewStringUTF(gEnv, fromcode);
    jobject fromCharset = (*gEnv)->CallStaticObjectMethod(gEnv, class_Charset, method_Charset_forName, fromcodeString);
    (*gEnv)->DeleteLocalRef(gEnv, fromcodeString);
    
    jthrowable ex;
	ex = (*gEnv)->ExceptionOccurred(gEnv);
    if (ex != NULL)
    {
        errno = EINVAL;
        return (iconv_t)-1;
    }
    
    jstring tocodeString = (*gEnv)->NewStringUTF(gEnv, tocode);
    jobject toCharset = (*gEnv)->CallStaticObjectMethod(gEnv, class_Charset, method_Charset_forName, tocodeString);
    (*gEnv)->DeleteLocalRef(gEnv, tocodeString);

	ex = (*gEnv)->ExceptionOccurred(gEnv);
    if (ex != NULL)
    {
        errno = EINVAL;
        return (iconv_t)-1;
    }

    (*gEnv)->SetObjectArrayElement(gEnv, a, 0, fromCharset);
    (*gEnv)->SetObjectArrayElement(gEnv, a, 1, toCharset);

    return (*gEnv)->NewGlobalRef(gEnv, a);
}

size_t libiconv(iconv_t cd, const char** inbuf, size_t* inbytesleft, char** outbuf, size_t* outbytesleft)
{
    if (inbuf == NULL || *inbuf == NULL || outbuf == NULL || *outbuf == NULL)
    {
        return (size_t)-1;
    }

    jobjectArray a = (jobjectArray)cd;
    jobject fromCharset = (*gEnv)->GetObjectArrayElement(gEnv, a, 0);
    jobject toCharset = (*gEnv)->GetObjectArrayElement(gEnv, a, 1);

    jbyteArray byteArray = (*gEnv)->NewByteArray(gEnv, *inbytesleft);
    jbyte* bytes = (*gEnv)->GetByteArrayElements(gEnv, byteArray, NULL);
    memcpy(bytes, *inbuf, *inbytesleft);
    (*gEnv)->ReleaseByteArrayElements(gEnv, byteArray, bytes, 0);

    jobject byteBuffer = (*gEnv)->CallStaticObjectMethod(gEnv, class_ByteBuffer, method_ByteBuffer_wrap, byteArray);
    jobject charBuffer = (*gEnv)->CallObjectMethod(gEnv, fromCharset, method_Charset_decode, byteBuffer);
    byteBuffer = (*gEnv)->CallObjectMethod(gEnv, toCharset, method_Charset_encode, charBuffer);

    jint size = (*gEnv)->CallIntMethod(gEnv, byteBuffer, method_ByteBuffer_limit);
    size_t outSize = min(size, *outbytesleft);

    byteArray = (*gEnv)->CallObjectMethod(gEnv, byteBuffer, method_ByteBuffer_array);
    bytes = (*gEnv)->GetByteArrayElements(gEnv, byteArray, NULL);
    memcpy(*outbuf, bytes, outSize);
    (*gEnv)->ReleaseByteArrayElements(gEnv, byteArray, bytes, 0);

    *inbuf += size;
    *inbytesleft -= size;
    *outbuf += size;
    *outbytesleft -= size;

    return size;
}

jint libiconv_close(iconv_t cd)
{
    (*gEnv)->DeleteGlobalRef(gEnv, (jobjectArray)cd);
    return 0;
}