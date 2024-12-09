// smile_learning_BayesianSearch.cpp

#include "jsmile.h"

extern "C"
{
JSMILE_IMPLEMENT_WRAPPER(learning_BayesianSearch, DSL_bs);

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_learning_BayesianSearch_setMaxParents(JNIEnv *env, jobject obj, jint maxParents)
{
    JSMILE_ENTER;
    GetPtr(env, obj)->maxParents = maxParents;
    JSMILE_LEAVE;
}

JNIEXPORT int JNICALL Java_smile_learning_BayesianSearch_getMaxParents(JNIEnv *env, jobject obj)
{
    JSMILE_ENTER;
    JSMILE_RETURN(GetPtr(env, obj)->maxParents);
}

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_learning_BayesianSearch_setMaxSearchTime(JNIEnv *env, jobject obj, jint searchTime)
{
    JSMILE_ENTER;
    GetPtr(env, obj)->maxSearchTime = searchTime;
    JSMILE_LEAVE;
}

JNIEXPORT int JNICALL Java_smile_learning_BayesianSearch_getMaxSearchTime(JNIEnv *env, jobject obj)
{
    JSMILE_ENTER;
    JSMILE_RETURN(GetPtr(env, obj)->maxSearchTime);
}

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_learning_BayesianSearch_setRandSeed(JNIEnv *env, jobject obj, jint seed)
{
    JSMILE_ENTER;
    GetPtr(env, obj)->seed = seed;
    JSMILE_LEAVE;
}

JNIEXPORT int JNICALL Java_smile_learning_BayesianSearch_getRandSeed(JNIEnv *env, jobject obj)
{
    JSMILE_ENTER;
    JSMILE_RETURN(GetPtr(env, obj)->seed);
}
//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_learning_BayesianSearch_setIterationCount(JNIEnv *env, jobject obj, jint count)
{
    JSMILE_ENTER;
    GetPtr(env, obj)->nrIteration = count;
    JSMILE_LEAVE;
}

JNIEXPORT int JNICALL Java_smile_learning_BayesianSearch_getIterationCount(JNIEnv *env, jobject obj)
{
    JSMILE_ENTER;
    JSMILE_RETURN(GetPtr(env, obj)->nrIteration);
}

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_learning_BayesianSearch_setPriorSampleSize(JNIEnv *env, jobject obj, jint sampleSize)
{
    JSMILE_ENTER;
    GetPtr(env, obj)->priorSampleSize = sampleSize;
    JSMILE_LEAVE;
}

JNIEXPORT int JNICALL Java_smile_learning_BayesianSearch_getPriorSampleSize(JNIEnv *env, jobject obj)
{
    JSMILE_ENTER;
    JSMILE_RETURN(GetPtr(env, obj)->priorSampleSize);
}

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_learning_BayesianSearch_setLinkProbability(JNIEnv *env, jobject obj, jdouble prob)
{
    JSMILE_ENTER;
    GetPtr(env, obj)->linkProbability = prob;
    JSMILE_LEAVE;
}

JNIEXPORT double JNICALL Java_smile_learning_BayesianSearch_getLinkProbability(JNIEnv *env, jobject obj)
{
    JSMILE_ENTER;
    JSMILE_RETURN(GetPtr(env, obj)->linkProbability);
}

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_learning_BayesianSearch_setPriorLinkProbability(JNIEnv *env, jobject obj, jdouble prob)
{
    JSMILE_ENTER;
    GetPtr(env, obj)->priorLinkProbability = prob;
    JSMILE_LEAVE;
}

JNIEXPORT double JNICALL Java_smile_learning_BayesianSearch_getPriorLinkProbability(JNIEnv *env, jobject obj)
{
    JSMILE_ENTER;
    JSMILE_RETURN(GetPtr(env, obj)->priorLinkProbability);
}

//-------------------------------------------------------------------------------

JNIEXPORT jobject JNICALL Java_smile_learning_BayesianSearch_getBkKnowledge(JNIEnv *env, jobject obj) 
{
    JSMILE_ENTER;
    DSL_bs *bs = GetPtr(env, obj);
	jobject jBkk = GetBkKnowledge(env, bs->bkk);
	JSMILE_RETURN(jBkk);
}

JNIEXPORT void JNICALL Java_smile_learning_BayesianSearch_setBkKnowledge(JNIEnv *env, jobject obj, jobject jBkk) 
{
    JSMILE_ENTER;
	DSL_bs *bs = GetPtr(env, obj);
    DSL_bkgndKnowledge *bkk = static_cast<DSL_bkgndKnowledge *>(GetNativePtr(env, jBkk));
	bs->bkk = *bkk;
    JSMILE_LEAVE;
}

//-------------------------------------------------------------------------------

JNIEXPORT jobject JNICALL Java_smile_learning_BayesianSearch_learn(JNIEnv *env, jobject obj, jobject jDataset) 
{
    JSMILE_ENTER;

    jobject jNetwork = CreateNewNetwork(env);
	DSL_network *net = GetNetworkPtr(env, jNetwork);	
	DSL_dataset *ds = GetDataSet(env, jDataset);
	DSL_bs *bs = GetPtr(env, obj);
		
	double bestScore;
    int res = bs->Learn(*ds, *net, NULL, NULL, &bestScore);
	if (DSL_OKAY != res) 
	{
		ThrowSmileException(env, "Learn", res);
	}
	jfieldID fidLastScore = env->GetFieldID(env->GetObjectClass(obj), "lastScore", "D");
	env->SetDoubleField(obj, fidLastScore, bestScore);

    JSMILE_RETURN(jNetwork);
}

} // end extern "C"
