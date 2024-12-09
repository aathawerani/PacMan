// smile_learning_Validator.h

#include "jsmile.h"

using namespace std;

template <>
DSL_validator* CreateWrapped(JNIEnv *env, jobject obj, jobject param)
{
	jobjectArray paramArray = reinterpret_cast<jobjectArray>(param);
    jobject jNet = env->GetObjectArrayElement(paramArray, 0);
	jobject jDataset = env->GetObjectArrayElement(paramArray, 1);
	jobjectArray jMatching = reinterpret_cast<jobjectArray>(env->GetObjectArrayElement(paramArray, 2));
	DSL_network *net = GetNetworkPtr(env, jNet);
	DSL_dataset *ds = GetDataSet(env, jDataset);
	vector<DSL_datasetMatch> nativeMatching;
	ConvertMatching(env, jMatching, nativeMatching);
	return new DSL_validator(*ds, *net, nativeMatching);
}

extern "C"
{
JSMILE_IMPLEMENT_WRAPPER(learning_Validator, DSL_validator);

JNIEXPORT void JNICALL Java_smile_learning_Validator_addClassNode__I(JNIEnv *env, jobject obj, jint nodeHandle) 
{
	JSMILE_ENTER;
    DSL_validator *v = GetPtr(env, obj);
	DSL_network *net = GetRelatedNetworkPtr(env, obj);
	ValidateNodeHandle(net, nodeHandle);
	int res = v->AddClassNode(nodeHandle);
	if (res != DSL_OKAY) 
    {
		ThrowSmileException(env, "AddClassNode", res);
    }
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_learning_Validator_addClassNode__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId) 
{
	JSMILE_ENTER;
    DSL_network *net = GetRelatedNetworkPtr(env, obj);
	Java_smile_learning_Validator_addClassNode__I(env, obj, ValidateNodeId(env, net, nodeId));
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_learning_Validator_test(JNIEnv *env, jobject obj) 
{
	JSMILE_ENTER;
    int res = GetPtr(env, obj)->Test();
	if (res != DSL_OKAY) 
    {
		ThrowSmileException(env, "Test", res);
    }
	JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_learning_Validator_leaveOneOut(JNIEnv *env, jobject obj, jobject em) 
{
	JSMILE_ENTER;
	DSL_em *nativeEM = static_cast<DSL_em *>(GetNativePtr(env, em));
    int res = GetPtr(env, obj)->LeaveOneOut(*nativeEM);
	if (res != DSL_OKAY) 
    {
		ThrowSmileException(env, "LeaveOneOut", res);
    }
	JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_learning_Validator_kFold(JNIEnv *env, jobject obj, jobject em, jint foldCount, jint foldingRandSeed) 
{
	JSMILE_ENTER;
	DSL_em *nativeEM = static_cast<DSL_em *>(GetNativePtr(env, em));
    int res = GetPtr(env, obj)->KFold(*nativeEM, foldCount, foldingRandSeed);
	if (res != DSL_OKAY) 
    {
		ThrowSmileException(env, "KFold", res);
    }
	JSMILE_LEAVE;
}

JNIEXPORT jobject JNICALL Java_smile_learning_Validator_getResultDataSet(JNIEnv *env, jobject obj) 
{
	JSMILE_ENTER;
	jclass cls = env->FindClass("smile/learning/DataSet");
    jmethodID mid = env->GetMethodID(cls, "<init>", "()V");
    jobject jDataSet = env->NewObject(cls, mid);
	DSL_dataset *ds = GetDataSet(env, jDataSet);
	GetPtr(env, obj)->GetResultDataset(*ds);
	JSMILE_RETURN(jDataSet);
}

JNIEXPORT jobjectArray JNICALL Java_smile_learning_Validator_getConfusionMatrix__I(JNIEnv *env, jobject obj, jint nodeHandle) 
{
	JSMILE_ENTER;
    DSL_validator *v = GetPtr(env, obj);
	vector<vector<int> >  nativeMtx;
	int res = v->GetConfusionMatrix(nodeHandle, nativeMtx);
	if (res != DSL_OKAY) 
    {
		ThrowSmileException(env, "GetConfusionMatrix", res);
    }

	int size = int(nativeMtx.size());

	jclass cls = env->FindClass("[I");
	jobjectArray out = env->NewObjectArray(size, cls, NULL);
	for (int i = 0; i < size; i ++)
	{
		jintArray row = env->NewIntArray(size);
		env->SetIntArrayRegion(row, 0, size, (jint *)&nativeMtx[i].front());
		env->SetObjectArrayElement(out, i, row);
	}

    JSMILE_RETURN(out);
}

JNIEXPORT jobjectArray JNICALL Java_smile_learning_Validator_getConfusionMatrix__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId) 
{
	JSMILE_ENTER;
    DSL_network *net = GetRelatedNetworkPtr(env, obj);
	JSMILE_RETURN(Java_smile_learning_Validator_getConfusionMatrix__I(env, obj, ValidateNodeId(env, net, nodeId)));
}

JNIEXPORT jdouble JNICALL Java_smile_learning_Validator_getAccuracy__II(JNIEnv *env, jobject obj, jint nodeHandle, jint outcomeIndex) 
{
    JSMILE_ENTER;
	DSL_network *net = GetRelatedNetworkPtr(env, obj);
    DSL_node *node = ValidateOutcomeIndex(net, nodeHandle, outcomeIndex);
	double acc;
	int res = GetPtr(env, obj)->GetAccuracy(nodeHandle, outcomeIndex, acc);
	if (DSL_OKAY != res) 
    {
        ThrowSmileException(env, "GetAccuracy", res);
    }
    JSMILE_RETURN(acc);
}

JNIEXPORT jdouble JNICALL Java_smile_learning_Validator_getAccuracy__Ljava_lang_String_2I(JNIEnv *env, jobject obj, jstring nodeId, jint outcomeIndex) 
{
    JSMILE_ENTER;
	DSL_network *net = GetRelatedNetworkPtr(env, obj);
	JSMILE_RETURN(Java_smile_learning_Validator_getAccuracy__II(env, obj, ValidateNodeId(env, net, nodeId), outcomeIndex));
}

JNIEXPORT jdouble JNICALL Java_smile_learning_Validator_getAccuracy__ILjava_lang_String_2(JNIEnv *env, jobject obj, jint nodeHandle, jstring outcomeId) 
{
    JSMILE_ENTER;
	DSL_network *net = GetRelatedNetworkPtr(env, obj);
	JSMILE_RETURN(Java_smile_learning_Validator_getAccuracy__II(env, obj, nodeHandle, ValidateOutcomeId(env, net, nodeHandle, outcomeId)));
}

JNIEXPORT jdouble JNICALL Java_smile_learning_Validator_getAccuracy__Ljava_lang_String_2Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId, jstring outcomeId) 
{
    JSMILE_ENTER;
	DSL_network *net = GetRelatedNetworkPtr(env, obj);
    JSMILE_RETURN(Java_smile_learning_Validator_getAccuracy__ILjava_lang_String_2(env, obj, ValidateNodeId(env, net, nodeId), outcomeId));
}


} // end extern "C"
