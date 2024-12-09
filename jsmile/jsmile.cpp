// jsmile.cpp

#include "jsmile.h"
#include <locale.h>

using namespace std;

//-------------------------------------------------------------------------------

void ThrowSmileException(JNIEnv *env, const char *function, int errCode)
{
    string what = "SMILE error ";
    DSL_appendInt(what, errCode);
    what += " in function ";
    what += function;
    
    int errCount = ErrorH.GetNumberOfErrors();
    if (errCount > 0)
    {
        what += ". Logged information: ";
        for (int i = 0; i < errCount; i ++)
        {
            what += ErrorH.GetErrorMessage(i);
            what += "\n";
        }
        
        ErrorH.Flush();
    }

    env->ThrowNew(env->FindClass("smile/SMILEException"), what.c_str()); 
}

//-------------------------------------------------------------------------------


DSL_dataset* GetDataSet(JNIEnv *env, jobject obj) 
{
    return static_cast<DSL_dataset *>(GetNativePtr(env, obj));
}


//-------------------------------------------------------------------------------

DSL_network* GetRelatedNetworkPtr(JNIEnv *env, jobject obj)
{
    jclass cls = env->GetObjectClass(obj);
    jfieldID fid = env->GetFieldID(cls, "net", "Lsmile/Network;");
    return GetNetworkPtr(env, env->GetObjectField(obj, fid));
}

//-------------------------------------------------------------------------------

jobject GetBkKnowledge(JNIEnv *env, const DSL_bkgndKnowledge &bkk)
{
	jclass cls = env->FindClass("smile/learning/BkKnowledge");
    jmethodID mid = env->GetMethodID(cls, "<init>", "()V");
    jobject jBkk = env->NewObject(cls, mid);
	DSL_bkgndKnowledge *nativeBkk = static_cast<DSL_bkgndKnowledge *>(GetNativePtr(env, jBkk));
	*nativeBkk = bkk;
	return jBkk;
}

//-------------------------------------------------------------------------------

jobject CreateNewNetwork(JNIEnv *env)
{
    jclass cls = env->FindClass("smile/Network");
    jmethodID mid = env->GetMethodID(cls, "<init>", "()V");
    return env->NewObject(cls, mid);
}

//-------------------------------------------------------------------------------

DSL_node* ValidateNodeHandle(DSL_network *net, int handle)
{
    DSL_node *node = net->GetNode(handle);
	if (NULL == node)
    {
        string msg = "Invalid node handle: ";
        DSL_appendInt(msg, handle);
        throw invalid_argument(msg);
    }

    return node;
}

int ValidateNodeId(JNIEnv *env, DSL_network *net, jstring nodeId)
{
    NativeString ns(env, nodeId);
    int handle = net->FindNode(ns);
    if (handle < 0)
    {
        string msg = "Invalid node id: ";
        msg += ns;
        throw invalid_argument(msg);
    }
    return handle;
}

DSL_node* ValidateOutcomeIndex(DSL_network *net, int nodeHandle, int outcomeIndex) 
{
	DSL_node *node = ValidateNodeHandle(net, nodeHandle);
    int count = node->Definition()->GetNumberOfOutcomes();
	if (outcomeIndex < 0 || outcomeIndex >= count) 
    {
		string msg;
		msg = "Invalid outcome index ";
		msg += outcomeIndex;
		msg += " for node '";
		msg += node->Info().Header().GetId();
		msg += "', valid indices are 0..";
        DSL_appendInt(msg, count - 1);
		throw invalid_argument(msg);
	}

    return node;
}

int ValidateOutcomeId(JNIEnv *env, DSL_network *net, int nodeHandle, jstring outcomeId) 
{
	DSL_node *node = ValidateNodeHandle(net, nodeHandle);

    NativeString native(env, outcomeId);
    DSL_idArray *outcomeNames = node->Definition()->GetOutcomesNames();
	int outcomeIndex = outcomeNames->FindPosition(native);
	if (outcomeIndex < 0) 
    {
		string msg;
		msg = "Invalid outcome identifier '";
        msg += native;    
		msg += "' for node '";
		msg += node->Info().Header().GetId();
		msg += "'";
		throw invalid_argument(msg);
	}
	
	return outcomeIndex;
}

//-------------------------------------------------------------------------------

jintArray CopyIntArray(JNIEnv *env, const DSL_intArray &native) 
{
	int count = native.NumItems();
	jintArray arr = env->NewIntArray(count);
	env->SetIntArrayRegion(arr, 0, count, (const jint *)native.Items());
	return arr;
}

jdoubleArray CopyDoubleArray(JNIEnv *env, DSL_doubleArray &native) 
{
    int count = native.GetSize();
    jdoubleArray arr = env->NewDoubleArray(count);
	env->SetDoubleArrayRegion(arr, 0, count, native.Items());
	return arr;
}


jobjectArray CopyStringArray(JNIEnv *env, const DSL_stringArray &native)
{
    jclass clazz = env->FindClass("java/lang/String");
	int count = native.NumItems();
	jobjectArray arr = env->NewObjectArray(count, clazz, NULL);
	for (int i = 0; i < count; i ++) 
    {
		env->SetObjectArrayElement(arr, i, env->NewStringUTF(native[i]));
	}
	return arr;
}


jobjectArray HandlesToIds(JNIEnv *env, DSL_network *net, const DSL_intArray &native) 
{
	jclass clazz = env->FindClass("java/lang/String");
    int count = native.NumItems();
	jobjectArray arr = env->NewObjectArray(count, clazz, NULL);
	for (int i = 0; i < count; i ++) 
    {
		const char *id = net->GetNode(native[i])->Info().Header().GetId();
		env->SetObjectArrayElement(arr, i, env->NewStringUTF(id));
	}

	return arr;
}

//-------------------------------------------------------------------------------

DefLocaleCtx::DefLocaleCtx()
{
	char *loc = setlocale(LC_NUMERIC, NULL);
	if (NULL != loc)
	{
		defLocale = loc;
	}
	setlocale(LC_NUMERIC, "C");
}

DefLocaleCtx::~DefLocaleCtx()
{
	if (!defLocale.empty())
	{
		setlocale(LC_NUMERIC, defLocale.c_str());
	}
}

//-------------------------------------------------------------------------------

void ConvertMatching(JNIEnv *env, jobjectArray jMatching, vector<DSL_datasetMatch> &nativeMatching)
{
	int count = 0; 
	if (NULL == jMatching || !(count = env->GetArrayLength(jMatching)))
	{
		throw invalid_argument("No matching specified");
	}

	nativeMatching.resize(count);

	jclass clazz = env->FindClass("smile/learning/DataMatch");
	jfieldID fidCol = env->GetFieldID(clazz, "column", "I");
	jfieldID fidNode = env->GetFieldID(clazz, "node", "I");
	jfieldID fidSlice = env->GetFieldID(clazz, "slice", "I");

	for (int i = 0; i < count; i ++)
	{
		DSL_datasetMatch &nm = nativeMatching[i];
		jobject match = env->GetObjectArrayElement(jMatching, i);

		nm.column = env->GetIntField(match, fidCol);
		nm.node = env->GetIntField(match, fidNode);
		nm.slice = env->GetIntField(match, fidSlice);
	}
}