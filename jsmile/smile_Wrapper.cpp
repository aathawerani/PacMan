// smile_Wrapper.cpp

#include <jni.h>

extern "C"
{

jfieldID FID_ptrNative = 0;

JNIEXPORT void JNICALL Java_smile_Wrapper_nativeStaticInit(JNIEnv *env, jclass cls)
{
    FID_ptrNative = env->GetFieldID(cls, "ptrNative", "J");
}

} // end extern "C"
