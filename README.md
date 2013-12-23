What is this?
=============

This is a module which implemented prototypes of **iconv** utilizing jni. You can resolve **iconv** dependency when you compile other modules written in C using Android NDK.

Motive
======

- I needed a library written in C to use in an Android app, and it came with **iconv** dependency.
- The library was 100KB and **iconv** was **900KB** per architecture.
- It meant the app will become 3.6MB at least without any feature if it just supports all architectures that Android NDK supports (armeabi, armeabi-v7a, mips, x86).
- I really wanted to reduce the size at least.
- Finally it became only 17KB and it worked well in my case.

Limitation
==========
- Only supports basic functions.
   - [iconv_open](http://www.gnu.org/savannah-checkouts/gnu/libiconv/documentation/libiconv-1.13/iconv_open.3.html)
   - [iconv](http://www.gnu.org/savannah-checkouts/gnu/libiconv/documentation/libiconv-1.13/iconv.3.html)
   - [iconv_close](http://www.gnu.org/savannah-checkouts/gnu/libiconv/documentation/libiconv-1.13/iconv_close.3.html)

Configuration
=============

- Copy all files and directories under **libs/** into _&lt;your-directory>_.
- Add this at the top of _Android.mk_.

```make
include $(CLEAR_VARS)

LOCAL_MODULE := iconv
LOCAL_SRC_FILES := <your-directory>/$(TARGET_ARCH_ABI)/libiconv.so

include $(PREBUILT_SHARED_LIBRARY)
```

- So it becomes like this
 
```make
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := iconv
LOCAL_SRC_FILES := <your-directory>/$(TARGET_ARCH_ABI)/libiconv.so

include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := your_module
LOCAL_SHARED_LIBRARIES := iconv
LOCAL_SRC_FILES := your_module.c

include $(BUILD_SHARED_LIBRARY)
```

Usage
=====

1. Copy **libs/** to your project.
2. Copy **AndroidIconvProxy.jar** into **libs/** folder of your project.
3. Insert this code into your main activity.

```java
static {
    iconv.Proxy.init();
}
```
