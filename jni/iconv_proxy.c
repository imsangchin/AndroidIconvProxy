#include <stdlib.h>
#include <errno.h>
#include <jni.h>

#include "iconv.h"

#define min(a, b) (a < b ? a : b)

JavaVM* jvm;

jclass ByteBuffer;

jclass Charset;
jmethodID Charset_forName;
jmethodID Charset_newDecoder;
jmethodID Charset_newEncoder;

jmethodID ByteBuffer_array;
jmethodID ByteBuffer_limit;
jmethodID ByteBuffer_get;

jclass CodingErrorAction;
jfieldID CodingErrorAction_REPORT;

jclass CharsetDecoder;
jmethodID CharsetDecoder_onMalformedInput;
jmethodID CharsetDecoder_onUnmappableCharacter;
jmethodID CharsetDecoder_decode;

jclass CharsetEncoder;
jmethodID CharsetEncoder_onMalformedInput;
jmethodID CharsetEncoder_onUnmappableCharacter;
jmethodID CharsetEncoder_encode;

void Java_iconv_Proxy_init(JNIEnv* env, jclass clazz)
{
    if (jvm != NULL) return;
    if ((*env)->GetJavaVM(env, &jvm) != 0) return;

    // Charset

    Charset = (*env)->FindClass(env, "java/nio/charset/Charset");
    Charset_forName = (*env)->GetStaticMethodID(env, Charset, "forName", "(Ljava/lang/String;)Ljava/nio/charset/Charset;");
    Charset_newDecoder = (*env)->GetMethodID(env, Charset, "newDecoder", "()Ljava/nio/charset/CharsetDecoder;");
    Charset_newEncoder = (*env)->GetMethodID(env, Charset, "newEncoder", "()Ljava/nio/charset/CharsetEncoder;");

    // ByteBuffer

    ByteBuffer = (*env)->FindClass(env, "java/nio/ByteBuffer");
    ByteBuffer_array = (*env)->GetMethodID(env, ByteBuffer, "array", "()[B");
    ByteBuffer_limit = (*env)->GetMethodID(env, ByteBuffer, "limit", "()I");
    ByteBuffer_get = (*env)->GetMethodID(env, ByteBuffer, "get", "(I)B");

    // CodingErrorAction

    CodingErrorAction = (*env)->FindClass(env, "java/nio/charset/CodingErrorAction");
    CodingErrorAction_REPORT = (*env)->GetStaticFieldID(env, CodingErrorAction, "REPORT", "Ljava/nio/charset/CodingErrorAction;");

    // CharsetDecoder

    CharsetDecoder = (*env)->FindClass(env, "java/nio/charset/CharsetDecoder");
    CharsetDecoder_onMalformedInput = (*env)->GetMethodID(env, CharsetDecoder, "onMalformedInput",
        "(Ljava/nio/charset/CodingErrorAction;)Ljava/nio/charset/CharsetDecoder;");
    CharsetDecoder_onUnmappableCharacter = (*env)->GetMethodID(env, CharsetDecoder, "onUnmappableCharacter",
        "(Ljava/nio/charset/CodingErrorAction;)Ljava/nio/charset/CharsetDecoder;");
    CharsetDecoder_decode = (*env)->GetMethodID(env, CharsetDecoder, "decode",
        "(Ljava/nio/ByteBuffer;)Ljava/nio/CharBuffer;");

    // CharsetEncoder

    CharsetEncoder = (*env)->FindClass(env, "java/nio/charset/CharsetEncoder");
    CharsetEncoder_onMalformedInput = (*env)->GetMethodID(env, CharsetEncoder, "onMalformedInput",
        "(Ljava/nio/charset/CodingErrorAction;)Ljava/nio/charset/CharsetEncoder;");
    CharsetEncoder_onUnmappableCharacter = (*env)->GetMethodID(env, CharsetEncoder, "onUnmappableCharacter",
        "(Ljava/nio/charset/CodingErrorAction;)Ljava/nio/charset/CharsetEncoder;");
    CharsetEncoder_encode = (*env)->GetMethodID(env, CharsetEncoder, "encode",
        "(Ljava/nio/CharBuffer;)Ljava/nio/ByteBuffer;");

    Charset = (*env)->NewGlobalRef(env, Charset);
    ByteBuffer = (*env)->NewGlobalRef(env, ByteBuffer);
    CodingErrorAction = (*env)->NewGlobalRef(env, CodingErrorAction);
    CharsetDecoder = (*env)->NewGlobalRef(env, CharsetDecoder);
    CharsetEncoder = (*env)->NewGlobalRef(env, CharsetEncoder);
}

/*jboolean Java_iconv_Proxy_test1(JNIEnv* env, jclass clazz)
{
    const char* str = "asdf";
    size_t slen = strlen(str);

    char outbuf[10];
    char* outptr = &outbuf[0];
    size_t outlen = 10;

    iconv_t latin1_cd = iconv_open("UTF-8","ISO8859-1");
    iconv(latin1_cd, &str, &slen, &outptr, &outlen);
    *outptr = 0;
    int cmp = strcmp(outbuf, "asdf");
    iconv_close(latin1_cd);

    return cmp == 0;
}*/

iconv_t libiconv_open(const char* tocode, const char* fromcode)
{
    return iconv_open(tocode, fromcode);
}

iconv_t iconv_open(const char* tocode, const char* fromcode)
{
    JNIEnv* env;
    if ((*jvm)->GetEnv(jvm, (void**)&env, JNI_VERSION_1_6) != JNI_OK) return NULL;

    jobjectArray a = (*env)->NewObjectArray(env, 2, Charset, NULL);

    jstring fromcodeString = (*env)->NewStringUTF(env, fromcode);
    jobject fromCharset = (*env)->CallStaticObjectMethod(env, Charset, Charset_forName, fromcodeString);
    (*env)->DeleteLocalRef(env, fromcodeString);
    
    jthrowable ex;
	ex = (*env)->ExceptionOccurred(env);
    if (ex != NULL)
    {
        (*env)->ExceptionClear(env);
        errno = EINVAL;
        return (iconv_t)-1;
    }
    
    jstring tocodeString = (*env)->NewStringUTF(env, tocode);
    jobject toCharset = (*env)->CallStaticObjectMethod(env, Charset, Charset_forName, tocodeString);
    (*env)->DeleteLocalRef(env, tocodeString);

	ex = (*env)->ExceptionOccurred(env);
    if (ex != NULL)
    {
        (*env)->ExceptionClear(env);
        errno = EINVAL;
        return (iconv_t)-1;
    }

    (*env)->SetObjectArrayElement(env, a, 0, fromCharset);
    (*env)->SetObjectArrayElement(env, a, 1, toCharset);

    return (*env)->NewGlobalRef(env, a);
}

size_t libiconv(iconv_t cd, const char** inbuf, size_t* inbytesleft, char** outbuf, size_t* outbytesleft)
{
    return iconv(cd, inbuf, inbytesleft, outbuf, outbytesleft);
}

size_t iconv(iconv_t cd, const char** inbuf, size_t* inbytesleft, char** outbuf, size_t* outbytesleft)
{
    JNIEnv* env;
    if ((*jvm)->GetEnv(jvm, (void**)&env, JNI_VERSION_1_6) != JNI_OK) return -1;

    if (inbuf == NULL || *inbuf == NULL || outbuf == NULL || *outbuf == NULL)
    {
        return (size_t)-1;
    }

    // Charset fromCharset = cd[0];
    // Charset toCharset = cd[1];

    jobjectArray a = (jobjectArray)cd;
    jobject fromCharset = (*env)->GetObjectArrayElement(env, a, 0);
    jobject toCharset = (*env)->GetObjectArrayElement(env, a, 1);

    // ByteBuffer byteBuffer = *inbuf;

    jobject byteBuffer = (*env)->NewDirectByteBuffer(env, (char*)*inbuf, *inbytesleft);

    // CharBuffer charBuffer;
    // try {
    //     charBuffer = fromCharset.newDecoder()
    //                             .onMalformedInput(CodingErrorAction.REPORT)
    //                             .onUnmappableCharacter(CodingErrorAction.REPORT)
    //                             .decode(byteBuffer);
    // } catch (Exception e) {
    //     errno = EINVAL;
    //     return -1;
    // }

    jobject decoder = (*env)->CallObjectMethod(env, fromCharset, Charset_newDecoder);
    jobject report = (*env)->GetStaticObjectField(env, CodingErrorAction, CodingErrorAction_REPORT);
    (*env)->CallObjectMethod(env, decoder, CharsetDecoder_onMalformedInput, report);
    (*env)->CallObjectMethod(env, decoder, CharsetDecoder_onUnmappableCharacter, report);
    jobject charBuffer = (*env)->CallObjectMethod(env, decoder, CharsetDecoder_decode, byteBuffer);

    jthrowable ex = (*env)->ExceptionOccurred(env);
    if (ex != NULL)
    {
        (*env)->ExceptionClear(env);
        errno = EINVAL;
        return -1;
    }

    (*env)->DeleteLocalRef(env, byteBuffer);
    (*env)->DeleteLocalRef(env, decoder);

    // try {
    //     byteBuffer = toCharset.newEncoder()
    //                           .onMalformedInput(CodingErrorAction.REPORT)
    //                           .onUnmappableCharacter(CodingErrorAction.REPORT)
    //                           .encode(charBuffer);
    // } catch (Exception e) {
    //     errno = EINVAL;
    //     return -1;
    // }

    jobject encoder = (*env)->CallObjectMethod(env, toCharset, Charset_newEncoder);
    (*env)->CallObjectMethod(env, encoder, CharsetEncoder_onMalformedInput, report);
    (*env)->CallObjectMethod(env, encoder, CharsetEncoder_onUnmappableCharacter, report);
    byteBuffer = (*env)->CallObjectMethod(env, encoder, CharsetEncoder_encode, charBuffer);

    ex = (*env)->ExceptionOccurred(env);
    if (ex != NULL)
    {
        (*env)->ExceptionClear(env);
        errno = EINVAL;
        return -1;
    }

    (*env)->DeleteLocalRef(env, charBuffer);

    //

    jint size = (*env)->CallIntMethod(env, byteBuffer, ByteBuffer_limit);
    size_t outSize = min(size, *outbytesleft);

    jbyteArray byteArray = (*env)->CallObjectMethod(env, byteBuffer, ByteBuffer_array);
    if ((*env)->ExceptionOccurred(env) == NULL)
    {
        jbyte* buffer = (*env)->GetByteArrayElements(env, byteArray, NULL);
        memcpy(*outbuf, buffer, outSize);
        (*env)->ReleaseByteArrayElements(env, byteArray, buffer, JNI_ABORT);
    }
    else
    {
        (*env)->ExceptionClear(env);

        int i;
        for (i = 0; i < size; ++i)
        {
            (*outbuf)[i] = (*env)->CallByteMethod(env, byteBuffer, ByteBuffer_get, i);
        }
    }

    //

    *inbuf += *inbytesleft;
    *inbytesleft = 0;
    *outbuf += outSize;
    *outbytesleft -= outSize;

    return size;
}

jint libiconv_close(iconv_t cd)
{
    return iconv_close(cd);
}

jint iconv_close(iconv_t cd)
{
    JNIEnv* env;
    if ((*jvm)->GetEnv(jvm, (void**)&env, JNI_VERSION_1_6) != JNI_OK) return -1;

    (*env)->DeleteGlobalRef(env, (jobjectArray)cd);
    return 0;
}
