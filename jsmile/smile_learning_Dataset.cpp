// smile_learning_Dataset.cpp

#include "jsmile.h"

using namespace std;

void ValidateVariable(DSL_dataset *ds, int variable) 
{
	if (variable < 0) 
    {
		throw runtime_error("Negative variable index");
	}

	int varCount = ds->GetNumberOfVariables();
	if (variable >= varCount) 
    {
		if (varCount == 0) 
        {
			throw runtime_error("DataSet object has no variables");
		}
		else 
        {
			string msg;
			msg += "Invalid variable index ";
            DSL_appendInt(msg, variable);
			msg += ", valid range is 0..";
            DSL_appendInt(msg, varCount - 1);
            throw runtime_error(msg);
		}
	}
}

//-------------------------------------------------------------------------------
void ValidateVariableRecord(DSL_dataset *ds, int variable, int record) 
{
	ValidateVariable(ds, variable);

	if (record < 0) 
    {
		throw runtime_error("Negative record index");
	}

	int recCount = ds->GetNumberOfRecords();
	if (record >= recCount) 
    {
		if (recCount == 0) 
        {
			throw runtime_error("DataSet object has no records");
		}
		else 
        {
			string msg;
			msg += "Invalid record index";
            DSL_appendInt(msg, record);
			msg += ", valid range is 0..";
			DSL_appendInt(msg, recCount - 1);
            throw runtime_error(msg);
		}
	}
}




//-------------------------------------------------------------------------------

extern "C" 
{

JSMILE_IMPLEMENT_WRAPPER(learning_DataSet, DSL_dataset);


//-------------------------------------------------------------------------------

JNIEXPORT jint JNICALL Java_smile_learning_DataSet_getRecordCount(JNIEnv *env, jobject obj) 
{
    JSMILE_ENTER;
    JSMILE_RETURN(GetPtr(env, obj)->GetNumberOfRecords());
}

JNIEXPORT jint JNICALL Java_smile_learning_DataSet_getVariableCount(JNIEnv *env, jobject obj) 
{
    JSMILE_ENTER;
    JSMILE_RETURN(GetPtr(env, obj)->GetNumberOfVariables());
}

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_learning_DataSet_addEmptyRecord(JNIEnv *env, jobject obj) 
{
	JSMILE_ENTER;
    DSL_dataset *ds = GetPtr(env, obj);
	ds->AddEmptyRecord();
    JSMILE_LEAVE;
}

//-------------------------------------------------------------------------------

JNIEXPORT jstring JNICALL Java_smile_learning_DataSet_getVariableId(JNIEnv *env, jobject obj, jint variableIndex) 
{
    JSMILE_ENTER;
    JSMILE_RETURN(env->NewStringUTF(GetPtr(env, obj)->GetId(variableIndex).c_str()));
}

//-------------------------------------------------------------------------------

JNIEXPORT jint JNICALL Java_smile_learning_DataSet_findVariable(JNIEnv *env, jobject obj, jstring variableId) 
{
    JSMILE_ENTER;
    JSMILE_RETURN(GetPtr(env, obj)->FindVariable(string(NativeString(env, variableId))));
}

//-------------------------------------------------------------------------------

JNIEXPORT jboolean JNICALL Java_smile_learning_DataSet_isMissing(JNIEnv *env, jobject obj, jint variable, jint record) 
{
	JSMILE_ENTER;
    DSL_dataset *ds = GetPtr(env, obj);
	ValidateVariableRecord(ds, variable, record);
    JSMILE_RETURN(ds->IsMissing(variable, record));
}

JNIEXPORT void JNICALL Java_smile_learning_DataSet_setMissing(JNIEnv *env, jobject obj, jint variable, jint record) 
{
    JSMILE_ENTER;
	DSL_dataset *ds = GetPtr(env, obj);
	ValidateVariableRecord(ds, variable, record);
	ds->SetMissing(variable, record);
    JSMILE_LEAVE;
}

//-------------------------------------------------------------------------------


JNIEXPORT jboolean JNICALL Java_smile_learning_DataSet_isDiscrete__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring variableId) 
{
	JSMILE_ENTER;
	DSL_dataset *ds = GetPtr(env, obj);
	NativeString nativeId(env, variableId);
	
	int variableIndex = ds->FindVariable(string(nativeId));
	if (variableIndex < 0)
	{
		string msg = "Variable does not exist in the dataset: ";
		msg += nativeId;
		throw runtime_error(msg);
	}

	JSMILE_RETURN(ds->IsDiscrete(variableIndex));
}

JNIEXPORT jboolean JNICALL Java_smile_learning_DataSet_isDiscrete__I(JNIEnv *env, jobject obj, jint variableIndex) 
{
	JSMILE_ENTER;
	DSL_dataset *ds = GetPtr(env, obj);
	ValidateVariable(ds, variableIndex);
    JSMILE_RETURN(ds->IsDiscrete(variableIndex));
}


//-------------------------------------------------------------------------------

JNIEXPORT jint JNICALL Java_smile_learning_DataSet_getInt(JNIEnv *env, jobject obj, jint variable, jint record) 
{
	JSMILE_ENTER;
    DSL_dataset *ds = GetPtr(env, obj);
	ValidateVariableRecord(ds, variable, record);
    JSMILE_RETURN(ds->GetInt(variable, record));
}

JNIEXPORT void JNICALL Java_smile_learning_DataSet_setInt(JNIEnv *env, jobject obj, jint variable, jint record, jint value) 
{
    JSMILE_ENTER;
	DSL_dataset *ds = GetPtr(env, obj);
	ValidateVariableRecord(ds, variable, record);
	ds->SetInt(variable, record, value);
    JSMILE_LEAVE;
}

//-------------------------------------------------------------------------------

JNIEXPORT jfloat JNICALL Java_smile_learning_DataSet_getFloat(JNIEnv *env, jobject obj, jint variable, jint record) 
{
    JSMILE_ENTER;
	DSL_dataset *ds = GetPtr(env, obj);
	ValidateVariableRecord(ds, variable, record);
    JSMILE_RETURN(ds->GetFloat(variable, record));
}

JNIEXPORT void JNICALL Java_smile_learning_DataSet_setFloat(JNIEnv *env, jobject obj, jint variable, jint record, jfloat value) 
{
    JSMILE_ENTER;
	DSL_dataset *ds = GetPtr(env, obj);
	ValidateVariableRecord(ds, variable, record);
	ds->SetFloat(variable, record, value);
    JSMILE_LEAVE;
}

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_learning_DataSet_addIntVariable(JNIEnv *env, jobject obj, jstring id, jint missingValue) 
{
    JSMILE_ENTER;
    DSL_dataset *ds = GetPtr(env, obj);
    int res = ds->AddIntVar(string(NativeString(env, id)), missingValue);
    if (DSL_OKAY != res) 
	{
		ThrowSmileException(env, "addIntVariable", res);
	}
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_learning_DataSet_addFloatVariable(JNIEnv *env, jobject obj, jstring id, jfloat missingValue) 
{
    JSMILE_ENTER;
    DSL_dataset *ds = GetPtr(env, obj);
    int res = ds->AddFloatVar(string(NativeString(env, id)), missingValue);
    if (DSL_OKAY != res) 
	{
		ThrowSmileException(env, "addFloatVariable", res);
	}
    JSMILE_LEAVE;
}

//-------------------------------------------------------------------------------

JNIEXPORT jobjectArray JNICALL Java_smile_learning_DataSet_getStateNames(JNIEnv *env, jobject obj, jint variable) 
{
	JSMILE_ENTER;
	DSL_dataset *ds = GetPtr(env, obj);
	ValidateVariable(ds, variable);

	const vector<string> &native = ds->GetStateNames(variable);
	int imax = int(native.size());
	
	jclass clazz = env->FindClass("java/lang/String");
	jobjectArray arr = env->NewObjectArray(imax, clazz, NULL);
	for (int i = 0; i < imax; i++) 
    {
		jstring jstr = env->NewStringUTF(native[i].c_str());
		env->SetObjectArrayElement(arr, i, jstr);
	}

    JSMILE_RETURN(arr);
}


JNIEXPORT void JNICALL Java_smile_learning_DataSet_setStateNames(JNIEnv *env, jobject obj, jint variable, jobjectArray names) 
{
	JSMILE_ENTER;
	DSL_dataset *ds = GetPtr(env, obj);
	ValidateVariable(ds, variable);

	int imax = env->GetArrayLength(names);
	vector<string> native(imax);

	for (int i = 0; i < imax; i++) 
    {
		jstring name = jstring(env->GetObjectArrayElement(names, i));
        if (NULL == name)
        {
            string msg = "State name can't be null (index ";
            DSL_appendInt(msg, i);
            msg += ')';
            throw invalid_argument(msg);
        }
		
        native[i] = NativeString(env, name);
	}

	int res = ds->SetStateNames(variable, native);
    if (DSL_OKAY != res) 
	{
		ThrowSmileException(env, "setStateNames", res);
	}

    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_learning_DataSet_readFile(JNIEnv *env, jobject obj, jstring filename, jstring missingValueToken, jint missingInt, jfloat missingFloat, jboolean columnIdsPresent)
{
    JSMILE_ENTER;

    DSL_datasetParseParams params;
    if (NULL != missingValueToken)
    {
        params.missingValueToken = NativeString(env, missingValueToken);
    }
    params.missingInt = missingInt;
    params.missingFloat = missingFloat;
	params.columnIdsPresent = columnIdsPresent ? true : false;

    DSL_dataset *ds = GetPtr(env, obj);
	string errMsg;
    DefLocaleCtx dlc;
    int res = ds->ReadFile(string(NativeString(env, filename)), &params, &errMsg);
    if (DSL_OKAY != res) 
	{
		ThrowSmileException(env, runtime_error(errMsg));
	}
    JSMILE_LEAVE;
}

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_learning_DataSet_writeFile(JNIEnv *env, jobject obj, jstring filename, jchar separator, jstring missingValueToken, jboolean columnIdsPresent)
{
    JSMILE_ENTER;

    DSL_datasetWriteParams params;
    if (NULL != missingValueToken)
    {
        params.missingValueToken = NativeString(env, missingValueToken);
    }

	if (separator)
	{
		params.separator = (char)separator;
	}
	
	params.columnIdsPresent = columnIdsPresent ? true : false;

    DSL_dataset *ds = GetPtr(env, obj);
	string errMsg;
    DefLocaleCtx dlc;
    int res = ds->WriteFile(string(NativeString(env, filename)), &params, &errMsg);
    if (DSL_OKAY != res) 
	{
		ThrowSmileException(env, runtime_error(errMsg));
	}
    JSMILE_LEAVE;
}

//-------------------------------------------------------------------------------

JNIEXPORT jobjectArray JNICALL Java_smile_learning_DataSet_matchNetwork(JNIEnv *env, jobject obj, jobject jnet)
{
	JSMILE_ENTER;

	DSL_dataset *ds = GetPtr(env, obj);
	DSL_network *net = GetNetworkPtr(env, jnet);
	std::string errMsg;
	vector<DSL_datasetMatch> nativeMatching;
	int res = ds->MatchNetwork(*net, nativeMatching, errMsg);
	if (DSL_OKAY != res)
	{
		ThrowSmileException(env, runtime_error(errMsg));
	}
	
	jobjectArray arr = NULL;

	int count = int(nativeMatching.size());
	if (count > 0)
	{
		jclass clazz = env->FindClass("smile/learning/DataMatch");
		jmethodID midCtor = env->GetMethodID(clazz, "<init>", "(III)V");
		arr = env->NewObjectArray(count, clazz, NULL);
		for (int i = 0; i < count; i ++) 
		{
			const DSL_datasetMatch &nm = nativeMatching[i];
			jobject elem = env->NewObject(clazz, midCtor, nm.column, nm.node, nm.slice);
			env->SetObjectArrayElement(arr, i, elem);
		}
	}

    JSMILE_RETURN(arr);
}

//-------------------------------------------------------------------------------

JNIEXPORT jdoubleArray JNICALL Java_smile_learning_DataSet_discretize(JNIEnv *env, jobject obj, jint variable, jint algorithm, jint intervals, jstring statePrefix)
{
	JSMILE_ENTER;
	DSL_dataset *ds = GetPtr(env, obj);
	int res;
	DSL_dataset::DiscretizeAlgorithm dalg = (DSL_dataset::DiscretizeAlgorithm)algorithm;
	string nativeStatePrefix;
	if (statePrefix)
	{
		NativeString ns(env, statePrefix);
		nativeStatePrefix = ns;
	}

	vector<double> edges;
	res = ds->Discretize(variable, dalg, intervals, nativeStatePrefix, edges);
    if (DSL_OKAY != res) 
	{
		ThrowSmileException(env, "Discretize", res);
	}
	
	int edgeCount = int(edges.size());
	jdoubleArray out = env->NewDoubleArray(edgeCount);

	env->SetDoubleArrayRegion(out, 0, edgeCount, &edges.front());

	JSMILE_RETURN(out);
}

} // end extern "C"
