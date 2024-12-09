// smile_DiagNetwork.h

#include "jsmile.h"
#include <math.h>

using namespace std;

#ifdef WIN32
#define isnan _isnan
#endif

template <>
DIAG_network* CreateWrapped(JNIEnv *env, jobject obj, jobject param)
{
    DSL_network *net = GetNetworkPtr(env, param);
    DIAG_network *diagnet = new DIAG_network;

	diagnet->LinkToNetwork(net);
	diagnet->CollectNetworkInfo();
	diagnet->SetDefaultStates();
	diagnet->UpdateFaultBeliefs();
	int mostLikelyFault = diagnet->FindMostLikelyFault();
	diagnet->SetPursuedFault(mostLikelyFault);

	return diagnet;
}

struct FaultInfo
{
	int index;
	int node;
	int outcome;
	double probability;
	bool isPursued;
};

int FaultInfoComp(const void *a, const void *b)
{
    const FaultInfo &lhs = *static_cast<const FaultInfo *>(a);
    const FaultInfo &rhs = *static_cast<const FaultInfo *>(b);

    double lp = lhs.probability;
	double rp = rhs.probability;

	if (DSL_NOT_RELEVANT == lp) lp = -1;
	if (DSL_NOT_RELEVANT == rp) rp = -1;
	
	if (lp > rp) return -1;
	if (lp < rp) return +1;

	if (lhs.isPursued && !rhs.isPursued) return -1;
	if (!lhs.isPursued && rhs.isPursued) return +1;

	if (lhs.node == rhs.node)
	{
		if (lhs.outcome < rhs.outcome) return -1;
		if (lhs.outcome > rhs.outcome) return +1;
	}
	
	return 0;
}



struct ObsInfo
{
    int node;
    double entropy;
	double cost;
	double infoGain;
};

inline int GetObsCompCategory(double infoGain)
{
    if (isnan(infoGain)) return 3;
	if (DSL_NOT_AVAILABLE == infoGain) return 2;
	if (DSL_NOT_RELEVANT == infoGain) return 1;
    return 0;
}

int ObsInfoComp(const void *a, const void *b)
{
    const ObsInfo &lhs = *static_cast<const ObsInfo *>(a);
    const ObsInfo &rhs = *static_cast<const ObsInfo *>(b);

    int lhsCat = GetObsCompCategory(lhs.infoGain);
	int rhsCat = GetObsCompCategory(rhs.infoGain);
	
	if (lhsCat < rhsCat) return -1;
	if (lhsCat > rhsCat) return +1;

	if (0 == lhsCat)
	{
		if (lhs.infoGain > rhs.infoGain) return -1;
		if (lhs.infoGain < rhs.infoGain) return +1;
	}

	return 0;
}


void CheckFaultIndex(DIAG_network *diagnet, int faultIndex) 
{
	int faultCount = int(diagnet->GetFaults().size());
	if (faultIndex < 0 || faultIndex >= faultCount) 
    {
		string msg;
		msg = "Invalid fault index ";
        DSL_appendInt(msg, faultIndex);
		msg += "; valid indices are 0..";
		DSL_appendInt(msg, faultCount - 1);
		throw invalid_argument(msg);
	}
}




extern "C"
{

JSMILE_IMPLEMENT_WRAPPER(DiagNetwork, DIAG_network);

//-------------------------------------------------------------------------------

JNIEXPORT jboolean JNICALL Java_smile_DiagNetwork_getDSep(JNIEnv *env, jobject obj) 
{
    JSMILE_ENTER;
    JSMILE_RETURN(0 != GetPtr(env, obj)->IsDSepEnabled());
}

JNIEXPORT void JNICALL Java_smile_DiagNetwork_setDSep(JNIEnv *env, jobject obj, jboolean value) 
{
	JSMILE_ENTER;
    GetPtr(env, obj)->EnableDSep(0 != value);
    JSMILE_LEAVE;
}

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_DiagNetwork_setPursuedFault(JNIEnv *env, jobject obj, jint faultIndex) 
{
	JSMILE_ENTER;
    DIAG_network *diagnet = GetPtr(env, obj);
	CheckFaultIndex(diagnet, faultIndex);
	int res = diagnet->SetPursuedFault(faultIndex);
    if (DSL_OKAY != res)
    {
        ThrowSmileException(env, "SetPursuedFault", res);
    }
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_DiagNetwork_setPursuedFaults(JNIEnv *env, jobject obj, jintArray faultIndices) 
{
	JSMILE_ENTER;

    DIAG_network *diagnet = GetPtr(env, obj);
	DSL_intArray native;
	int count = env->GetArrayLength(faultIndices);
	for (int i = 0; i < count; i++) 
    {
        jint faultIndex;
		env->GetIntArrayRegion(faultIndices, i, 1, &faultIndex);
		native.Add(faultIndex);
	}
	int res = diagnet->SetPursuedFaults(native);
    if (DSL_OKAY != res)
    {
        ThrowSmileException(env, "SetPursuedFaults", res);
    }

    JSMILE_LEAVE;
}

JNIEXPORT jint JNICALL Java_smile_DiagNetwork_getPursuedFault(JNIEnv *env, jobject obj) 
{
	JSMILE_ENTER;
    JSMILE_RETURN(GetPtr(env, obj)->GetPursuedFault());
}

JNIEXPORT jintArray JNICALL Java_smile_DiagNetwork_getPursuedFaults(JNIEnv *env, jobject obj) 
{
	JSMILE_ENTER;
    JSMILE_RETURN(CopyIntArray(env, GetPtr(env, obj)->GetPursuedFaults()));
}


//-------------------------------------------------------------------------------

JNIEXPORT jint JNICALL Java_smile_DiagNetwork_findMostLikelyFault(JNIEnv *env, jobject obj) 
{
	JSMILE_ENTER;
    JSMILE_RETURN(GetPtr(env, obj)->FindMostLikelyFault());
}

//-------------------------------------------------------------------------------

JNIEXPORT jint JNICALL Java_smile_DiagNetwork_getFaultCount(JNIEnv *env, jobject obj) 
{
	JSMILE_ENTER;
    JSMILE_RETURN(jint(GetPtr(env, obj)->GetFaults().size()));
}

//-------------------------------------------------------------------------------

JNIEXPORT jobject JNICALL Java_smile_DiagNetwork_getFault(JNIEnv *env, jobject obj, jint faultIndex) 
{
	JSMILE_ENTER;
	DIAG_network *diagnet = GetPtr(env, obj);
	CheckFaultIndex(diagnet, faultIndex);

	const DIAG_faultyState &nativeFaultInfo = diagnet->GetFaults()[faultIndex];
	
	int node = nativeFaultInfo.node;
	int state = nativeFaultInfo.state;
	DSL_nodeValue *value = diagnet->GetNetwork().GetNode(node)->Value();
	double probability = (value->IsValueValid() ? value->GetMatrix()->Subscript(state) : 0);
    bool isPursued = 
		(DSL_FALSE != const_cast<DSL_intArray &>(diagnet->GetPursuedFaults()).IsInList(faultIndex));

    jclass clazz = env->FindClass("smile/FaultInfo");
	jmethodID mid = env->GetMethodID(clazz, "<init>", "(IIIDZ)V");

    JSMILE_RETURN(env->NewObject(clazz, mid, faultIndex, node, state, probability, isPursued));
}

//-------------------------------------------------------------------------------

JNIEXPORT jint JNICALL Java_smile_DiagNetwork_getFaultNode(JNIEnv *env, jobject obj, jint faultIndex) 
{
    JSMILE_ENTER;
	DIAG_network *diagnet = GetPtr(env, obj);
	CheckFaultIndex(diagnet, faultIndex);
    JSMILE_RETURN(diagnet->GetFaults()[faultIndex].node);
}

JNIEXPORT jstring JNICALL Java_smile_DiagNetwork_getFaultNodeId(JNIEnv *env, jobject obj, jint faultIndex) 
{
	JSMILE_ENTER;
    DSL_network *net = GetRelatedNetworkPtr(env, obj);
	DSL_node *node = ValidateNodeHandle(net, Java_smile_DiagNetwork_getFaultNode(env, obj, faultIndex));
	JSMILE_RETURN(env->NewStringUTF(node->Info().Header().GetId()));
}

//-------------------------------------------------------------------------------

JNIEXPORT jint JNICALL Java_smile_DiagNetwork_getFaultOutcome(JNIEnv *env, jobject obj, jint faultIndex) 
{
    JSMILE_ENTER;
	DIAG_network *diagnet = GetPtr(env, obj);
	CheckFaultIndex(diagnet, faultIndex);
    JSMILE_RETURN(diagnet->GetFaults()[faultIndex].state);
}

JNIEXPORT jstring JNICALL Java_smile_DiagNetwork_getFaultOutcomeId(JNIEnv *env, jobject obj, jint faultIndex) 
{
	JSMILE_ENTER;
    DIAG_network *diagnet = GetPtr(env, obj);
    DSL_network *net = GetRelatedNetworkPtr(env, obj);
	CheckFaultIndex(diagnet, faultIndex);
    const DIAG_faultyState& fs = diagnet->GetFaults()[faultIndex];
    DSL_node *node = ValidateOutcomeIndex(net, fs.node, fs.state);
    DSL_idArray *outcomes = node->Definition()->GetOutcomesNames();
    JSMILE_RETURN(env->NewStringUTF(outcomes->Subscript(fs.state)));
}

//-------------------------------------------------------------------------------

JNIEXPORT jint JNICALL Java_smile_DiagNetwork_getFaultIndex__II(JNIEnv *env, jobject obj, jint nodeHandle, jint outcomeIndex) 
{
    JSMILE_ENTER;
    int res = GetPtr(env, obj)->FindFault(nodeHandle, outcomeIndex);
    if (res < 0)
    {
        ThrowSmileException(env, "FindFault", res);
    }
    JSMILE_RETURN(res);
}

JNIEXPORT jint JNICALL Java_smile_DiagNetwork_getFaultIndex__ILjava_lang_String_2(JNIEnv *env, jobject obj, jint nodeHandle, jstring outcomeId) 
{
	JSMILE_ENTER;
    DSL_network *net = GetRelatedNetworkPtr(env, obj);
    JSMILE_RETURN(Java_smile_DiagNetwork_getFaultIndex__II(env, obj, nodeHandle, ValidateOutcomeId(env, net, nodeHandle, outcomeId)));
}

JNIEXPORT jint JNICALL Java_smile_DiagNetwork_getFaultIndex__Ljava_lang_String_2I(JNIEnv *env, jobject obj, jstring nodeId, jint outcomeIndex) 
{
	JSMILE_ENTER;
    DSL_network *net = GetRelatedNetworkPtr(env, obj);
    JSMILE_RETURN(Java_smile_DiagNetwork_getFaultIndex__II(env, obj, ValidateNodeId(env, net, nodeId), outcomeIndex));
}

JNIEXPORT jint JNICALL Java_smile_DiagNetwork_getFaultIndex__Ljava_lang_String_2Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId, jstring outcomeId) 
{
	JSMILE_ENTER;
    DSL_network *net = GetRelatedNetworkPtr(env, obj);
    JSMILE_RETURN(Java_smile_DiagNetwork_getFaultIndex__ILjava_lang_String_2(env, obj, ValidateNodeId(env, net, nodeId), outcomeId));
}

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_DiagNetwork_restart(JNIEnv *env, jobject obj) 
{
	JSMILE_ENTER;
	DIAG_network *diagnet = GetPtr(env, obj);
	diagnet->RestartDiagnosis();
	JSMILE_LEAVE;
}


JNIEXPORT jobject JNICALL Java_smile_DiagNetwork_updateNative(JNIEnv *env, jobject obj, jint multiFaultAlgorithm) 
{
    JSMILE_ENTER;
	
	DIAG_network *diagnet = GetPtr(env, obj);
	diagnet->UpdateFaultBeliefs();
	int res = diagnet->ComputeTestStrengths(multiFaultAlgorithm);
    if (DSL_OKAY != res)
    {
        ThrowSmileException(env, "ComputeTestStrengths", res);
        return NULL;        
    }
	
	DSL_intArray &unperformed = diagnet->GetUnperformedTests();
    int testCount = unperformed.NumItems();
    vector<ObsInfo> ti(testCount);
	const vector<DIAG_testInfo> &stats = diagnet->GetTestStatistics();
    for (int i = 0; i < testCount; i ++)
    {
        int handle = unperformed[i];
		const DIAG_testInfo &st = stats[i];
		ti[i].node = handle;
		ti[i].infoGain = st.strength;
		ti[i].entropy = st.entropy;
		ti[i].cost = st.cost;
    }

	const vector<DIAG_faultyState> &nativeFaults = diagnet->GetFaults();
	int faultCount = int(nativeFaults.size());
    vector<FaultInfo> fi(faultCount);
	for (int i = 0; i < faultCount; i ++)
	{
		double p = DSL_NOT_RELEVANT;
		const DIAG_faultyState fs = nativeFaults[i];
		DSL_nodeValue *value = diagnet->GetNetwork().GetNode(fs.node)->Value();
		if (value->IsValueValid())
		{
			p = value->GetMatrix()->Subscript(fs.state);
		}
		
		fi[i].index = i;
		fi[i].node = fs.node;
		fi[i].outcome = fs.state;
		fi[i].probability = p;
        fi[i].isPursued = false;
	}

	const DSL_intArray &pursuedFaults = diagnet->GetPursuedFaults();
	for (int i = 0; i < pursuedFaults.NumItems(); i ++)
	{
		fi[pursuedFaults[i]].isPursued = true;
	}

    qsort(&ti.front(), testCount, sizeof(ObsInfo), ObsInfoComp);
    qsort(&fi.front(), faultCount, sizeof(FaultInfo), FaultInfoComp);

    jclass clsObs = env->FindClass("smile/ObservationInfo");
    jmethodID midObs = env->GetMethodID(clsObs, "<init>", "(IDDD)V");
    jobjectArray observations = env->NewObjectArray(testCount, clsObs, NULL);
    for (int i = 0; i < testCount; i ++)
    {
        const ObsInfo &n = ti[i];
        jobject elem = env->NewObject(clsObs, midObs, n.node, n.entropy, n.cost, n.infoGain);
        env->SetObjectArrayElement(observations, i, elem);
    }

    jclass clsFault = env->FindClass("smile/FaultInfo");
    jmethodID midFault = env->GetMethodID(clsFault, "<init>", "(IIIDZ)V");
    jobjectArray faults = env->NewObjectArray(faultCount, clsFault, NULL);
    for (int i = 0; i < faultCount; i ++)
    {
        const FaultInfo &n = fi[i];
        jobject elem = env->NewObject(clsFault, midFault, n.index, n.node, n.outcome, n.probability, n.isPursued);
        env->SetObjectArrayElement(faults, i, elem);
    }

    
    jclass clsRes = env->FindClass("smile/DiagResults");
    jmethodID midRes = env->GetMethodID(clsRes, "<init>", "([Lsmile/ObservationInfo;[Lsmile/FaultInfo;)V");
	
	JSMILE_RETURN(env->NewObject(clsRes, midRes, observations, faults));
}

//-------------------------------------------------------------------------------

JNIEXPORT jint JNICALL Java_smile_DiagNetwork_getUnperformedTestCount(JNIEnv *env, jobject obj) 
{
	JSMILE_ENTER;
    JSMILE_RETURN(GetPtr(env, obj)->GetUnperformedTests().NumItems());
}

//-------------------------------------------------------------------------------

JNIEXPORT jintArray JNICALL Java_smile_DiagNetwork_getUnperformedObservations(JNIEnv *env, jobject obj) 
{
	JSMILE_ENTER;
    JSMILE_RETURN(CopyIntArray(env, GetPtr(env, obj)->GetUnperformedTests()));
}

JNIEXPORT jobjectArray JNICALL Java_smile_DiagNetwork_getUnperformedObservationIds(JNIEnv *env, jobject obj) 
{
	JSMILE_ENTER;
	DIAG_network *diagnet = GetPtr(env, obj);
    DSL_network *net = GetRelatedNetworkPtr(env, obj);
    JSMILE_RETURN(HandlesToIds(env, net, diagnet->GetUnperformedTests()));
}

//-------------------------------------------------------------------------------

JNIEXPORT jboolean JNICALL Java_smile_DiagNetwork_mandatoriesInstantiated(JNIEnv *env, jobject obj) 
{
    JSMILE_ENTER;
    JSMILE_RETURN(GetPtr(env, obj)->MandatoriesInstantiated());
}

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_DiagNetwork_instantiateObservation__II(JNIEnv *env, jobject obj, jint nodeHandle, jint outcomeIndex) 
{
	JSMILE_ENTER;
	DSL_network *net = GetRelatedNetworkPtr(env, obj);
    ValidateOutcomeIndex(net, nodeHandle, outcomeIndex);
	int res = GetPtr(env, obj)->InstantiateObservation(nodeHandle, outcomeIndex);
    if (DSL_OKAY != res)
    {
        ThrowSmileException(env, "InstantiateObservation", res);
    }
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_DiagNetwork_instantiateObservation__ILjava_lang_String_2(JNIEnv *env, jobject obj, jint nodeHandle, jstring outcomeId) 
{
	JSMILE_ENTER;
    DSL_network *net = GetRelatedNetworkPtr(env, obj);
    Java_smile_DiagNetwork_instantiateObservation__II(env, obj, nodeHandle, ValidateOutcomeId(env, net, nodeHandle, outcomeId));
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_DiagNetwork_instantiateObservation__Ljava_lang_String_2I(JNIEnv *env, jobject obj, jstring nodeId, jint outcomeIndex) 
{
    JSMILE_ENTER;
    DSL_network *net = GetRelatedNetworkPtr(env, obj);
    Java_smile_DiagNetwork_instantiateObservation__II(env, obj, ValidateNodeId(env, net, nodeId), outcomeIndex);
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_DiagNetwork_instantiateObservation__Ljava_lang_String_2Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId, jstring outcomeId) 
{
    JSMILE_ENTER;
    DSL_network *net = GetRelatedNetworkPtr(env, obj);
    Java_smile_DiagNetwork_instantiateObservation__ILjava_lang_String_2(env, obj, ValidateNodeId(env, net, nodeId), outcomeId);
    JSMILE_LEAVE;
}

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_DiagNetwork_releaseObservation__I(JNIEnv *env, jobject obj, jint nodeHandle) 
{
	JSMILE_ENTER;
	DSL_network *net = GetRelatedNetworkPtr(env, obj);
	ValidateNodeHandle(net, nodeHandle);
	int res = GetPtr(env, obj)->ReleaseObservation(nodeHandle);
	if (DSL_OKAY != res)
    {
        ThrowSmileException(env, "ReleaseObservation", res);
    }
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_DiagNetwork_releaseObservation__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId) 
{
	JSMILE_ENTER;
    Java_smile_DiagNetwork_releaseObservation__I(env, obj, ValidateNodeId(env, GetRelatedNetworkPtr(env, obj), nodeId));
    JSMILE_LEAVE;
}

} // end extern "C"
