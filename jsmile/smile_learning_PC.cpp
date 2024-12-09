// smile_learning_PC.cpp

#include "jsmile.h"

extern "C"
{
JSMILE_IMPLEMENT_WRAPPER(learning_PC, DSL_pc)

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_learning_PC_setMaxAdjacency(JNIEnv *env, jobject obj, jint maxAdjacency)
{
    JSMILE_ENTER;
    GetPtr(env, obj)->maxAdjacency = maxAdjacency;
    JSMILE_LEAVE;
}

JNIEXPORT int JNICALL Java_smile_learning_PC_getMaxAdjacency(JNIEnv *env, jobject obj)
{
    JSMILE_ENTER;
    JSMILE_RETURN(GetPtr(env, obj)->maxAdjacency);
}

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_learning_PC_setSignificance(JNIEnv *env, jobject obj, jdouble significance)
{
    JSMILE_ENTER;
    GetPtr(env, obj)->significance = significance;
    JSMILE_LEAVE;
}

JNIEXPORT double JNICALL Java_smile_learning_PC_getSignificance(JNIEnv *env, jobject obj)
{
    JSMILE_ENTER;
    JSMILE_RETURN(GetPtr(env, obj)->significance);
}

//-------------------------------------------------------------------------------

JNIEXPORT jobject JNICALL Java_smile_learning_PC_getBkKnowledge(JNIEnv *env, jobject obj) 
{
    JSMILE_ENTER;
    DSL_pc *pc = GetPtr(env, obj);
	jobject jBkk = GetBkKnowledge(env, pc->bkk);
    JSMILE_RETURN(jBkk);
}

JNIEXPORT void JNICALL Java_smile_learning_PC_setBkKnowledge(JNIEnv *env, jobject obj, jobject jBkk) 
{
	JSMILE_ENTER;
    DSL_pc *pc = GetPtr(env, obj);
	DSL_bkgndKnowledge *bkk = static_cast<DSL_bkgndKnowledge *>(GetNativePtr(env, jBkk));
	pc->bkk = *bkk;
    JSMILE_LEAVE;
}

//-------------------------------------------------------------------------------

JNIEXPORT jobject JNICALL Java_smile_learning_PC_learn(JNIEnv *env, jobject obj, jobject jDataset)
{
    JSMILE_ENTER;

	jclass cls = env->FindClass("smile/learning/Pattern");
    jmethodID mid = env->GetMethodID(cls, "<init>", "()V");
    jobject jPattern = env->NewObject(cls, mid);

	DSL_pattern *pattern = static_cast<DSL_pattern *>(GetNativePtr(env, jPattern));
	DSL_pc *pc = static_cast<DSL_pc *>(GetNativePtr(env, obj));
	DSL_dataset *ds = GetDataSet(env, jDataset);

	int res = pc->Learn(*ds, *pattern);
	if (DSL_OKAY != res) ThrowSmileException(env, "Learn", res);

	JSMILE_RETURN(jPattern);
}

} // end extern "C"

