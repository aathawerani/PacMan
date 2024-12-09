// smile_learning_TAN.cpp

#include "jsmile.h"

extern "C"
{
JSMILE_IMPLEMENT_WRAPPER(learning_TAN, DSL_tan);

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_learning_TAN_setMaxSearchTime(JNIEnv *env, jobject obj, jint searchTime)
{
    JSMILE_ENTER;
    GetPtr(env, obj)->maxSearchTime = searchTime;
    JSMILE_LEAVE;
}

JNIEXPORT int JNICALL Java_smile_learning_TAN_getMaxSearchTime(JNIEnv *env, jobject obj)
{
    JSMILE_ENTER;
    JSMILE_RETURN(GetPtr(env, obj)->maxSearchTime);
}

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_learning_TAN_setRandSeed(JNIEnv *env, jobject obj, jint seed)
{
    JSMILE_ENTER;
    GetPtr(env, obj)->seed = seed;
    JSMILE_LEAVE;
}

JNIEXPORT int JNICALL Java_smile_learning_TAN_getRandSeed(JNIEnv *env, jobject obj)
{
    JSMILE_ENTER;
    JSMILE_RETURN(GetPtr(env, obj)->seed);
}

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_learning_TAN_setClassVariableId(JNIEnv *env, jobject obj, jstring classVariableId)
{
    JSMILE_ENTER;
    GetPtr(env, obj)->classvar = NativeString(env, classVariableId);
    JSMILE_LEAVE;
}

JNIEXPORT jstring JNICALL Java_smile_learning_TAN_getClassVariableId(JNIEnv *env, jobject obj)
{
    JSMILE_ENTER;
    JSMILE_RETURN(env->NewStringUTF(GetPtr(env, obj)->classvar.c_str()));
}

//-------------------------------------------------------------------------------


JNIEXPORT jobject JNICALL Java_smile_learning_TAN_learn(JNIEnv *env, jobject obj, jobject jDataset) 
{
    JSMILE_ENTER;
    JSMILE_RETURN(Learn(GetPtr(env, obj), env, NULL, jDataset));
}

} // end extern "C"
