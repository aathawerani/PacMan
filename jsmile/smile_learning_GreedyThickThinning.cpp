// smile_learning_GreedyThickThinning.cpp

#include "jsmile.h"

extern "C"
{
JSMILE_IMPLEMENT_WRAPPER(learning_GreedyThickThinning, DSL_greedyThickThinning);

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_learning_GreedyThickThinning_setMaxParents(JNIEnv *env, jobject obj, jint maxParents)
{
    JSMILE_ENTER;
    GetPtr(env, obj)->maxParents = maxParents;
    JSMILE_LEAVE;
}

JNIEXPORT int JNICALL Java_smile_learning_GreedyThickThinning_getMaxParents(JNIEnv *env, jobject obj)
{
    JSMILE_ENTER;
    JSMILE_RETURN(GetPtr(env, obj)->maxParents);
}

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_learning_GreedyThickThinning_setNetWeight(JNIEnv *env, jobject obj, jdouble netWeight)
{
    JSMILE_ENTER;
    GetPtr(env, obj)->netWeight = netWeight;
    JSMILE_LEAVE;
}

JNIEXPORT double JNICALL Java_smile_learning_GreedyThickThinning_getNetWeight(JNIEnv *env, jobject obj)
{
    JSMILE_ENTER;
    JSMILE_RETURN(GetPtr(env, obj)->netWeight);
}

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_learning_GreedyThickThinning_setPriorsMethod(JNIEnv *env, jobject obj, int priorsMethod)
{
    JSMILE_ENTER;
    GetPtr(env, obj)->priors = DSL_greedyThickThinning::PriorsType(priorsMethod);
    JSMILE_LEAVE;
}

JNIEXPORT int JNICALL Java_smile_learning_GreedyThickThinning_getPriorsMethod(JNIEnv *env, jobject obj)
{
    JSMILE_ENTER;
    JSMILE_RETURN(GetPtr(env, obj)->priors);
}


//-------------------------------------------------------------------------------

JNIEXPORT jobject JNICALL Java_smile_learning_GreedyThickThinning_getBkKnowledge(JNIEnv *env, jobject obj) 
{
    JSMILE_ENTER;
    DSL_greedyThickThinning *gtt = GetPtr(env, obj);
	jobject jBkk = GetBkKnowledge(env, gtt->bkk);
	JSMILE_RETURN(jBkk);
}

JNIEXPORT void JNICALL Java_smile_learning_GreedyThickThinning_setBkKnowledge(JNIEnv *env, jobject obj, jobject jBkk) 
{
    JSMILE_ENTER;
	DSL_greedyThickThinning *gtt = GetPtr(env, obj);
    DSL_bkgndKnowledge *bkk = static_cast<DSL_bkgndKnowledge *>(GetNativePtr(env, jBkk));
	gtt->bkk = *bkk;
    JSMILE_LEAVE;
}

//-------------------------------------------------------------------------------

JNIEXPORT jobject JNICALL Java_smile_learning_GreedyThickThinning_learn(JNIEnv *env, jobject obj, jobject jDataset) 
{
    JSMILE_ENTER;
    JSMILE_RETURN(Learn(GetPtr(env, obj), env, NULL, jDataset));
}

} // end extern "C"
