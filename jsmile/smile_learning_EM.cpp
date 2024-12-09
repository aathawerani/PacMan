// smile_learning_EM.cpp

#include "jsmile.h"

using namespace std;

void RunEmLearn(JNIEnv *, jobject, jobject, jobject, jobjectArray, const std::vector<int>&);

extern "C"
{
JSMILE_IMPLEMENT_WRAPPER(learning_EM, DSL_em);

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_learning_EM_setEqSampleSize(JNIEnv *env, jobject obj, jint size) 
{
    JSMILE_ENTER;
    GetPtr(env, obj)->SetEquivalentSampleSize((float)size);
    JSMILE_LEAVE;
}

JNIEXPORT jint JNICALL Java_smile_learning_EM_getEqSampleSize(JNIEnv *env, jobject obj) 
{
    JSMILE_ENTER;
    JSMILE_RETURN(int(GetPtr(env, obj)->GetEquivalentSampleSize()));
}

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_learning_EM_setRandomizeParameters(JNIEnv *env, jobject obj, jboolean rp) 
{
	JSMILE_ENTER;
    GetPtr(env, obj)->SetRandomizeParameters(rp != 0);
    JSMILE_LEAVE;
}

JNIEXPORT jboolean JNICALL Java_smile_learning_EM_getRandomizeParameters(JNIEnv *env, jobject obj) 
{
    JSMILE_ENTER;
    JSMILE_RETURN(GetPtr(env, obj)->GetRandomizeParameters());
}

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_learning_EM_setSeed(JNIEnv *env, jobject obj, jint seed) 
{
	JSMILE_ENTER;
    GetPtr(env, obj)->SetSeed(seed);
    JSMILE_LEAVE;
}

JNIEXPORT jboolean JNICALL Java_smile_learning_EM_getSeed(JNIEnv *env, jobject obj) 
{
    JSMILE_ENTER;
    JSMILE_RETURN(GetPtr(env, obj)->GetSeed());
}

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_learning_EM_setUniformizeParameters(JNIEnv *env, jobject obj, jboolean up) 
{
	JSMILE_ENTER;
    GetPtr(env, obj)->SetUniformizeParameters(up != 0);
    JSMILE_LEAVE;
}

JNIEXPORT jboolean JNICALL Java_smile_learning_EM_getUniformizeParameters(JNIEnv *env, jobject obj) 
{
    JSMILE_ENTER;
    JSMILE_RETURN(GetPtr(env, obj)->GetUniformizeParameters());
}

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_learning_EM_setRelevance(JNIEnv *env, jobject obj, jboolean rel) 
{
	JSMILE_ENTER;
    GetPtr(env, obj)->SetRelevance(rel != 0);
    JSMILE_LEAVE;
}

JNIEXPORT jboolean JNICALL Java_smile_learning_EM_getRelevance(JNIEnv *env, jobject obj) 
{
    JSMILE_ENTER;
    JSMILE_RETURN(GetPtr(env, obj)->GetRelevance());
}


//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_learning_EM_learn__Lsmile_learning_DataSet_2Lsmile_Network_2_3Lsmile_learning_DataMatch_2_3Ljava_lang_String_2(JNIEnv *env, jobject obj, jobject jDataSet, jobject jNetwork, jobjectArray jMatching, jobjectArray jFixedNodes)
{
	JSMILE_ENTER;
    
	vector<int> nativeFixedNodes;
	if (NULL != jFixedNodes)
	{
		int count = env->GetArrayLength(jFixedNodes);
		nativeFixedNodes.resize(count);
		for (int i = 0; i < count; i ++)
		{
			nativeFixedNodes[i] = ValidateNodeId(env, jNetwork, (jstring)env->GetObjectArrayElement(jFixedNodes, i));
		}
	}

	RunEmLearn(env, obj, jDataSet, jNetwork, jMatching, nativeFixedNodes);
    
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_learning_EM_learn__Lsmile_learning_DataSet_2Lsmile_Network_2_3Lsmile_learning_DataMatch_2_3I(JNIEnv *env, jobject obj, jobject jDataSet, jobject jNetwork, jobjectArray jMatching, jintArray jFixedNodes)
{
	JSMILE_ENTER;
    
	vector<int> nativeFixedNodes;
	if (NULL != jFixedNodes)
	{
		int count = env->GetArrayLength(jFixedNodes);
		nativeFixedNodes.resize(count);
		env->GetIntArrayRegion(jFixedNodes, 0, count, (jint *)(&nativeFixedNodes.front()));
	}

	RunEmLearn(env, obj, jDataSet, jNetwork, jMatching, nativeFixedNodes);
    
    JSMILE_LEAVE;
}

} // end extern "C"

void RunEmLearn(JNIEnv *env, jobject obj, jobject jDataSet, jobject jNetwork, jobjectArray jMatching, const std::vector<int> &nativeFixedNodes)
{
    DSL_network *net = GetNetworkPtr(env, jNetwork);
    DSL_dataset *ds = GetDataSet(env, jDataSet);
	vector<DSL_datasetMatch> nativeMatching;
	ConvertMatching(env, jMatching, nativeMatching);
	double score;
    int res = GetPtr(env, obj)->Learn(*ds, *net, nativeMatching, nativeFixedNodes, &score);
    if (DSL_OKAY != res) ThrowSmileException(env, "Learn", res);
	jfieldID fidLastScore = env->GetFieldID(env->GetObjectClass(obj), "lastScore", "D");
	env->SetDoubleField(obj, fidLastScore, score);

}

