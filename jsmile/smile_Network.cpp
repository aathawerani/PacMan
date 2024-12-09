// smile_Network.cpp

#include "jsmile.h"
#include <cctype>

using namespace std;

int DSL_distributed_EM_helper(DSL_network &, bool, std::vector<std::vector<DSL_Dmatrix *> > &);

//-------------------------------------------------------------------------------

class FieldHelper
{
public:
    FieldHelper(JNIEnv *env, jobject obj)
    {
        this->env = env;
        this->obj = obj;
        cls = env->GetObjectClass(obj);
    }
    
    int GetInt(const char * name) const
    {
        jfieldID fid = env->GetFieldID(cls, name, "I");
        return env->GetIntField(obj, fid);
    }

    double GetDouble(const char * name) const
    {
        jfieldID fid = env->GetFieldID(cls, name, "D");
        return env->GetDoubleField(obj, fid);
    }

private:
    JNIEnv *env;
    jobject obj;
    jclass cls;
};


void ValidateId(JNIEnv *env, jstring jid) 
{
	NativeString id(env, jid);

	bool valid = true;
	int len = int(strlen(id));
	if (0 == len || !isalpha(id[0]))
    {
		// id has to start with a letter
        valid = false;
    }
	else 
    {
		// An id has to contains only letters, digits, or underscores:
        for (int i = 1; i < len; i ++)
        {
            char c = id[i];
            if (!(isalpha(c) || isdigit(c) || '_' == c))
            {
                valid = false;
                break;
            }
        }
	}

	if (!valid) 
    {
		string msg;
		msg = "Identifier '";
		msg += id;
		msg += "' is invalid - it should start with a letter and contain only letters, digits and underscores";
		throw invalid_argument(msg);
	}
}

//-------------------------------------------------------------------------------

// Win32 and SMILE put R component in lowest 8 bits and B in bits 16-23,
// while Java does otherwise. We need to swap the B and R; 
// X == SwapBR(SwapBR(X))
inline int SwapBR(int rgb)
{
    int r = rgb & 0xFF;
    int g = rgb & 0xFF00;
    int b = rgb & 0xFF0000;
    return (r << 16) | g | (b >> 16);
}

jobject GetColor(JNIEnv *env, jobject obj, jint nodeHandle, int (DSL_screenInfo::*field))
{
    DSL_node *node = ValidateNodeHandle(env, obj, nodeHandle);
    int rgb = node->Info().Screen().*field;
    jclass clazz = env->FindClass("java/awt/Color");
    jmethodID mid = env->GetMethodID(clazz, "<init>", "(I)V");
    return env->NewObject(clazz, mid, SwapBR(rgb));
}

void SetColor(JNIEnv *env, jobject obj, jint nodeHandle, int (DSL_screenInfo::*field), jobject color)
{
    DSL_node *node = ValidateNodeHandle(env, obj, nodeHandle);
    jclass clazz = env->GetObjectClass(color);
    jmethodID mid = env->GetMethodID(clazz, "getRGB", "()I");
    int rgb = env->CallIntMethod(color, mid);
    node->Info().Screen().*field = SwapBR(rgb);
}

//-------------------------------------------------------------------------------

void EnsureEvidenceIsSet(DSL_node *node)
{
	if (!node->Value()->IsEvidence()) 
	{
		string msg;
		msg = "Node '";
		msg+= node->Info().Header().GetId();
		msg += "' has no evidence set";
		throw runtime_error(msg);
	}
}

//-------------------------------------------------------------------------------

inline DSL_extraDefinition* GetExtraDef(JNIEnv *env, jobject obj, int handle)
{
    return ValidateNodeHandle(env, obj, handle)->ExtraDefinition();
}

//-------------------------------------------------------------------------------

const DSL_doubleArray* GetTemporalDefinitionArray(DSL_node *node, int order)
{
	DSL_nodeDefinition *nodeDef = node->Definition();
	const DSL_doubleArray *arr = NULL;

	switch (nodeDef->GetType())
	{
	case DSL_NOISY_MAX:
		{
			const DSL_Dmatrix *mtx = static_cast<DSL_noisyMAX *>(nodeDef)->GetTemporalCiWeights(order);
			if (NULL != mtx)
			{
				arr = &mtx->GetItems();
			}
		}
		break;

	default:
		arr = nodeDef->GetTemporalDefinition(order);
		break;
	}

	if (NULL == arr) 
    {
		string msg;
		msg = "Cannot get the temporal definition of node '";
		msg += node->Definition()->GetTypeName();
		msg += "' of node '";
		msg += node->Info().Header().GetId();
		msg += "' as an array of doubles";
		throw runtime_error(msg);
	}

	return arr;
}


void ValidateDefinitionSize(JNIEnv *env, DSL_node * node, const DSL_doubleArray& nativeArr, jdoubleArray arr)
{	
	int arrLength = env->GetArrayLength(arr);
	int nativeLength = nativeArr.GetSize();
	if (arrLength != nativeLength)
	{
		string msg;
		msg = "Invalid definition array size for node '";
		msg += node->Info().Header().GetId();
		msg += "': expected ";
		DSL_appendInt(msg, nativeLength);
		msg += " and got ";
		DSL_appendInt(msg, arrLength);

		if (DSL_NOISY_MAX == node->Definition()->GetType())
		{
			msg += "(the type of this node is NoisyMax -- you need to pass noisy weights as parameter, including constrained columns)";
		}

		throw invalid_argument(msg);
	}
}


DSL_doubleArray *GetDefinitionArray(DSL_node *node) 
{
	DSL_nodeDefinition *nodeDef = node->Definition();
	DSL_doubleArray *arr = NULL;

	switch (nodeDef->GetType())	
    {
	case DSL_NOISY_MAX:
		arr = &static_cast<DSL_noisyMAX *>(nodeDef)->GetCiWeights().GetItems();
		break;
	default:
		nodeDef->GetDefinition(&arr);
		break;
	}

	return arr;
}

template <class T>
T* GetRestrictedDef(JNIEnv *env, jobject obj, int nodeHandle, int typeId, const char *typeName)
{
	DSL_node *node = ValidateNodeHandle(env, obj, nodeHandle);
	DSL_nodeDefinition *nodeDef = node->Definition();
	if (nodeDef->GetType() != typeId) 
    {
		string msg;
		msg = "Node '";
		msg += node->Info().Header().GetId();
		msg += "' must be of type ";
		msg += typeName;
		throw invalid_argument(msg);
	}

	return static_cast<T *>(nodeDef);
}

DSL_noisyMAX* GetNoisyDef(JNIEnv *env, jobject obj, int nodeHandle) 
{
	return GetRestrictedDef<DSL_noisyMAX>(env, obj, nodeHandle, DSL_NOISY_MAX, "NoisyMax");
}

DSL_demorgan* GetDeMorganDef(JNIEnv *env, jobject obj, int nodeHandle)
{
	return GetRestrictedDef<DSL_demorgan>(env, obj, nodeHandle, DSL_DEMORGAN, "DeMorgan");
}

DSL_equation* GetEqDef(JNIEnv *env, jobject obj, int nodeHandle) 
{
	return GetRestrictedDef<DSL_equation>(env, obj, nodeHandle, DSL_EQUATION, "Equation");
}

DSL_mau* GetMauDef(JNIEnv *env, jobject obj, int nodeHandle) 
{
	return GetRestrictedDef<DSL_mau>(env, obj, nodeHandle, DSL_MAU, "MAU");
}


int ValidateParentId(JNIEnv *env, DSL_network *net, int nodeHandle, jstring parentId) 
{
	DSL_node *node = ValidateNodeHandle(net, nodeHandle);
	DSL_intArray &parents = net->GetParents(nodeHandle);

    NativeString native(env, parentId);
    int count = parents.NumItems();
    for (int i = 0; i < count; i ++)
    {
        const char *otherId = net->GetNode(parents[i])->Info().Header().GetId();
        if (0 == strcmp(native, otherId))
        {
            return i;
        }
    }

	string msg;
	msg = "Node '";
	msg += node->Info().Header().GetId();
	msg += "' is not a child of node '";
	msg += native;
	msg += "'";
    throw invalid_argument(msg);
	
	return -1;
}

inline int ValidateParentId(JNIEnv *env, jobject obj, int nodeHandle, jstring parentId)
{
    return ValidateParentId(env, GetNetworkPtr(env, obj), nodeHandle, parentId);
}

int GetDefaultOutcome(JNIEnv *env, jobject obj, int nodeHandle) 
{
    DSL_extraDefinition *extraDef = GetExtraDef(env, obj, nodeHandle);
    return extraDef->IsSetToDefault() ? extraDef->GetDefaultOutcome() : -1;
}


//-------------------------------------------------------------------------------

int FindMainSubmodel(JNIEnv *env, jobject obj)
{
    DSL_submodelHandler &s = GetNetworkPtr(env, obj)->GetSubmodelHandler();
    int res = -1;
    for (int h = s.GetFirstSubmodel(); h >= 0; h = s.GetNextSubmodel(h))
    {
        if (s.IsMainSubmodel(h))
        {
            res = h;
            break;
        }
    }
    if (res < 0)
    {
        throw runtime_error("Main submodel not found");
    }

    return res;
}

DSL_submodel* ValidateSubmodelHandle(DSL_network *net, int handle) 
{
	DSL_submodel *sub = net->GetSubmodelHandler().GetSubmodel(handle);
	if (NULL == sub) 
    {
		string msg;
		msg = "Invalid submodel handle: ";
		DSL_appendInt(msg, handle);
		throw invalid_argument(msg);
	}
	
	return sub;
}

inline DSL_submodel* ValidateSubmodelHandle(JNIEnv *env, jobject obj, int handle)
{
    return ValidateSubmodelHandle(GetNetworkPtr(env, obj), handle);
}

int ValidateSubmodelId(JNIEnv *env, jobject obj, jstring submodelId) 
{
    NativeString native(env, submodelId);
    DSL_network *net = GetNetworkPtr(env, obj);
	
	int handle = net->GetSubmodelHandler().FindSubmodel(native.GetChars());
	if (handle < 0) 
    {
		string msg;
        msg = "Invalid submodel id: '";
		msg += native;
		msg += "'";
		throw invalid_argument(msg);
	}

    return handle;
}

//-------------------------------------------------------------------------------

void SetHeaderField(JNIEnv *env, jstring value, DSL_header &hdr, int (DSL_header::*fxn)(const char *), const char *fxnName)
{
	NativeString native(env, value);
    int res = (hdr.*fxn)(native.GetChars());
    if (DSL_OKAY != res)
    {
        ThrowSmileException(env, fxnName, res);
    }
}

inline void SetHeaderId(JNIEnv *env, jstring id, DSL_header &hdr)
{
    ValidateId(env, id);
    SetHeaderField(env, id, hdr, &DSL_header::SetId, "SetId");
}

inline void SetHeaderName(JNIEnv *env, jstring name, DSL_header &hdr)
{
    SetHeaderField(env, name, hdr, &DSL_header::SetName, "SetName");
}

inline void SetHeaderComment(JNIEnv *env, jstring comment, DSL_header &hdr)
{
    SetHeaderField(env, comment, hdr, &DSL_header::SetComment, "SetComment");
}

//-------------------------------------------------------------------------------

jobjectArray GetDocumentation(JNIEnv *env, DSL_documentation &docs)
{
	int count = docs.GetNumberOfDocuments();
		
	jclass clazz = env->FindClass("smile/DocItemInfo");
	jmethodID mid = env->GetMethodID(clazz, "<init>", "(Ljava/lang/String;Ljava/lang/String;)V");
	jobjectArray arr = env->NewObjectArray(count, clazz, NULL);
		
    for (int i = 0; i < count; i++) 
    {
		jstring title = env->NewStringUTF(docs.GetDocumentTitle(i));
		jstring path = env->NewStringUTF(docs.GetDocumentPath(i));
		jobject dii = env->NewObject(clazz, mid, title, path);
		env->SetObjectArrayElement(arr, i, dii);
	}

    return arr;
}

void SetDocumentation(JNIEnv *env, jobjectArray documentation, DSL_documentation &docs)
{
	docs.DeleteAllDocuments();

    jfieldID fidTitle, fidPath;
    int count = env->GetArrayLength(documentation);
	for (int i = 0; i < count; i++) 
    {
        jobject elem = env->GetObjectArrayElement(documentation, i);
        if (NULL == elem)
        {
            string msg = "Documentation array element name can't be null (index ";
            DSL_appendInt(msg, i);
            msg += ')';
            throw invalid_argument(msg);
        }

        if (0 == i)
        {
            jclass clazz = env->GetObjectClass(elem);
            fidTitle = env->GetFieldID(clazz, "title", "Ljava/lang/String;");
	        fidPath = env->GetFieldID(clazz, "path", "Ljava/lang/String;");
        }

        NativeString title(env, (jstring)env->GetObjectField(elem, fidTitle));
        NativeString path(env, (jstring)env->GetObjectField(elem, fidPath));
		docs.AddDocument(title.GetChars(), path.GetChars());
	}
}

//-------------------------------------------------------------------------------

void SetUserProps(JNIEnv *env, jobjectArray properties, DSL_userProperties &ups) 
{
    DSL_userProperties nativeProps;
    
    jfieldID fidName, fidValue;
    int count = env->GetArrayLength(properties);
	for (int i = 0; i < count; i++) 
    {
		jobject elem = env->GetObjectArrayElement(properties, i);
        if (NULL == elem)
        {
            string msg = "Properties array element name can't be null (index ";
            DSL_appendInt(msg, i);
            msg += ')';
            throw invalid_argument(msg);
        }

        if (0 == i)
        {
	        jclass clsUp = env->GetObjectClass(elem);
            fidName = env->GetFieldID(clsUp, "name", "Ljava/lang/String;");
	        fidValue = env->GetFieldID(clsUp, "value", "Ljava/lang/String;");
        }

        NativeString name(env, (jstring)env->GetObjectField(elem, fidName));
        NativeString value(env, (jstring)env->GetObjectField(elem, fidValue));
        		
		if (nativeProps.FindProperty(name) >= 0) 
        {
			string msg;
			msg = "Duplicate property name: ";
			msg += name;
			throw invalid_argument(msg);
		}
		
		int res = nativeProps.AddProperty(name, value);
		if (DSL_OKAY != res) 
        {
			string msg;
			msg = "Can't add property (";
			msg += name;
            msg += ",";
            msg += value;
            msg += "), error code: ";
            DSL_appendInt(msg, res);
			throw runtime_error(msg);
		}
	}
	
	ups = nativeProps;
}

jobjectArray GetUserProps(JNIEnv *env, DSL_userProperties &nativeProps) 
{
	jclass clsUp = env->FindClass("smile/UserProperty");
	jmethodID midUpInit = env->GetMethodID(clsUp, "<init>", "(Ljava/lang/String;Ljava/lang/String;)V");
	
    int count = nativeProps.GetNumberOfProperties();
	jobjectArray aup = env->NewObjectArray(count, clsUp, NULL);
	for (int i = 0; i < count; i++) 
    {
		jstring name = env->NewStringUTF(nativeProps.GetPropertyName(i));
		jstring value = env->NewStringUTF(nativeProps.GetPropertyValue(i));
		jobject up = env->NewObject(clsUp, midUpInit, name, value);
		env->SetObjectArrayElement(aup, i, up);
	}
	
	return aup;
}

//-------------------------------------------------------------------------------

jobjectArray GetTemporalInfo(JNIEnv *env, DSL_network *net, const vector<pair<int, int> > &native)
{
	jclass clsTi = env->FindClass("smile/TemporalInfo");
	jmethodID midTiInit = env->GetMethodID(clsTi, "<init>", "(ILjava/lang/String;I)V");

	int count = int(native.size());
	jobjectArray ati = env->NewObjectArray(count, clsTi, NULL);
	for (int i = 0; i < count; i ++)
	{
		int handle = native[i].first;
		int order = native[i].second;
		jstring id = env->NewStringUTF(net->GetNode(handle)->Info().Header().GetId());
		jobject ti = env->NewObject(clsTi, midTiInit, handle, id, order);
		env->SetObjectArrayElement(ati, i, ti);
	}

	return ati;
}


//-------------------------------------------------------------------------------

void AddArcHelper(JNIEnv *env, jobject obj, jint parent, jint child, dsl_arcType arcLayer)
{
    ErrorH.Flush();	
    DSL_network *net = GetNetworkPtr(env, obj);
	ValidateNodeHandle(net, parent);
	ValidateNodeHandle(net, child);
	int res = net->AddArc(parent, child, arcLayer);
	if (DSL_OKAY != res) 
    {
        ThrowSmileException(env, "AddArc", res);
	}
}

void DelArcHelper(JNIEnv *env, jobject obj, jint parent, jint child, dsl_arcType arcLayer)
{
	DSL_network *net = GetNetworkPtr(env, obj);
	ValidateNodeHandle(net, parent);
	ValidateNodeHandle(net, child);
	int res = net->RemoveArc(parent, child, arcLayer);
    if (DSL_OKAY != res)
    {
        ThrowSmileException(env, "RemoveArc", res);
    }
}

jintArray GetParentsHelper(JNIEnv *env, jobject obj, jint node, dsl_arcType arcLayer)
{
    DSL_network *net = GetNetworkPtr(env, obj);
	ValidateNodeHandle(env, obj, node);
	return CopyIntArray(env, net->GetParents(node, arcLayer));
}

jintArray GetChildrenHelper(JNIEnv *env, jobject obj, jint node, dsl_arcType arcLayer)
{
    DSL_network *net = GetNetworkPtr(env, obj);
	ValidateNodeHandle(env, obj, node);
	return CopyIntArray(env, net->GetChildren(node, arcLayer));
}

jobjectArray GetParentIdsHelper(JNIEnv *env, jobject obj, jint node, dsl_arcType arcLayer)
{
    DSL_network *net = GetNetworkPtr(env, obj);
	ValidateNodeHandle(env, obj, node);
	return HandlesToIds(env, net, net->GetParents(node, arcLayer));
}

jobjectArray GetChildIdsHelper(JNIEnv *env, jobject obj, jint node, dsl_arcType arcLayer)
{
    DSL_network *net = GetNetworkPtr(env, obj);
	ValidateNodeHandle(env, obj, node);
	return HandlesToIds(env, net, net->GetChildren(node, arcLayer));
}


//-------------------------------------------------------------------------------

jobject GetPosition(JNIEnv *env, const DSL_screenInfo &si)
{
	jclass clazz = env->FindClass("java/awt/Rectangle");
	jmethodID methodInit = env->GetMethodID(clazz, "<init>", "(IIII)V");
		
	const DSL_rectangle &pos = si.position;
	int x = pos.center_X - (pos.width / 2);
	int y = pos.center_Y - (pos.height / 2);
	int width = pos.width;
	int height = pos.height;

	return env->NewObject(clazz, methodInit, x, y, width, height);
}

void SetPosition(DSL_screenInfo &si, int x, int y, int width, int height)
{
	DSL_rectangle &pos = si.position;
	pos.width = width;
	pos.height = height;
	pos.center_X = x + (width / 2);
	pos.center_Y = y + (height / 2);
}

//-------------------------------------------------------------------------------

extern "C" 
{

JSMILE_IMPLEMENT_WRAPPER(Network, DSL_network);

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_Network_readFile(JNIEnv *env, jobject obj, jstring fileName) 
{
    JSMILE_ENTER;
    
	ErrorH.Flush();
    DSL_network *net = GetPtr(env, obj);
	NativeString nativeFileName(env, fileName);
	DefLocaleCtx dlc;
	int res = net->ReadFile(nativeFileName);
	if (DSL_OKAY != res)
    {
        ThrowSmileException(env, "ReadFile", res);
    }
	
    JSMILE_LEAVE;
}

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_Network_readString(JNIEnv *env, jobject obj, jstring xdslContent) 
{
    JSMILE_ENTER;
    
	ErrorH.Flush();
    DSL_network *net = GetPtr(env, obj);
	NativeString nativeXdslContent(env, xdslContent);
	DefLocaleCtx dlc;
	int res = net->ReadString(nativeXdslContent);
	if (DSL_OKAY != res)
    {
        ThrowSmileException(env, "ReadString", res);
    }
	
    JSMILE_LEAVE;
}

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_Network_writeFile(JNIEnv *env, jobject obj, jstring fileName) 
{
	JSMILE_ENTER;

	ErrorH.Flush();
    DSL_network *net = GetPtr(env, obj);
	NativeString nativeFileName(env, fileName);
	DefLocaleCtx dlc;
	int res = net->WriteFile(nativeFileName);
	if (DSL_OKAY != res)
    {
        ThrowSmileException(env, "WriteFile", res);
    }

    JSMILE_LEAVE;
}

//-------------------------------------------------------------------------------

JNIEXPORT jstring JNICALL Java_smile_Network_writeString(JNIEnv *env, jobject obj) 
{
	JSMILE_ENTER;

	ErrorH.Flush();
    DSL_network *net = GetPtr(env, obj);
	DefLocaleCtx dlc;
	string xdslOutput;
	int res = net->WriteString(xdslOutput);
	if (DSL_OKAY != res)
    {
        ThrowSmileException(env, "WriteStromg", res);
    }
	JSMILE_RETURN(env->NewStringUTF(xdslOutput.c_str()));
}

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_Network_updateBeliefs(JNIEnv *env, jobject obj) 
{
	JSMILE_ENTER;
    
    ErrorH.Flush();
	DSL_network *net = GetPtr(env, obj);
    int res = net->UpdateBeliefs();
	if (DSL_OKAY != res)
    {
        ThrowSmileException(env, "UpdateBeliefs", res);
    }

    JSMILE_LEAVE;
}

JNIEXPORT jdouble JNICALL Java_smile_Network_probEvidence(JNIEnv *env, jobject obj)
{
    JSMILE_ENTER;
    double pe = 0;
    if (!GetPtr(env, obj)->CalcProbEvidence(pe))
    {
        ThrowSmileException(env, "CalcPropEvidence", -1);
    }
    JSMILE_RETURN(pe);
}

//-------------------------------------------------------------------------------

JNIEXPORT jobject JNICALL Java_smile_Network_annealedMap___3ILsmile_AnnealedMapTuning_2(JNIEnv *env, jobject obj, jintArray mapNodes, jobject tuning)
{
    JSMILE_ENTER;
    DSL_network *net = GetPtr(env, obj);
    int seed = 0;
    
    if (NULL != tuning)
    {
        FieldHelper h(env, tuning);
        
        DSL_AnnealedMAPParams n;
        n.speed = h.GetDouble("speed");
	    n.Tmin = h.GetDouble("Tmin");
	    n.Tinit = h.GetDouble("Tinit");
	    n.kReheat = h.GetDouble("kReheat");
	    n.kMAP = h.GetInt("kMAP");
	    n.kRFC = h.GetDouble("kRFC");
	    n.numCycle = h.GetInt("numCycle");
	    n.iReheatSteps = h.GetInt("iReheatSteps");
	    n.iStopSteps = h.GetInt("iStopSteps");
        net->SetAnnealedMAPParams(n);
        
        seed = h.GetInt("randSeed");
    }

    vector<pair<int, int> > ev;
    ev.reserve(32);
    for (int h = net->GetFirstNode(); h >= 0; h = net->GetNextNode(h))
    {
        DSL_nodeValue *v = net->GetNode(h)->Value();
        if (v->IsEvidence())
        {
            ev.push_back(make_pair(h, v->GetEvidence()));
        }
    }
    
	int mapNodeCount = env->GetArrayLength(mapNodes);
    vector<int> nativeMapNodes(mapNodeCount);
    env->GetIntArrayRegion(mapNodes, 0, mapNodeCount, (jint *)(&nativeMapNodes.front()));

    vector<int> nativeMapStates;
    double probM1E, probE;
    int res = net->AnnealedMAP(ev, nativeMapNodes, nativeMapStates, probM1E, probE, seed);
    if (DSL_OKAY != res)
    {
        ThrowSmileException(env, "AnnealedMAP", res);
    }

    jclass clsRes = env->FindClass("smile/AnnealedMapResults");
    jmethodID midRes = env->GetMethodID(clsRes, "<init>", "(DD[I)V");
    jintArray mapStates = env->NewIntArray(mapNodeCount);
    env->SetIntArrayRegion(mapStates, 0, mapNodeCount, (jint *)(&nativeMapStates.front()));
    
    JSMILE_RETURN(env->NewObject(clsRes, midRes, probM1E, probE, mapStates));
}

JNIEXPORT jobject JNICALL Java_smile_Network_annealedMap___3Ljava_lang_String_2Lsmile_AnnealedMapTuning_2(JNIEnv *env, jobject obj, jobjectArray mapNodes, jobject tuning)
{
    JSMILE_ENTER;
    
    DSL_network *net = GetPtr(env, obj);

    int count = env->GetArrayLength(mapNodes);
    jintArray handles = env->NewIntArray(count);
	for (int i = 0; i < count; i ++)
    {
        jstring id = jstring(env->GetObjectArrayElement(mapNodes, i));
        jint elem = ValidateNodeId(env, net, id);
        env->SetIntArrayRegion(handles, i, 1, &elem);
    }
    
    JSMILE_RETURN(Java_smile_Network_annealedMap___3ILsmile_AnnealedMapTuning_2(env, obj, handles, tuning));
}

//-------------------------------------------------------------------------------

JNIEXPORT int JNICALL Java_smile_Network_getSampleCount(JNIEnv *env, jobject obj) 
{
    JSMILE_ENTER;
    JSMILE_RETURN(GetPtr(env, obj)->GetNumberOfSamples());
}

JNIEXPORT void JNICALL Java_smile_Network_setSampleCount(JNIEnv *env, jobject obj, int sampleCount) 
{
    JSMILE_ENTER;
    GetPtr(env, obj)->SetNumberOfSamples(sampleCount);
    JSMILE_LEAVE;
}

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_Network_setId(JNIEnv *env, jobject obj, jstring networkId) 
{
    JSMILE_ENTER;	
    SetHeaderId(env, networkId, GetPtr(env, obj)->Header());
    JSMILE_LEAVE;
}

JNIEXPORT jstring JNICALL Java_smile_Network_getId(JNIEnv *env, jobject obj) 
{
    JSMILE_ENTER;
    JSMILE_RETURN(env->NewStringUTF(GetPtr(env, obj)->Header().GetId()));
}

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_Network_setName(JNIEnv *env, jobject obj, jstring name) 
{
    JSMILE_ENTER;	
    SetHeaderName(env, name, GetPtr(env, obj)->Header());
    JSMILE_LEAVE;
}

JNIEXPORT jstring JNICALL Java_smile_Network_getName(JNIEnv *env, jobject obj) 
{
    JSMILE_ENTER;
    JSMILE_RETURN(env->NewStringUTF(GetPtr(env, obj)->Header().GetName()));
}

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_Network_setDescription(JNIEnv *env, jobject obj, jstring description)	
{
    JSMILE_ENTER;	
 	SetHeaderComment(env, description, GetPtr(env, obj)->Header());
    JSMILE_LEAVE;
}

JNIEXPORT jstring JNICALL Java_smile_Network_getDescription(JNIEnv *env, jobject obj) 
{
    JSMILE_ENTER;
    JSMILE_RETURN(env->NewStringUTF(GetPtr(env, obj)->Header().GetComment()));
}

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_Network_setBayesianAlgorithm(JNIEnv *env, jobject obj, jint algorithmType) 
{
    JSMILE_ENTER;
    GetPtr(env, obj)->SetDefaultBNAlgorithm(algorithmType);
    JSMILE_LEAVE;
}

JNIEXPORT jint JNICALL Java_smile_Network_getBayesianAlgorithm(JNIEnv *env, jobject obj) 
{
    JSMILE_ENTER;
    JSMILE_RETURN(GetPtr(env, obj)->GetDefaultBNAlgorithm());
}

JNIEXPORT void JNICALL Java_smile_Network_setInfluenceDiagramAlgorithm(JNIEnv *env, jobject obj, jint algorithmType) 
{
    JSMILE_ENTER;
    GetPtr(env, obj)->SetDefaultIDAlgorithm(algorithmType);
    JSMILE_LEAVE;
}

JNIEXPORT jint JNICALL Java_smile_Network_getInfluenceDiagramAlgorithm(JNIEnv *env, jobject obj) 
{
    JSMILE_ENTER;
    JSMILE_RETURN(GetPtr(env, obj)->GetDefaultIDAlgorithm());
}

//-------------------------------------------------------------------------------

JNIEXPORT jint JNICALL Java_smile_Network_addNode__ILjava_lang_String_2(JNIEnv *env, jobject obj, jint nodeType, jstring nodeId) 
{
	JSMILE_ENTER;
    DSL_network *net = GetPtr(env, obj);
    int handle;
    if (NULL == nodeId)
    {
        handle = net->AddNode(nodeType, NULL);
    }
    else
    {
        ValidateId(env, nodeId);
        NativeString id(env, nodeId);
		handle = net->AddNode(nodeType, const_cast<char *>(static_cast<const char *>(id)));
    }
	
    if (handle < 0)
    {
        ThrowSmileException(env, "AddNode", handle);
    }
	
    JSMILE_RETURN(handle);
}

JNIEXPORT jint JNICALL Java_smile_Network_addNode__I(JNIEnv *env, jobject obj, jint nodeType) 
{
    return Java_smile_Network_addNode__ILjava_lang_String_2(env, obj, nodeType, NULL);
}

//-------------------------------------------------------------------------------

JNIEXPORT jint JNICALL Java_smile_Network_getNodeType__I(JNIEnv *env, jobject obj, jint nodeHandle) 
{
    JSMILE_ENTER;
	DSL_node *node = ValidateNodeHandle(env, obj, nodeHandle);
	JSMILE_RETURN(node->Definition()->GetType());
}

JNIEXPORT jint JNICALL Java_smile_Network_getNodeType__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId) 
{
    JSMILE_ENTER;
    JSMILE_RETURN(Java_smile_Network_getNodeType__I(env, obj, ValidateNodeId(env, obj, nodeId)));
}

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_Network_setNodeType__II(JNIEnv *env, jobject obj, jint nodeHandle, jint type)
{
	JSMILE_ENTER;
	DSL_node *node = ValidateNodeHandle(env, obj, nodeHandle);
    int res = node->ChangeType(type);
    if (DSL_OKAY != res)
    {
        ThrowSmileException(env, "setNodeType", res);
    }
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_setNodeType__Ljava_lang_String_2I(JNIEnv *env, jobject obj, jstring nodeId, jint type)
{
	JSMILE_ENTER;
    Java_smile_Network_setNodeType__II(env, obj, ValidateNodeId(env, obj, nodeId), type);
    JSMILE_LEAVE;
}

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_Network_deleteNode__I(JNIEnv *env, jobject obj, jint nodeHandle) 
{
    JSMILE_ENTER;
    DSL_network *net = GetPtr(env, obj);
	ValidateNodeHandle(net, nodeHandle);
	net->DeleteNode(nodeHandle);
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_deleteNode__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId) 
{
    JSMILE_ENTER;
    Java_smile_Network_deleteNode__I(env, obj, ValidateNodeId(env, obj, nodeId));
    JSMILE_LEAVE;
}

//-------------------------------------------------------------------------------

JNIEXPORT jint JNICALL Java_smile_Network_getNodeCount(JNIEnv *env, jobject obj) 
{
    JSMILE_ENTER;
    JSMILE_RETURN(GetPtr(env, obj)->GetNumberOfNodes());
}

//-------------------------------------------------------------------------------

JNIEXPORT jint JNICALL Java_smile_Network_getFirstNode(JNIEnv *env, jobject obj) 
{
	JSMILE_ENTER;
	int nodeHandle = GetPtr(env, obj)->GetFirstNode();
    if (nodeHandle < 0) nodeHandle = -1;
    JSMILE_RETURN(nodeHandle);
}

JNIEXPORT jint JNICALL Java_smile_Network_getNextNode(JNIEnv *env, jobject obj, jint nodeHandle) 
{
    JSMILE_ENTER;
    int nextNodeHandle =  GetPtr(env, obj)->GetNextNode(nodeHandle);
    if (nextNodeHandle < 0) nextNodeHandle = -1;
	JSMILE_RETURN(nextNodeHandle);
}

//-------------------------------------------------------------------------------

JNIEXPORT jint JNICALL Java_smile_Network_getNode(JNIEnv *env, jobject obj, jstring nodeId) 
{
	JSMILE_ENTER;
    JSMILE_RETURN(ValidateNodeId(env, obj, nodeId));
}

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_Network_setNodeId__ILjava_lang_String_2(JNIEnv *env, jobject obj, jint nodeHandle, jstring id) 
{
	JSMILE_ENTER;
	DSL_node *node = ValidateNodeHandle(env, obj, nodeHandle);
    SetHeaderId(env, id, node->Info().Header());
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_setNodeId__Ljava_lang_String_2Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring oldId, jstring newId) 
{
	JSMILE_ENTER;
    Java_smile_Network_setNodeId__ILjava_lang_String_2(env, obj, ValidateNodeId(env, obj, oldId), newId);
    JSMILE_LEAVE;
}

JNIEXPORT jstring JNICALL Java_smile_Network_getNodeId(JNIEnv *env, jobject obj, jint nodeHandle) 
{
	JSMILE_ENTER;
    DSL_node *node = ValidateNodeHandle(env, obj, nodeHandle);
    JSMILE_RETURN(env->NewStringUTF(node->Info().Header().GetId()));
}

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_Network_setNodeName__ILjava_lang_String_2(JNIEnv *env, jobject obj, jint nodeHandle, jstring name) 
{
	JSMILE_ENTER;
    DSL_node *node = ValidateNodeHandle(env, obj, nodeHandle);
    SetHeaderName(env, name, node->Info().Header());
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_setNodeName__Ljava_lang_String_2Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId, jstring name) 
{
    JSMILE_ENTER;
    Java_smile_Network_setNodeName__ILjava_lang_String_2(env, obj, ValidateNodeId(env, obj, nodeId), name);
    JSMILE_LEAVE;
}

JNIEXPORT jstring JNICALL Java_smile_Network_getNodeName__I(JNIEnv *env, jobject obj, jint nodeHandle) 
{
	JSMILE_ENTER;
    DSL_node *node = ValidateNodeHandle(env, obj, nodeHandle);
    JSMILE_RETURN(env->NewStringUTF(node->Info().Header().GetName()));
}

JNIEXPORT jstring JNICALL Java_smile_Network_getNodeName__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId) 
{
    JSMILE_ENTER;
    JSMILE_RETURN(Java_smile_Network_getNodeName__I(env, obj, ValidateNodeId(env, obj, nodeId)));
}

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_Network_setNodeDescription__ILjava_lang_String_2(JNIEnv *env, jobject obj, jint nodeHandle, jstring description) 
{
    JSMILE_ENTER;
    DSL_node *node = ValidateNodeHandle(env, obj, nodeHandle);
    SetHeaderComment(env, description, node->Info().Header());
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_setNodeDescription__Ljava_lang_String_2Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId, jstring description) 
{
    JSMILE_ENTER;
    Java_smile_Network_setNodeDescription__ILjava_lang_String_2(env, obj, ValidateNodeId(env, obj, nodeId), description);
    JSMILE_LEAVE;
}

JNIEXPORT jstring JNICALL Java_smile_Network_getNodeDescription__I(JNIEnv *env, jobject obj, jint nodeHandle) 
{
	JSMILE_ENTER;
    DSL_node *node = ValidateNodeHandle(env, obj, nodeHandle);
    JSMILE_RETURN(env->NewStringUTF(node->Info().Header().GetComment()));
}

JNIEXPORT jstring JNICALL Java_smile_Network_getNodeDescription__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId) 
{
    JSMILE_ENTER;
    JSMILE_RETURN(Java_smile_Network_getNodeDescription__I(env, obj, ValidateNodeId(env, obj, nodeId)));
}

//-------------------------------------------------------------------------------

JNIEXPORT jintArray JNICALL Java_smile_Network_getAllNodes(JNIEnv *env, jobject obj) 
{
    JSMILE_ENTER;
	DSL_network *net = GetPtr(env, obj);
	DSL_intArray nodes;
	net->GetAllNodes(nodes);
    JSMILE_RETURN(CopyIntArray(env, nodes));
}

JNIEXPORT jobjectArray JNICALL Java_smile_Network_getAllNodeIds(JNIEnv *env, jobject obj) 
{
    JSMILE_ENTER;
	DSL_network *net = GetPtr(env, obj);
	DSL_intArray nodes;
	net->GetAllNodes(nodes);
    JSMILE_RETURN(HandlesToIds(env, net, nodes));
}

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_Network_addArc__II(JNIEnv *env, jobject obj, jint parentHandle, jint childHandle) 
{
    JSMILE_ENTER;
    AddArcHelper(env, obj, parentHandle, childHandle, dsl_normalArc);
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_addArc__Ljava_lang_String_2Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring parentId, jstring childId) 
{
	JSMILE_ENTER;
    AddArcHelper(env, obj, ValidateNodeId(env, obj, parentId), ValidateNodeId(env, obj, childId), dsl_normalArc);
    JSMILE_LEAVE;
}

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_Network_deleteArc__II(JNIEnv *env, jobject obj, jint parentHandle, jint childHandle)
{
	JSMILE_ENTER;
    DelArcHelper(env, obj, parentHandle, childHandle, dsl_normalArc);
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_deleteArc__Ljava_lang_String_2Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring parentId, jstring childId) 
{
    JSMILE_ENTER;
    DelArcHelper(env, obj, ValidateNodeId(env, obj, parentId), ValidateNodeId(env, obj, childId), dsl_normalArc);
    JSMILE_LEAVE;
}

//-------------------------------------------------------------------------------

JNIEXPORT jint JNICALL Java_smile_Network_getOutcomeCount__I(JNIEnv *env, jobject obj, jint nodeHandle) 
{
	JSMILE_ENTER;
    DSL_node *node = ValidateNodeHandle(env, obj, nodeHandle);
    JSMILE_RETURN(node->Definition()->GetNumberOfOutcomes());
}

JNIEXPORT jint JNICALL Java_smile_Network_getOutcomeCount__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId) 
{
	JSMILE_ENTER;
    JSMILE_RETURN(Java_smile_Network_getOutcomeCount__I(env, obj, ValidateNodeId(env, obj, nodeId)));
}

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_Network_addOutcome__ILjava_lang_String_2(JNIEnv *env, jobject obj, jint nodeHandle, jstring outcomeId) 
{
	JSMILE_ENTER;
    ValidateId(env, outcomeId);
    DSL_node *node = ValidateNodeHandle(env, obj, nodeHandle);
    NativeString native(env, outcomeId);
    int res = node->Definition()->AddOutcome(native.GetChars());
    if (DSL_OKAY != res)
    {
        ThrowSmileException(env, "AddOutcome", res);
    }
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_addOutcome__Ljava_lang_String_2Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId, jstring outcomeId) 
{
    JSMILE_ENTER;
    Java_smile_Network_addOutcome__ILjava_lang_String_2(env, obj, ValidateNodeId(env, obj, nodeId), outcomeId);
    JSMILE_LEAVE;
}

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_Network_insertOutcome__IILjava_lang_String_2(JNIEnv *env, jobject obj, jint nodeHandle, jint position, jstring outcomeId) 
{
	JSMILE_ENTER;
    ValidateId(env, outcomeId);
    DSL_node *node = ValidateNodeHandle(env, obj, nodeHandle);
    NativeString native(env, outcomeId);
    int res = node->Definition()->InsertOutcome(position, native.GetChars());
    if (DSL_OKAY != res)
    {
        ThrowSmileException(env, "InsertOutcome", res);
    }
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_insertOutcome__Ljava_lang_String_2ILjava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId, jint position, jstring outcomeId) 
{
    JSMILE_ENTER;
    Java_smile_Network_insertOutcome__IILjava_lang_String_2(env, obj, ValidateNodeId(env, obj, nodeId), position, outcomeId);
    JSMILE_LEAVE;
}

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_Network_deleteOutcome__II(JNIEnv *env, jobject obj, jint nodeHandle, jint outcomeIndex) 
{
    JSMILE_ENTER;
    DSL_node *node = ValidateOutcomeIndex(env, obj, nodeHandle, outcomeIndex);
	int res = node->Definition()->RemoveOutcome(outcomeIndex);
    if (DSL_OKAY != res)
    {
        ThrowSmileException(env, "RemoveOutcome", res);
    }
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_deleteOutcome__Ljava_lang_String_2I(JNIEnv *env, jobject obj, jstring nodeId, jint outcomeIndex) 
{
    JSMILE_ENTER;
    Java_smile_Network_deleteOutcome__II(env, obj, ValidateNodeId(env, obj, nodeId), outcomeIndex);
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_deleteOutcome__ILjava_lang_String_2(JNIEnv *env, jobject obj, jint nodeHandle, jstring outcomeId) 
{
    JSMILE_ENTER;
    Java_smile_Network_deleteOutcome__II(env, obj, nodeHandle, ValidateOutcomeId(env, obj, nodeHandle, outcomeId));
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_deleteOutcome__Ljava_lang_String_2Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId, jstring outcomeId) 
{
    JSMILE_ENTER;
    Java_smile_Network_deleteOutcome__ILjava_lang_String_2(env, obj, ValidateNodeId(env, obj, nodeId), outcomeId);
    JSMILE_LEAVE;
}

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_Network_setOutcomeId__IILjava_lang_String_2(JNIEnv *env, jobject obj, jint nodeHandle, jint outcomeIndex, jstring id) 
{
	JSMILE_ENTER;
    DSL_node *node = ValidateOutcomeIndex(env, obj, nodeHandle, outcomeIndex);
    NativeString native(env, id);
    int res = node->Definition()->RenameOutcome(outcomeIndex, native.GetChars());
    if (DSL_OKAY != res)
    {
        ThrowSmileException(env, "RenameOutcome", res);
    }
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_setOutcomeId__Ljava_lang_String_2ILjava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId, jint outcomeIndex, jstring id) 
{
	JSMILE_ENTER;
    Java_smile_Network_setOutcomeId__IILjava_lang_String_2(env, obj, ValidateNodeId(env, obj, nodeId), outcomeIndex, id);
    JSMILE_LEAVE;
}

//-------------------------------------------------------------------------------

JNIEXPORT jstring JNICALL Java_smile_Network_getOutcomeId__II(JNIEnv *env, jobject obj, jint nodeHandle, jint outcomeIndex) 
{
    JSMILE_ENTER;
    DSL_node *node = ValidateOutcomeIndex(env, obj, nodeHandle, outcomeIndex);
    const char *id = node->Definition()->GetOutcomesNames()->Subscript(outcomeIndex);
    JSMILE_RETURN(env->NewStringUTF(id));
}

JNIEXPORT jstring JNICALL Java_smile_Network_getOutcomeId__Ljava_lang_String_2I(JNIEnv *env, jobject obj, jstring nodeId, jint outcomeIndex) 
{
	JSMILE_ENTER;	
    JSMILE_RETURN(Java_smile_Network_getOutcomeId__II(env, obj, ValidateNodeId(env, obj, nodeId), outcomeIndex));
}

//-------------------------------------------------------------------------------

JNIEXPORT jobjectArray JNICALL Java_smile_Network_getOutcomeIds__I(JNIEnv *env, jobject obj, jint nodeHandle) 
{
    JSMILE_ENTER;
    DSL_idArray* native = ValidateNodeHandle(env, obj, nodeHandle)->Definition()->GetOutcomesNames();
    JSMILE_RETURN(CopyStringArray(env, *native));
}

JNIEXPORT jobjectArray JNICALL Java_smile_Network_getOutcomeIds__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId) 
{
    JSMILE_ENTER;
    JSMILE_RETURN(Java_smile_Network_getOutcomeIds__I(env, obj, ValidateNodeId(env, obj, nodeId)));
}

//-------------------------------------------------------------------------------

JNIEXPORT jintArray JNICALL Java_smile_Network_getParents__I(JNIEnv *env, jobject obj, jint nodeHandle) 
{
	JSMILE_ENTER;
    JSMILE_RETURN(GetParentsHelper(env, obj, nodeHandle, dsl_normalArc));
}

JNIEXPORT jintArray JNICALL Java_smile_Network_getParents__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId) 
{
	JSMILE_ENTER;
    JSMILE_RETURN(Java_smile_Network_getParents__I(env, obj, ValidateNodeId(env, obj, nodeId)));
}

JNIEXPORT jobjectArray JNICALL Java_smile_Network_getParentIds__I(JNIEnv *env, jobject obj, jint nodeHandle) 
{
    JSMILE_ENTER;
    JSMILE_RETURN(GetParentIdsHelper(env, obj, nodeHandle, dsl_normalArc));
}

JNIEXPORT jobjectArray JNICALL Java_smile_Network_getParentIds__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId) 
{
	JSMILE_ENTER;
    JSMILE_RETURN(Java_smile_Network_getParentIds__I(env, obj, ValidateNodeId(env, obj, nodeId)));
}

JNIEXPORT jintArray JNICALL Java_smile_Network_getChildren__I(JNIEnv *env, jobject obj, jint nodeHandle) 
{
	JSMILE_ENTER;
    JSMILE_RETURN(GetChildrenHelper(env, obj, nodeHandle, dsl_normalArc));
}

JNIEXPORT jintArray JNICALL Java_smile_Network_getChildren__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId) 
{
	JSMILE_ENTER;
    JSMILE_RETURN(Java_smile_Network_getChildren__I(env, obj, ValidateNodeId(env, obj, nodeId)));
}

JNIEXPORT jobjectArray JNICALL Java_smile_Network_getChildIds__I(JNIEnv *env, jobject obj, jint nodeHandle) 
{
    JSMILE_ENTER;
    JSMILE_RETURN(GetChildIdsHelper(env, obj, nodeHandle, dsl_normalArc));
}

JNIEXPORT jobjectArray JNICALL Java_smile_Network_getChildIds__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId) 
{
	JSMILE_ENTER;
    JSMILE_RETURN(Java_smile_Network_getChildIds__I(env, obj, ValidateNodeId(env, obj, nodeId)));
}

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_Network_setNodeDefinition__I_3D(JNIEnv *env, jobject obj, jint nodeHandle, jdoubleArray definition) 
{
	JSMILE_ENTER;
    
	DSL_node *node = ValidateNodeHandle(env, obj, nodeHandle);
	DSL_nodeDefinition *nodeDef = node->Definition();
	DSL_doubleArray *arrDef = GetDefinitionArray(node);
	
	if (NULL == arrDef) 
    {
		string msg;
		msg = "Cannot get node definition '";
		msg += node->Definition()->GetTypeName();
		msg += "' of node '";
		msg += node->Info().Header().GetId();
		msg += "' as an array of doubles";
		throw invalid_argument(msg);
	}

	bool isNoisy = (nodeDef->GetType() == DSL_NOISY_MAX);
	int imax = arrDef->GetSize();
	if (env->GetArrayLength(definition) != imax) 
    {
		string msg;
		msg = "Invalid definition array size for node '";
		msg += node->Info().Header().GetId();
		msg += "': expected ";
		DSL_appendInt(msg, imax);
		msg += " and got ";
		DSL_appendInt(msg, env->GetArrayLength(definition));

		if (isNoisy)
        {
			msg += "(the type of this node is NoisyMax -- you need to pass noisy weights as parameter, including constrained columns)";
        }

		throw invalid_argument(msg);
	}

	DSL_nodeValue *nodeValue = node->Value();
	if (NULL != nodeValue) nodeValue->SetValueInvalid();
	
	env->GetDoubleArrayRegion(definition, 0, imax, arrDef->Items());

	// special care needed for noisyMAX nodes -- they need to update their CPTs:
	if (isNoisy) static_cast<DSL_noisyMAX *>(nodeDef)->CiToCpt();

    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_setNodeDefinition__Ljava_lang_String_2_3D(JNIEnv *env, jobject obj, jstring nodeId, jdoubleArray definition) 
{
	JSMILE_ENTER;
    Java_smile_Network_setNodeDefinition__I_3D(env, obj, ValidateNodeId(env, obj, nodeId), definition);
    JSMILE_LEAVE;
}

//-------------------------------------------------------------------------------

JNIEXPORT jdoubleArray JNICALL Java_smile_Network_getNodeValue__I(JNIEnv *env, jobject obj, jint nodeHandle) 
{
	JSMILE_ENTER;

	DSL_node *node = ValidateNodeHandle(env, obj, nodeHandle);
	DSL_nodeValue *nodeValue = node->Value();

	if (!nodeValue->IsValueValid()) 
    {
		string msg;
		msg = "Value not valid - node '";
		msg += node->Info().Header().GetId();
		msg += "'";
		throw runtime_error(msg);
	}

	jdoubleArray nodeValueArray;
	if (node->Value()->GetType() == DSL_EQUATIONEVALUATION)
	{
		DSL_valEqEvaluation *eqValue = static_cast<DSL_valEqEvaluation *>(nodeValue);
		double p[2];
		p[0] = eqValue->GetSampleMean();
		p[1] = eqValue->GetSampleStdDev();
		if (0 == p[1])
		{
			nodeValueArray = env->NewDoubleArray(2);
			env->SetDoubleArrayRegion(nodeValueArray, 0, 2, p);
		}
		else
		{
			const vector<pair<double, double> > &samples = eqValue->GetSamples();
			int sampleCount = int(samples.size());
			nodeValueArray = env->NewDoubleArray(2 + 2 * sampleCount);
			env->SetDoubleArrayRegion(nodeValueArray, 0, 2, p);
			for (int i = 0; i < sampleCount; i ++)
			{
				p[0] = samples[i].first;
				p[1] = samples[i].second;
				env->SetDoubleArrayRegion(nodeValueArray, 2 + 2 * i, 2, p);
			}
		}
	}
	else
	{
		DSL_Dmatrix *m = NULL;
		int res = nodeValue->GetValue(&m);
		if (DSL_OKAY != res)  
		{
			ThrowSmileException(env, "GetValue", res);
		}
		nodeValueArray = CopyDoubleArray(env, m->GetItems());
	}

	JSMILE_RETURN(nodeValueArray);
}

JNIEXPORT jdoubleArray JNICALL Java_smile_Network_getNodeValue__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId) 
{
    JSMILE_ENTER;
    JSMILE_RETURN(Java_smile_Network_getNodeValue__I(env, obj, ValidateNodeId(env, obj, nodeId)));
}

//-------------------------------------------------------------------------------

JNIEXPORT jdoubleArray JNICALL Java_smile_Network_getNodeDefinition__I(JNIEnv *env, jobject obj, jint nodeHandle) 
{
	JSMILE_ENTER;

	DSL_node *node = ValidateNodeHandle(env, obj, nodeHandle);
	DSL_doubleArray *arrDef = GetDefinitionArray(node);
	
	if (NULL == arrDef) 
    {
		string msg;
		msg = "Cannot get node definition '";
		msg += node->Definition()->GetTypeName();
		msg += "' of node '";
		msg += node->Info().Header().GetId();
		msg += "' as an array of doubles";
		throw runtime_error(msg);
	}
	
    JSMILE_RETURN(CopyDoubleArray(env, *arrDef));
}

JNIEXPORT jdoubleArray JNICALL Java_smile_Network_getNodeDefinition__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId) 
{
    JSMILE_ENTER;
	JSMILE_RETURN(Java_smile_Network_getNodeDefinition__I(env, obj, ValidateNodeId(env, obj, nodeId)));
}

//-------------------------------------------------------------------------------

JNIEXPORT jboolean JNICALL Java_smile_Network_isValueValid__I(JNIEnv *env, jobject obj, jint nodeHandle) 
{
    JSMILE_ENTER;
    DSL_node *node = ValidateNodeHandle(env, obj, nodeHandle);
    JSMILE_RETURN(0 != node->Value()->IsValueValid());
}

JNIEXPORT jboolean JNICALL Java_smile_Network_isValueValid__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId) 
{
    JSMILE_ENTER;
    JSMILE_RETURN(Java_smile_Network_isValueValid__I(env, obj, ValidateNodeId(env, obj, nodeId)));
}

//-------------------------------------------------------------------------------

JNIEXPORT jintArray JNICALL Java_smile_Network_getValueIndexingParents__I(JNIEnv *env, jobject obj, jint nodeHandle) 
{
	JSMILE_ENTER;
    DSL_node *node = ValidateNodeHandle(env, obj, nodeHandle);
    JSMILE_RETURN(CopyIntArray(env, node->Value()->GetIndexingParents()));
}

JNIEXPORT jintArray JNICALL Java_smile_Network_getValueIndexingParents__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId) 
{
	JSMILE_ENTER;
    JSMILE_RETURN(Java_smile_Network_getValueIndexingParents__I(env, obj, ValidateNodeId(env, obj, nodeId)));
}

//-------------------------------------------------------------------------------

JNIEXPORT jobjectArray JNICALL Java_smile_Network_getValueIndexingParentIds__I(JNIEnv *env, jobject obj, jint nodeHandle) 
{
	JSMILE_ENTER;
    DSL_network *net = GetPtr(env, obj);
    DSL_node *node = ValidateNodeHandle(net, nodeHandle);
    JSMILE_RETURN(HandlesToIds(env, net, node->Value()->GetIndexingParents()));
}

JNIEXPORT jobjectArray JNICALL Java_smile_Network_getValueIndexingParentIds__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId) 
{
	JSMILE_ENTER;
    JSMILE_RETURN(Java_smile_Network_getValueIndexingParentIds__I(env, obj, ValidateNodeId(env, obj, nodeId)));
}

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_Network_setNoisyParentStrengths__II_3I(JNIEnv *env, jobject obj, jint nodeHandle, jint parentIndex, jintArray strengths) 
{
	JSMILE_ENTER;

	DSL_noisyMAX *noisyDef = GetNoisyDef(env, obj, nodeHandle);
	
	int imax = noisyDef->GetNumOfParentOutcomes(parentIndex);
	if (env->GetArrayLength(strengths) != imax) 
    {
		string msg;
		msg = "Invalid parent strength array size for node '";
		msg += GetPtr(env, obj)->GetNode(nodeHandle)->Info().Header().GetId();
		msg += "': expected ";
		DSL_appendInt(msg, imax);
		msg += " and got ";
		DSL_appendInt(msg, env->GetArrayLength(strengths));
		throw invalid_argument(msg);
	}

	DSL_intArray &nativeStrengths = noisyDef->GetParentOutcomeStrengths(parentIndex);
	env->GetIntArrayRegion(strengths, 0, imax, (jint *)(nativeStrengths.Items()));
	for (int i = 0; i < imax; i++) 
    {
        jint j;
		env->GetIntArrayRegion(strengths, i, 1, &j);
		nativeStrengths[i] = j;
	}

	noisyDef->CiToCpt();

    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_setNoisyParentStrengths__Ljava_lang_String_2I_3I(JNIEnv *env, jobject obj, jstring nodeId, jint parentIndex, jintArray strengths) 
{
    JSMILE_ENTER;
    Java_smile_Network_setNoisyParentStrengths__II_3I(env, obj, ValidateNodeId(env, obj, nodeId), parentIndex, strengths);
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_setNoisyParentStrengths__ILjava_lang_String_2_3I(JNIEnv *env, jobject obj, jint nodeHandle, jstring parentId, jintArray strengths) 
{
    JSMILE_ENTER;
    Java_smile_Network_setNoisyParentStrengths__II_3I(env, obj, nodeHandle, ValidateParentId(env, obj, nodeHandle, parentId), strengths);
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_setNoisyParentStrengths__Ljava_lang_String_2Ljava_lang_String_2_3I(JNIEnv *env, jobject obj, jstring nodeId, jstring parentId, jintArray strengths) 
{
    JSMILE_ENTER;
    int nodeHandle = ValidateNodeId(env, obj, nodeId);
    Java_smile_Network_setNoisyParentStrengths__II_3I(env, obj, 
        nodeHandle,
        ValidateParentId(env, obj, nodeHandle, parentId), strengths);
    JSMILE_LEAVE;
}

//-------------------------------------------------------------------------------

JNIEXPORT jintArray JNICALL Java_smile_Network_getNoisyParentStrengths__II(JNIEnv *env, jobject obj, jint nodeHandle, jint parentIndex) 
{
	JSMILE_ENTER;
    DSL_noisyMAX *noisyDef = GetNoisyDef(env, obj, nodeHandle);
    JSMILE_RETURN(CopyIntArray(env, noisyDef->GetParentOutcomeStrengths(parentIndex)));
}

JNIEXPORT jintArray JNICALL Java_smile_Network_getNoisyParentStrengths__Ljava_lang_String_2I(JNIEnv *env, jobject obj, jstring nodeId, jint parentIndex) 
{
    JSMILE_ENTER;
    JSMILE_RETURN(Java_smile_Network_getNoisyParentStrengths__II(env, obj, ValidateNodeId(env, obj, nodeId), parentIndex));
}

JNIEXPORT jintArray JNICALL Java_smile_Network_getNoisyParentStrengths__ILjava_lang_String_2(JNIEnv *env, jobject obj, jint nodeHandle, jstring parentId) 
{
	JSMILE_ENTER;
    JSMILE_RETURN(Java_smile_Network_getNoisyParentStrengths__II(env, obj, nodeHandle, ValidateParentId(env, obj, nodeHandle, parentId)));
}

JNIEXPORT jintArray JNICALL Java_smile_Network_getNoisyParentStrengths__Ljava_lang_String_2Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId, jstring parentId) 
{
    JSMILE_ENTER;
    int nodeHandle = ValidateNodeId(env, obj, nodeId);
    JSMILE_RETURN(Java_smile_Network_getNoisyParentStrengths__II(env, obj, nodeHandle, ValidateParentId(env, obj, nodeHandle, parentId)));
}

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_Network_setTarget__IZ(JNIEnv *env, jobject obj, jint nodeHandle, jboolean target) 
{
    JSMILE_ENTER;

    DSL_network *net = GetPtr(env, obj);
    DSL_node *node = ValidateNodeHandle(net, nodeHandle);
	int res;
	if (target)
    {
		res = net->SetTarget(nodeHandle);
    }
	else
    {
		res = net->UnSetTarget(nodeHandle);
    }

	if (DSL_OKAY != res) 
    {
		string msg;
		msg = "Cannot change target status for node '";
		msg += node->Info().Header().GetId();
		msg += "'";
		throw runtime_error(msg);
	}

    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_setTarget__Ljava_lang_String_2Z(JNIEnv *env, jobject obj, jstring nodeId, jboolean target) 
{
    JSMILE_ENTER;
    Java_smile_Network_setTarget__IZ(env, obj, ValidateNodeId(env, obj, nodeId), target);
    JSMILE_LEAVE;
}

//-------------------------------------------------------------------------------

JNIEXPORT jboolean JNICALL Java_smile_Network_isTarget__I(JNIEnv *env, jobject obj, jint nodeHandle) 
{
	JSMILE_ENTER;
    DSL_network *net = GetPtr(env, obj);
    ValidateNodeHandle(net, nodeHandle);
    JSMILE_RETURN(0 != net->IsTarget(nodeHandle));
}

JNIEXPORT jboolean JNICALL Java_smile_Network_isTarget__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId) 
{
	JSMILE_ENTER;
    JSMILE_RETURN(Java_smile_Network_isTarget__I(env, obj, ValidateNodeId(env, obj, nodeId)));
}

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_Network_clearAllTargets(JNIEnv *env, jobject obj) 
{
	JSMILE_ENTER;
    GetPtr(env, obj)->ClearAllTargets();
    JSMILE_LEAVE;
}

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_Network_setEvidence__II(JNIEnv *env, jobject obj, jint nodeHandle, jint outcomeIndex) 
{
    JSMILE_ENTER;
    DSL_node *node = ValidateOutcomeIndex(env, obj, nodeHandle, outcomeIndex);
	int res = node->Value()->SetEvidence(int(outcomeIndex));
	if (DSL_OKAY != res) 
    {
        ThrowSmileException(env, "SetEvidence", res);
    }
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_setEvidence__Ljava_lang_String_2I(JNIEnv *env, jobject obj, jstring nodeId, jint outcomeIndex) 
{
    JSMILE_ENTER;
	Java_smile_Network_setEvidence__II(env, obj, ValidateNodeId(env, obj, nodeId), outcomeIndex);
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_setEvidence__ILjava_lang_String_2(JNIEnv *env, jobject obj, jint nodeHandle, jstring outcomeId) 
{
    JSMILE_ENTER;
	Java_smile_Network_setEvidence__II(env, obj, nodeHandle, ValidateOutcomeId(env, obj, nodeHandle, outcomeId));
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_setEvidence__Ljava_lang_String_2Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId, jstring outcomeId) 
{
    JSMILE_ENTER;
    Java_smile_Network_setEvidence__ILjava_lang_String_2(env, obj, ValidateNodeId(env, obj, nodeId), outcomeId);
    JSMILE_LEAVE;
}

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_Network_setVirtualEvidence__I_3D(JNIEnv *env, jobject obj, jint nodeHandle, jdoubleArray evidence) 
{
	JSMILE_ENTER;
    DSL_node *node = ValidateNodeHandle(env, obj, nodeHandle);
	int count = env->GetArrayLength(evidence);
	vector<double> nativeEvidence(count);
	env->GetDoubleArrayRegion(evidence, 0, count, &nativeEvidence.front());
	int res = node->Value()->SetVirtualEvidence(nativeEvidence);
	if (DSL_OKAY != res) 
    {
        ThrowSmileException(env, "SetVirtualEvidence", res);
	}
	JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_setVirtualEvidence__Ljava_lang_String_2_3D(JNIEnv *env, jobject obj, jstring nodeId, jdoubleArray evidence) 
{
    JSMILE_ENTER;
	Java_smile_Network_setVirtualEvidence__I_3D(env, obj, ValidateNodeId(env, obj, nodeId), evidence);
    JSMILE_LEAVE;
}


JNIEXPORT jdoubleArray JNICALL Java_smile_Network_getVirtualEvidence__I(JNIEnv *env, jobject obj, jint nodeHandle) 
{
	JSMILE_ENTER;
		
	DSL_node *node = ValidateNodeHandle(env, obj, nodeHandle);

	vector<double> evidence;
	int res = node->Value()->GetVirtualEvidence(evidence);
	if (res < 0) 
    {
	    ThrowSmileException(env, "GetVirtualEvidence", res);
	}
	
	int count = int(evidence.size());
	jdoubleArray arr = env->NewDoubleArray(count);
	env->SetDoubleArrayRegion(arr, 0, count, &evidence.front());

	JSMILE_RETURN(arr);
}

JNIEXPORT jdoubleArray JNICALL Java_smile_Network_getVirtualEvidence__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId) 
{
    JSMILE_ENTER;
    JSMILE_RETURN(Java_smile_Network_getVirtualEvidence__I(env, obj, ValidateNodeId(env, obj, nodeId)));
}

//-------------------------------------------------------------------------------

JNIEXPORT jint JNICALL Java_smile_Network_getEvidence__I(JNIEnv *env, jobject obj, jint nodeHandle) 
{
	JSMILE_ENTER;
		
	DSL_node *node = ValidateNodeHandle(env, obj, nodeHandle);
	EnsureEvidenceIsSet(node);

	int res = node->Value()->GetEvidence();
	if (res < 0) 
    {
	    ThrowSmileException(env, "GetEvidence", res);
	}
		
	JSMILE_RETURN(res);
}

JNIEXPORT jint JNICALL Java_smile_Network_getEvidence__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId) 
{
    JSMILE_ENTER;
    JSMILE_RETURN(Java_smile_Network_getEvidence__I(env, obj, ValidateNodeId(env, obj, nodeId)));
}

//-------------------------------------------------------------------------------

JNIEXPORT jstring JNICALL Java_smile_Network_getEvidenceId__I(JNIEnv *env, jobject obj, jint nodeHandle) 
{
    JSMILE_ENTER;
    jstring res = NULL;
    int e = Java_smile_Network_getEvidence__I(env, obj, nodeHandle);
    if (e >= 0)
    {
        res = Java_smile_Network_getOutcomeId__II(env, obj, nodeHandle, e);
    }
    JSMILE_RETURN(res);
}

JNIEXPORT jstring JNICALL Java_smile_Network_getEvidenceId__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId) 
{
    JSMILE_ENTER;
    JSMILE_RETURN(Java_smile_Network_getEvidenceId__I(env, obj, ValidateNodeId(env, obj, nodeId)));
}

//-------------------------------------------------------------------------------

JNIEXPORT jboolean JNICALL Java_smile_Network_isEvidence__I(JNIEnv *env, jobject obj, jint nodeHandle) 
{
	JSMILE_ENTER;
    DSL_node *node = ValidateNodeHandle(env, obj, nodeHandle);
    JSMILE_RETURN(0 != node->Value()->IsEvidence())
}

JNIEXPORT jboolean JNICALL Java_smile_Network_isEvidence__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId) 
{
    JSMILE_ENTER;
    JSMILE_RETURN(Java_smile_Network_isEvidence__I(env, obj, ValidateNodeId(env, obj, nodeId)));
}

JNIEXPORT jboolean JNICALL Java_smile_Network_isRealEvidence__I(JNIEnv *env, jobject obj, jint nodeHandle) 
{
	JSMILE_ENTER;
    DSL_node *node = ValidateNodeHandle(env, obj, nodeHandle);
    JSMILE_RETURN(0 != node->Value()->IsRealEvidence())
}

JNIEXPORT jboolean JNICALL Java_smile_Network_isRealEvidence__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId) 
{
    JSMILE_ENTER;
    JSMILE_RETURN(Java_smile_Network_isRealEvidence__I(env, obj, ValidateNodeId(env, obj, nodeId)));
}

JNIEXPORT jboolean JNICALL Java_smile_Network_isPropagatedEvidence__I(JNIEnv *env, jobject obj, jint nodeHandle) 
{
	JSMILE_ENTER;
    DSL_node *node = ValidateNodeHandle(env, obj, nodeHandle);
    JSMILE_RETURN(0 != node->Value()->IsPropagatedEvidence())
}

JNIEXPORT jboolean JNICALL Java_smile_Network_isPropagatedEvidence__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId) 
{
    JSMILE_ENTER;
    JSMILE_RETURN(Java_smile_Network_isPropagatedEvidence__I(env, obj, ValidateNodeId(env, obj, nodeId)));
}

JNIEXPORT jboolean JNICALL Java_smile_Network_isVirtualEvidence__I(JNIEnv *env, jobject obj, jint nodeHandle) 
{
	JSMILE_ENTER;
    DSL_node *node = ValidateNodeHandle(env, obj, nodeHandle);
    JSMILE_RETURN(0 != node->Value()->IsVirtualEvidence())
}

JNIEXPORT jboolean JNICALL Java_smile_Network_isVirtualEvidence__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId) 
{
    JSMILE_ENTER;
    JSMILE_RETURN(Java_smile_Network_isVirtualEvidence__I(env, obj, ValidateNodeId(env, obj, nodeId)));
}



//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_Network_clearEvidence__I(JNIEnv *env, jobject obj, jint nodeHandle) 
{
    JSMILE_ENTER;
	DSL_node *node = ValidateNodeHandle(env, obj, nodeHandle);
	int res = node->Value()->ClearEvidence();
    if (DSL_OKAY != res) 
    {
	    ThrowSmileException(env, "ClearEvidence", res);
	}
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_clearEvidence__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId) 
{
    JSMILE_ENTER;
    Java_smile_Network_clearEvidence__I(env, obj, ValidateNodeId(env, obj, nodeId));
    JSMILE_LEAVE;
}

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_Network_clearAllEvidence(JNIEnv *env, jobject obj) 
{
    JSMILE_ENTER;
    GetPtr(env, obj)->ClearAllEvidence();
    JSMILE_LEAVE;
}


//-------------------------------------------------------------------------------
// Visualization (GeNIe)
//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_Network_setNodePosition__IIIII(JNIEnv *env, jobject obj, jint nodeHandle, jint x, jint y, jint width, jint height) 
{
	JSMILE_ENTER;
    DSL_node *node = ValidateNodeHandle(env, obj, nodeHandle);
	SetPosition(node->Info().Screen(), x, y, width, height);
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_setNodePosition__Ljava_lang_String_2IIII(JNIEnv *env, jobject obj, jstring nodeId, jint x, jint y, jint width, jint height) 
{
    JSMILE_ENTER;
    Java_smile_Network_setNodePosition__IIIII(env, obj, ValidateNodeId(env, obj, nodeId), x, y, width, height);
    JSMILE_LEAVE;
}

//-------------------------------------------------------------------------------

JNIEXPORT jobject JNICALL Java_smile_Network_getNodePosition__I(JNIEnv *env, jobject obj, jint nodeHandle) 
{
    JSMILE_ENTER;
    DSL_node *node = ValidateNodeHandle(env, obj, nodeHandle);
	JSMILE_RETURN(GetPosition(env, node->Info().Screen()));
}

JNIEXPORT jobject JNICALL Java_smile_Network_getNodePosition__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId) 
{
    JSMILE_ENTER;
    JSMILE_RETURN(Java_smile_Network_getNodePosition__I(env, obj, ValidateNodeId(env, obj, nodeId)));
}

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_Network_setNodeBgColor__ILjava_awt_Color_2(JNIEnv *env, jobject obj, jint nodeHandle, jobject color) 
{
    JSMILE_ENTER;
    SetColor(env, obj, nodeHandle, &DSL_screenInfo::color, color);
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_setNodeBgColor__Ljava_lang_String_2Ljava_awt_Color_2(JNIEnv *env, jobject obj, jstring nodeId, jobject color) 
{
	JSMILE_ENTER;
    Java_smile_Network_setNodeBgColor__ILjava_awt_Color_2(env, obj, ValidateNodeId(env, obj, nodeId), color);
    JSMILE_LEAVE;
}

JNIEXPORT jobject JNICALL Java_smile_Network_getNodeBgColor__I(JNIEnv *env, jobject obj, jint nodeHandle) 
{
    JSMILE_ENTER;
    JSMILE_RETURN(GetColor(env, obj, nodeHandle, &DSL_screenInfo::color));
}

JNIEXPORT jobject JNICALL Java_smile_Network_getNodeBgColor__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId) 
{
    JSMILE_ENTER;
	JSMILE_RETURN(Java_smile_Network_getNodeBgColor__I(env, obj, ValidateNodeId(env, obj, nodeId)));
}

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_Network_setNodeTextColor__ILjava_awt_Color_2(JNIEnv *env, jobject obj, jint nodeHandle, jobject color) 
{
    JSMILE_ENTER;
    SetColor(env, obj, nodeHandle, &DSL_screenInfo::fontColor, color);
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_setNodeTextColor__Ljava_lang_String_2Ljava_awt_Color_2(JNIEnv *env, jobject obj, jstring nodeId, jobject color) 
{
	JSMILE_ENTER;
    Java_smile_Network_setNodeTextColor__ILjava_awt_Color_2(env, obj, ValidateNodeId(env, obj, nodeId), color);
    JSMILE_LEAVE;
}

JNIEXPORT jobject JNICALL Java_smile_Network_getNodeTextColor__I(JNIEnv *env, jobject obj, jint nodeHandle) 
{
    JSMILE_ENTER;
    JSMILE_RETURN(GetColor(env, obj, nodeHandle, &DSL_screenInfo::fontColor));
}

JNIEXPORT jobject JNICALL Java_smile_Network_getNodeTextColor__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId) 
{
    JSMILE_ENTER;
	JSMILE_RETURN(Java_smile_Network_getNodeTextColor__I(env, obj, ValidateNodeId(env, obj, nodeId)));
}

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_Network_setNodeBorderColor__ILjava_awt_Color_2(JNIEnv *env, jobject obj, jint nodeHandle, jobject color) 
{
    JSMILE_ENTER;
    SetColor(env, obj, nodeHandle, &DSL_screenInfo::borderColor, color);
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_setNodeBorderColor__Ljava_lang_String_2Ljava_awt_Color_2(JNIEnv *env, jobject obj, jstring nodeId, jobject color) 
{
	JSMILE_ENTER;
    Java_smile_Network_setNodeBorderColor__ILjava_awt_Color_2(env, obj, ValidateNodeId(env, obj, nodeId), color);
    JSMILE_LEAVE;
}

JNIEXPORT jobject JNICALL Java_smile_Network_getNodeBorderColor__I(JNIEnv *env, jobject obj, jint nodeHandle) 
{
    JSMILE_ENTER;
    JSMILE_RETURN(GetColor(env, obj, nodeHandle, &DSL_screenInfo::borderColor));
}

JNIEXPORT jobject JNICALL Java_smile_Network_getNodeBorderColor__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId) 
{
    JSMILE_ENTER;
	JSMILE_RETURN(Java_smile_Network_getNodeBorderColor__I(env, obj, ValidateNodeId(env, obj, nodeId)));
}


//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_Network_setNodeBorderWidth__II(JNIEnv *env, jobject obj, jint nodeHandle, jint width) 
{
	JSMILE_ENTER;
    DSL_node *node = ValidateNodeHandle(env, obj, nodeHandle);
    node->Info().Screen().borderThickness = width;
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_setNodeBorderWidth__Ljava_lang_String_2I(JNIEnv *env, jobject obj, jstring nodeId, jint width) 
{
	JSMILE_ENTER;
    Java_smile_Network_setNodeBorderWidth__II(env, obj, ValidateNodeId(env, obj, nodeId), width);
    JSMILE_LEAVE;
}

JNIEXPORT jint JNICALL Java_smile_Network_getNodeBorderWidth__I(JNIEnv *env, jobject obj, jint nodeHandle) 
{
    JSMILE_ENTER;
    DSL_node *node = ValidateNodeHandle(env, obj, nodeHandle);
    JSMILE_RETURN(node->Info().Screen().borderThickness);
}

JNIEXPORT jint JNICALL Java_smile_Network_getNodeBorderWidth__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId) 
{
    JSMILE_ENTER;
    JSMILE_RETURN(Java_smile_Network_getNodeBorderWidth__I(env, obj, ValidateNodeId(env, obj, nodeId)));
}

//-------------------------------------------------------------------------------
// Submodels
//-------------------------------------------------------------------------------

JNIEXPORT jint JNICALL Java_smile_Network_getSubmodel(JNIEnv *env, jobject obj, jstring id) 
{
	JSMILE_ENTER;
    JSMILE_RETURN(ValidateSubmodelId(env, obj, id));
}

JNIEXPORT jint JNICALL Java_smile_Network_getMainSubmodel(JNIEnv *env, jobject obj) 
{
    JSMILE_ENTER;
    JSMILE_RETURN(FindMainSubmodel(env, obj));
}

JNIEXPORT jstring JNICALL Java_smile_Network_getMainSubmodelId(JNIEnv *env, jobject obj) 
{
    JSMILE_ENTER;
    int h = FindMainSubmodel(env, obj);
    DSL_submodel *sub = GetPtr(env, obj)->GetSubmodelHandler().GetSubmodel(h);
    JSMILE_RETURN(env->NewStringUTF(sub->header.GetId()));
}

//-------------------------------------------------------------------------------

JNIEXPORT jint JNICALL Java_smile_Network_getFirstSubmodel(JNIEnv *env, jobject obj) 
{
    JSMILE_ENTER;
	DSL_network *net = GetPtr(env, obj);
    JSMILE_RETURN(net->GetSubmodelHandler().GetFirstSubmodel());
}

JNIEXPORT jint JNICALL Java_smile_Network_getNextSubmodel(JNIEnv *env, jobject obj, jint handle) 
{
	JSMILE_ENTER;
	DSL_network *net = GetPtr(env, obj);
	int nextSubmodelHandle = net->GetSubmodelHandler().GetNextSubmodel(handle);
	if (nextSubmodelHandle < 0)
    {
		nextSubmodelHandle = -1;
    }
    JSMILE_RETURN(nextSubmodelHandle);
}

//-------------------------------------------------------------------------------

JNIEXPORT jint JNICALL Java_smile_Network_getSubmodelCount(JNIEnv *env, jobject obj) 
{
	JSMILE_ENTER;
    JSMILE_RETURN(GetPtr(env, obj)->GetSubmodelHandler().GetNumberOfSubmodels());
}

//-------------------------------------------------------------------------------

JNIEXPORT jint JNICALL Java_smile_Network_addSubmodel__ILjava_lang_String_2(JNIEnv *env, jobject obj, jint parentHandle, jstring id) 
{
    JSMILE_ENTER;
    ValidateId(env, id);
    NativeString native(env, id);
    int res = GetPtr(env, obj)->GetSubmodelHandler().CreateSubmodel(parentHandle, native.GetChars());
    if (res < 0)
    {
        ThrowSmileException(env, "CreateSubmodel", res);
    }
    JSMILE_RETURN(res);
}

JNIEXPORT jint JNICALL Java_smile_Network_addSubmodel__Ljava_lang_String_2Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring parentId, jstring id) 
{
    JSMILE_ENTER;
    JSMILE_RETURN(Java_smile_Network_addSubmodel__ILjava_lang_String_2(env, obj, ValidateSubmodelId(env, obj, parentId), id));
}



//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_Network_deleteSubmodel__I(JNIEnv *env, jobject obj, jint handle) 
{
    JSMILE_ENTER;
	DSL_network *net = GetPtr(env, obj);
	ValidateSubmodelHandle(net, handle);
	int res = net->GetSubmodelHandler().DeleteSubmodel(handle);
    if (DSL_OKAY != res)
    {
        ThrowSmileException(env, "DeleteSubmodel", res);
    }
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_deleteSubmodel__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring id) 
{
	JSMILE_ENTER;
    Java_smile_Network_deleteSubmodel__I(env, obj, ValidateSubmodelId(env, obj, id));
    JSMILE_LEAVE;
}

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_Network_setSubmodelId(JNIEnv *env, jobject obj, jint handle, jstring id) 
{
    JSMILE_ENTER;
    DSL_submodel *sub = ValidateSubmodelHandle(env, obj, handle);
    SetHeaderId(env, id, sub->header);
    JSMILE_LEAVE;
}

JNIEXPORT jstring JNICALL Java_smile_Network_getSubmodelId(JNIEnv *env, jobject obj, jint handle) 
{
    JSMILE_ENTER;
    DSL_submodel *sub = ValidateSubmodelHandle(env, obj, handle);
    JSMILE_RETURN(env->NewStringUTF(sub->header.GetId()));
}

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_Network_setSubmodelName__ILjava_lang_String_2(JNIEnv *env, jobject obj, jint handle, jstring name) 
{
    JSMILE_ENTER;
    DSL_submodel *sub = ValidateSubmodelHandle(env, obj, handle);
    SetHeaderName(env, name, sub->header);
    JSMILE_LEAVE;
}
 
JNIEXPORT void JNICALL Java_smile_Network_setSubmodelName__Ljava_lang_String_2Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring id, jstring name) 
{
    JSMILE_ENTER;
    Java_smile_Network_setSubmodelName__ILjava_lang_String_2(env, obj, ValidateSubmodelId(env, obj, id), name);
    JSMILE_LEAVE;
}

JNIEXPORT jstring JNICALL Java_smile_Network_getSubmodelName__I(JNIEnv *env, jobject obj, jint handle) 
{
	JSMILE_ENTER;
    DSL_submodel *sub = ValidateSubmodelHandle(env, obj, handle);
    JSMILE_RETURN(env->NewStringUTF(sub->header.GetName()));
}

JNIEXPORT jstring JNICALL Java_smile_Network_getSubmodelName__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring id) 
{
    JSMILE_ENTER;
    JSMILE_RETURN(Java_smile_Network_getSubmodelName__I(env, obj, ValidateSubmodelId(env, obj, id)));
}

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_Network_setSubmodelDescription__ILjava_lang_String_2(JNIEnv *env, jobject obj, jint handle, jstring description) 
{
    JSMILE_ENTER;
    DSL_submodel *sub = ValidateSubmodelHandle(env, obj, handle);
    SetHeaderComment(env, description, sub->header);
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_setSubmodelDescrpition__Ljava_lang_String_2Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring id, jstring description) 
{
    JSMILE_ENTER;
    Java_smile_Network_setSubmodelDescription__ILjava_lang_String_2(env, obj, ValidateSubmodelId(env, obj, id), description);
    JSMILE_LEAVE;
}

JNIEXPORT jstring JNICALL Java_smile_Network_getSubmodelDescription__I(JNIEnv *env, jobject obj, jint handle) 
{
    JSMILE_ENTER;
    DSL_submodel *sub = ValidateSubmodelHandle(env, obj, handle);
	JSMILE_RETURN(env->NewStringUTF(sub->header.GetComment()));
}

JNIEXPORT jstring JNICALL Java_smile_Network_getSubmodelDescription__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring id) 
{
    JSMILE_ENTER;
    JSMILE_RETURN(Java_smile_Network_getSubmodelDescription__I(env, obj, ValidateSubmodelId(env, obj, id)));
}

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_Network_setSubmodelOfNode__II(JNIEnv *env, jobject obj, jint submodelHandle, jint nodeHandle) 
{
    JSMILE_ENTER;
    DSL_network *net = GetPtr(env, obj);
	DSL_node *node = ValidateNodeHandle(net, nodeHandle);
    ValidateSubmodelHandle(net, submodelHandle);
	int res = node->SetSubmodel(submodelHandle);
    if (DSL_OKAY != res)
    {
        ThrowSmileException(env, "SetSubmodel", res);
    }
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_setSubmodelOfNode__Ljava_lang_String_2Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring submodelId, jstring nodeId) 
{
	JSMILE_ENTER;
    Java_smile_Network_setSubmodelOfNode__II(env, obj, ValidateSubmodelId(env, obj, submodelId), ValidateNodeId(env, obj, nodeId));
    JSMILE_LEAVE;
}

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_Network_setSubmodelOfSubmodel__II(JNIEnv *env, jobject obj, jint parentHandle, jint childHandle) 
{
    JSMILE_ENTER;
    DSL_network *net = GetPtr(env, obj);
	ValidateSubmodelHandle(net, childHandle);
    ValidateSubmodelHandle(net, parentHandle);
    int res = net->GetSubmodelHandler().MoveSubmodel(childHandle, parentHandle);
    if (DSL_OKAY != res)
    {
        ThrowSmileException(env, "SetSubmodel", res);
    }
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_setSubmodelOfSubmodel__Ljava_lang_String_2Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring parentId, jstring childId)
{
	JSMILE_ENTER;
    Java_smile_Network_setSubmodelOfSubmodel__II(env, obj, ValidateSubmodelId(env, obj, parentId), ValidateSubmodelId(env, obj, childId));
    JSMILE_LEAVE;
}

//-------------------------------------------------------------------------------

JNIEXPORT jint JNICALL Java_smile_Network_getSubmodelOfNode__I(JNIEnv *env, jobject obj, jint nodeHandle) 
{
    JSMILE_ENTER;
    DSL_node *node = ValidateNodeHandle(env, obj, nodeHandle);
    JSMILE_RETURN(node->GetSubmodel());
}

JNIEXPORT jint JNICALL Java_smile_Network_getSubmodelOfNode__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId) 
{
    JSMILE_ENTER;
    JSMILE_RETURN(Java_smile_Network_getSubmodelOfNode__I(env, obj, ValidateNodeId(env, obj, nodeId)));
}

JNIEXPORT jint JNICALL Java_smile_Network_getSubmodelOfSubmodel__I(JNIEnv *env, jobject obj, jint submodelHandle) 
{
    JSMILE_ENTER;
    DSL_network *net = GetPtr(env, obj);
    ValidateSubmodelHandle(net, submodelHandle);
    JSMILE_RETURN(net->GetSubmodelHandler().GetParent(submodelHandle));
}

JNIEXPORT jint JNICALL Java_smile_Network_getSubmodelOfSubmodel__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring submodelId) 
{
    JSMILE_ENTER;
    JSMILE_RETURN(Java_smile_Network_getSubmodelOfSubmodel__I(env, obj, ValidateSubmodelId(env, obj, submodelId)));
}

//-------------------------------------------------------------------------------

JNIEXPORT jobject JNICALL Java_smile_Network_getSubmodelPosition__I(JNIEnv *env, jobject obj, jint submodelHandle) 
{
    JSMILE_ENTER;
    DSL_submodel *sub = ValidateSubmodelHandle(env, obj, submodelHandle);
	JSMILE_RETURN(GetPosition(env, sub->info));
}

JNIEXPORT jobject JNICALL Java_smile_Network_getSubmodelPosition__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring submodelId) 
{
    JSMILE_ENTER;
    JSMILE_RETURN(Java_smile_Network_getSubmodelPosition__I(env, obj, ValidateSubmodelId(env, obj, submodelId)));
}

JNIEXPORT void JNICALL Java_smile_Network_setSubmodelPosition__IIIII(JNIEnv *env, jobject obj, jint submodelHandle, jint x, jint y, jint width, jint height) 
{
	JSMILE_ENTER;
    DSL_submodel *sub = ValidateSubmodelHandle(env, obj, submodelHandle);
	SetPosition(sub->info, x, y, width, height);
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_setSubmodelPosition__Ljava_lang_String_2IIII(JNIEnv *env, jobject obj, jstring submodelId, jint x, jint y, jint width, jint height)
{
    JSMILE_ENTER;
    Java_smile_Network_setSubmodelPosition__IIIII(env, obj, ValidateSubmodelId(env, obj, submodelId), x, y, width, height);
    JSMILE_LEAVE;
}


//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
// Diagnosis
//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_Network_setNodeDiagType__II(JNIEnv *env, jobject obj, jint nodeHandle, jint type) 
{
	JSMILE_ENTER;
	GetExtraDef(env, obj, nodeHandle)->SetType(static_cast<DSL_extraDefinition::troubleType>(type));	
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_setNodeDiagType__Ljava_lang_String_2I(JNIEnv *env, jobject obj, jstring nodeId, jint type) 
{
    JSMILE_ENTER;
	Java_smile_Network_setNodeDiagType__II(env, obj, ValidateNodeId(env, obj, nodeId), type);
    JSMILE_LEAVE;
}

JNIEXPORT jint JNICALL Java_smile_Network_getNodeDiagType__I(JNIEnv *env, jobject obj, jint nodeHandle) 
{
	JSMILE_ENTER;
    JSMILE_RETURN(GetExtraDef(env, obj, nodeHandle)->GetType());
}

JNIEXPORT jint JNICALL Java_smile_Network_getNodeDiagType__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId) 
{
    JSMILE_ENTER;
    JSMILE_RETURN(Java_smile_Network_getNodeDiagType__I(env, obj, ValidateNodeId(env, obj, nodeId)));
}

//-------------------------------------------------------------------------------

JNIEXPORT jstring JNICALL Java_smile_Network_getNodeQuestion__I(JNIEnv *env, jobject obj, jint nodeHandle) 
{
	JSMILE_ENTER;
    JSMILE_RETURN(env->NewStringUTF(GetExtraDef(env, obj, nodeHandle)->GetQuestion().c_str()));
}

JNIEXPORT jstring JNICALL Java_smile_Network_getNodeQuestion__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId) 
{
	JSMILE_ENTER;
    JSMILE_RETURN(Java_smile_Network_getNodeQuestion__I(env, obj, ValidateNodeId(env, obj, nodeId)));
}

JNIEXPORT void JNICALL Java_smile_Network_setNodeQuestion__ILjava_lang_String_2(JNIEnv *env, jobject obj, jint nodeHandle, jstring question) 
{
    JSMILE_ENTER;
    NativeString native(env, question);
    GetExtraDef(env, obj, nodeHandle)->GetQuestion() = native;
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_setNodeQuestion__Ljava_lang_String_2Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId, jstring question) 
{
    JSMILE_ENTER;
    Java_smile_Network_setNodeQuestion__ILjava_lang_String_2(env, obj, ValidateNodeId(env, obj, nodeId), question);
    JSMILE_LEAVE;
}

//-------------------------------------------------------------------------------

JNIEXPORT jstring JNICALL Java_smile_Network_getOutcomeFix__II(JNIEnv *env, jobject obj, jint nodeHandle, jint outcomeIndex) 
{
	JSMILE_ENTER;
    DSL_node *node = ValidateOutcomeIndex(env, obj, nodeHandle, outcomeIndex);
    JSMILE_RETURN(env->NewStringUTF(node->ExtraDefinition()->GetStateRepairInfo(outcomeIndex)));
}

JNIEXPORT jstring JNICALL Java_smile_Network_getOutcomeFix__Ljava_lang_String_2I(JNIEnv *env, jobject obj, jstring nodeId, jint outcomeIndex) 
{
	JSMILE_ENTER;
    JSMILE_RETURN(Java_smile_Network_getOutcomeFix__II(env, obj, ValidateNodeId(env, obj, nodeId), outcomeIndex));
}

JNIEXPORT jstring JNICALL Java_smile_Network_getOutcomeFix__ILjava_lang_String_2(JNIEnv *env, jobject obj, jint nodeHandle, jstring outcomeId) 
{
	JSMILE_ENTER;
    JSMILE_RETURN(Java_smile_Network_getOutcomeFix__II(env, obj, nodeHandle, ValidateOutcomeId(env, obj, nodeHandle, outcomeId)));
}

JNIEXPORT jstring JNICALL Java_smile_Network_getOutcomeFix__Ljava_lang_String_2Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId, jstring outcomeId) 
{
	JSMILE_ENTER;
    JSMILE_RETURN(Java_smile_Network_getOutcomeFix__ILjava_lang_String_2(env, obj, ValidateNodeId(env, obj, nodeId), outcomeId));
}

JNIEXPORT void JNICALL Java_smile_Network_setOutcomeFix__IILjava_lang_String_2(JNIEnv *env, jobject obj, jint nodeHandle, jint outcomeIndex, jstring treatment) 
{
    JSMILE_ENTER;
    NativeString native(env, treatment);
    DSL_node *node = ValidateOutcomeIndex(env, obj, nodeHandle, outcomeIndex);
    node->ExtraDefinition()->SetStateRepairInfo(outcomeIndex, native.GetChars());
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_setOutcomeFix__Ljava_lang_String_2ILjava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId, jint outcomeIndex, jstring treatment) 
{
	JSMILE_ENTER;
    Java_smile_Network_setOutcomeFix__IILjava_lang_String_2(env, obj, ValidateNodeId(env, obj, nodeId), outcomeIndex, treatment);
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_setOutcomeFix__ILjava_lang_String_2Ljava_lang_String_2(JNIEnv *env, jobject obj, jint nodeHandle, jstring outcomeId, jstring treatment) 
{
	JSMILE_ENTER;
    Java_smile_Network_setOutcomeFix__IILjava_lang_String_2(env, obj, nodeHandle, ValidateOutcomeId(env, obj, nodeHandle, outcomeId), treatment);
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_setOutcomeFix__Ljava_lang_String_2Ljava_lang_String_2Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId, jstring outcomeId, jstring treatment) 
{
	JSMILE_ENTER;
    Java_smile_Network_setOutcomeFix__ILjava_lang_String_2Ljava_lang_String_2(env, obj, ValidateNodeId(env, obj, nodeId), outcomeId, treatment);
    JSMILE_LEAVE;
}

//-------------------------------------------------------------------------------

JNIEXPORT jstring JNICALL Java_smile_Network_getOutcomeDescription__II(JNIEnv *env, jobject obj, jint nodeHandle, jint outcomeIndex) 
{
	JSMILE_ENTER;
    DSL_node *node = ValidateOutcomeIndex(env, obj, nodeHandle, outcomeIndex);
    JSMILE_RETURN(env->NewStringUTF(node->ExtraDefinition()->GetStateDescription(outcomeIndex)));
}

JNIEXPORT jstring JNICALL Java_smile_Network_getOutcomeDescription__Ljava_lang_String_2I(JNIEnv *env, jobject obj, jstring nodeId, jint outcomeIndex) 
{
	JSMILE_ENTER;
    JSMILE_RETURN(Java_smile_Network_getOutcomeDescription__II(env, obj, ValidateNodeId(env, obj, nodeId), outcomeIndex));
}

JNIEXPORT jstring JNICALL Java_smile_Network_getOutcomeDescription__ILjava_lang_String_2(JNIEnv *env, jobject obj, jint nodeHandle, jstring outcomeId) 
{
	JSMILE_ENTER;
    JSMILE_RETURN(Java_smile_Network_getOutcomeDescription__II(env, obj, nodeHandle, ValidateOutcomeId(env, obj, nodeHandle, outcomeId)));
}

JNIEXPORT jstring JNICALL Java_smile_Network_getOutcomeDescription__Ljava_lang_String_2Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId, jstring outcomeId) 
{
	JSMILE_ENTER;
    JSMILE_RETURN(Java_smile_Network_getOutcomeDescription__ILjava_lang_String_2(env, obj, ValidateNodeId(env, obj, nodeId), outcomeId));
}

JNIEXPORT void JNICALL Java_smile_Network_setOutcomeDescription__IILjava_lang_String_2(JNIEnv *env, jobject obj, jint nodeHandle, jint outcomeIndex, jstring description) 
{
    JSMILE_ENTER;
    NativeString native(env, description);
    DSL_node *node = ValidateOutcomeIndex(env, obj, nodeHandle, outcomeIndex);
    node->ExtraDefinition()->SetStateDescription(outcomeIndex, native.GetChars());
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_setOutcomeDescription__Ljava_lang_String_2ILjava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId, jint outcomeIndex, jstring description) 
{
	JSMILE_ENTER;
    Java_smile_Network_setOutcomeDescription__IILjava_lang_String_2(env, obj, ValidateNodeId(env, obj, nodeId), outcomeIndex, description);
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_setOutcomeDescription__ILjava_lang_String_2Ljava_lang_String_2(JNIEnv *env, jobject obj, jint nodeHandle, jstring outcomeId, jstring description) 
{
	JSMILE_ENTER;
    Java_smile_Network_setOutcomeDescription__IILjava_lang_String_2(env, obj, nodeHandle, ValidateOutcomeId(env, obj, nodeHandle, outcomeId), description);
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_setOutcomeDescription__Ljava_lang_String_2Ljava_lang_String_2Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId, jstring outcomeId, jstring description) 
{
	JSMILE_ENTER;
    Java_smile_Network_setOutcomeDescription__ILjava_lang_String_2Ljava_lang_String_2(env, obj, ValidateNodeId(env, obj, nodeId), outcomeId, description);
    JSMILE_LEAVE;
}

//-------------------------------------------------------------------------------

JNIEXPORT jstring JNICALL Java_smile_Network_getOutcomeLabel__II(JNIEnv *env, jobject obj, jint nodeHandle, jint outcomeIndex) 
{
	JSMILE_ENTER;
    DSL_node *node = ValidateOutcomeIndex(env, obj, nodeHandle, outcomeIndex);
    JSMILE_RETURN(env->NewStringUTF(node->ExtraDefinition()->GetFaultLabels()[outcomeIndex]));
}

JNIEXPORT jstring JNICALL Java_smile_Network_getOutcomeLabel__Ljava_lang_String_2I(JNIEnv *env, jobject obj, jstring nodeId, jint outcomeIndex) 
{
	JSMILE_ENTER;
    JSMILE_RETURN(Java_smile_Network_getOutcomeLabel__II(env, obj, ValidateNodeId(env, obj, nodeId), outcomeIndex));
}

JNIEXPORT jstring JNICALL Java_smile_Network_getOutcomeLabel__ILjava_lang_String_2(JNIEnv *env, jobject obj, jint nodeHandle, jstring outcomeId) 
{
	JSMILE_ENTER;
    JSMILE_RETURN(Java_smile_Network_getOutcomeLabel__II(env, obj, nodeHandle, ValidateOutcomeId(env, obj, nodeHandle, outcomeId)));
}

JNIEXPORT jstring JNICALL Java_smile_Network_getOutcomeLabel__Ljava_lang_String_2Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId, jstring outcomeId) 
{
	JSMILE_ENTER;
    JSMILE_RETURN(Java_smile_Network_getOutcomeLabel__ILjava_lang_String_2(env, obj, ValidateNodeId(env, obj, nodeId), outcomeId));
}

JNIEXPORT void JNICALL Java_smile_Network_setOutcomeLabel__IILjava_lang_String_2(JNIEnv *env, jobject obj, jint nodeHandle, jint outcomeIndex, jstring label) 
{
    JSMILE_ENTER;
    NativeString native(env, label);
    DSL_node *node = ValidateOutcomeIndex(env, obj, nodeHandle, outcomeIndex);
    node->ExtraDefinition()->SetLabel(outcomeIndex, native);
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_setOutcomeLabel__Ljava_lang_String_2ILjava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId, jint outcomeIndex, jstring label) 
{
	JSMILE_ENTER;
    Java_smile_Network_setOutcomeLabel__IILjava_lang_String_2(env, obj, ValidateNodeId(env, obj, nodeId), outcomeIndex, label);
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_setOutcomeLabel__ILjava_lang_String_2Ljava_lang_String_2(JNIEnv *env, jobject obj, jint nodeHandle, jstring outcomeId, jstring label) 
{
	JSMILE_ENTER;
    Java_smile_Network_setOutcomeLabel__IILjava_lang_String_2(env, obj, nodeHandle, ValidateOutcomeId(env, obj, nodeHandle, outcomeId), label);
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_setOutcomeLabel__Ljava_lang_String_2Ljava_lang_String_2Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId, jstring outcomeId, jstring label) 
{
	JSMILE_ENTER;
    Java_smile_Network_setOutcomeLabel__ILjava_lang_String_2Ljava_lang_String_2(env, obj, ValidateNodeId(env, obj, nodeId), outcomeId, label);
    JSMILE_LEAVE;
}

//-------------------------------------------------------------------------------

JNIEXPORT jboolean JNICALL Java_smile_Network_isFaultOutcome__II(JNIEnv *env, jobject obj, jint nodeHandle, jint outcomeIndex) 
{
	JSMILE_ENTER;
    DSL_node *node = ValidateOutcomeIndex(env, obj, nodeHandle, outcomeIndex);
    JSMILE_RETURN(node->ExtraDefinition()->IsFaultState(outcomeIndex));
}

JNIEXPORT jboolean JNICALL Java_smile_Network_isFaultOutcome__Ljava_lang_String_2I(JNIEnv *env, jobject obj, jstring nodeId, jint outcomeIndex) 
{
	JSMILE_ENTER;
    JSMILE_RETURN(Java_smile_Network_isFaultOutcome__II(env, obj, ValidateNodeId(env, obj, nodeId), outcomeIndex));
}

JNIEXPORT jboolean JNICALL Java_smile_Network_isFaultOutcome__ILjava_lang_String_2(JNIEnv *env, jobject obj, jint nodeHandle, jstring outcomeId) 
{
	JSMILE_ENTER;
    JSMILE_RETURN(Java_smile_Network_isFaultOutcome__II(env, obj, nodeHandle, ValidateOutcomeId(env, obj, nodeHandle, outcomeId)));
}

JNIEXPORT jboolean JNICALL Java_smile_Network_isFaultOutcome__Ljava_lang_String_2Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId, jstring outcomeId) 
{
	JSMILE_ENTER;
    JSMILE_RETURN(Java_smile_Network_isFaultOutcome__ILjava_lang_String_2(env, obj, ValidateNodeId(env, obj, nodeId), outcomeId));
}

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_Network_setFaultOutcome__IIZ(JNIEnv *env, jobject obj, jint nodeHandle, jint outcomeIndex, jboolean fault) 
{
	JSMILE_ENTER;
    DSL_node *node = ValidateOutcomeIndex(env, obj, nodeHandle, outcomeIndex);
    node->ExtraDefinition()->SetFaultState(outcomeIndex, fault ? DSL_TRUE : DSL_FALSE);
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_setFaultOutcome__Ljava_lang_String_2IZ(JNIEnv *env, jobject obj, jstring nodeId, jint outcomeIndex, jboolean fault) 
{
	JSMILE_ENTER;
    Java_smile_Network_setFaultOutcome__IIZ(env, obj, ValidateNodeId(env, obj, nodeId), outcomeIndex, fault);
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_setFaultOutcome__ILjava_lang_String_2Z(JNIEnv *env, jobject obj, jint nodeHandle, jstring outcomeId, jboolean fault) 
{
    JSMILE_ENTER;
	Java_smile_Network_setFaultOutcome__IIZ(env, obj, nodeHandle, ValidateOutcomeId(env, obj, nodeHandle, outcomeId), fault);
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_setFaultOutcome__Ljava_lang_String_2Ljava_lang_String_2Z(JNIEnv *env, jobject obj, jstring nodeId, jstring outcomeId, jboolean fault) 
{
    JSMILE_ENTER;
    Java_smile_Network_setFaultOutcome__ILjava_lang_String_2Z(env, obj, ValidateNodeId(env, obj, nodeId), outcomeId, fault);
    JSMILE_LEAVE;
}

//-------------------------------------------------------------------------------

JNIEXPORT jint JNICALL Java_smile_Network_getDefaultOutcome__I(JNIEnv *env, jobject obj, jint nodeHandle) 
{
	JSMILE_ENTER;
    JSMILE_RETURN(GetDefaultOutcome(env, obj, nodeHandle));
}

JNIEXPORT jint JNICALL Java_smile_Network_getDefaultOutcome__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId) 
{
    JSMILE_ENTER;
    JSMILE_RETURN(Java_smile_Network_getDefaultOutcome__I(env, obj, ValidateNodeId(env, obj, nodeId)));
}

//-------------------------------------------------------------------------------

JNIEXPORT jstring JNICALL Java_smile_Network_getDefaultOutcomeId__I(JNIEnv *env, jobject obj, jint nodeHandle) 
{
	JSMILE_ENTER;
    jstring ret = NULL;
    DSL_network *net = GetPtr(env, obj);
	int defOutcome = GetDefaultOutcome(env, obj, nodeHandle);
	if (defOutcome >= 0) 
    {
        DSL_node *node = ValidateNodeHandle(env, obj, nodeHandle);
        ret = env->NewStringUTF(node->Definition()->GetOutcomesNames()->Subscript(defOutcome));
    }

    JSMILE_RETURN(ret);
}


JNIEXPORT jstring JNICALL Java_smile_Network_getDefaultOutcomeId__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId) 
{
    JSMILE_ENTER;
    JSMILE_RETURN(Java_smile_Network_getDefaultOutcomeId__I(env, obj, ValidateNodeId(env, obj, nodeId)));
}

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_Network_setDefaultOutcome__II(JNIEnv *env, jobject obj, jint nodeHandle, jint outcomeIndex) 
{
	JSMILE_ENTER;
    
    DSL_extraDefinition *extraDef = NULL;
    bool setDef = false;
    if (outcomeIndex >= 0)
    {
        extraDef = ValidateOutcomeIndex(env, obj, nodeHandle, outcomeIndex)->ExtraDefinition();
        extraDef->SetDefaultOutcome(outcomeIndex);
        setDef = true;
    }
    else
    {
        extraDef = ValidateNodeHandle(env, obj, nodeHandle)->ExtraDefinition();
    }

    bool ranked = extraDef->IsRanked();
	bool mandatory = extraDef->IsMandatory();
	extraDef->SetFlags(ranked, mandatory, setDef);

    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_setDefaultOutcome__Ljava_lang_String_2I(JNIEnv *env, jobject obj, jstring nodeId, jint outcomeIndex) 
{
	JSMILE_ENTER;
    Java_smile_Network_setDefaultOutcome__II(env, obj, ValidateNodeId(env, obj, nodeId), outcomeIndex);
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_setDefaultOutcome__ILjava_lang_String_2(JNIEnv *env, jobject obj, jint nodeHandle, jstring outcomeId) 
{
    JSMILE_ENTER;
    Java_smile_Network_setDefaultOutcome__II(env, obj, nodeHandle, ValidateOutcomeId(env, obj, nodeHandle, outcomeId));
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_setDefaultOutcome__Ljava_lang_String_2Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId, jstring outcomeId) 
{
	JSMILE_ENTER;
    Java_smile_Network_setDefaultOutcome__ILjava_lang_String_2(env, obj, ValidateNodeId(env, obj, nodeId), outcomeId);
    JSMILE_LEAVE;
}

//-------------------------------------------------------------------------------
// User Properties
//-------------------------------------------------------------------------------

JNIEXPORT jobjectArray JNICALL Java_smile_Network_getUserProperties(JNIEnv *env, jobject obj) 
{
    JSMILE_ENTER;
    JSMILE_RETURN(GetUserProps(env, GetPtr(env, obj)->UserProperties()));
}

JNIEXPORT void JNICALL Java_smile_Network_setUserProperties(JNIEnv *env, jobject obj, jobjectArray properties) 
{
	JSMILE_ENTER;
    SetUserProps(env, properties, GetPtr(env, obj)->UserProperties());
    JSMILE_LEAVE;
}

JNIEXPORT jobjectArray JNICALL Java_smile_Network_getNodeUserProperties__I(JNIEnv *env, jobject obj, jint nodeHandle) 
{
	JSMILE_ENTER;
    JSMILE_RETURN(GetUserProps(env, ValidateNodeHandle(env, obj, nodeHandle)->Info().UserProperties()));
}

JNIEXPORT jobjectArray JNICALL Java_smile_Network_getNodeUserProperties__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId) 
{
	JSMILE_ENTER;
    JSMILE_RETURN(Java_smile_Network_getNodeUserProperties__I(env, obj, ValidateNodeId(env, obj, nodeId)));
}

JNIEXPORT void JNICALL Java_smile_Network_setNodeUserProperties__I_3Lsmile_UserProperty_2(JNIEnv *env, jobject obj, jint nodeHandle, jobjectArray properties) 
{
	JSMILE_ENTER;
    DSL_node *node = ValidateNodeHandle(env, obj, nodeHandle);
	SetUserProps(env, properties, node->Info().UserProperties());
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_setNodeUserProperties__Ljava_lang_String_2_3Lsmile_UserProperty_2(JNIEnv *env, jobject obj, jstring nodeId, jobjectArray properties) 
{
	JSMILE_ENTER;
    Java_smile_Network_setNodeUserProperties__I_3Lsmile_UserProperty_2(env, obj, ValidateNodeId(env, obj, nodeId), properties);
    JSMILE_LEAVE;
}

//-------------------------------------------------------------------------------
// Documentation
//-------------------------------------------------------------------------------

JNIEXPORT jobjectArray JNICALL Java_smile_Network_getNodeDocumentation__I(JNIEnv *env, jobject obj, jint nodeHandle)
{
    JSMILE_ENTER;
    DSL_node *node = ValidateNodeHandle(env, obj, nodeHandle);
    JSMILE_RETURN(GetDocumentation(env, node->Info().Documentation()));
}

JNIEXPORT jobjectArray JNICALL Java_smile_Network_getNodeDocumentation__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId)
{
    JSMILE_ENTER;
    JSMILE_RETURN(Java_smile_Network_getNodeDocumentation__I(env, obj, ValidateNodeId(env, obj, nodeId)));
}

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_Network_setNodeDocumentation__I_3Lsmile_DocItemInfo_2(JNIEnv *env, jobject obj, jint nodeHandle, jobjectArray documentation)
{
    JSMILE_ENTER;
    DSL_node *node = ValidateNodeHandle(env, obj, nodeHandle);
    SetDocumentation(env, documentation, node->Info().Documentation());
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_setNodeDocumentation__Ljava_lang_String_2_3Lsmile_DocItemInfo_2(JNIEnv * env, jobject obj, jstring nodeId, jobjectArray documentation)
{
    JSMILE_ENTER;
    Java_smile_Network_setNodeDocumentation__I_3Lsmile_DocItemInfo_2(env, obj, ValidateNodeId(env, obj, nodeId), documentation);
    JSMILE_LEAVE;
}

//-------------------------------------------------------------------------------

JNIEXPORT jobjectArray JNICALL Java_smile_Network_getOutcomeDocumentation__II(JNIEnv *env, jobject obj, jint nodeHandle, jint outcomeIndex) 
{
	JSMILE_ENTER;
    DSL_node *node = ValidateOutcomeIndex(env, obj, nodeHandle, outcomeIndex);
	JSMILE_RETURN(GetDocumentation(env, node->ExtraDefinition()->GetDocumentation(outcomeIndex)));
}

JNIEXPORT jobjectArray JNICALL Java_smile_Network_getOutcomeDocumentation__Ljava_lang_String_2I(JNIEnv *env, jobject obj, jstring nodeId, jint outcomeIndex) 
{
	JSMILE_ENTER;
    JSMILE_RETURN(Java_smile_Network_getOutcomeDocumentation__II(env, obj, ValidateNodeId(env, obj, nodeId), outcomeIndex));
}

JNIEXPORT jobjectArray JNICALL Java_smile_Network_getOutcomeDocumentation__ILjava_lang_String_2(JNIEnv *env, jobject obj, jint nodeHandle, jstring outcomeId) 
{
    JSMILE_ENTER;
    JSMILE_RETURN(Java_smile_Network_getOutcomeDocumentation__II(env, obj, nodeHandle, ValidateOutcomeId(env, obj, nodeHandle, outcomeId)));
}


JNIEXPORT jobjectArray JNICALL Java_smile_Network_getOutcomeDocumentation__Ljava_lang_String_2Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId, jstring outcomeId) 
{
	JSMILE_ENTER;
    JSMILE_RETURN(Java_smile_Network_getOutcomeDocumentation__ILjava_lang_String_2(env, obj, ValidateNodeId(env, obj, nodeId), outcomeId));
}

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_Network_setOutcomeDocumentation__II_3Lsmile_DocItemInfo_2(JNIEnv *env, jobject obj, jint nodeHandle, jint outcomeIndex, jobjectArray documentation) 
{
	JSMILE_ENTER;
    DSL_node *node = ValidateOutcomeIndex(env, obj, nodeHandle, outcomeIndex);
    SetDocumentation(env, documentation, node->ExtraDefinition()->GetDocumentation(outcomeIndex));
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_setOutcomeDocumentation__ILjava_lang_String_2_3Lsmile_DocItemInfo_2(JNIEnv *env, jobject obj, jint nodeHandle, jstring outcomeId, jobjectArray documentation) 
{
    JSMILE_ENTER;
    Java_smile_Network_setOutcomeDocumentation__II_3Lsmile_DocItemInfo_2(env, obj, nodeHandle, ValidateOutcomeId(env, obj, nodeHandle, outcomeId), documentation);
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_setOutcomeDocumentation__Ljava_lang_String_2I_3Lsmile_DocItemInfo_2(JNIEnv *env, jobject obj, jstring nodeId, jint outcomeIndex, jobjectArray documentation) 
{
    JSMILE_ENTER;
    Java_smile_Network_setOutcomeDocumentation__II_3Lsmile_DocItemInfo_2(env, obj, ValidateNodeId(env, obj, nodeId), outcomeIndex, documentation);
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_setOutcomeDocumentation__Ljava_lang_String_2Ljava_lang_String_2_3Lsmile_DocItemInfo_2(JNIEnv *env, jobject obj, jstring nodeId, jstring outcomeId, jobjectArray documentation) 
{
	JSMILE_ENTER;
    Java_smile_Network_setOutcomeDocumentation__ILjava_lang_String_2_3Lsmile_DocItemInfo_2(env, obj, ValidateNodeId(env, obj, nodeId), outcomeId, documentation);
    JSMILE_LEAVE;
}

//-------------------------------------------------------------------------------

JNIEXPORT jboolean JNICALL Java_smile_Network_isRanked__I(JNIEnv *env, jobject obj, jint nodeHandle) 
{
	JSMILE_ENTER;
    DSL_extraDefinition *ed = GetExtraDef(env, obj, nodeHandle);
    JSMILE_RETURN(ed->IsRanked());
}

JNIEXPORT jboolean JNICALL Java_smile_Network_isRanked__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId) 
{
    JSMILE_ENTER;
    JSMILE_RETURN(Java_smile_Network_isRanked__I(env, obj, ValidateNodeId(env, obj, nodeId)));
}

JNIEXPORT void JNICALL Java_smile_Network_setRanked__IZ(JNIEnv *env, jobject obj, jint nodeHandle, jboolean ranked) 
{
    JSMILE_ENTER;
    DSL_extraDefinition *ed = GetExtraDef(env, obj, nodeHandle);
    ed->SetFlags(0 != ranked, ed->IsMandatory(), ed->IsSetToDefault());
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_setRanked__Ljava_lang_String_2Z(JNIEnv *env, jobject obj, jstring nodeId, jboolean ranked)
{
    JSMILE_ENTER;
    Java_smile_Network_setRanked__IZ(env, obj, ValidateNodeId(env, obj, nodeId), ranked);
    JSMILE_LEAVE;
}

//-------------------------------------------------------------------------------

JNIEXPORT jboolean JNICALL Java_smile_Network_isMandatory__I(JNIEnv *env, jobject obj, jint nodeHandle) 
{
	JSMILE_ENTER;
    DSL_extraDefinition *ed = GetExtraDef(env, obj, nodeHandle);
    JSMILE_RETURN(ed->IsMandatory());
}

JNIEXPORT jboolean JNICALL Java_smile_Network_isMandatory__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId) 
{
    JSMILE_ENTER;
    JSMILE_RETURN(Java_smile_Network_isMandatory__I(env, obj, ValidateNodeId(env, obj, nodeId)));
}

JNIEXPORT void JNICALL Java_smile_Network_setMandatory__IZ(JNIEnv *env, jobject obj, jint nodeHandle, jboolean mandatory) 
{
    JSMILE_ENTER;
    DSL_extraDefinition *ed = GetExtraDef(env, obj, nodeHandle);
    ed->SetFlags(ed->IsRanked(), 0 != mandatory, ed->IsSetToDefault());
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_setMandatory__Ljava_lang_String_2Z(JNIEnv *env, jobject obj, jstring nodeId, jboolean mandatory)
{
    JSMILE_ENTER;
    Java_smile_Network_setMandatory__IZ(env, obj, ValidateNodeId(env, obj, nodeId), mandatory);
    JSMILE_LEAVE;
}

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_Network_addCostArc__II(JNIEnv *env, jobject obj, jint parentHandle, jint childHandle) 
{
    JSMILE_ENTER;
    AddArcHelper(env, obj, parentHandle, childHandle, dsl_costObserve);
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_addCostArc__Ljava_lang_String_2Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring parentId, jstring childId) 
{
	JSMILE_ENTER;
    AddArcHelper(env, obj, ValidateNodeId(env, obj, parentId), ValidateNodeId(env, obj, childId), dsl_costObserve);
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_deleteCostArc__II(JNIEnv *env, jobject obj, jint parentHandle, jint childHandle)
{
	JSMILE_ENTER;
    DelArcHelper(env, obj, parentHandle, childHandle, dsl_costObserve);
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_deleteCostArc__Ljava_lang_String_2Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring parentId, jstring childId) 
{
    JSMILE_ENTER;
    DelArcHelper(env, obj, ValidateNodeId(env, obj, parentId), ValidateNodeId(env, obj, childId), dsl_costObserve);
    JSMILE_LEAVE;
}

//-------------------------------------------------------------------------------

JNIEXPORT jintArray JNICALL Java_smile_Network_getCostParents__I(JNIEnv *env, jobject obj, jint nodeHandle) 
{
	JSMILE_ENTER;
    JSMILE_RETURN(GetParentsHelper(env, obj, nodeHandle, dsl_costObserve));
}

JNIEXPORT jintArray JNICALL Java_smile_Network_getCostParents__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId) 
{
	JSMILE_ENTER;
    JSMILE_RETURN(Java_smile_Network_getCostParents__I(env, obj, ValidateNodeId(env, obj, nodeId)));
}

JNIEXPORT jobjectArray JNICALL Java_smile_Network_getCostParentIds__I(JNIEnv *env, jobject obj, jint nodeHandle) 
{
    JSMILE_ENTER;
    JSMILE_RETURN(GetParentIdsHelper(env, obj, nodeHandle, dsl_costObserve));
}

JNIEXPORT jobjectArray JNICALL Java_smile_Network_getCostParentIds__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId) 
{
	JSMILE_ENTER;
    JSMILE_RETURN(Java_smile_Network_getCostParentIds__I(env, obj, ValidateNodeId(env, obj, nodeId)));
}

JNIEXPORT jintArray JNICALL Java_smile_Network_getCostChildren__I(JNIEnv *env, jobject obj, jint nodeHandle) 
{
	JSMILE_ENTER;
    JSMILE_RETURN(GetChildrenHelper(env, obj, nodeHandle, dsl_costObserve));
}

JNIEXPORT jintArray JNICALL Java_smile_Network_getCostChildren__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId) 
{
	JSMILE_ENTER;
    JSMILE_RETURN(Java_smile_Network_getCostChildren__I(env, obj, ValidateNodeId(env, obj, nodeId)));
}

JNIEXPORT jobjectArray JNICALL Java_smile_Network_getCostChildIds__I(JNIEnv *env, jobject obj, jint nodeHandle) 
{
    JSMILE_ENTER;
    JSMILE_RETURN(GetChildIdsHelper(env, obj, nodeHandle, dsl_costObserve));
}

JNIEXPORT jobjectArray JNICALL Java_smile_Network_getCostChildIds__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId) 
{
	JSMILE_ENTER;
    JSMILE_RETURN(Java_smile_Network_getCostChildIds__I(env, obj, ValidateNodeId(env, obj, nodeId)));
}

//-------------------------------------------------------------------------------

JNIEXPORT jdoubleArray JNICALL Java_smile_Network_getNodeCost__I(JNIEnv *env, jobject obj, jint nodeHandle) 
{
	JSMILE_ENTER;
	DSL_node *node = ValidateNodeHandle(env, obj, nodeHandle);
    JSMILE_RETURN(CopyDoubleArray(env, node->ObservCost()->GetCosts().GetItems()));
}

JNIEXPORT jdoubleArray JNICALL Java_smile_Network_getNodeCost__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId) 
{
    JSMILE_ENTER;
    JSMILE_RETURN(Java_smile_Network_getNodeCost__I(env, obj, ValidateNodeId(env, obj, nodeId)));
}

JNIEXPORT void JNICALL Java_smile_Network_setNodeCost__I_3D(JNIEnv *env, jobject obj, jint nodeHandle, jdoubleArray cost)
{
    JSMILE_ENTER;
    
	DSL_node *node = ValidateNodeHandle(env, obj, nodeHandle);
    DSL_Dmatrix &mtx = node->ObservCost()->GetCosts();

    int newCostSize = env->GetArrayLength(cost);
    int expCostSize = mtx.GetSize();
    if (newCostSize != expCostSize)
    {
        string msg;
		msg = "Invalid cost array size for node '";
		msg += node->Info().Header().GetId();
		msg += "': expected ";
		DSL_appendInt(msg, expCostSize);
		msg += " and got ";
		DSL_appendInt(msg, newCostSize);
        throw invalid_argument(msg);
    }
    
	env->GetDoubleArrayRegion(cost, 0, expCostSize, mtx.GetItems().Items());
    
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_setNodeCost__Ljava_lang_String_2_3D(JNIEnv *env, jobject obj, jstring nodeId, jdoubleArray cost) 
{
    JSMILE_ENTER;
    Java_smile_Network_setNodeCost__I_3D(env, obj, ValidateNodeId(env, obj, nodeId), cost);
    JSMILE_LEAVE;
}

//-------------------------------------------------------------------------------

// DBN (dynamic networks) support

JNIEXPORT jint JNICALL Java_smile_Network_getSliceCount(JNIEnv *env, jobject obj)
{
    JSMILE_ENTER;
    JSMILE_RETURN(GetPtr(env, obj)->GetNumberOfSlices());
}

JNIEXPORT void JNICALL Java_smile_Network_setSliceCount(JNIEnv *env, jobject obj, jint sliceCount) 
{
    JSMILE_ENTER;
    GetPtr(env, obj)->SetNumberOfSlices(sliceCount);
    JSMILE_LEAVE;
}

JNIEXPORT jint JNICALL Java_smile_Network_getNodeTemporalType__I(JNIEnv *env, jobject obj, jint nodeHandle) 
{
    JSMILE_ENTER;
	DSL_network *net = GetPtr(env, obj);
	ValidateNodeHandle(net, nodeHandle);
	JSMILE_RETURN(net->GetTemporalType(nodeHandle));
}

JNIEXPORT jint JNICALL Java_smile_Network_getNodeTemporalType__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId) 
{
    JSMILE_ENTER;
    JSMILE_RETURN(Java_smile_Network_getNodeTemporalType__I(env, obj, ValidateNodeId(env, obj, nodeId)));
}

JNIEXPORT void JNICALL Java_smile_Network_setNodeTemporalType__II(JNIEnv *env, jobject obj, jint nodeHandle, jint temporalType)
{
	JSMILE_ENTER;
	DSL_network *net = GetPtr(env, obj);
	ValidateNodeHandle(net, nodeHandle);
	int res = net->SetTemporalType(nodeHandle, static_cast<dsl_temporalType>(temporalType));
    if (DSL_OKAY != res)
    {
        ThrowSmileException(env, "setNodeTemporalType", res);
    }
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_setNodeTemporalType__Ljava_lang_String_2I(JNIEnv *env, jobject obj, jstring nodeId, jint temporalType)
{
	JSMILE_ENTER;
    Java_smile_Network_setNodeTemporalType__II(env, obj, ValidateNodeId(env, obj, nodeId), temporalType);
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_addTemporalArc__III(JNIEnv *env, jobject obj, jint parentHandle, jint childHandle, jint order) 
{
    JSMILE_ENTER;
    ErrorH.Flush();	
	DSL_network *net = GetPtr(env, obj);
	ValidateNodeHandle(net, parentHandle);
	ValidateNodeHandle(net, childHandle);
	int res = net->AddTemporalArc(parentHandle, childHandle, order);
	if (DSL_OKAY != res) 
    {
        ThrowSmileException(env, "AddTemporalArc", res);
	}
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_addTemporalArc__Ljava_lang_String_2Ljava_lang_String_2I(JNIEnv *env, jobject obj, jstring parentId, jstring childId, jint order) 
{
	JSMILE_ENTER;
	Java_smile_Network_addTemporalArc__III(env, obj, ValidateNodeId(env, obj, parentId), ValidateNodeId(env, obj, childId), order);
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_deleteTemporalArc__III(JNIEnv *env, jobject obj, jint parentHandle, jint childHandle, jint order) 
{
    JSMILE_ENTER;
    ErrorH.Flush();	
	DSL_network *net = GetPtr(env, obj);
	ValidateNodeHandle(net, parentHandle);
	ValidateNodeHandle(net, childHandle);
	int res = net->RemoveTemporalArc(parentHandle, childHandle, order);
	if (DSL_OKAY != res) 
    {
        ThrowSmileException(env, "RemoveTemporalArc", res);
	}
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_deleteTemporalArc__Ljava_lang_String_2Ljava_lang_String_2I(JNIEnv *env, jobject obj, jstring parentId, jstring childId, jint order) 
{
	JSMILE_ENTER;
	Java_smile_Network_deleteTemporalArc__III(env, obj, ValidateNodeId(env, obj, parentId), ValidateNodeId(env, obj, childId), order);
    JSMILE_LEAVE;
}

JNIEXPORT jboolean JNICALL Java_smile_Network_temporalArcExists__III(JNIEnv *env, jobject obj, jint parentHandle, jint childHandle, jint order) 
{
    JSMILE_ENTER;
	DSL_network *net = GetPtr(env, obj);
	ValidateNodeHandle(net, parentHandle);
	ValidateNodeHandle(net, childHandle);
	JSMILE_RETURN(net->TemporalArcExists(parentHandle, childHandle, order));
}

JNIEXPORT jboolean JNICALL Java_smile_Network_temporalArcExists__Ljava_lang_String_2Ljava_lang_String_2I(JNIEnv *env, jobject obj, jstring parentId, jstring childId, jint order) 
{
	JSMILE_ENTER;
	JSMILE_RETURN(Java_smile_Network_temporalArcExists__III(env, obj, ValidateNodeId(env, obj, parentId), ValidateNodeId(env, obj, childId), order));
}

JNIEXPORT jint JNICALL Java_smile_Network_getMaxTemporalOrder(JNIEnv *env, jobject obj)
{
    JSMILE_ENTER;
    JSMILE_RETURN(GetPtr(env, obj)->GetMaxTemporalOrder());
}

JNIEXPORT jint JNICALL Java_smile_Network_getMaxNodeTemporalOrder__I(JNIEnv *env, jobject obj, jint nodeHandle) 
{
    JSMILE_ENTER;
	DSL_network *net = GetPtr(env, obj);
	ValidateNodeHandle(net, nodeHandle);
	JSMILE_RETURN(net->GetMaxTemporalOrder(nodeHandle));
}

JNIEXPORT jint JNICALL Java_smile_Network_getMaxNodeTemporalOrder__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId) 
{
    JSMILE_ENTER;
    JSMILE_RETURN(Java_smile_Network_getMaxNodeTemporalOrder__I(env, obj, ValidateNodeId(env, obj, nodeId)));
}

JNIEXPORT jintArray JNICALL Java_smile_Network_getTemporalOrders__I(JNIEnv *env, jobject obj, jint nodeHandle) 
{
	JSMILE_ENTER;
	DSL_network *net = GetPtr(env, obj);
	ValidateNodeHandle(net, nodeHandle);
	DSL_intArray nativeOrders;
	int res = net->GetTemporalOrders(nodeHandle, nativeOrders);
	if (DSL_OKAY != res)
	{
		ThrowSmileException(env, "GetTemporalOrders", res);
	}
	JSMILE_RETURN(CopyIntArray(env, nativeOrders));
}

JNIEXPORT jintArray JNICALL Java_smile_Network_getTemporalOrders__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId) 
{
	JSMILE_ENTER;
    JSMILE_RETURN(Java_smile_Network_getTemporalOrders__I(env, obj, ValidateNodeId(env, obj, nodeId)));
}

JNIEXPORT jobjectArray JNICALL Java_smile_Network_getTemporalChildren__I(JNIEnv *env, jobject obj, jint nodeHandle)
{
	JSMILE_ENTER;
	DSL_network *net = GetPtr(env, obj);
	ValidateNodeHandle(net, nodeHandle);
	vector<pair<int, int> > native;
	int res = net->GetTemporalChildren(nodeHandle, native);
	if (DSL_OKAY != res)
	{
		ThrowSmileException(env, "GetTemporalChildren", res);
	}
	JSMILE_RETURN(GetTemporalInfo(env, net, native));
}

JNIEXPORT jobjectArray JNICALL Java_smile_Network_getTemporalChildren__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId)
{
	JSMILE_ENTER;
	JSMILE_RETURN(Java_smile_Network_getTemporalChildren__I(env, obj, ValidateNodeId(env, obj, nodeId)));
}

JNIEXPORT jobjectArray JNICALL Java_smile_Network_getTemporalParents__II(JNIEnv *env, jobject obj, jint nodeHandle, jint order)
{
	JSMILE_ENTER;
	DSL_network *net = GetPtr(env, obj);
	ValidateNodeHandle(net, nodeHandle);
	DSL_intArray native1;
	int res = net->GetTemporalParents(nodeHandle, order, native1);
	if (DSL_OKAY != res)
	{
		ThrowSmileException(env, "GetTemporalParents", res);
	}
	int count = native1.NumItems();
	vector<pair<int, int> > native2(count);
	for (int i = 0; i < count; i ++)
	{
		native2[i].first = native1[i];
		native2[i].second = order;
	}
	JSMILE_RETURN(GetTemporalInfo(env, net, native2));
}

JNIEXPORT jobjectArray JNICALL Java_smile_Network_getTemporalParents__Ljava_lang_String_2I(JNIEnv *env, jobject obj, jstring nodeId, jint order)
{
	JSMILE_ENTER;
    JSMILE_RETURN(Java_smile_Network_getTemporalParents__II(env, obj, ValidateNodeId(env, obj, nodeId), order));
}

JNIEXPORT jobjectArray JNICALL Java_smile_Network_getUnrolledParents__II(JNIEnv *env, jobject obj, jint nodeHandle, jint order)
{
	JSMILE_ENTER;
	DSL_network *net = GetPtr(env, obj);
	ValidateNodeHandle(net, nodeHandle);
	vector<pair<int, int> > native;
	int res = net->GetUnrolledParents(nodeHandle, order, native);
	if (DSL_OKAY != res)
	{
		ThrowSmileException(env, "GetUnrolledParents", res);
	}
	JSMILE_RETURN(GetTemporalInfo(env, net, native));
}

JNIEXPORT jobjectArray JNICALL Java_smile_Network_getUnrolledParents__Ljava_lang_String_2I(JNIEnv *env, jobject obj, jstring nodeId, jint order)
{
	JSMILE_ENTER;
    JSMILE_RETURN(Java_smile_Network_getUnrolledParents__II(env, obj, ValidateNodeId(env, obj, nodeId), order));
}

JNIEXPORT jobjectArray JNICALL Java_smile_Network_getUnrolledParents__I(JNIEnv *env, jobject obj, jint nodeHandle)
{
	JSMILE_ENTER;
	DSL_network *net = GetPtr(env, obj);
	ValidateNodeHandle(net, nodeHandle);
	vector<pair<int, int> > native;
	int res = net->GetUnrolledParents(nodeHandle, native);
	if (DSL_OKAY != res)
	{
		ThrowSmileException(env, "GetTemporalParents", res);
	}
	JSMILE_RETURN(GetTemporalInfo(env, net, native));
}

JNIEXPORT jobjectArray JNICALL Java_smile_Network_getUnrolledParents__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId)
{
	JSMILE_ENTER;
	JSMILE_RETURN(Java_smile_Network_getUnrolledParents__I(env, obj, ValidateNodeId(env, obj, nodeId)));
}

JNIEXPORT jobject Java_smile_Network_unroll(JNIEnv *env, jobject obj)
{
	JSMILE_ENTER;

	DSL_network *net = GetPtr(env, obj);
	jobject unrolled = CreateNewNetwork(env);
	DSL_network *nativeUnrolled = GetPtr(env, unrolled);
	vector<int> nativeMapping;
	int res = net->UnrollNetwork(*nativeUnrolled, nativeMapping);
	if (DSL_OKAY != res)
	{
		ThrowSmileException(env, "UnrollNetwork", res);
	}

	int count = int(nativeMapping.size());
	jintArray mapping = env->NewIntArray(count);
	env->SetIntArrayRegion(mapping, 0, count, (jint *)(&nativeMapping.front()));

	jclass clazz = env->FindClass("smile/UnrollResults");
	jmethodID mid = env->GetMethodID(clazz, "<init>", "(Lsmile/Network;[I)V");
    JSMILE_RETURN(env->NewObject(clazz, mid, unrolled, mapping));
}

JNIEXPORT jboolean JNICALL Java_smile_Network_hasTemporalEvidence__I(JNIEnv *env, jobject obj, jint nodeHandle) 
{
	JSMILE_ENTER;
    DSL_node *node = ValidateNodeHandle(env, obj, nodeHandle);
    JSMILE_RETURN(0 != node->Value()->HasTemporalEvidence())
}

JNIEXPORT jboolean JNICALL Java_smile_Network_hasTemporalEvidence__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId) 
{
    JSMILE_ENTER;
    JSMILE_RETURN(Java_smile_Network_hasTemporalEvidence__I(env, obj, ValidateNodeId(env, obj, nodeId)));
}

JNIEXPORT jboolean JNICALL Java_smile_Network_isTemporalEvidence__II(JNIEnv *env, jobject obj, jint nodeHandle, jint slice) 
{
	JSMILE_ENTER;
    DSL_node *node = ValidateNodeHandle(env, obj, nodeHandle);
    JSMILE_RETURN(0 != node->Value()->IsTemporalEvidence(slice))
}

JNIEXPORT jboolean JNICALL Java_smile_Network_isTemporalEvidence__Ljava_lang_String_2I(JNIEnv *env, jobject obj, jstring nodeId, jint slice) 
{
    JSMILE_ENTER;
	JSMILE_RETURN(Java_smile_Network_isTemporalEvidence__II(env, obj, ValidateNodeId(env, obj, nodeId), slice));
}

JNIEXPORT jint JNICALL Java_smile_Network_getTemporalEvidence__II(JNIEnv *env, jobject obj, jint nodeHandle, jint slice) 
{
	JSMILE_ENTER;
    DSL_node *node = ValidateNodeHandle(env, obj, nodeHandle);
	int res = node->Value()->GetTemporalEvidence(slice);
	if (res < 0) 
    {
	    ThrowSmileException(env, "GetTemporalEvidence", res);
	}
	JSMILE_RETURN(res)
}

JNIEXPORT jint JNICALL Java_smile_Network_getTemporalEvidence__Ljava_lang_String_2I(JNIEnv *env, jobject obj, jstring nodeId, jint slice) 
{
    JSMILE_ENTER;
	JSMILE_RETURN(Java_smile_Network_getTemporalEvidence__II(env, obj, ValidateNodeId(env, obj, nodeId), slice));
}

JNIEXPORT jstring JNICALL Java_smile_Network_getTemporalEvidenceId__II(JNIEnv *env, jobject obj, jint nodeHandle, jint slice) 
{
	JSMILE_ENTER;
    jstring res = NULL;
	int e = Java_smile_Network_getTemporalEvidence__II(env, obj, nodeHandle, slice);
    if (e >= 0)
    {
        res = Java_smile_Network_getOutcomeId__II(env, obj, nodeHandle, e);
    }
	JSMILE_RETURN(res);
}

JNIEXPORT jstring JNICALL Java_smile_Network_getTemporalEvidenceId__Ljava_lang_String_2I(JNIEnv *env, jobject obj, jstring nodeId, jint slice) 
{
    JSMILE_ENTER;
	JSMILE_RETURN(Java_smile_Network_getTemporalEvidenceId__II(env, obj, ValidateNodeId(env, obj, nodeId), slice));
}

JNIEXPORT void JNICALL Java_smile_Network_setTemporalEvidence__III(JNIEnv *env, jobject obj, jint nodeHandle, jint slice, jint outcomeIndex) 
{
    JSMILE_ENTER;
    DSL_node *node = ValidateOutcomeIndex(env, obj, nodeHandle, outcomeIndex);
	int res = node->Value()->SetTemporalEvidence(slice, outcomeIndex);
	if (DSL_OKAY != res) 
    {
        ThrowSmileException(env, "SetTemporalEvidence", res);
    }
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_setTemporalEvidence__Ljava_lang_String_2II(JNIEnv *env, jobject obj, jstring nodeId, jint slice, jint outcomeIndex) 
{
    JSMILE_ENTER;
	Java_smile_Network_setTemporalEvidence__III(env, obj, ValidateNodeId(env, obj, nodeId), slice, outcomeIndex);
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_setTemporalEvidence__IILjava_lang_String_2(JNIEnv *env, jobject obj, jint nodeHandle, jint slice, jstring outcomeId) 
{
    JSMILE_ENTER;
	Java_smile_Network_setTemporalEvidence__III(env, obj, nodeHandle, slice, ValidateOutcomeId(env, obj, nodeHandle, outcomeId));
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_setTemporalEvidence__Ljava_lang_String_2ILjava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId, jint slice, jstring outcomeId) 
{
    JSMILE_ENTER;
    Java_smile_Network_setTemporalEvidence__IILjava_lang_String_2(env, obj, ValidateNodeId(env, obj, nodeId), slice, outcomeId);
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_clearTemporalEvidence__II(JNIEnv *env, jobject obj, jint nodeHandle, jint slice) 
{
	JSMILE_ENTER;
	DSL_node *node = ValidateNodeHandle(env, obj, nodeHandle);
	int res = node->Value()->ClearTemporalEvidence(slice);
    if (DSL_OKAY != res) 
    {
	    ThrowSmileException(env, "ClearTemporalEvidence", res);
	}
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_clearTemporalEvidence__Ljava_lang_String_2I(JNIEnv *env, jobject obj, jstring nodeId, jint slice) 
{
    JSMILE_ENTER;
	Java_smile_Network_clearTemporalEvidence__II(env, obj, ValidateNodeId(env, obj, nodeId), slice);
	JSMILE_LEAVE;
}

JNIEXPORT jdoubleArray JNICALL Java_smile_Network_getNodeTemporalDefinition__II(JNIEnv *env, jobject obj, jint nodeHandle, jint order) 
{
	JSMILE_ENTER;

	DSL_network *net = GetPtr(env, obj);
	DSL_node *node = ValidateNodeHandle(net, nodeHandle);
	const DSL_doubleArray* arr = GetTemporalDefinitionArray(node, order);

	JSMILE_RETURN(CopyDoubleArray(env, *const_cast<DSL_doubleArray *>(arr)));
}

JNIEXPORT jdoubleArray JNICALL Java_smile_Network_getNodeTemporalDefinition__Ljava_lang_String_2I(JNIEnv *env, jobject obj, jstring nodeId, jint order) 
{
    JSMILE_ENTER;
	JSMILE_RETURN(Java_smile_Network_getNodeTemporalDefinition__II(env, obj, ValidateNodeId(env, obj, nodeId), order));
}

JNIEXPORT void JNICALL Java_smile_Network_setNodeTemporalDefinition__II_3D(JNIEnv *env, jobject obj, jint nodeHandle, jint order, jdoubleArray definition) 
{
	JSMILE_ENTER;

	DSL_network *net = GetPtr(env, obj);
	DSL_node *node = ValidateNodeHandle(net, nodeHandle);
	const DSL_doubleArray* arr = GetTemporalDefinitionArray(node, order);

	ValidateDefinitionSize(env, node, *arr, definition);

	DSL_doubleArray native = *arr;

	env->GetDoubleArrayRegion(definition, 0, native.GetSize(), native.Items());

	DSL_nodeDefinition *def = node->Definition();
	switch (def->GetType())
	{
	case DSL_NOISY_MAX:
		{
			DSL_noisyMAX *noisy = static_cast<DSL_noisyMAX *>(def);
			DSL_Dmatrix mtx = *noisy->GetTemporalCiWeights(order);
			mtx.GetItems() = native;
			int res = noisy->SetTemporalCiWeights(order, mtx);
			if (DSL_OKAY != res)
			{
				ThrowSmileException(env, "SetTemporalCiWeights", res);
			}
			break;
		}
	default:
		{
			int res = def->SetTemporalDefinition(order, native);
			if (DSL_OKAY != res)
			{
				ThrowSmileException(env, "SetTemporalDefinition", res);
			}
		}
		break;
	}

	DSL_nodeValue *nodeValue = node->Value();
	if (NULL != nodeValue) nodeValue->SetValueInvalid();

	JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_setNodeTemporalDefinition__Ljava_lang_String_2I_3D(JNIEnv *env, jobject obj, jstring nodeId, jint order, jdoubleArray definition) 
{
	JSMILE_ENTER;
	Java_smile_Network_setNodeTemporalDefinition__II_3D(env, obj, ValidateNodeId(env, obj, nodeId), order, definition);
	JSMILE_LEAVE;
}

JNIEXPORT jobjectArray JNICALL Java_smile_Network_getNoisyTemporalParentStrengths__II(JNIEnv *env, jobject obj, jint nodeHandle, jint order)
{
	JSMILE_ENTER;

	DSL_noisyMAX* noisy = GetNoisyDef(env, obj, nodeHandle);
	vector<DSL_intArray> native;
	noisy->GetTemporalParentOutcomeStrengths(order, native);

	jclass intArrCls = env->FindClass("[I");

	int parentCount = int(native.size());
	jobjectArray arr = env->NewObjectArray(parentCount, intArrCls, NULL);
	if (NULL != arr)
	{
		for (int i = 0; i < parentCount; i ++)
		{
			int count = native[i].NumItems();;
			jintArray strengths = env->NewIntArray(count);
			if (NULL == strengths)
			{
				break;
			}

			env->SetIntArrayRegion(strengths, 0, count, (const jint *)native[i].Items());
			env->SetObjectArrayElement(arr, i, strengths);
		}
	}
	
	JSMILE_RETURN(arr);
}

JNIEXPORT jobjectArray JNICALL Java_smile_Network_getNoisyTemporalParentStrengths__Ljava_lang_String_2I(JNIEnv *env, jobject obj, jstring nodeId, jint order)
{
	JSMILE_ENTER;
	JSMILE_RETURN(Java_smile_Network_getNoisyTemporalParentStrengths__II(env, obj, ValidateNodeId(env, obj, nodeId), order));
}


JNIEXPORT void JNICALL Java_smile_Network_setNoisyTemporalParentStrengths__II_3_3I(JNIEnv *env, jobject obj, jint nodeHandle, jint order, jobjectArray strengths)
{
	JSMILE_ENTER;

	DSL_noisyMAX* noisy = GetNoisyDef(env, obj, nodeHandle);

	int parentCount = env->GetArrayLength(strengths);
	vector<DSL_intArray> native(parentCount);
	for (int i = 0; i < parentCount; i ++)
	{
		jintArray s = static_cast<jintArray>(env->GetObjectArrayElement(strengths, i));
		int elemCount = env->GetArrayLength(s);
		for (int j = 0; j < elemCount; j ++)
		{
			jint x;
			env->GetIntArrayRegion(s, j, 1, &x);
			native[i].Add(x);
		}
	}
	
	int res = noisy->SetTemporalParentOutcomeStrengths(order, native);
	if (DSL_OKAY != res)
	{
		ThrowSmileException(env, "SetTemporalParentOutcomeStrengths", res);
	}
	
	JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_setNoisyTemporalParentStrengths__Ljava_lang_String_2I_3_3I(JNIEnv *env, jobject obj, jstring nodeId, jint order, jobjectArray strengths)
{
	JSMILE_ENTER;
	Java_smile_Network_setNoisyTemporalParentStrengths__II_3_3I(env, obj, ValidateNodeId(env, obj, nodeId), order, strengths);
	JSMILE_LEAVE;
}


JNIEXPORT void JNICALL Java_smile_Network_setNodeEquation__ILjava_lang_String_2(JNIEnv *env, jobject obj, jint nodeHandle, jstring equation)
{
	JSMILE_ENTER;
	DSL_equation *def = GetEqDef(env, obj, nodeHandle);
	int errPos;
	string errMsg;
	NativeString nativeEq(env, equation);
	int res = def->SetEquation(nativeEq.GetChars(), &errPos, &errMsg);
	if (DSL_OKAY != res)
	{
		string exMsg = "Invalid equation, error at position ";
		DSL_appendInt(exMsg, errPos);
		exMsg += ": ";
		exMsg += errMsg;
		throw invalid_argument(exMsg);
	}
	JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_setNodeEquation__Ljava_lang_String_2Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId, jstring equation)
{
	JSMILE_ENTER;
	Java_smile_Network_setNodeEquation__ILjava_lang_String_2(env, obj, ValidateNodeId(env, obj, nodeId), equation);
	JSMILE_LEAVE;
}

JNIEXPORT jstring JNICALL Java_smile_Network_getNodeEquation__I(JNIEnv *env, jobject obj, jint nodeHandle)
{
	JSMILE_ENTER;
	DSL_equation *def = GetEqDef(env, obj, nodeHandle);
	string eq;
	def->GetEquation().Write(eq);
	JSMILE_RETURN(env->NewStringUTF(eq.c_str()));
}

JNIEXPORT jstring JNICALL Java_smile_Network_getNodeEquation__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId)
{
    JSMILE_ENTER;
	JSMILE_RETURN(Java_smile_Network_getNodeEquation__I(env, obj, ValidateNodeId(env, obj, nodeId)));
}

JNIEXPORT void JNICALL Java_smile_Network_setNodeEquationBounds__IDD(JNIEnv *env, jobject obj, jint nodeHandle, jdouble lo, jdouble hi)
{
	JSMILE_ENTER;
	DSL_equation *def = GetEqDef(env, obj, nodeHandle);
	def->SetBounds(lo, hi);
	JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_setNodeEquationBounds__Ljava_lang_String_2DD(JNIEnv *env, jobject obj, jstring nodeId, jdouble lo, jdouble hi)
{
	JSMILE_ENTER;
	Java_smile_Network_setNodeEquationBounds__IDD(env, obj, ValidateNodeId(env, obj, nodeId), lo, hi);
	JSMILE_LEAVE;
}

JNIEXPORT jdoubleArray JNICALL Java_smile_Network_getNodeEquationBounds__I(JNIEnv *env, jobject obj, jint nodeHandle)
{
	JSMILE_ENTER;
	DSL_equation *def = GetEqDef(env, obj, nodeHandle);
	double bounds[2];
	def->GetBounds(bounds[0], bounds[1]);
    jdoubleArray res = env->NewDoubleArray(2);
	env->SetDoubleArrayRegion(res, 0, 2, bounds);
	JSMILE_RETURN(res);
}

JNIEXPORT jdoubleArray JNICALL Java_smile_Network_getNodeEquationBounds__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId)
{
	JSMILE_ENTER;
	JSMILE_RETURN(Java_smile_Network_getNodeEquationBounds__I(env, obj, ValidateNodeId(env, obj, nodeId)));
}

JNIEXPORT void JNICALL Java_smile_Network_setContEvidence__ID(JNIEnv *env, jobject obj, jint nodeHandle, jdouble evidence) 
{
    JSMILE_ENTER;
	DSL_node *node = ValidateNodeHandle(env, obj, nodeHandle);
	int res = node->Value()->SetEvidence(evidence);
	if (DSL_OKAY != res) 
    {
        ThrowSmileException(env, "SetEvidence", res);
    }
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_setContEvidence__Ljava_lang_String_2D(JNIEnv *env, jobject obj, jstring nodeId, jdouble evidence)
{
	JSMILE_ENTER;
	Java_smile_Network_setContEvidence__ID(env, obj, ValidateNodeId(env, obj, nodeId), evidence);
	JSMILE_LEAVE;
}

JNIEXPORT jdouble JNICALL Java_smile_Network_getContEvidence__I(JNIEnv *env, jobject obj, jint nodeHandle)
{
	JSMILE_ENTER;
	DSL_node *node = ValidateNodeHandle(env, obj, nodeHandle);
	EnsureEvidenceIsSet(node);
	double evidence;
	int res = node->Value()->GetEvidence(evidence);
	JSMILE_RETURN(evidence);
}

JNIEXPORT jdouble JNICALL Java_smile_Network_getContEvidence__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId)
{
	JSMILE_ENTER;
	JSMILE_RETURN(Java_smile_Network_getContEvidence__I(env, obj, ValidateNodeId(env, obj, nodeId)));
}

//-------------------------------------------------------------------------------

JNIEXPORT void JNICALL Java_smile_Network_setDeMorganPriorBelief__ID(JNIEnv *env, jobject obj, jint nodeHandle, jdouble belief)
{
	JSMILE_ENTER;
	GetDeMorganDef(env, obj, nodeHandle)->SetPriorBelief(belief);
	JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_setDeMorganPriorBelief__Ljava_lang_String_2D(JNIEnv *env, jobject obj, jstring nodeId, jdouble belief)
{
	JSMILE_ENTER;
	Java_smile_Network_setDeMorganPriorBelief__ID(env, obj, ValidateNodeId(env, obj, nodeId), belief);
	JSMILE_LEAVE;
}

JNIEXPORT jdouble JNICALL Java_smile_Network_getDeMorganPriorBelief__I(JNIEnv *env, jobject obj, jint nodeHandle)
{
	JSMILE_ENTER;
	JSMILE_RETURN(GetDeMorganDef(env, obj, nodeHandle)->GetPriorBelief());
}

JNIEXPORT jdouble JNICALL Java_smile_Network_getDeMorganPriorBelief__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId) 
{
	JSMILE_ENTER;
	JSMILE_RETURN(Java_smile_Network_getDeMorganPriorBelief__I(env, obj, ValidateNodeId(env, obj, nodeId)));
}

JNIEXPORT void JNICALL Java_smile_Network_setDeMorganParentType__III(JNIEnv *env, jobject obj, jint nodeHandle, jint parentIndex, jint parentType)
{
	JSMILE_ENTER;
	int res = GetDeMorganDef(env, obj, nodeHandle)->SetParentType(parentIndex, parentType);
	if (DSL_OKAY != res) 
	{
		ThrowSmileException(env, "SetParentType", res);
	}
	JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_setDeMorganParentType__Ljava_lang_String_2II(JNIEnv *env, jobject obj, jstring nodeId, jint parentIndex, jint parentType)
{
	JSMILE_ENTER;
	Java_smile_Network_setDeMorganParentType__III(env, obj, ValidateNodeId(env, obj, nodeId), parentIndex, parentType);
	JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_setDeMorganParentType__ILjava_lang_String_2I(JNIEnv *env, jobject obj, jint nodeHandle, jstring parentId, jint parentType) 
{
	JSMILE_ENTER;
	Java_smile_Network_setDeMorganParentType__III(env, obj, nodeHandle, ValidateParentId(env, obj, nodeHandle, parentId), parentType);
	JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_Network_setDeMorganParentType__Ljava_lang_String_2Ljava_lang_String_2I
  (JNIEnv *env, jobject obj, jstring nodeId, jstring parentId, jint parentType)
{
	JSMILE_ENTER;
	Java_smile_Network_setDeMorganParentType__ILjava_lang_String_2I(env, obj, ValidateNodeId(env, obj, nodeId), parentId, parentType);
	JSMILE_LEAVE;
}

JNIEXPORT jint JNICALL Java_smile_Network_getDeMorganParentType__II
  (JNIEnv *env, jobject obj, jint nodeHandle, jint parentIndex)
{
	JSMILE_ENTER;
	int res = GetDeMorganDef(env, obj, nodeHandle)->GetParentType(parentIndex);
	if (res < 0)
	{
		ThrowSmileException(env, "GetParentType", res);
	}
	JSMILE_RETURN(res);
}

JNIEXPORT jint JNICALL Java_smile_Network_getDeMorganParentType__Ljava_lang_String_2I
  (JNIEnv *env, jobject obj, jstring nodeId, jint parentIndex)
{
	JSMILE_ENTER;
	JSMILE_RETURN(Java_smile_Network_getDeMorganParentType__II(env, obj, ValidateNodeId(env, obj, nodeId), parentIndex));
}


JNIEXPORT jint JNICALL Java_smile_Network_getDeMorganParentType__ILjava_lang_String_2
  (JNIEnv *env, jobject obj, jint nodeHandle, jstring parentId)
{
	JSMILE_ENTER;
	JSMILE_RETURN(Java_smile_Network_getDeMorganParentType__II(env, obj, nodeHandle, ValidateParentId(env, obj, nodeHandle, parentId)));
}

JNIEXPORT jint JNICALL Java_smile_Network_getDeMorganParentType__Ljava_lang_String_2Ljava_lang_String_2
  (JNIEnv *env, jobject obj, jstring nodeId, jstring parentId)
{
	JSMILE_ENTER;
	JSMILE_RETURN(Java_smile_Network_getDeMorganParentType__ILjava_lang_String_2(env, obj, ValidateNodeId(env, obj, nodeId), parentId));
}


JNIEXPORT void JNICALL Java_smile_Network_setDeMorganParentWeight__IID(JNIEnv *env, jobject obj, jint nodeHandle, jint parentIndex, jdouble weight)
{
	JSMILE_ENTER;
	int res = GetDeMorganDef(env, obj, nodeHandle)->SetParentWeight(parentIndex, weight);
	if (res < 0)
	{
		ThrowSmileException(env, "SetParentWeight", res);
	}
	JSMILE_LEAVE;
}


JNIEXPORT void JNICALL Java_smile_Network_setDeMorganParentWeight__Ljava_lang_String_2ID(JNIEnv *env, jobject obj, jstring nodeId, jint parentIndex, jdouble weight)
{
	JSMILE_ENTER;
	Java_smile_Network_setDeMorganParentWeight__IID(env, obj, ValidateNodeId(env, obj, nodeId), parentIndex, weight);
	JSMILE_LEAVE;
}


JNIEXPORT void JNICALL Java_smile_Network_setDeMorganParentWeight__ILjava_lang_String_2D(JNIEnv *env, jobject obj, jint nodeHandle, jstring parentId, jdouble weight)
{
	JSMILE_ENTER;
	Java_smile_Network_setDeMorganParentWeight__IID(env, obj, nodeHandle, ValidateParentId(env, obj, nodeHandle, parentId), weight);
	JSMILE_LEAVE;
}


JNIEXPORT void JNICALL Java_smile_Network_setDeMorganParentWeight__Ljava_lang_String_2Ljava_lang_String_2D(JNIEnv *env, jobject obj, jstring nodeId, jstring parentId, jdouble weight)
{
	JSMILE_ENTER;
	Java_smile_Network_setDeMorganParentWeight__ILjava_lang_String_2D(env, obj, ValidateNodeId(env, obj, nodeId), parentId, weight);
	JSMILE_LEAVE;
}

JNIEXPORT jdouble JNICALL Java_smile_Network_getDeMorganParentWeight__II(JNIEnv *env, jobject obj, jint nodeHandle, jint parentIndex)
{
	JSMILE_ENTER;
	double weight = GetDeMorganDef(env, obj, nodeHandle)->GetParentWeight(parentIndex);
	if (weight < 0)
	{
		ThrowSmileException(env, "GetParentWeight", -1);
	}
	JSMILE_RETURN(weight);
}

JNIEXPORT jdouble JNICALL Java_smile_Network_getDeMorganParentWeight__Ljava_lang_String_2I(JNIEnv *env, jobject obj, jstring nodeId, jint parentIndex)
{
	JSMILE_ENTER;
	JSMILE_RETURN(Java_smile_Network_getDeMorganParentWeight__II(env, obj, ValidateNodeId(env, obj, nodeId), parentIndex));
}

JNIEXPORT jdouble JNICALL Java_smile_Network_getDeMorganParentWeight__ILjava_lang_String_2(JNIEnv *env, jobject obj, jint nodeHandle, jstring parentId)
{
	JSMILE_ENTER;
	JSMILE_RETURN(Java_smile_Network_getDeMorganParentWeight__II(env, obj, nodeHandle, ValidateParentId(env, obj, nodeHandle, parentId)));
}

JNIEXPORT jdouble JNICALL Java_smile_Network_getDeMorganParentWeight__Ljava_lang_String_2Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId, jstring parentId)
{
	JSMILE_ENTER;
	JSMILE_RETURN(Java_smile_Network_getDeMorganParentWeight__ILjava_lang_String_2(env, obj, ValidateNodeId(env, obj, nodeId), parentId));
}

//-------------------------------------------------------------------------------

JNIEXPORT jobjectArray JNICALL Java_smile_Network_getMauExpressions__I(JNIEnv *env, jobject obj, jint nodeHandle)
{
	JSMILE_ENTER;
	vector<string> out;
	GetMauDef(env, obj, nodeHandle)->GetExpressions(out);
    jclass clazz = env->FindClass("java/lang/String");
	int count = int(out.size());
	jobjectArray arr = env->NewObjectArray(count, clazz, NULL);
	for (int i = 0; i < count; i ++) 
    {
		env->SetObjectArrayElement(arr, i, env->NewStringUTF(out[i].c_str()));
	}
	JSMILE_RETURN(arr);
}

JNIEXPORT jobjectArray JNICALL Java_smile_Network_getMauExpressions__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId)
{
	JSMILE_ENTER;
	JSMILE_RETURN(Java_smile_Network_getMauExpressions__I(env, obj, ValidateNodeId(env, obj, nodeId)));
}


JNIEXPORT void JNICALL Java_smile_Network_setMauExpressions__I_3Ljava_lang_String_2(JNIEnv *env, jobject obj, jint nodeHandle, jobjectArray expressions)
{
	JSMILE_ENTER;
	DSL_mau *mau = GetMauDef(env, obj, nodeHandle);
	vector<string> ex;
	if (NULL != expressions)
	{
		int count = env->GetArrayLength(expressions);
		ex.resize(count);
		for (int i = 0; i < count; i ++)
		{
			NativeString nats(env, jstring(env->GetObjectArrayElement(expressions, i)));
			ex[i] = nats;
		}
	}

	string errMsg;
	if (DSL_OKAY != mau->SetExpressions(ex, &errMsg))
	{
		ThrowSmileException(env, runtime_error(errMsg));
	}

	JSMILE_LEAVE;
}


JNIEXPORT void JNICALL Java_smile_Network_setMauExpressions__Ljava_lang_String_2_3Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring nodeId, jobjectArray expressions)
{
	JSMILE_ENTER;
	Java_smile_Network_setMauExpressions__I_3Ljava_lang_String_2(env, obj, ValidateNodeId(env, obj, nodeId), expressions);
	JSMILE_LEAVE;
}

JNIEXPORT jobject Java_smile_Network_cloneme(JNIEnv *env, jobject obj)
{
	JSMILE_ENTER;
	DSL_network *net = GetPtr(env, obj);
	jobject theclone = CreateNewNetwork(env);
	DSL_network *newclone = GetPtr(env, theclone);
	*newclone = *net;
	JSMILE_RETURN(theclone);
}

JNIEXPORT void JNICALL Java_smile_Network_distributedHelperEM(JNIEnv *env, jobject obj, jdoubleArray counts)
{
	JSMILE_ENTER;
    
	if (NULL == counts) 
    {
		throw invalid_argument("missing counts array");
	}

	DSL_network *net = GetPtr(env, obj);
	vector<vector<DSL_Dmatrix *> > nativeCounts(1 + net->GetLastNode());
	
	for (int h = net->GetFirstNode(); h >= 0; h = net->GetNextNode(h))
	{
		nativeCounts[h].resize(1);
		nativeCounts[h][0] = new DSL_Dmatrix(*(net->GetNode(h)->Definition()->GetMatrix()));
		nativeCounts[h][0]->FillWith(0);
	}
	
	
	DSL_distributed_EM_helper(*net, false, nativeCounts);
	
	int pos = 0;
	for (int h = net->GetFirstNode(); h >= 0; h = net->GetNextNode(h))
	{
		const DSL_Dmatrix *mtx = nativeCounts[h][0];
		int len = mtx->GetSize();
		env->SetDoubleArrayRegion(counts, pos, len, mtx->GetItems().Items()); 
		pos += len;
		delete nativeCounts[h][0];
	}

    JSMILE_LEAVE;

}
//-------------------------------------------------------------------------------


} // end extern "C"
