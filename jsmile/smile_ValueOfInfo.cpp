// smile_ValueOfInfo.h

#include "jsmile.h"


using namespace std;

template <>
DSL_valueOfInformation* CreateWrapped(JNIEnv *env, jobject obj, jobject param)
{
    return new DSL_valueOfInformation(GetNetworkPtr(env, param));
}

extern "C"
{
JSMILE_IMPLEMENT_WRAPPER(ValueOfInfo, DSL_valueOfInformation);
    
//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_ValueOfInfo_update(JNIEnv *env, jobject obj) 
{
    JSMILE_ENTER;
	DSL_valueOfInformation *voi = GetPtr(env, obj);
	DSL_network *net = GetRelatedNetworkPtr(env, obj);
	int res = net->ValueOfInformation(*voi);
    if (DSL_OKAY != res)
    {
        ThrowSmileException(env, "Update", res);
    }
    JSMILE_LEAVE;
}

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_ValueOfInfo_addNode__I(JNIEnv *env, jobject obj, jint nodeHandle) 
{
	JSMILE_ENTER;
    DSL_valueOfInformation *voi = GetPtr(env, obj);
	DSL_network *net = GetRelatedNetworkPtr(env, obj);
	ValidateNodeHandle(net, nodeHandle);
	int res = voi->AddNode(nodeHandle);
	if (res != DSL_OKAY) 
    {
        string msg = "Cannot add node ";
        msg += net->GetNode(nodeHandle)->Info().Header().GetId();
        msg += " to ValueOfInfo object";
        throw runtime_error(msg);
    }
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_ValueOfInfo_addNode__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId) 
{
	JSMILE_ENTER;
    DSL_network *net = GetRelatedNetworkPtr(env, obj);
	Java_smile_ValueOfInfo_addNode__I(env, obj, ValidateNodeId(env, net, nodeId));
    JSMILE_LEAVE;
}

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_ValueOfInfo_removeNode__I(JNIEnv *env, jobject obj, jint nodeHandle) 
{
	JSMILE_ENTER;
    DSL_valueOfInformation *voi = GetPtr(env, obj);
	DSL_network *net = GetRelatedNetworkPtr(env, obj);
	ValidateNodeHandle(net, nodeHandle);
	int res = voi->RemoveNode(nodeHandle);
	if (res != DSL_OKAY) 
    {
        string msg = "Cannot remove node ";
        msg += net->GetNode(nodeHandle)->Info().Header().GetId();
        msg += " to ValueOfInfo object";
        throw runtime_error(msg);
    }
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_ValueOfInfo_removeNode__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId) 
{
	JSMILE_ENTER;
    DSL_network *net = GetRelatedNetworkPtr(env, obj);
	Java_smile_ValueOfInfo_removeNode__I(env, obj, ValidateNodeId(env, net, nodeId));
    JSMILE_LEAVE;
}

//-------------------------------------------------------------------------------

JNIEXPORT jintArray JNICALL Java_smile_ValueOfInfo_getAllNodes(JNIEnv *env, jobject obj) 
{
    JSMILE_ENTER;
    JSMILE_RETURN(CopyIntArray(env, GetPtr(env, obj)->GetNodes()));
}

JNIEXPORT jobjectArray JNICALL Java_smile_ValueOfInfo_getAllNodeIds(JNIEnv *env, jobject obj) 
{
	JSMILE_ENTER;
    DSL_valueOfInformation *voi = GetPtr(env, obj);
    DSL_network *net = GetRelatedNetworkPtr(env, obj);
    JSMILE_RETURN(HandlesToIds(env, net, voi->GetNodes()));
}

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_ValueOfInfo_setDecision__I(JNIEnv *env, jobject obj, jint nodeHandle) 
{
    JSMILE_ENTER;

	DSL_valueOfInformation *voi = GetPtr(env, obj);
	DSL_network *net = GetRelatedNetworkPtr(env, obj);
	ValidateNodeHandle(net, nodeHandle);
    int res = voi->SetDecision(nodeHandle);
	if (res != DSL_OKAY) 
    {
        string msg = "Cannot set decision on ValueOfInfo object to node ";
        msg += net->GetNode(nodeHandle)->Info().Header().GetId();
        throw runtime_error(msg);
    }

    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_ValueOfInfo_setDecision__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId) 
{
	JSMILE_ENTER;
    DSL_network *net = GetRelatedNetworkPtr(env, obj);
    Java_smile_ValueOfInfo_setDecision__I(env, obj, ValidateNodeId(env, net, nodeId));
    JSMILE_LEAVE;
}

//-------------------------------------------------------------------------------

JNIEXPORT jint JNICALL Java_smile_ValueOfInfo_getDecision(JNIEnv *env, jobject obj) 
{
    JSMILE_ENTER;
    JSMILE_RETURN(GetPtr(env, obj)->GetDecision());
}

JNIEXPORT jstring JNICALL Java_smile_ValueOfInfo_getDecisionId(JNIEnv *env, jobject obj) 
{
	JSMILE_ENTER;
    DSL_valueOfInformation *voi = GetPtr(env, obj);
	DSL_network *net = GetRelatedNetworkPtr(env, obj);
    JSMILE_RETURN(env->NewStringUTF(net->GetNode(voi->GetDecision())->Info().Header().GetId()));
}

//-------------------------------------------------------------------------------

JNIEXPORT jintArray JNICALL Java_smile_ValueOfInfo_getAllDecisions(JNIEnv *env, jobject obj) 
{
    JSMILE_ENTER;
    JSMILE_RETURN(CopyIntArray(env, GetPtr(env, obj)->GetDecisions()));
}

JNIEXPORT jobjectArray JNICALL Java_smile_ValueOfInfo_getAllDecisionIds(JNIEnv *env, jobject obj) 
{
	JSMILE_ENTER;
    DSL_valueOfInformation *voi = GetPtr(env, obj);
    DSL_network *net = GetRelatedNetworkPtr(env, obj);
    JSMILE_RETURN(HandlesToIds(env, net, voi->GetDecisions()));
}

//-------------------------------------------------------------------------------

JNIEXPORT jintArray JNICALL Java_smile_ValueOfInfo_getAllActions(JNIEnv *env, jobject obj) 
{
	JSMILE_ENTER;
    JSMILE_RETURN(CopyIntArray(env, GetPtr(env, obj)->GetActions()));
}

JNIEXPORT jobjectArray JNICALL Java_smile_ValueOfInfo_getAllActionIds(JNIEnv *env, jobject obj) 
{
	JSMILE_ENTER;
	DSL_network *net = GetRelatedNetworkPtr(env, obj);
    JSMILE_RETURN(HandlesToIds(env, net, GetPtr(env, obj)->GetActions()));
}

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_ValueOfInfo_setPointOfView__I(JNIEnv *env, jobject obj, jint nodeHandle) 
{
	JSMILE_ENTER;
	DSL_valueOfInformation *voi = GetPtr(env, obj);
	DSL_network *net = GetRelatedNetworkPtr(env, obj);
	ValidateNodeHandle(net, nodeHandle);
    int res = voi->SetPointOfView(nodeHandle);
	if (res != DSL_OKAY) 
    {
        string msg = "Cannot set point of view on ValueOfInfo object to node ";
        msg += net->GetNode(nodeHandle)->Info().Header().GetId();
        throw runtime_error(msg);
    }
	
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_ValueOfInfo_setPointOfView__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId) 
{
	JSMILE_ENTER;
    DSL_network *net = GetRelatedNetworkPtr(env, obj);
    Java_smile_ValueOfInfo_setPointOfView__I(env, obj, ValidateNodeId(env, net, nodeId));
    JSMILE_LEAVE;
}

//-------------------------------------------------------------------------------

JNIEXPORT jint JNICALL Java_smile_ValueOfInfo_getPointOfView(JNIEnv *env, jobject obj) 
{
	JSMILE_ENTER;
    JSMILE_RETURN(GetPtr(env, obj)->GetPointOfView());
}

JNIEXPORT jstring JNICALL Java_smile_ValueOfInfo_getPointOfViewId(JNIEnv *env, jobject obj) 
{
    JSMILE_ENTER;
	DSL_valueOfInformation *voi = GetPtr(env, obj);
	DSL_network *net = GetRelatedNetworkPtr(env, obj);
	JSMILE_RETURN(env->NewStringUTF(net->GetNode(voi->GetPointOfView())->Info().Header().GetId()));
}

//-------------------------------------------------------------------------------

JNIEXPORT jintArray JNICALL Java_smile_ValueOfInfo_getIndexingNodes(JNIEnv *env, jobject obj) 
{
	JSMILE_ENTER;
    JSMILE_RETURN(CopyIntArray(env, GetPtr(env, obj)->GetIndexingNodes()));
}

JNIEXPORT jobjectArray JNICALL Java_smile_ValueOfInfo_getIndexingNodeIds(JNIEnv *env, jobject obj) 
{
	JSMILE_ENTER;
	DSL_valueOfInformation *voi = GetPtr(env, obj);
	DSL_network *net = GetRelatedNetworkPtr(env, obj);
    JSMILE_RETURN(HandlesToIds(env, net, voi->GetIndexingNodes()));
}

//-------------------------------------------------------------------------------

JNIEXPORT jdoubleArray JNICALL Java_smile_ValueOfInfo_getValues(JNIEnv *env, jobject obj) 
{
	JSMILE_ENTER;
    JSMILE_RETURN(CopyDoubleArray(env, GetPtr(env, obj)->GetValues().GetItems()))
}


} // end extern "C"
