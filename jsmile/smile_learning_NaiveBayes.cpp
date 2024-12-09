// smile_learning_NaiveBayes.cpp

#include "jsmile.h"

extern "C"
{
JSMILE_IMPLEMENT_WRAPPER(learning_NaiveBayes, DSL_nb);

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_learning_NaiveBayes_setNetWeight(JNIEnv *env, jobject obj, jdouble netWeight)
{
    JSMILE_ENTER;
    JSMILE_LEAVE;
}

JNIEXPORT double JNICALL Java_smile_learning_NaiveBayes_getNetWeight(JNIEnv *env, jobject obj)
{
    JSMILE_ENTER;
    JSMILE_RETURN(0);
}

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_learning_NaiveBayes_setPriorsMethod(JNIEnv *env, jobject obj, int priorsMethod)
{
    JSMILE_ENTER;
    JSMILE_LEAVE;
}

JNIEXPORT int JNICALL Java_smile_learning_NaiveBayes_getPriorsMethod(JNIEnv *env, jobject obj)
{
    JSMILE_ENTER;
    JSMILE_RETURN(0);
}

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_learning_NaiveBayes_setClassVariableId(JNIEnv *env, jobject obj, jstring classVariableId)
{
    JSMILE_ENTER;
    GetPtr(env, obj)->classVariableId = NativeString(env, classVariableId);
    JSMILE_LEAVE;
}

JNIEXPORT jstring JNICALL Java_smile_learning_NaiveBayes_getClassVariableId(JNIEnv *env, jobject obj)
{
    JSMILE_ENTER;
    JSMILE_RETURN(env->NewStringUTF(GetPtr(env, obj)->classVariableId.c_str()));
}

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_learning_NaiveBayes_setFeatureSelection(JNIEnv *env, jobject obj, jboolean featureSelection)
{
    JSMILE_ENTER;
    JSMILE_LEAVE;
}

JNIEXPORT jboolean JNICALL Java_smile_learning_NaiveBayes_getFeatureSelection(JNIEnv *env, jobject obj)
{
    JSMILE_ENTER;
    JSMILE_RETURN(false);
}

//-------------------------------------------------------------------------------

JNIEXPORT jobject JNICALL Java_smile_learning_NaiveBayes_learn(JNIEnv *env, jobject obj, jobject jDataset)
{
    JSMILE_ENTER;
    JSMILE_RETURN(Learn(GetPtr(env, obj), env, NULL, jDataset));
}

} // end extern "C"
