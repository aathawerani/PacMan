// smileNet.cpp

#include "stdafx.h"

using namespace std;

using namespace System;
using namespace System::Collections;
using namespace System::Runtime::InteropServices;
using namespace System::Reflection;
using namespace System::Drawing;
using namespace System::IO;

[assembly:AssemblyVersion("1.1.*")];

namespace Smile
{

// this is non-gc class
class StringToCharPtr
{
public:
	StringToCharPtr(String *str) { pStr = static_cast<char *>(Marshal::StringToHGlobalAnsi(str).ToPointer()); }
	operator char*() const { return pStr; }
	~StringToCharPtr() { Marshal::FreeHGlobal(pStr); }
private:
	char *pStr; 
};


Int32 CopyIntArray(const DSL_intArray& native)__gc[]
{
	int imax = native.NumItems();
	Int32 ar[] = new Int32[imax];
	for (int i = 0; i < imax; i ++)
	{
		ar[i] = native[i];
	}
	return ar;
}

Double CopyDoubleArray(DSL_doubleArray &native)__gc[]
{
	int imax = native.GetSize();
	Double ar[] = new Double[imax];
	for (int i = 0; i < imax; i ++)
	{
		ar[i] = native[i];
	}
	return ar;
}

	
//---------------------------------------------------
// SmileException
public __gc class SmileException : public Exception
{
public:
	SmileException(String *message) : Exception(message) {}

	static void CheckSmileStatus(String *msg, int smileErrorCode)
	{
		if (DSL_OKAY != smileErrorCode)
		{
			throw new SmileException(msg, smileErrorCode);
		}
	}

// 'public private' is the equivalent of C#'s 'internal' modifier
public private:	
	SmileException(String *msg, int smileErrorCode) : Exception(MsgFromSmileErr(msg, smileErrorCode)) {}

private:
	static String* MsgFromSmileErr(String *msg, int smileErrorCode)
	{
		String *completeMsg = String::Format("{0}, SMILE error code {1}", msg, smileErrorCode.ToString());
		return completeMsg;
	}
};

//---------------------------------------------------

public __value struct DocItemInfo
{
	String *title;
	String *path;
};

public __value struct UserProperty
{
	String *name;
	String *value;
};


UserProperty ConvertUserProps(DSL_userProperties &up)__gc[]
{
	int imax = up.GetNumberOfProperties();
	UserProperty ar[] = new UserProperty[imax];
	for (int i = 0; i < imax; i ++)
	{
		ar[i].name = up.GetPropertyName(i);			
		ar[i].value = up.GetPropertyValue(i);
	}

	return ar;
}


void ConvertUserProps(UserProperty ar[], DSL_userProperties &up)
{
	// potential problem with compiler/linker.
	// when p is local stack allocated object
	// linker spews a number of LNK2005s
	// (including one about missing _main!)
	// this happens only in 'Public' config

	DSL_userProperties *p = new DSL_userProperties;

	for (int i = 0; i < ar->Length; i ++)
	{
		StringToCharPtr szName(ar[i].name);
		StringToCharPtr szValue(ar[i].value);

		if (p->FindProperty(szName) >= 0)
		{
			delete p;
			String *msg = String::Format("Duplicate property name: {0}", ar[i].name); 
			throw new SmileException(msg);
		}
		
		int res = p->AddProperty(szName, szValue);
		if (DSL_OKAY != res)
		{
			delete p;
			String *msg = String::Format("Can't add property: {0}={1}", ar[i].name, ar[i].value); 
			throw new SmileException(msg, res);		
			break;
		}
	}

	up = *p;
	delete p;
}







DocItemInfo ConvertDocumentation(DSL_documentation &docs)__gc[]
{
	int imax = docs.GetNumberOfDocuments();
	DocItemInfo ar[] = new DocItemInfo[imax];
	for (int i = 0; i < imax; i ++)
	{
		ar[i].title = docs.GetDocumentTitle(i);			
		ar[i].path = docs.GetDocumentPath(i);
	}

	return ar;
}

void ConvertDocumentation(DocItemInfo ar[], DSL_documentation &docs)
{
	docs.DeleteAllDocuments();

	for (int i = 0; i < ar->Length; i ++)
	{
		StringToCharPtr szTitle(ar[i].title);
		StringToCharPtr szPath(ar[i].path);
		const char *title = szTitle;
		const char *path = szPath;
		docs.AddDocument(const_cast<char *>(title), const_cast<char *>(path));
	}
}

	

//---------------------------------------------------
// NodeEnumerator
private __gc class NodeEnumerator : public IEnumerator
{
public:	
	NodeEnumerator(DSL_network *net)
	{
		this->net = net;
		current = -1;
	}

private:
	Object * IEnumerator::get_Current() { return __box(current); }
	void IEnumerator::Reset() { current = -1; }
	
	bool IEnumerator::MoveNext()
	{
		if (current == -1)
		{
			current = net->GetFirstNode();
		}
		else
		{
			current = net->GetNextNode(current);
		}

		return current >= 0;
	}
	
	DSL_network *net;
	int current;
};

//---------------------------------------------------

public __gc class AnnealedMapResults
{
public:
	double probM1E;
    double probE;
	int mapStates __gc[];
};

public __gc class AnnealedMapTuning
{
public:
	double speed;		// Annealing speed
	double Tmin;		// Mininum temperature
	double Tinit;		// Initial temperature
	double kReheat;	    // RFC coefficient
	int kMAP;			// Number of best solutions we want
	double kRFC;		// coefficient for RFC
	int numCycle;		// Number of iterations per cycle;
	int iReheatSteps;	// Number of no-improvement iterations before reheating
	int iStopSteps;	    // Number of no-improvement iterations before stopping
	int randSeed;       // pass non-zero to ensure repeatability for given input
};


//---------------------------------------------------

private __gc class WrappedObject
{
protected:
	WrappedObject()
	{
		// this hack is needed only for .Net framework 1.1
		static bool crtInit = false;
		if (!crtInit)
		{
			__crt_dll_initialize();
			crtInit = true;
		}
	}
};

[DefaultMember("Node")]
public __gc class Network : public WrappedObject, public IEnumerable, public IDisposable, public ICloneable
{
public:
	Network()
	{
		net = new DSL_network;
	}

	~Network()
	{
		delete net;
	}

	System::Object __gc * Clone()
	{
		Network *clone = new Network;
		if (NULL != net)
		{
			*clone->net = *net;
		}

		return clone;
	}

	void Dispose()
	{
		delete net;
		net = NULL;
		GC::SuppressFinalize(this);
	}

	IEnumerator * GetEnumerator()
	{
		return new NodeEnumerator(net);
	}

	__value enum BayesianAlgorithmType
	{
		Lauritzen = DSL_ALG_BN_LAURITZEN,
		Henrion = DSL_ALG_BN_HENRION,
		LSampling = DSL_ALG_BN_LSAMPLING,
		SelfImportance = DSL_ALG_BN_SELFIMPORTANCE,
		HeuristicImportance = DSL_ALG_BN_HEURISTICIMPORTANCE,
		BackSampling = DSL_ALG_BN_BACKSAMPLING,
		AisSampling = DSL_ALG_BN_AISSAMPLING,
		EpisSampling = DSL_ALG_BN_EPISSAMPLING
	};

	__value enum InfluenceDiagramAlgorithmType
	{
		PolicyEvaluation = DSL_ALG_ID_COOPERSOLVING,
		FindBestPolicy = DSL_ALG_ID_SHACHTER,			
	};

	__value enum NodeType
	{
		Cpt = DSL_CPT,
		NoisyMax = DSL_NOISY_MAX,
		NoisyAdder = DSL_NOISY_ADDER,
		TruthTable = DSL_TRUTHTABLE,
		Table = DSL_TABLE,
		List = DSL_LIST,
		Mau = DSL_MAU,
        DeMorgan = DSL_DEMORGAN
	};

	__value enum NodeDiagType
	{
		Fault = DSL_extraDefinition::target,
		Observation = DSL_extraDefinition::observation,
		Auxiliary = DSL_extraDefinition::auxiliary
	};

	__value enum NoisyAdderFunction
	{
		Average = DSL_noisyAdder::Function::fun_average,
		SingleFault = DSL_noisyAdder::Function::fun_single_fault
	};

    __value enum DeMorganParentType
    {
        Inhibitor = DSL_DEMORGAN_INHIBITOR,
        Requirement = DSL_DEMORGAN_REQUIREMENT,
        Cause = DSL_DEMORGAN_CAUSE,
        Barrier = DSL_DEMORGAN_BARRIER,
    };

	void ReadFile(String *filename)
	{
		StringToCharPtr szFile(filename);
		int res = net->ReadFile(szFile);
		SmileException::CheckSmileStatus("ReadFile failed", res);
	}

	void WriteFile(String *filename)
	{
		StringToCharPtr szFile(filename);
		int res = net->WriteFile(szFile);
		SmileException::CheckSmileStatus("WriteFile failed", res);
	}

	__property BayesianAlgorithmType get_BayesianAlgorithm()
	{
		return static_cast<BayesianAlgorithmType>(net->GetDefaultBNAlgorithm());
	}

	__property void set_BayesianAlgorithm(BayesianAlgorithmType value)
	{
		net->SetDefaultBNAlgorithm(value);
	}

	__property InfluenceDiagramAlgorithmType get_InfluenceDiagramAlgorithm()
	{
		return static_cast<InfluenceDiagramAlgorithmType>(net->GetDefaultIDAlgorithm());
	}

	__property void set_InfluenceDiagramAlgorithm(InfluenceDiagramAlgorithmType value)
	{
		net->SetDefaultIDAlgorithm(value);
	}

	__property String* get_Id()
	{
		return new String(net->Header().GetId());
	}

	__property void set_Id(String *value)
	{
		ValidateId(value);
		StringToCharPtr szId(value);
		net->Header().SetId(szId);
	}

	__property String* get_Name()
	{
		return new String(net->Header().GetName());
	}

	__property void set_Name(String *value)
	{
		StringToCharPtr szName(value);
		net->Header().SetName(szName);
	}

	__property String* get_Description()
	{
		return new String(net->Header().GetComment());
	}

	__property void set_Description(String *value)
	{
		StringToCharPtr szComment(value);
		net->Header().SetComment(szComment);
	}

	__property int get_NodeCount()
	{
		return net->GetNumberOfNodes();
	}

    __property int get_SampleCount()
    {
        return net->GetNumberOfSamples();
    }

    __property void set_SampleCount(int value)
    {
        int res = net->SetNumberOfSamples(value);
        SmileException::CheckSmileStatus("Invalid sample count", res);
    }

    __property int get_SliceCount()
    {
        return net->GetNumberOfSlices();
    }

    __property void set_SliceCount(int value)
    {
        int res = net->SetNumberOfSlices(value);
        SmileException::CheckSmileStatus("Invalid slice count", res);
    }


	__property String* get_Node(int handle)
	{
		return GetNodeId(handle);
	}

    void UpdateBeliefs()
	{
		int res = net->UpdateBeliefs();
        SmileException::CheckSmileStatus("UpdateBeliefs failed", res);
	}

    double ProbEvidence()
    {
        double pe = 0;
        if (!net->CalcProbEvidence(pe))
        {
			throw new SmileException("CalcProbEvidence failed");
        }

        return pe;
    }

    AnnealedMapResults* AnnealedMap(Int32 mapNodes[], AnnealedMapTuning *tuning)
    {
        int seed = 0;

        if (NULL != tuning)
        {
            DSL_AnnealedMAPParams p;
            p.speed = tuning->speed;
	        p.Tmin = tuning->Tmin;
	        p.Tinit = tuning->Tinit;
	        p.kReheat = tuning->kReheat;
	        p.kMAP = tuning->kMAP;
	        p.kRFC = tuning->kRFC;
	        p.numCycle = tuning->numCycle;
	        p.iReheatSteps = tuning->iReheatSteps;
	        p.iStopSteps = tuning->iStopSteps;
            net->SetAnnealedMAPParams(p);

            seed = tuning->randSeed;
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

        int mapNodeCount = mapNodes->Length;
        vector<int> nativeMapNodes(mapNodeCount);
        for (int i = 0; i < mapNodeCount; i ++) nativeMapNodes[i] = mapNodes[i];

        vector<int> nativeMapStates;
        double probM1E, probE;
        int res = net->AnnealedMAP(ev, nativeMapNodes, nativeMapStates, probM1E, probE, seed);
        SmileException::CheckSmileStatus("AnnealedMap failed", res);
                
        AnnealedMapResults *am = new AnnealedMapResults;
        am->probM1E = probM1E;
        am->probE = probE;
        am->mapStates = new Int32[mapNodeCount];
        for (int i = 0; i < mapNodeCount; i ++) am->mapStates[i] = nativeMapStates[i];

        return am;
    }

    AnnealedMapResults* AnnealedMap(String* mapNodes[], AnnealedMapTuning *tuning)
    {
        int count = mapNodes->Length;
        Int32 handles[] = new Int32[count];
        for (int i = 0; i < count; i ++)
        {
            handles[i] = ValidateNodeId(mapNodes[i]);
        }
        return AnnealedMap(handles, tuning);
    }

	UserProperty GetUserProperties()__gc[]
	{
		return ConvertUserProps(net->UserProperties());
	}

	void SetUserProperties(UserProperty props[])
	{
		ConvertUserProps(props, net->UserProperties());
	}

	int GetFirstNode()
	{
		return net->GetFirstNode();
	}

	int GetNextNode(int nodeHandle)
	{
		return net->GetNextNode(nodeHandle);
	}
	
	int GetNode(String *nodeId)
	{
		return ValidateNodeId(nodeId);
	}

	Int32 GetAllNodes()__gc[]
	{
		DSL_intArray allNodes;
		net->GetAllNodes(allNodes);
		return CopyIntArray(allNodes);
	}

	String* GetAllNodeIds()__gc[]
	{
		DSL_intArray allNodes;
		net->GetAllNodes(allNodes);
		return HandlesToIds(allNodes);
	}

	NodeType GetNodeType(int nodeHandle)
	{
		DSL_node *node = ValidateNodeHandle(nodeHandle);
		return static_cast<NodeType>(node->Definition()->GetType());
	}

	NodeType GetNodeType(String *nodeId)
	{
		return GetNodeType(ValidateNodeId(nodeId));
	}

    void SetNodeType(int nodeHandle, NodeType type)
    {
        DSL_node *node = ValidateNodeHandle(nodeHandle);
        int res = node->ChangeType(type);
        SmileException::CheckSmileStatus("Can't change node type", res);
    }

    void SetNodeType(String *nodeId, NodeType type)
    {
        SetNodeType(ValidateNodeId(nodeId), type);
    }
	
	int AddNode(NodeType nodeType)
	{
		return AddNodeHelper(nodeType, NULL);
	}

	int AddNode(NodeType nodeType, String *nodeId)
	{
		ValidateId(nodeId);
		StringToCharPtr szId(nodeId);
		return AddNodeHelper(nodeType, szId);
	}

	void AddArc(int parentHandle, int childHandle)
	{
		AddArcHelper(parentHandle, childHandle, dsl_normalArc);
	}

	void AddArc(String *parentId, String *childId)
	{
		AddArc(ValidateNodeId(parentId), ValidateNodeId(childId));
	}

	void AddOutcome(String *nodeId, String *outcomeId)
	{
		AddOutcome(ValidateNodeId(nodeId), outcomeId);
	}

	void AddOutcome(int nodeHandle, String *outcomeId)
	{
		ValidateId(outcomeId);
		ValidateNodeHandle(nodeHandle);
		StringToCharPtr szId(outcomeId);
		int res = net->GetNode(nodeHandle)->Definition()->AddOutcome(szId);
		if (DSL_OKAY != res)
		{
			String *msg = String::Format(
				"Can't add outcome {0} to node {1}", 
				outcomeId,
				GetNodeId(nodeHandle));
			throw new SmileException(msg, res);
		}
	}

	void InsertOutcome(String *nodeId, int position, String *outcomeId)
	{
		InsertOutcome(ValidateNodeId(nodeId), position, outcomeId);
	}

	void InsertOutcome(int nodeHandle, int position, String *outcomeId)
	{
		ValidateId(outcomeId);
		ValidateNodeHandle(nodeHandle);
		StringToCharPtr szId(outcomeId);
		int res = net->GetNode(nodeHandle)->Definition()->InsertOutcome(position, szId);
		if (DSL_OKAY != res)
		{
			String *msg = String::Format(
				"Can't insert outcome {0} in node {1} at position {2}", 
				outcomeId,
				position.ToString(),
				GetNodeId(nodeHandle));
			throw new SmileException(msg, res);
		}
	}
			
	void DeleteOutcome(String *nodeId, int outcomeIndex)
	{
		DeleteOutcome(ValidateNodeId(nodeId), outcomeIndex);
	}

	void DeleteOutcome(int nodeHandle, int outcomeIndex)
	{
		ValidateNodeHandle(nodeHandle);
		int res = net->GetNode(nodeHandle)->Definition()->RemoveOutcome(outcomeIndex);
		if (DSL_OKAY != res)
		{
			String *msg = String::Format(
				"Can't delete outcome {0} from node {1}", 
				outcomeIndex.ToString(),
				GetNodeId(nodeHandle));
			throw new SmileException(msg, res);
		}
	}

	void DeleteOutcome(int nodeHandle, String *outcomeId)
	{
		DeleteOutcome(nodeHandle, ValidateOutcomeId(nodeHandle, outcomeId));
	}

	void DeleteOutcome(String *nodeId, String *outcomeId)
	{
		DeleteOutcome(ValidateNodeId(nodeId), outcomeId);
	}


	void DeleteNode(String *nodeId)
	{
		DeleteNode(ValidateNodeId(nodeId));
	}

	void DeleteNode(int nodeHandle)
	{
		ValidateNodeHandle(nodeHandle);
		net->DeleteNode(nodeHandle);
	}

	void DeleteArc(String *parentId, String *childId)
	{
		DeleteArc(ValidateNodeId(parentId), ValidateNodeId(childId));
	}

	void DeleteArc(int parentHandle, int childHandle)
	{
		DeleteArcHelper(parentHandle, childHandle, dsl_normalArc);
	}

	Int32 GetParents(String *nodeId)__gc[]	
	{
		return GetParents(ValidateNodeId(nodeId));
	}

	Int32 GetParents(int nodeHandle)__gc[]
	{
		ValidateNodeHandle(nodeHandle);
		return CopyIntArray(net->GetParents(nodeHandle));
	}

	String* GetParentIds(String *nodeId)__gc[]
	{
		return GetParentIds(ValidateNodeId(nodeId));
	}

	String* GetParentIds(int nodeHandle)__gc[]
	{
		ValidateNodeHandle(nodeHandle);
		return HandlesToIds(net->GetParents(nodeHandle));
	}

	Int32 GetChildren(String *nodeId)__gc[]	
	{
		return GetChildren(ValidateNodeId(nodeId));
	}

	Int32 GetChildren(int nodeHandle)__gc[]
	{
		ValidateNodeHandle(nodeHandle);
		return CopyIntArray(net->GetChildren(nodeHandle));
	}

	String* GetChildIds(String *nodeId)__gc[]
	{
		return GetChildIds(ValidateNodeId(nodeId));
	}

	String* GetChildIds(int nodeHandle)__gc[]
	{
		ValidateNodeHandle(nodeHandle);
		return HandlesToIds(net->GetChildren(nodeHandle));
	}

	String* GetNodeId(int nodeHandle)
	{
		DSL_node *node = ValidateNodeHandle(nodeHandle);
		return new String(node->Info().Header().GetId());
	}

	String* GetNodeName(String *nodeId)
	{
		return GetNodeName(ValidateNodeId(nodeId));
	}

	void SetNodeId(String *oldId, String *newId)
	{
		SetNodeId(ValidateNodeId(oldId), newId);
	}

	void SetNodeId(int nodeHandle, String *id)
	{
		DSL_node *node = ValidateNodeHandle(nodeHandle);
		StringToCharPtr szId(id);
		int res = node->Info().Header().SetId(szId);
		if (DSL_OKAY != res)
		{
			String *msg = String::Format(
				"Can't set new id for node {0}",
				GetNodeId(nodeHandle));
			throw new SmileException(msg, res);
		}
	}


	String* GetNodeName(int nodeHandle)
	{
		DSL_node *node = ValidateNodeHandle(nodeHandle);
		return new String(node->Info().Header().GetName());
	}

	void SetNodeName(String *nodeId, String *name)
	{
		SetNodeName(ValidateNodeId(nodeId), name);
	}

	void SetNodeName(int nodeHandle, String *name)
	{
		DSL_node *node = ValidateNodeHandle(nodeHandle);
		StringToCharPtr szName(name);
		int res = node->Info().Header().SetName(szName);
		if (DSL_OKAY != res)
		{
			String *msg = String::Format(
				"Can't set new name for node {0}",
				GetNodeId(nodeHandle));
			throw new SmileException(msg, res);
		}
	}



	UserProperty GetNodeUserProperties(int handle)__gc[]
	{
		return ConvertUserProps(
			ValidateNodeHandle(handle)->Info().UserProperties());
	}

	UserProperty GetNodeUserProperties(String *nodeId)__gc[]
	{
		return GetNodeUserProperties(ValidateNodeId(nodeId));
	}

	void SetNodeUserProperties(int handle, UserProperty props[])
	{
		ConvertUserProps(props, 
			ValidateNodeHandle(handle)->Info().UserProperties());
	}

	void SetNodeUserProperties(String *nodeId, UserProperty props[])
	{
		SetNodeUserProperties(ValidateNodeId(nodeId), props);
	}


	String* GetNodeDescription(int nodeHandle)
	{
		DSL_node *node = ValidateNodeHandle(nodeHandle);
		return new String(node->Info().Header().GetComment());
	}

	String* GetNodeDescription(String *nodeId)
	{
		return GetNodeDescription(ValidateNodeId(nodeId));
	}

	void SetNodeDescription(int nodeHandle, String *description)
	{
		DSL_node *node = ValidateNodeHandle(nodeHandle);
		StringToCharPtr szDesc(description);
		node->Info().Header().SetComment(szDesc);
	}

	void SetNodeDescription(String *nodeId, String *description)
	{
		SetNodeDescription(ValidateNodeId(nodeId), description);
	}


	DocItemInfo GetNodeDocumentation(int nodeHandle)__gc[]
	{
		DSL_node *node = ValidateNodeHandle(nodeHandle);
		return ConvertDocumentation(node->Info().Documentation());
	}
	
	DocItemInfo GetNodeDocumentation(String *nodeId)__gc[]
	{
		return GetNodeDocumentation(ValidateNodeId(nodeId));
	}
	

	void SetNodeDocumentation(int nodeHandle, DocItemInfo documentation[])
	{
		DSL_node *node = ValidateNodeHandle(nodeHandle);
		ConvertDocumentation(documentation, node->Info().Documentation());
	}
	
	void SetNodeDocumentation(String *nodeId, DocItemInfo documentation[])
	{
		SetNodeDocumentation(ValidateNodeId(nodeId), documentation);
	}


	Double GetNodeDefinition(String *nodeId)__gc []
	{
		return GetNodeDefinition(ValidateNodeId(nodeId));
	}

	Double GetNodeDefinition(int nodeHandle)__gc []
	{
		DSL_node *node = ValidateNodeHandle(nodeHandle);
		DSL_doubleArray *pArray = GetDefinitionArray(node);
		if (NULL == pArray)
		{
			DSL_nodeDefinition *nodeDef = node->Definition();
			String *msg = String::Format(
				"Can't get node definition {0} in node {1} as array of doubles", 
				new String(nodeDef->GetTypeName()), 
				GetNodeId(nodeHandle));
			throw new SmileException(msg);
		}

		return CopyDoubleArray(*pArray);
	}

	void SetNodeDefinition(String *nodeId, Double definition[])
	{
		SetNodeDefinition(ValidateNodeId(nodeId), definition);
	}

	void SetNodeDefinition(int nodeHandle, Double definition[])
	{
		DSL_node *node = ValidateNodeHandle(nodeHandle);
		DSL_nodeDefinition *nodeDef = node->Definition();
		DSL_doubleArray *pArray = GetDefinitionArray(node);
		if (NULL == pArray)
		{
			String *msg = String::Format(
				"Can't set node definition {0} in node {1} as array of doubles", 
				new String(nodeDef->GetTypeName()), 
				GetNodeId(nodeHandle));
			throw new SmileException(msg);
		}

		int nativeType = nodeDef->GetType();
		bool isNoisy = 
			(DSL_NOISY_MAX == nativeType) || 
			(DSL_NOISY_ADDER == nativeType);
		int imax = pArray->GetSize();
		if (imax != definition->Length)
		{
			String *msg = String::Format(
				"Invalid definition array size for node {0}. Expected {1} and got {2}", 
				GetNodeId(nodeHandle),
				Int32(imax).ToString(),
				definition->Length.ToString());

			if (isNoisy)
			{
				msg = String::Format(
					"{0}\nThis node is NoisyMax or NoisyAdder. "
					"You need to pass noisy weights as parameter, including constrained columns",
					msg);
			}

			throw new SmileException(msg);
		}

		DSL_nodeValue *pVal = node->Value();
		if (NULL != pVal) pVal->SetValueInvalid();

		for (int i = 0; i < imax; i ++)
		{
			(*pArray)[i] = definition[i];
		}

		// special care needed for noisyMAX nodes
		// they need to update their CPTs
		if (isNoisy)
		{
			static_cast<DSL_ciDefinition *>(nodeDef)->CiToCpt();
		}
	}

			
	Int32 GetNoisyParentStrengths(int nodeHandle, int parentIndex)__gc []
	{
		DSL_noisyMAX* noisyMaxDef = GetNoisyMaxDef(nodeHandle);
		return CopyIntArray(noisyMaxDef->GetParentOutcomeStrengths(parentIndex));
	}

	Int32 GetNoisyParentStrengths(String *nodeId, int parentIndex)__gc []
	{
		return GetNoisyParentStrengths(ValidateNodeId(nodeId), parentIndex);
	}

	Int32 GetNoisyParentStrengths(int nodeHandle, String *parentId)__gc []
	{
		return GetNoisyParentStrengths(nodeHandle, ValidateParentId(nodeHandle, parentId));
	}

	Int32 GetNoisyParentStrengths(String *nodeId, String *parentId)__gc []
	{
		return GetNoisyParentStrengths(ValidateNodeId(nodeId), parentId);
	}


	void SetNoisyParentStrengths(int nodeHandle, int parentIndex, Int32 strengths[])
	{
		DSL_noisyMAX* noisyMaxDef = GetNoisyMaxDef(nodeHandle);
		ValidateParentIndex(nodeHandle, parentIndex);
		
		int imax = noisyMaxDef->GetNumOfParentOutcomes(parentIndex);
		if (imax != strengths->Length)
		{
			String *msg = String::Format(
				"Invalid parent strength array size for node {0}. Expected {1} and got {2}", 
				GetNodeId(nodeHandle),
				Int32(imax).ToString(),
				strengths->Length.ToString());				

			throw new SmileException(msg);
		}

		DSL_intArray& nativeStrengths = noisyMaxDef->GetParentOutcomeStrengths(parentIndex);
		for (int i = 0; i < imax; i ++)
		{
			nativeStrengths[i] = strengths[i];
		}

		noisyMaxDef->CiToCpt();
	}

	void SetNoisyParentStrengths(String* nodeId, int parentIndex, Int32 strengths[])
	{
		SetNoisyParentStrengths(ValidateNodeId(nodeId), parentIndex, strengths);
	}

	void SetNoisyParentStrengths(int nodeHandle, String *parentId, Int32 strengths[])
	{
		SetNoisyParentStrengths(nodeHandle, ValidateParentId(nodeHandle, parentId), strengths);
	}

	void SetNoisyParentStrengths(String* nodeId, String *parentId, Int32 strengths[])
	{
		SetNoisyParentStrengths(ValidateNodeId(nodeId), parentId, strengths);
	}

	NoisyAdderFunction GetNoisyAdderFunction(int nodeHandle)
	{
		DSL_noisyAdder *noisyAdderDef = GetNoisyAdderDef(nodeHandle);
		return static_cast<NoisyAdderFunction>(noisyAdderDef->GetFunction());
	}

	NoisyAdderFunction GetNoisyAdderFunction(String *nodeId)
	{
		return GetNoisyAdderFunction(ValidateNodeId(nodeId));
	}

	void SetNoisyAdderFunction(int nodeHandle, NoisyAdderFunction function)
	{
		DSL_noisyAdder *noisyAdderDef = GetNoisyAdderDef(nodeHandle);
		int res = noisyAdderDef->SetFunction(static_cast<DSL_noisyAdder::Function>(function));
		SmileException::CheckSmileStatus("Can't set NoisyAdder function", res);
		if (!noisyAdderDef->KeepSynchronized())
		{
			noisyAdderDef->CiToCpt();
		}
	}

	void SetNoisyAdderFunction(String *nodeId, NoisyAdderFunction function)
	{
		SetNoisyAdderFunction(ValidateNodeId(nodeId), function);
	}

	int GetNoisyDistinguishedOutcome(int nodeHandle)
	{
		DSL_noisyAdder *noisyAdderDef = GetNoisyAdderDef(nodeHandle);
		return noisyAdderDef->GetDistinguishedState();
	}

	int GetNoisyDistinguishedOutcome(String *nodeId)
	{
		return GetNoisyDistinguishedOutcome(ValidateNodeId(nodeId));
	}

	String* GetNoisyDistinguishedOutcomeId(int nodeHandle)
	{
		return GetOutcomeId(nodeHandle, GetNoisyDistinguishedOutcome(nodeHandle));
	}

	String* GetNoisyDistinguishedOutcomeId(String *nodeId)
	{
		return GetNoisyDistinguishedOutcomeId(ValidateNodeId(nodeId));
	}

	void SetNoisyDistinguishedOutcome(int nodeHandle, int outcomeIndex)
	{
		DSL_noisyAdder *noisyAdderDef = GetNoisyAdderDef(nodeHandle);
		ValidateOutcomeIndex(nodeHandle, outcomeIndex);
		int res = noisyAdderDef->SetDistinguishedState(outcomeIndex);
		SmileException::CheckSmileStatus("Can't set distinguished outcome", res);

		if (!noisyAdderDef->KeepSynchronized())
		{
			noisyAdderDef->CiToCpt();
		}
	}

	void SetNoisyDistinguishedOutcome(int nodeHandle, String *outcomeId)
	{
		SetNoisyDistinguishedOutcome(nodeHandle, ValidateOutcomeId(nodeHandle, outcomeId));
	}

	void SetNoisyDistinguishedOutcome(String *nodeId, int outcomeIndex)
	{
		SetNoisyDistinguishedOutcome(ValidateNodeId(nodeId), outcomeIndex);
	}

	void SetNoisyDistinguishedOutcome(String *nodeId, String *outcomeId)
	{
		SetNoisyDistinguishedOutcome(ValidateNodeId(nodeId), outcomeId);
	}

	int GetNoisyParentDistinguishedOutcome(int nodeHandle, int parentIndex)
	{
		DSL_noisyAdder *noisyAdderDef = GetNoisyAdderDef(nodeHandle);
		ValidateParentIndex(nodeHandle, parentIndex);
		return noisyAdderDef->GetParentDistinguishedState(parentIndex);
	}

	int GetNoisyParentDistinguishedOutcome(int nodeHandle, String *parentId)
	{
		return GetNoisyParentDistinguishedOutcome(nodeHandle, ValidateParentId(nodeHandle, parentId));
	}

	int GetNoisyParentDistinguishedOutcome(String *nodeId, String *parentId)
	{
		return GetNoisyParentDistinguishedOutcome(ValidateNodeId(nodeId), parentId);
	}

	int GetNoisyParentDistinguishedOutcome(String *nodeId, int parentIndex)
	{
		return GetNoisyParentDistinguishedOutcome(ValidateNodeId(nodeId), parentIndex);
	}

	String* GetNoisyParentDistinguishedOutcomeId(int nodeHandle, int parentIndex)
	{
		DSL_noisyAdder *noisyAdderDef = GetNoisyAdderDef(nodeHandle);
		ValidateParentIndex(nodeHandle, parentIndex);
		return GetOutcomeId(nodeHandle, noisyAdderDef->GetParentDistinguishedState(parentIndex));
	}

	String* GetNoisyParentDistinguishedOutcomeId(int nodeHandle, String *parentId)
	{
		return GetNoisyParentDistinguishedOutcomeId(nodeHandle, ValidateParentId(nodeHandle, parentId));
	}

	String* GetNoisyParentDistinguishedOutcomeId(String *nodeId, String *parentId)
	{
		return GetNoisyParentDistinguishedOutcomeId(ValidateNodeId(nodeId), parentId);
	}

	String* GetNoisyParentDistinguishedOutcomeId(String *nodeId, int parentIndex)
	{
		return GetNoisyParentDistinguishedOutcomeId(ValidateNodeId(nodeId), parentIndex);
	}

	void SetNoisyParentDistinguishedOutcome(int nodeHandle, int parentIndex, int parentOutcomeIndex)
	{
		DSL_noisyAdder *noisyAdderDef = GetNoisyAdderDef(nodeHandle);
		int parentHandle = ValidateParentIndex(nodeHandle, parentIndex);
		ValidateOutcomeIndex(parentHandle, parentOutcomeIndex);
		int res = noisyAdderDef->SetParentDistinguishedState(parentIndex, parentOutcomeIndex);
		SmileException::CheckSmileStatus("Can't set parent distinguished outcome", res);
		if (!noisyAdderDef->KeepSynchronized())
		{
			noisyAdderDef->CiToCpt();
		}
	}

	void SetNoisyParentDistinguishedOutcome(String* nodeHandle, int parentIndex, int parentOutcomeIndex)
	{
		SetNoisyParentDistinguishedOutcome(ValidateNodeId(nodeHandle), parentIndex, parentOutcomeIndex);
	}

	void SetNoisyParentDistinguishedOutcome(int nodeHandle, String *parentId, int parentOutcomeIndex)
	{
		SetNoisyParentDistinguishedOutcome(nodeHandle, ValidateParentId(nodeHandle, parentId), parentOutcomeIndex);
	}

	void SetNoisyParentDistinguishedOutcome(int nodeHandle, int parentIndex, String *parentOutcomeId)
	{
		SetNoisyParentDistinguishedOutcome(
			nodeHandle, 
			parentIndex,
			ValidateOutcomeId(ValidateParentIndex(nodeHandle, parentIndex), parentOutcomeId));
	}

	void SetNoisyParentDistinguishedOutcome(String *nodeId, String *parentId, int parentOutcomeIndex)
	{
		SetNoisyParentDistinguishedOutcome(ValidateNodeId(nodeId), parentId, parentOutcomeIndex);
	}

	void SetNoisyParentDistinguishedOutcome(int nodeHandle, String *parentId, String *parentOutcomeId)
	{
		SetNoisyParentDistinguishedOutcome(nodeHandle, ValidateParentId(nodeHandle, parentId), parentOutcomeId);
	}

	void SetNoisyParentDistinguishedOutcome(String *nodeId, int parentIndex, String *parentOutcomeId)
	{
		SetNoisyParentDistinguishedOutcome(ValidateNodeId(nodeId), parentIndex, parentOutcomeId);
	}

	void SetNoisyParentDistinguishedOutcome(String *nodeId, String *parentId, String *parentOutcomeId)
	{
		SetNoisyParentDistinguishedOutcome(ValidateNodeId(nodeId), parentId, parentOutcomeId);
	}


	double GetNoisyParentWeight(int nodeHandle, int parentIndex)
	{
		DSL_noisyAdder *noisyAdderDef = GetNoisyAdderDef(nodeHandle);
		ValidateParentIndex(nodeHandle, parentIndex);
		return noisyAdderDef->GetParentWeight(parentIndex);
	}

	double GetNoisyParentWeight(String *nodeId, int parentIndex)
	{
		return GetNoisyParentWeight(ValidateNodeId(nodeId), parentIndex);
	}

	double GetNoisyParentWeight(int nodeHandle, String *parentId)
	{
		return GetNoisyParentWeight(nodeHandle, ValidateParentId(nodeHandle, parentId));
	}

	double GetNoisyParentWeight(String *nodeId, String *parentId)
	{
		return GetNoisyParentWeight(ValidateNodeId(nodeId), parentId);
	}

	void SetNoisyParentWeight(int nodeHandle, int parentIndex, double weight)
	{
		DSL_noisyAdder *noisyAdderDef = GetNoisyAdderDef(nodeHandle);
		ValidateParentIndex(nodeHandle, parentIndex);
		int res = noisyAdderDef->SetParentWeight(parentIndex, weight);
		SmileException::CheckSmileStatus("Can't set parent weight", res);
		if (!noisyAdderDef->KeepSynchronized())
		{
			noisyAdderDef->CiToCpt();
		}
	}

	void SetNoisyParentWeight(String *nodeId, int parentIndex, double weight)
	{
		SetNoisyParentWeight(ValidateNodeId(nodeId), parentIndex, weight);
	}

	void SetNoisyParentWeight(int nodeHandle, String *parentId, double weight)
	{
		SetNoisyParentWeight(nodeHandle, ValidateParentId(nodeHandle, parentId), weight);
	}

	void SetNoisyParentWeight(String *nodeId, String *parentId, double weight)
	{
		SetNoisyParentWeight(ValidateNodeId(nodeId), parentId, weight);
	}


    void SetDeMorganPriorBelief(int nodeHandle, double belief)
    {
        DSL_demorgan *dm = GetDeMorganDef(nodeHandle);
		dm->SetPriorBelief(belief);
    }

    void SetDeMorganPriorBelief(String *nodeId, double belief)
    {
        SetDeMorganPriorBelief(ValidateNodeId(nodeId), belief);
    }

    double GetDeMorganPriorBelief(int nodeHandle)
    {
        DSL_demorgan *dm = GetDeMorganDef(nodeHandle);
		return dm->GetPriorBelief();
    }

    double GetDeMorganPriorBelief(String *nodeId)
    {
        return GetDeMorganPriorBelief(ValidateNodeId(nodeId));
    }

    void SetDeMorganParentType(int nodeHandle, int parentIndex, DeMorganParentType type)
    {
		DSL_demorgan *dm = GetDeMorganDef(nodeHandle);
		ValidateParentIndex(nodeHandle, parentIndex);
		int res = dm->SetParentType(parentIndex, type);
		SmileException::CheckSmileStatus("Can't set parent type", res);
    }

    void SetDeMorganParentType(String *nodeId, int parentIndex, DeMorganParentType type)
    {
        SetDeMorganParentType(ValidateNodeId(nodeId), parentIndex, type);
    }
    
    void SetDeMorganParentType(int nodeHandle, String *parentId, DeMorganParentType type)
    {
        SetDeMorganParentType(nodeHandle, ValidateParentId(nodeHandle, parentId), type);
    }

    void SetDeMorganParentType(String *nodeId, String *parentId, DeMorganParentType type)
    {
        SetDeMorganParentType(ValidateNodeId(nodeId), parentId, type);
    }


    DeMorganParentType GetDeMorganParentType(int nodeHandle, int parentIndex)
    {
        DSL_demorgan *dm = GetDeMorganDef(nodeHandle);
		ValidateParentIndex(nodeHandle, parentIndex);
        return static_cast<DeMorganParentType>(dm->GetParentType(parentIndex));
    }

    DeMorganParentType GetDeMorganParentType(String *nodeId, int parentIndex)
    {
        return GetDeMorganParentType(ValidateNodeId(nodeId), parentIndex);
    }
    
    DeMorganParentType GetDeMorganParentType(int nodeHandle, String *parentId)
    {
        return GetDeMorganParentType(nodeHandle, ValidateParentId(nodeHandle, parentId));
    }

    DeMorganParentType GetDeMorganParentType(String *nodeId, String *parentId)
    {
        return GetDeMorganParentType(ValidateNodeId(nodeId), parentId);
    }


    void SetDeMorganParentWeight(int nodeHandle, int parentIndex, double weight)
    {
		DSL_demorgan *dm = GetDeMorganDef(nodeHandle);
		ValidateParentIndex(nodeHandle, parentIndex);
		int res = dm->SetParentWeight(parentIndex, weight);
		SmileException::CheckSmileStatus("Can't set parent weight", res);
    }

    void SetDeMorganParentWeight(String *nodeId, int parentIndex, double weight)
    {
        SetDeMorganParentWeight(ValidateNodeId(nodeId), parentIndex, weight);
    }
    
    void SetDeMorganParentWeight(int nodeHandle, String *parentId, double weight)
    {
        SetDeMorganParentWeight(nodeHandle, ValidateParentId(nodeHandle, parentId), weight);
    }

    void SetDeMorganParentWeight(String *nodeId, String *parentId, double weight)
    {
        SetDeMorganParentWeight(ValidateNodeId(nodeId), parentId, weight);
    }


    double GetDeMorganParentWeight(int nodeHandle, int parentIndex)
    {
        DSL_demorgan *dm = GetDeMorganDef(nodeHandle);
		ValidateParentIndex(nodeHandle, parentIndex);
        return dm->GetParentWeight(parentIndex);
    }

    double GetDeMorganParentWeight(String *nodeId, int parentIndex)
    {
        return GetDeMorganParentWeight(ValidateNodeId(nodeId), parentIndex);
    }
    
    double GetDeMorganParentWeight(int nodeHandle, String *parentId)
    {
        return GetDeMorganParentWeight(nodeHandle, ValidateParentId(nodeHandle, parentId));
    }

    double GetDeMorganParentWeight(String *nodeId, String *parentId)
    {
        return GetDeMorganParentWeight(ValidateNodeId(nodeId), parentId);
    }



	bool IsValueValid(String *nodeId)
	{
		return IsValueValid(ValidateNodeId(nodeId));
	}

	bool IsValueValid(int nodeHandle)
	{
		return 0 != ValidateNodeHandle(nodeHandle)->Value()->IsValueValid();
	}

	Int32 GetValueIndexingParents(int nodeHandle)__gc[]
	{
		DSL_node *node = ValidateNodeHandle(nodeHandle);
		DSL_nodeValue *value = node->Value();
		return CopyIntArray(value->GetIndexingParents());
	}
	
	Int32 GetValueIndexingParents(String *nodeId)__gc[]
	{
		return GetValueIndexingParents(ValidateNodeId(nodeId));
	}

	String* GetValueIndexingParentIds(int nodeHandle)__gc[]
	{
		DSL_node *node = ValidateNodeHandle(nodeHandle);
		DSL_nodeValue *value = node->Value();
		return HandlesToIds(value->GetIndexingParents());
	}

	String* GetValueIndexingParentIds(String *nodeId)__gc[]
	{
		return GetValueIndexingParentIds(ValidateNodeId(nodeId));
	}

	Double GetNodeValue(String *nodeId)__gc []
	{
		return GetNodeValue(ValidateNodeId(nodeId));
	}

	Double GetNodeValue(int nodeHandle)__gc []
	{
		DSL_node *node = ValidateNodeHandle(nodeHandle);
		DSL_nodeValue *value = node->Value();

		if (!value->IsValueValid())
		{
			String *msg = String::Format(
				"Value not valid, node {0}", GetNodeId(nodeHandle));
			throw new SmileException(msg);
		}

		DSL_Dmatrix *m = NULL;
		int res = value->GetValue(&m);
		if (DSL_OKAY != res)
		{
			String *msg = String::Format(
				"Can't get node value {0} in node {1} as array of doubles", 
				value->GetType().ToString(), 
				GetNodeId(nodeHandle));
			throw new SmileException(msg, res);
		}

		return CopyDoubleArray(m->GetItems());
	}

	void ClearAllTargets()
	{
		net->ClearAllTargets();
	}

	void SetTarget(int nodeHandle, bool target)
	{
		ValidateNodeHandle(nodeHandle);
		int res;
		if (target)
		{
			res = net->SetTarget(nodeHandle);
		}
		else
		{
			res = net->UnSetTarget(nodeHandle);
		}

		if (res != DSL_OKAY)
		{
			String *msg = String::Format(
				"Can't change target status for node {0}",
				GetNodeId(nodeHandle));
			throw new SmileException(msg, res);
		}
	}

	void SetTarget(String *nodeId, bool target)
	{
		SetTarget(ValidateNodeId(nodeId), target);
	}

	bool IsTarget(int nodeHandle)
	{
		ValidateNodeHandle(nodeHandle);
		return 0 != net->IsTarget(nodeHandle);
	}

	bool IsTarget(String *nodeId)
	{
		return IsTarget(ValidateNodeId(nodeId));
	}
	
	void ClearAllEvidence()
	{
		net->ClearAllEvidence();
	}

	void ClearEvidence(String *nodeId)
	{
		return ClearEvidence(ValidateNodeId(nodeId));
	}

	void ClearEvidence(int nodeHandle)
	{
		DSL_node* node = ValidateNodeHandle(nodeHandle);
		int res = node->Value()->ClearEvidence();
		if (DSL_OKAY != res)
		{
			String *msg = String::Format(
				"Can't clear evidence for node {0}", 
				GetNodeId(nodeHandle));
			throw new SmileException(msg, res);
		}
	}
	
	bool IsEvidence(String *nodeId)
	{
		return IsEvidence(ValidateNodeId(nodeId));
	}

	bool IsEvidence(int nodeHandle)
	{
		return 0 != ValidateNodeHandle(nodeHandle)->Value()->IsEvidence();
	}

	bool IsRealEvidence(String *nodeId)
	{
		return IsRealEvidence(ValidateNodeId(nodeId));
	}

	bool IsRealEvidence(int nodeHandle)
	{
		return 0 != ValidateNodeHandle(nodeHandle)->Value()->IsRealEvidence();
	}

	bool IsPropagatedEvidence(String *nodeId)
	{
		return IsPropagatedEvidence(ValidateNodeId(nodeId));
	}

	bool IsPropagatedEvidence(int nodeHandle)
	{
		return 0 != ValidateNodeHandle(nodeHandle)->Value()->IsPropagatedEvidence();
	}

	void SetEvidence(int nodeHandle, int outcomeIndex)
	{
		DSL_node* node = ValidateNodeHandle(nodeHandle);
		int res = node->Value()->SetEvidence(outcomeIndex);
			
		if (DSL_OKAY != res)
		{
			String *msg = String::Format(
				"Can't set evidence to outcome {0} for node {1}", 
				Int32(outcomeIndex).ToString(),
				GetNodeId(nodeHandle));
			throw new SmileException(msg, res);
		}
	}

	void SetEvidence(String *nodeId, int outcomeIndex)
	{
		SetEvidence(ValidateNodeId(nodeId), outcomeIndex);
	}

	void SetEvidence(int nodeHandle, String *outcomeId)
	{
		SetEvidence(nodeHandle, ValidateOutcomeId(nodeHandle, outcomeId));
	}

	void SetEvidence(String *nodeId, String *outcomeId)
	{
		SetEvidence(ValidateNodeId(nodeId), outcomeId);
	}

	int GetEvidence(int nodeHandle)
	{
		DSL_nodeValue *value = ValidateNodeHandle(nodeHandle)->Value();
		if (!value->IsEvidence())
		{
			String *msg = String::Format(
				"Node {0} has no evidence set", 
				GetNodeId(nodeHandle));
			throw new SmileException(msg);
		}

		int result = value->GetEvidence();
		if (result < 0)
		{
			String *msg = String::Format(
				"Can't get evidence of node {0}", 
				GetNodeId(nodeHandle));
			throw new SmileException(msg);
		}
		
		return result;
	}

	int GetEvidence(String *nodeId)
	{
		return GetEvidence(ValidateNodeId(nodeId));
	}

	String* GetEvidenceId(String *nodeId)
	{
		return GetOutcomeId(nodeId, GetEvidence(nodeId));
	}

	String* GetEvidenceId(int nodeHandle)
	{
		return GetOutcomeId(nodeHandle, GetEvidence(nodeHandle));
	}

	bool IsTemporalEvidence(String *nodeId, int slice)
	{
		return IsTemporalEvidence(ValidateNodeId(nodeId), slice);
	}

	bool IsTemporalEvidence(int nodeHandle, int slice)
	{
		return 0 != ValidateNodeHandle(nodeHandle)->Value()->IsTemporalEvidence(slice);
	}

	int GetTemporalEvidence(int nodeHandle, int slice)
	{
		DSL_nodeValue *value = ValidateNodeHandle(nodeHandle)->Value();
		if (!value->IsTemporalEvidence(slice))
		{
			String *msg = String::Format(
				"Node {0} has no evidence in slice {1}", 
				GetNodeId(nodeHandle), Int32(slice).ToString());
			throw new SmileException(msg);
		}
		return value->GetTemporalEvidence(slice);
	}

	int GetTemporalEvidence(String *nodeId, int slice)
	{
		return GetTemporalEvidence(ValidateNodeId(nodeId), slice);
	}

	String* GetTemporalEvidenceId(int nodeHandle, int slice)
	{
		return GetOutcomeId(nodeHandle, GetTemporalEvidence(nodeHandle, slice));
	}

	String* GetTemporalEvidenceId(String *nodeId, int slice)
	{
		return GetTemporalEvidenceId(ValidateNodeId(nodeId), slice);
	}

	void SetTemporalEvidence(int nodeHandle, int slice, int outcomeIndex)
	{
		DSL_nodeValue *value = ValidateNodeHandle(nodeHandle)->Value();
		int res = value->SetTemporalEvidence(slice, outcomeIndex);
		if (DSL_OKAY != res)
		{
			String *msg = String::Format(
				"Can't set temporal evidence to outcome {0} in slice {1} for node {2}", 
				Int32(outcomeIndex).ToString(),
				Int32(slice).ToString(),
				GetNodeId(nodeHandle));
			throw new SmileException(msg, res);
		}
	}

	void SetTemporalEvidence(int nodeHandle, int slice, String* outcomeId)
	{
		SetTemporalEvidence(nodeHandle, slice, ValidateOutcomeId(nodeHandle, outcomeId));
	}

	void SetTemporalEvidence(String *nodeId, int slice, int outcomeIndex)
	{
		SetTemporalEvidence(ValidateNodeId(nodeId), slice, outcomeIndex);
	}

	void SetTemporalEvidence(String *nodeId, int slice, String* outcomeId)
	{
		SetTemporalEvidence(ValidateNodeId(nodeId), slice, outcomeId);
	}

	void ControlValue(int nodeHandle, int outcomeIndex)
	{
		DSL_beliefVector *bv = ValidateBeliefVector(nodeHandle);
		int res = bv->ControlValue(outcomeIndex);
			
		if (DSL_OKAY != res)
		{
			String *msg = String::Format(
				"Can't control value to outcome {0} for node {1}", 
				Int32(outcomeIndex).ToString(),
				GetNodeId(nodeHandle));
			throw new SmileException(msg, res);
		}
	}

	void ControlValue(String *nodeId, int outcomeIndex)
	{
		ControlValue(ValidateNodeId(nodeId), outcomeIndex);
	}

	void ControlValue(int nodeHandle, String *outcomeId)
	{
		ControlValue(nodeHandle, ValidateOutcomeId(nodeHandle, outcomeId));
	}

	void ControlValue(String *nodeId, String *outcomeId)
	{
		ControlValue(ValidateNodeId(nodeId), outcomeId);
	}

	int GetControlledValue(int nodeHandle)
	{
		DSL_beliefVector *bv = ValidateBeliefVector(nodeHandle);
		if (!bv->IsControlled())
		{
			String *msg = String::Format(
				"Node {0} is not controlled", 
				GetNodeId(nodeHandle));
			throw new SmileException(msg);
		}

		int result = bv->GetControlledValue();
		if (result < 0)
		{
			String *msg = String::Format(
				"Can't get controlledValue of node {0}", 
				GetNodeId(nodeHandle));
			throw new SmileException(msg);
		}
		
		return result;
	}

	int GetControlledValue(String *nodeId)
	{
		return GetControlledValue(ValidateNodeId(nodeId));
	}

	String* GetControlledValueId(String *nodeId)
	{
		return GetOutcomeId(nodeId, GetControlledValue(nodeId));
	}

	String* GetControlledValueId(int nodeHandle)
	{
		return GetOutcomeId(nodeHandle, GetControlledValue(nodeHandle));
	}

	bool IsControlled(String *nodeId)
	{
		return IsControlled(ValidateNodeId(nodeId));
	}

	bool IsControlled(int nodeHandle)
	{
		return 0 != ValidateBeliefVector(nodeHandle)->IsControlled();
	}

	bool IsControllable(String *nodeId)
	{
		return IsControllable(ValidateNodeId(nodeId));
	}

	bool IsControllable(int nodeHandle)
	{
		return 0 != ValidateBeliefVector(nodeHandle)->IsControllable();
	}

	void ClearControlledValue(String *nodeId)
	{
		return ClearControlledValue(ValidateNodeId(nodeId));
	}

	void ClearControlledValue(int nodeHandle)
	{
		DSL_beliefVector *bv = ValidateBeliefVector(nodeHandle);
		int res = bv->ClearControlledValue();
		if (DSL_OKAY != res)
		{
			String *msg = String::Format(
				"Can't clear controlled value for node {0}", 
				GetNodeId(nodeHandle));
			throw new SmileException(msg, res);
		}
	}

	String* GetOutcomeId(String *nodeId, int outcomeIndex)
	{
		return GetOutcomeId(ValidateNodeId(nodeId), outcomeIndex);
	}

	String* GetOutcomeId(int nodeHandle, int outcomeIndex)
	{
		DSL_node *node = ValidateNodeHandle(nodeHandle);
		DSL_idArray * outcomeNames = node->Definition()->GetOutcomesNames();
		const char* id = outcomeNames->Subscript(outcomeIndex);
		if (NULL == id)
		{
			String *msg = String::Format("Outcome index {0} out of range for node {1}", 
				Int32(outcomeIndex).ToString(),
				GetNodeId(nodeHandle));
			throw new SmileException(msg);
		}

		return new String(id);
	}

	void SetOutcomeId(String *nodeId, int outcomeIndex, String *id)
	{
		SetOutcomeId(ValidateNodeId(nodeId), outcomeIndex, id);
	}

	void SetOutcomeId(int nodeHandle, int outcomeIndex, String *id)
	{
		DSL_node *node = ValidateNodeHandle(nodeHandle);
		DSL_idArray * outcomeNames = node->Definition()->GetOutcomesNames();
		if (NULL == outcomeNames->Subscript(outcomeIndex))
		{
			String *msg = String::Format("Outcome index {0} out of range for node {1}", 
				Int32(outcomeIndex).ToString(),
				GetNodeId(nodeHandle));
			throw new SmileException(msg);
		}

		StringToCharPtr szId(id);
		int res = node->Definition()->RenameOutcome(outcomeIndex, szId);
		if (DSL_OKAY != res)
		{
			String *msg = String::Format("Can't set ID for outcome {0} in node {1}", 
				Int32(outcomeIndex).ToString(),
				GetNodeId(nodeHandle));
			throw new SmileException(msg, res);
		}
	}

	String* GetOutcomeIds(int nodeHandle)__gc[]
	{
		ValidateNodeHandle(nodeHandle);

		DSL_idArray *arrId = net->GetNode(nodeHandle)->Definition()->GetOutcomesNames();
		if (NULL == arrId)
		{
			String *msg = String::Format("Node {0} does not have outcomes", 
				GetNodeId(nodeHandle));
			throw new SmileException(msg);
		}
		
		int imax = arrId->NumItems();
		String* ar[] = new String*[imax];
		for (int i = 0; i < imax; i ++)
		{
			ar[i] = new String(arrId->Subscript(i));
		}
		return ar;
	}

	String* GetOutcomeIds(String *nodeId)__gc[]
	{
		return GetOutcomeIds(ValidateNodeId(nodeId));
	}

	int GetOutcomeCount(int nodeHandle) 
	{
		DSL_node* node = ValidateNodeHandle(nodeHandle);
		return node->Definition()->GetNumberOfOutcomes();
	}
	
	int GetOutcomeCount(String *nodeId)
	{
		return GetOutcomeCount(ValidateNodeId(nodeId));
	}
	
	// -----------------------------
	// --- Visualization (GeNIe) ---
	// -----------------------------

	System::Drawing::Rectangle GetNodePosition(int nodeHandle) {
		DSL_node *node = ValidateNodeHandle(nodeHandle);
		DSL_rectangle &pos = node->Info().Screen().position;

		int x = pos.center_X - (pos.width / 2);
		int y = pos.center_Y - (pos.height / 2);
		int width = pos.width;
		int height = pos.height;

		return System::Drawing::Rectangle(x, y, width, height);
	}
	
	System::Drawing::Rectangle GetNodePosition(String *nodeId) {
		return GetNodePosition(ValidateNodeId(nodeId));
	}

	void SetNodePosition(int nodeHandle, int x, int y, int width, int height)
	{
		DSL_node *node = ValidateNodeHandle(nodeHandle);
		DSL_rectangle &pos = node->Info().Screen().position;

		pos.width = width;
		pos.height = height;
		pos.center_X = x + (width / 2);
		pos.center_Y = y + (height / 2);
	}

	void SetNodePosition(String *nodeId, int x, int y, int width, int height)
	{
		SetNodePosition(ValidateNodeId(nodeId), x, y, width, height);
	}

	void SetNodePosition(int nodeHandle, System::Drawing::Rectangle rect)
	{
		SetNodePosition(nodeHandle, rect.X, rect.Y, rect.Width, rect.Height);
	}

	void SetNodePosition(char *nodeId, System::Drawing::Rectangle rect)
	{
		SetNodePosition(ValidateNodeId(nodeId), rect.X, rect.Y, rect.Width, rect.Height);
	}

	Color GetNodeBgColor(int nodeHandle)
	{
		return _GetScrInfoColor(nodeHandle, &DSL_screenInfo::color);
	}

	Color GetNodeBgColor(String *nodeId)
	{
		return GetNodeBgColor(ValidateNodeId(nodeId));
	}

	Color GetNodeTextColor(int nodeHandle)
	{
		return _GetScrInfoColor(nodeHandle, &DSL_screenInfo::fontColor);
	}

	Color GetNodeTextColor(String *nodeId)
	{
		return GetNodeTextColor(ValidateNodeId(nodeId));
	}

	Color GetNodeBorderColor(int nodeHandle)
	{
		return _GetScrInfoColor(nodeHandle, &DSL_screenInfo::borderColor);
	}

	Color GetNodeBorderColor(String *nodeId)
	{
		return GetNodeBorderColor(ValidateNodeId(nodeId));
	}

	int GetNodeBorderWidth(int nodeHandle)
	{
		return _NodeScrInfoRef(nodeHandle, &DSL_screenInfo::borderThickness);		
	}

	int GetNodeBorderWidth(String *nodeId)
	{
		return GetNodeBorderWidth(ValidateNodeId(nodeId));
	}
	
	void SetNodeBgColor(int nodeHandle, Color color)
	{
		_SetScrInfoColor(nodeHandle, &DSL_screenInfo::color, color);
	}

	void SetNodeBgColor(String *nodeId, Color color)
	{
		SetNodeBgColor(ValidateNodeId(nodeId), color);
	}

	void SetNodeTextColor(int nodeHandle, Color color)
	{
		_SetScrInfoColor(nodeHandle, &DSL_screenInfo::fontColor, color);
	}

	void SetNodeTextColor(String *nodeId, Color color)
	{
		SetNodeTextColor(ValidateNodeId(nodeId), color);
	}

	void SetNodeBorderColor(int nodeHandle, Color color)
	{
		_SetScrInfoColor(nodeHandle, &DSL_screenInfo::borderColor, color);
	}

	void SetNodeBorderColor(String *nodeId, Color color)
	{
		SetNodeBorderColor(ValidateNodeId(nodeId), color);
	}

	void SetNodeBorderWidth(int nodeHandle, int width)
	{
		_NodeScrInfoRef(nodeHandle, &DSL_screenInfo::borderThickness) = width;		
	}

	void SetNodeBorderWidth(String *nodeId, int width)
	{
		SetNodeBorderWidth(ValidateNodeId(nodeId), width);
	}

	// ------------------------------
	// Diagnostic properties of nodes
	// ------------------------------
	
	NodeDiagType GetNodeDiagType(int nodeHandle)
	{
		DSL_node *node = ValidateNodeHandle(nodeHandle);
		return static_cast<NodeDiagType>(node->ExtraDefinition()->GetType());
	}

	void SetNodeDiagType(int nodeHandle, NodeDiagType diagType)
	{
		DSL_node *node = ValidateNodeHandle(nodeHandle);
		node->ExtraDefinition()->SetType(static_cast<DSL_extraDefinition::troubleType>(diagType));
	}

	void SetNodeDiagType(String *nodeId, NodeDiagType diagType)
	{
		SetNodeDiagType(ValidateNodeId(nodeId), diagType);
	}

	NodeDiagType GetNodeDiagType(String *nodeId)
	{
		return GetNodeDiagType(ValidateNodeId(nodeId));
	}

	bool IsRanked(int nodeHandle)
	{
		DSL_extraDefinition *extraDef = GetExtraDef(nodeHandle);
		return extraDef->IsRanked();
	}

	bool IsRanked(String *nodeId)
	{
		return IsRanked(ValidateNodeId(nodeId));
	}

	void SetRanked(int nodeHandle, bool ranked)
	{
		DSL_extraDefinition *extraDef = GetExtraDef(nodeHandle);
		extraDef->SetFlags(ranked, extraDef->IsMandatory(), extraDef->IsSetToDefault());
	}

	void SetRanked(String *nodeId, bool ranked)
	{
		SetRanked(ValidateNodeId(nodeId), ranked);
	}

	bool IsMandatory(int nodeHandle)
	{
		DSL_extraDefinition *extraDef = GetExtraDef(nodeHandle);
		return extraDef->IsMandatory();
	}

	bool IsMandatory(String *nodeId)
	{
		return IsMandatory(ValidateNodeId(nodeId));
	}

	void SetMandatory(int nodeHandle, bool mandatory)
	{
		DSL_extraDefinition *extraDef = GetExtraDef(nodeHandle);
		extraDef->SetFlags(extraDef->IsRanked(), mandatory, extraDef->IsSetToDefault());
	}

	void SetMandatory(String *nodeId, bool mandatory)
	{
		SetMandatory(ValidateNodeId(nodeId), mandatory);
	}


	void AddCostArc(int parentHandle, int childHandle)
	{
		AddArcHelper(parentHandle, childHandle, dsl_costObserve);
	}

	void AddCostArc(String *parentId, String *childId)
	{
		AddCostArc(ValidateNodeId(parentId), ValidateNodeId(childId));
	}

	void DeleteCostArc(int parentHandle, int childHandle)
	{
		DeleteArcHelper(parentHandle, childHandle, dsl_costObserve);
	}

	void DeleteCostArc(String *parentId, String *childId)
	{
		DeleteCostArc(ValidateNodeId(parentId), ValidateNodeId(childId));
	}
	

	Int32 GetCostParents(String *nodeId)__gc[]	
	{
		return GetCostParents(ValidateNodeId(nodeId));
	}

	Int32 GetCostParents(int nodeHandle)__gc[]
	{
		ValidateNodeHandle(nodeHandle);
		return CopyIntArray(net->GetParents(nodeHandle, dsl_costObserve));
	}

	String* GetCostParentIds(String *nodeId)__gc[]
	{
		return GetCostParentIds(ValidateNodeId(nodeId));
	}

	String* GetCostParentIds(int nodeHandle)__gc[]
	{
		ValidateNodeHandle(nodeHandle);
		return HandlesToIds(net->GetParents(nodeHandle, dsl_costObserve));
	}

	Int32 GetCostChildren(String *nodeId)__gc[]	
	{
		return GetCostChildren(ValidateNodeId(nodeId));
	}

	Int32 GetCostChildren(int nodeHandle)__gc[]
	{
		ValidateNodeHandle(nodeHandle);
		return CopyIntArray(net->GetChildren(nodeHandle, dsl_costObserve));
	}

	String* GetCostChildIds(String *nodeId)__gc[]
	{
		return GetCostChildIds(ValidateNodeId(nodeId));
	}

	String* GetCostChildIds(int nodeHandle)__gc[]
	{
		ValidateNodeHandle(nodeHandle);
		return HandlesToIds(net->GetChildren(nodeHandle, dsl_costObserve));
	}

	String* GetNodeQuestion(int nodeHandle)
	{
		DSL_extraDefinition *extraDef = GetExtraDef(nodeHandle);
		return new String(extraDef->GetQuestion().c_str());
	}

	Double GetNodeCost(String *nodeId)__gc []
	{
		return GetNodeCost(ValidateNodeId(nodeId));
	}

	Double GetNodeCost(int nodeHandle)__gc []
	{
		DSL_nodeCost *cost = GetCost(nodeHandle);
		if (NULL == cost)
		{
			String *msg = String::Format(
				"Can't get node cost for node {0}", 
				GetNodeId(nodeHandle));
			throw new SmileException(msg);
		}

		return CopyDoubleArray(cost->GetCosts().GetItems());
	}

	void SetNodeCost(String *nodeId, Double cost[])
	{
		SetNodeCost(ValidateNodeId(nodeId), cost);
	}

	void SetNodeCost(int nodeHandle, Double cost[])
	{
		DSL_nodeCost *pCost = GetCost(nodeHandle);
		if (NULL == pCost)
		{
			String *msg = String::Format(
				"Can't get node cost for node {0}", 
				GetNodeId(nodeHandle));
			throw new SmileException(msg);
		}

		
		DSL_Dmatrix &mtx = pCost->GetCosts();
		int imax = mtx.GetSize();
		if (imax != cost->Length)
		{
			String *msg = String::Format(
				"Invalid cost array size for node {0}. Expected {1} and got {2}", 
				GetNodeId(nodeHandle),
				Int32(imax).ToString(),
				cost->Length.ToString());
			throw new SmileException(msg);
		}

		for (int i = 0; i < imax; i ++)
		{
			mtx[i] = cost[i];
		}
	}

	String* GetNodeQuestion(String *nodeId)
	{
		return GetNodeQuestion(ValidateNodeId(nodeId));
	}

	void SetNodeQuestion(int nodeHandle, String *question)
	{
		DSL_extraDefinition *extraDef = GetExtraDef(nodeHandle);
		StringToCharPtr szQuestion(question);
		extraDef->GetQuestion() = szQuestion;
	}

	void SetNodeQuestion(String *nodeId, String *question)
	{
		SetNodeQuestion(ValidateNodeId(nodeId), question);
	}

	DocItemInfo GetOutcomeDocumentation(int nodeHandle, int outcomeIndex)__gc[]
	{
		DSL_extraDefinition *extraDef = GetExtraDef(nodeHandle);
		ValidateOutcomeIndex(nodeHandle, outcomeIndex);
		return ConvertDocumentation(extraDef->GetDocumentation(outcomeIndex));
	}

	DocItemInfo GetOutcomeDocumentation(String* nodeId, int outcomeIndex)__gc[]
	{
		return GetOutcomeDocumentation(ValidateNodeId(nodeId), outcomeIndex);
	}

	DocItemInfo GetOutcomeDocumentation(String* nodeId, String *outcomeId)__gc[]
	{
		return GetOutcomeDocumentation(ValidateNodeId(nodeId), outcomeId);
	}
	
	DocItemInfo GetOutcomeDocumentation(int nodeHandle, String* outcomeId)__gc[]
	{
		return GetOutcomeDocumentation(nodeHandle, ValidateOutcomeId(nodeHandle, outcomeId));
	}


	void SetOutcomeDocumentation(int nodeHandle, int outcomeIndex, DocItemInfo documentation[])
	{
		DSL_extraDefinition *extraDef = GetExtraDef(nodeHandle);
		ValidateOutcomeIndex(nodeHandle, outcomeIndex);
		
		ConvertDocumentation(documentation, extraDef->GetDocumentation(outcomeIndex)); 
	}

	void SetOutcomeDocumentation(int nodeHandle, String *outcomeId, DocItemInfo documentation[])
	{
		SetOutcomeDocumentation(nodeHandle, ValidateOutcomeId(nodeHandle, outcomeId), documentation);
	}
	
	void SetOutcomeDocumentation(String *nodeId, String *outcomeId, DocItemInfo documentation[])
	{
		SetOutcomeDocumentation(ValidateNodeId(nodeId), outcomeId, documentation);
	}

	void SetOutcomeDocumentation(String *nodeId, int outcomeIndex, DocItemInfo documentation[])
	{
		SetOutcomeDocumentation(ValidateNodeId(nodeId), outcomeIndex, documentation);
	}

	String* GetOutcomeFix(int nodeHandle, int outcomeIndex)
	{
		DSL_extraDefinition *extraDef = GetExtraDef(nodeHandle);
		ValidateOutcomeIndex(nodeHandle, outcomeIndex);
		return extraDef->GetStateRepairInfo(outcomeIndex);
	}

	String* GetOutcomeFix(String *nodeId, int outcomeIndex)
	{
		return GetOutcomeFix(ValidateNodeId(nodeId), outcomeIndex);
	}

	String* GetOutcomeFix(int nodeHandle, String *outcomeId)
	{
		return GetOutcomeFix(nodeHandle, ValidateOutcomeId(nodeHandle, outcomeId));
	}

	String* GetOutcomeFix(String* nodeId, String* outcomeId)
	{
		return GetOutcomeFix(ValidateNodeId(nodeId), outcomeId);
	}

	void SetOutcomeFix(int nodeHandle, int outcomeIndex, String* treatment)
	{
		DSL_extraDefinition *extraDef = GetExtraDef(nodeHandle);
		ValidateOutcomeIndex(nodeHandle, outcomeIndex);
		StringToCharPtr szTreatment(treatment);
		extraDef->SetStateRepairInfo(outcomeIndex, szTreatment);
	}

	void SetOutcomeFix(String *nodeId, int outcomeIndex, String* treatment)
	{
		SetOutcomeFix(ValidateNodeId(nodeId), outcomeIndex, treatment);
	}

	void SetOutcomeFix(int nodeHandle, String *outcomeId, String* treatment)
	{
		SetOutcomeFix(nodeHandle, ValidateOutcomeId(nodeHandle, outcomeId), treatment);
	}

	void SetOutcomeFix(String* nodeId, String* outcomeId, String* treatment)
	{
		SetOutcomeFix(ValidateNodeId(nodeId), outcomeId, treatment);
	}



	String* GetOutcomeDescription(int nodeHandle, int outcomeIndex)
	{
		DSL_extraDefinition *extraDef = GetExtraDef(nodeHandle);
		ValidateOutcomeIndex(nodeHandle, outcomeIndex);
		return new String(extraDef->GetStateDescription(outcomeIndex));
	}

	String* GetOutcomeDescription(String *nodeId, int outcomeIndex)
	{
		return GetOutcomeDescription(ValidateNodeId(nodeId), outcomeIndex);
	}

	String* GetOutcomeDescription(int nodeHandle, String *outcomeId)
	{
		return GetOutcomeDescription(nodeHandle, ValidateOutcomeId(nodeHandle, outcomeId));
	}

	String* GetOutcomeDescription(String* nodeId, String* outcomeId)
	{
		return GetOutcomeDescription(ValidateNodeId(nodeId), outcomeId);
	}

	void SetOutcomeDescription(int nodeHandle, int outcomeIndex, String* description)
	{
		DSL_extraDefinition *extraDef = GetExtraDef(nodeHandle);
		ValidateOutcomeIndex(nodeHandle, outcomeIndex);
		StringToCharPtr szDescription(description);
		extraDef->SetStateDescription(outcomeIndex, szDescription);
	}

	void SetOutcomeDescription(String *nodeId, int outcomeIndex, String* description)
	{
		SetOutcomeDescription(ValidateNodeId(nodeId), outcomeIndex, description);
	}

	void SetOutcomeDescription(int nodeHandle, String *outcomeId, String* description)
	{
		SetOutcomeDescription(nodeHandle, ValidateOutcomeId(nodeHandle, outcomeId), description);
	}

	void SetOutcomeDescription(String* nodeId, String* outcomeId, String* description)
	{
		SetOutcomeDescription(ValidateNodeId(nodeId), outcomeId, description);
	}



	String* GetOutcomeLabel(int nodeHandle, int outcomeIndex)
	{
		DSL_extraDefinition *extraDef = GetExtraDef(nodeHandle);
		ValidateOutcomeIndex(nodeHandle, outcomeIndex);
		return new String(extraDef->GetFaultLabels().Subscript(outcomeIndex));
	}

	String* GetOutcomeLabel(String *nodeId, int outcomeIndex)
	{
		return GetOutcomeLabel(ValidateNodeId(nodeId), outcomeIndex);
	}

	String* GetOutcomeLabel(int nodeHandle, String *outcomeId)
	{
		return GetOutcomeLabel(nodeHandle, ValidateOutcomeId(nodeHandle, outcomeId));
	}

	String* GetOutcomeLabel(String* nodeId, String* outcomeId)
	{
		return GetOutcomeLabel(ValidateNodeId(nodeId), outcomeId);
	}

	bool SetOutcomeLabel(int nodeHandle, int outcomeIndex, String* label)
	{
		DSL_extraDefinition *extraDef = GetExtraDef(nodeHandle);
		ValidateOutcomeIndex(nodeHandle, outcomeIndex);
		StringToCharPtr szLabel(label);
		return 0 != extraDef->SetLabel(outcomeIndex, szLabel);
	}

	bool SetOutcomeLabel(String *nodeId, int outcomeIndex, String* label)
	{
		return SetOutcomeLabel(ValidateNodeId(nodeId), outcomeIndex, label);
	}

	bool SetOutcomeLabel(int nodeHandle, String *outcomeId, String* label)
	{
		return SetOutcomeLabel(nodeHandle, ValidateOutcomeId(nodeHandle, outcomeId), label);
	}

	bool SetOutcomeLabel(String* nodeId, String* outcomeId, String* label)
	{
		return SetOutcomeLabel(ValidateNodeId(nodeId), outcomeId, label);
	}


	bool IsFaultOutcome(int nodeHandle, int outcomeIndex)
	{
		DSL_extraDefinition *extraDef = GetExtraDef(nodeHandle);
		ValidateOutcomeIndex(nodeHandle, outcomeIndex);
		return 0 != extraDef->IsFaultState(outcomeIndex);
	}

	bool IsFaultOutcome(String *nodeId, int outcomeIndex)
	{
		return IsFaultOutcome(ValidateNodeId(nodeId), outcomeIndex);
	}

	bool IsFaultOutcome(int nodeHandle, String *outcomeId)
	{
		return IsFaultOutcome(nodeHandle, ValidateOutcomeId(nodeHandle, outcomeId));
	}

	bool IsFaultOutcome(String* nodeId, String* outcomeId)
	{
		return IsFaultOutcome(ValidateNodeId(nodeId), outcomeId);
	}

	void SetFaultOutcome(int nodeHandle, int outcomeIndex, bool fault)
	{
		DSL_extraDefinition *extraDef = GetExtraDef(nodeHandle);
		ValidateOutcomeIndex(nodeHandle, outcomeIndex);
		extraDef->SetFaultState(outcomeIndex, fault ? DSL_TRUE : DSL_FALSE);
	}

	void SetFaultOutcome(String *nodeId, int outcomeIndex, bool fault)
	{
		SetFaultOutcome(ValidateNodeId(nodeId), outcomeIndex, fault);
	}

	void SetFaultOutcome(int nodeHandle, String *outcomeId, bool fault)
	{
		SetFaultOutcome(nodeHandle, ValidateOutcomeId(nodeHandle, outcomeId), fault);
	}

	void SetFaultOutcome(String* nodeId, String* outcomeId, bool fault)
	{
		SetFaultOutcome(ValidateNodeId(nodeId), outcomeId, fault);
	}

	int GetDefaultOutcome(int nodeHandle)
	{
		DSL_extraDefinition *extraDef = GetExtraDef(nodeHandle);
		if (extraDef->IsSetToDefault())
		{
			return extraDef->GetDefaultOutcome();
		}
		else
		{
			return -1;
		}
	}

	int GetDefaultOutcome(String* nodeId)
	{
		return GetDefaultOutcome(ValidateNodeId(nodeId));
	}

	String* GetDefaultOutcomeId(int nodeHandle)
	{
		int defOutcome = GetDefaultOutcome(nodeHandle);
		if (defOutcome >= 0)
		{
			return GetOutcomeId(nodeHandle, defOutcome);
		}
		else
		{
			return NULL;
		}
	}

	String* GetDefaultOutcomeId(String *nodeId)
	{
		return GetDefaultOutcomeId(ValidateNodeId(nodeId));
	}

	void SetDefaultOutcome(int nodeHandle, int defOutcome)
	{
		DSL_node *node = ValidateNodeHandle(nodeHandle);
		DSL_nodeDefinition *def = node->Definition();
		if (defOutcome > def->GetNumberOfOutcomes())
		{
			String *msg = String::Format(
				"Invalid outcome index for node {0} in SetDefaultOutcome: {1}", 
				GetNodeId(nodeHandle),
				Int32(defOutcome).ToString());
			throw new SmileException(msg);
		}

		DSL_extraDefinition *extraDef = GetExtraDef(nodeHandle);
		bool setToDefault = false;
		if (defOutcome > 0)
		{
			extraDef->SetDefaultOutcome(defOutcome);
			setToDefault = true;
		}

		bool ranked = extraDef->IsRanked();
		bool mandatory = extraDef->IsMandatory();
		extraDef->SetFlags(ranked, mandatory, setToDefault);
	}

	void SetDefaultOutcome(String *nodeId, int defOutcome)
	{
		SetDefaultOutcome(ValidateNodeId(nodeId), defOutcome);
	}

	void SetDefaultOutcome(int nodeHandle, String* defOutcomeId)
	{
		int outcomeIndex = -1;
		if (NULL != defOutcomeId)
		{
			outcomeIndex = ValidateOutcomeId(nodeHandle, defOutcomeId);
		}

		SetDefaultOutcome(nodeHandle, outcomeIndex);
	}

	void SetDefaultOutcome(String *nodeId, String* defOutcomeId)
	{
		SetDefaultOutcome(ValidateNodeId(nodeId), defOutcomeId);
	}

	// ------------------------------

private public:
	DSL_network* _GetDslNet()
	{
		return net;
	}

	DSL_node* ValidateNodeHandle(int handle)
	{
		DSL_node *node = net->GetNode(handle);
		if (NULL == node)
		{
			String *msg = String::Format("Invalid node handle: {0}", Int32(handle).ToString());
			throw new SmileException(msg);
		}
		return node;
	}

	int ValidateNodeId(String *nodeId)
	{
		StringToCharPtr szId(nodeId);
		int handle = net->FindNode(szId);
		if (handle < 0)
		{
			String *msg = String::Format("Invalid node identifier: {0}", nodeId);
			throw new SmileException(msg);
		}
		return handle;
	}

	int ValidateOutcomeId(int nodeHandle, String *outcomeId)
	{
		DSL_node *node = ValidateNodeHandle(nodeHandle);
		DSL_idArray * outcomeNames = node->Definition()->GetOutcomesNames();
		
		StringToCharPtr szOutcome(outcomeId);
		int outcomeIndex = outcomeNames->FindPosition(szOutcome);
		if (outcomeIndex < 0)
		{
			String *msg = String::Format(
				"Invalid outcome identifier {0} for node {1}", 
				outcomeId,
				GetNodeId(nodeHandle));
			throw new SmileException(msg);
		}
		
		return outcomeIndex;
	}

	void ValidateOutcomeIndex(int nodeHandle, int outcomeIndex)
	{
		DSL_node *node = ValidateNodeHandle(nodeHandle);
		if (outcomeIndex < 0 || outcomeIndex >= node->Definition()->GetNumberOfOutcomes())
		{
			String *msg = String::Format(
				"Invalid outcome index {0} for node {1}", 
				Int32(outcomeIndex).ToString(),
				GetNodeId(nodeHandle));
			throw new SmileException(msg);
		}
	}

	DSL_beliefVector* ValidateBeliefVector(int nodeHandle)
	{
		DSL_node* node = ValidateNodeHandle(nodeHandle);
		DSL_nodeValue *val = node->Value();
		int valType = val->GetType();
		if (DSL_BELIEFVECTOR != valType)
		{
			String *msg = String::Format(
				"Invalid node value type for node {0} - was {1}, expected {2} (DSL_BELIEFVECTOR)",
				GetNodeId(nodeHandle),
				Int32(valType).ToString(),
				Int32(DSL_BELIEFVECTOR).ToString());
			throw new SmileException(msg);
		}

		return static_cast<DSL_beliefVector*>(val);
	}


	int AddNodeHelper(int nodeType, char *nodeId)
	{
		int newNode = net->AddNode(nodeType, nodeId);
		if (newNode < 0)
		{
			String *msg = String::Format(
				"Can't add node, SMILE error code {0}", 
				Int32(newNode).ToString());
			throw new SmileException(msg);
		}

		return newNode;
	}

	void ValidateId(String *id)
	{
		bool valid = true;
		int len = id->Length;
		if (0 == len || !Char::IsLetter(id, 0))
		{
			valid = false;
		}
		else
		{
			for (int i = 1; i < len; i ++)
			{
				if (!Char::IsLetterOrDigit(id, i))
				{
					if (id->Chars[i] != '_')
					{
						valid = false;
						break;
					}
				}
			}
		}

		if (!valid)
		{
			String *msg = String::Format(
				"Identifier '{0}' is invalid: should start with a letter and contain letters, digits and underscores", 
				id);
			throw new SmileException(msg);
		}
	}

	String* HandlesToIds(DSL_intArray& native)__gc[]
	{
		int imax = native.NumItems();
		String* ar[] = new String*[imax];
		for (int i = 0; i < imax; i ++)
		{
			ar[i] = new String(net->GetNode(native[i])->Info().Header().GetId());
		}
		return ar;
	}
	
	DSL_doubleArray* GetDefinitionArray(DSL_node *node)
	{
		DSL_nodeDefinition *nodeDef = node->Definition();
		DSL_doubleArray *pArray = NULL;

		switch (nodeDef->GetType())
		{
		case DSL_NOISY_MAX:
		case DSL_NOISY_ADDER:
			pArray = &static_cast<DSL_ciDefinition *>(nodeDef)->GetCiWeights().GetItems();
			break;
		default:
			nodeDef->GetDefinition(&pArray);
			break;
		}

		return pArray;
	}

	DSL_noisyMAX* GetNoisyMaxDef(int nodeHandle)
	{
		DSL_node *node = ValidateNodeHandle(nodeHandle);
		DSL_nodeDefinition *nodeDef = node->Definition();
		if (nodeDef->GetType() != DSL_NOISY_MAX)
		{
			String *msg = String::Format(
				"Node {0} is not NoisyMax",
				GetNodeId(nodeHandle));
			throw new SmileException(msg);
		}

		return static_cast<DSL_noisyMAX *>(nodeDef);
	}

	DSL_noisyAdder* GetNoisyAdderDef(int nodeHandle)
	{
		DSL_node *node = ValidateNodeHandle(nodeHandle);
		DSL_nodeDefinition *nodeDef = node->Definition();
		if (nodeDef->GetType() != DSL_NOISY_ADDER)
		{
			String *msg = String::Format(
				"Node {0} is not NoisyAdder",
				GetNodeId(nodeHandle));
			throw new SmileException(msg);
		}

		return static_cast<DSL_noisyAdder *>(nodeDef);
	}

	DSL_demorgan* GetDeMorganDef(int nodeHandle)
	{
		DSL_node *node = ValidateNodeHandle(nodeHandle);
		DSL_nodeDefinition *nodeDef = node->Definition();
		if (nodeDef->GetType() != DSL_DEMORGAN)
		{
			String *msg = String::Format(
				"Node {0} is not DeMorgan",
				GetNodeId(nodeHandle));
			throw new SmileException(msg);
		}

		return static_cast<DSL_demorgan *>(nodeDef);
	}


	int ValidateParentIndex(int nodeHandle, int parentIndex)
	{
		DSL_intArray &parents = net->GetParents(nodeHandle);
		int parentCount = parents.NumItems();
		if (parentIndex >= parentCount ||
			parentIndex < 0)
		{
			String *msg = String::Format(
				"Invalid parent index {0}: Node {1} has {2} parent(s)",
				parentIndex.ToString(),
				GetNodeId(nodeHandle),
				parentCount.ToString());
			throw new SmileException(msg);
		}

		return parents[parentIndex];
	}

	int ValidateParentId(int nodeHandle, String *parentId)
	{
		ValidateNodeHandle(nodeHandle);
		DSL_intArray& parents = net->GetParents(nodeHandle);

		StringToCharPtr szId(parentId);

		for (int i = 0; i < parents.NumItems(); i ++)
		{
			const char* id = net->GetNode(parents[i])->Info().Header().GetId();
			if (0 == strcmp(szId, id))
			{
				return i;
			}
		}

		String *msg = String::Format(
			"Node {0} is not a child of {1}",
			GetNodeId(nodeHandle),
			parentId);
		throw new SmileException(msg);
	}

	int& _NodeScrInfoRef(int nodeHandle, int DSL_screenInfo::*clrPtr)
	{
		DSL_node *node = ValidateNodeHandle(nodeHandle);
		DSL_screenInfo *screenInfo = &node->Info().Screen();

		return screenInfo->*clrPtr;
	}

	Color _GetScrInfoColor(int nodeHandle, int DSL_screenInfo::*clrPtr)
	{
		COLORREF c = _NodeScrInfoRef(nodeHandle, clrPtr);
		return Color::FromArgb(GetRValue(c), GetGValue(c), GetBValue(c));
	}

	void _SetScrInfoColor(int nodeHandle, int DSL_screenInfo::*clrPtr, Color color)
	{
		COLORREF c = RGB(color.R, color.G, color.B);
		_NodeScrInfoRef(nodeHandle, clrPtr) = c;
	}

	DSL_extraDefinition* GetExtraDef(int nodeHandle)
	{
		DSL_node * node = ValidateNodeHandle(nodeHandle);
		return node->ExtraDefinition();
	}

	DSL_nodeCost* GetCost(int nodeHandle)
	{
		DSL_node * node = ValidateNodeHandle(nodeHandle);
		return node->ObservCost();
	}

	void AddArcHelper(int parentHandle, int childHandle, dsl_arcType layer)
	{
		ValidateNodeHandle(parentHandle);
		ValidateNodeHandle(childHandle);
		int res = net->AddArc(parentHandle, childHandle, layer);
		if (DSL_OKAY != res)
		{
			String *msg = String::Format(
				"Can't add arc from {0} to {1}", 
				GetNodeId(parentHandle),
				GetNodeId(childHandle));
			throw new SmileException(msg, res);
		}
	}

	void DeleteArcHelper(int parentHandle, int childHandle, dsl_arcType layer)
	{
		ValidateNodeHandle(parentHandle);
		ValidateNodeHandle(childHandle);
		if (net->RemoveArc(parentHandle, childHandle, layer))
		{
			String *msg = String::Format(
				"Can't remove arc from {0} to {1}", 
				GetNodeId(parentHandle),
				GetNodeId(childHandle));
			throw new SmileException(msg);
		}
	}

	DSL_network *net;
};
//---------------------------------------------------

	
	
//---------------------------------------------------
// ValueOfInfo	
public __gc class ValueOfInfo : public WrappedObject, public IDisposable
{
public:
	ValueOfInfo(Network *net)
	{
		if (NULL == net)
		{
			throw new SmileException("Null network reference passed to ValueOfInfo constructor");
		}
	
		this->net = net;
		voi = new DSL_valueOfInformation(net->_GetDslNet());
	}

	~ValueOfInfo()
	{
		delete voi;
	}

	void Dispose()
	{
		delete voi;
		voi = NULL;
		GC::SuppressFinalize(this);
	}

	void Update()
	{
		int res = net->_GetDslNet()->ValueOfInformation(*voi);
		SmileException::CheckSmileStatus("Can't retrieve value of information", res);
	}

	Int32 GetAllDecisions()__gc[]
	{
		return CopyIntArray(voi->GetDecisions());
	}

	Int32 GetAllActions()__gc[]
	{
		return CopyIntArray(voi->GetActions());
	}

	String* GetAllDecisionIds()__gc[]
	{
		return net->HandlesToIds(voi->GetDecisions());
	}

	String* GetAllActionIds()__gc[]
	{
		return net->HandlesToIds(voi->GetActions());
	}

	Int32 GetAllNodes()__gc[]
	{
		return CopyIntArray(voi->GetNodes());
	}

	String* GetAllNodeIds()__gc[]
	{
		return net->HandlesToIds(voi->GetNodes());
	}


	void AddNode(int nodeHandle)
	{
		net->ValidateNodeHandle(nodeHandle);
		int res = voi->AddNode(nodeHandle);
		if (DSL_OKAY != res)
		{
			String *msg = String::Format("Can't add node to ValueOfInfo object, SMILE error code {0}", 
				Int32(res).ToString());
			throw new SmileException(msg);
		}
	}

	void AddNode(String *nodeId)
	{
		AddNode(net->ValidateNodeId(nodeId));
	}

	void RemoveNode(int nodeHandle)
	{
		net->ValidateNodeHandle(nodeHandle);
		int res = voi->RemoveNode(nodeHandle);
		if (DSL_OKAY != res)
		{
			String *msg = String::Format("Can't remove node from ValueOfInfo object, SMILE error code {0}", 
				Int32(res).ToString());
			throw new SmileException(msg);
		}
	}

	void RemoveNode(String *nodeId)
	{
		RemoveNode(net->ValidateNodeId(nodeId));
	}

	void SetDecision(int nodeHandle)
	{
		net->ValidateNodeHandle(nodeHandle);
		int res = voi->SetDecision(nodeHandle);
		if (DSL_OKAY != res)
		{
			String *msg = String::Format("Can't set decision on ValueOfInfo object, SMILE error code {0}", 
				Int32(res).ToString());
			throw new SmileException(msg);
		}
	}

	void SetDecision(String *nodeId)
	{
		SetDecision(net->ValidateNodeId(nodeId));
	}

	int GetDecision()
	{
		return voi->GetDecision();
	}

	String* GetDecisionId()
	{
		return new String(net->net->GetNode(voi->GetDecision())->Info().Header().GetId());
	}

	int GetPointOfView()
	{
		return voi->GetPointOfView();
	}

	String* GetPointOfViewId()
	{
		return new String(net->net->GetNode(voi->GetPointOfView())->Info().Header().GetId());
	}

	void SetPointOfView(int nodeHandle)
	{
		net->ValidateNodeHandle(nodeHandle);
		int res = voi->SetPointOfView(nodeHandle);
		if (DSL_OKAY != res)
		{
			String *msg = String::Format("Can't set point of view on ValueOfInfo object, SMILE error code {0}", 
				Int32(res).ToString());
			throw new SmileException(msg);
		}
	}

	void SetPointOfView(String *nodeId)
	{
		SetPointOfView(net->ValidateNodeId(nodeId));
	}

	Int32 GetIndexingNodes()__gc[]
	{
		return CopyIntArray(voi->GetIndexingNodes());
	}

	String* GetIndexingNodeIds()__gc[]
	{
		return net->HandlesToIds(voi->GetIndexingNodes());
	}
	
	Double GetValues()__gc[]
	{
		return CopyDoubleArray(voi->GetValues().GetItems());
	}

private:
	Network *net;
	DSL_valueOfInformation* voi;
};
//---------------------------------------------------


//---------------------------------------------------
// DiagNetwork
public __value struct FaultInfo : public IComparable
{
	int index;
	int node;
	int outcome;
	double probability;
	bool isPursued;

	int CompareTo(Object *obj)
	{
		FaultInfo *other = static_cast<FaultInfo __box*>(obj);

		double lhs = probability;
		double rhs = other->probability;

		if (DSL_NOT_RELEVANT == lhs) lhs = -1;
		if (DSL_NOT_RELEVANT == rhs) rhs = -1;
		
		if (lhs > rhs) return -1;
		if (lhs < rhs) return +1;

		if (isPursued && !other->isPursued) return -1;
		if (!isPursued && other->isPursued) return +1;

		if (node == other->node)
		{
			if (outcome < other->outcome) return -1;
			if (outcome > other->outcome) return +1;
		}
		
		return 0;
	}
};

public __value struct ObservationInfo : public IComparable
{
	int node;
	double entropy;
	double cost;
	double infoGain;

	int CompareTo(Object *obj)
	{
		ObservationInfo *other = static_cast<ObservationInfo __box*>(obj);
		
		int lhsCat = GetCompareCategory();
		int rhsCat = other->GetCompareCategory();
		
		if (lhsCat < rhsCat) return -1;
		if (lhsCat > rhsCat) return +1;

		if (0 == lhsCat)
		{
			if (infoGain > other->infoGain) return -1;
			if (infoGain < other->infoGain) return +1;
		}

		return 0;
	}

private:
	int GetCompareCategory()
	{
		if (_isnan(infoGain)) return 3;
		if (DSL_NOT_AVAILABLE == infoGain) return 2;
		if (DSL_NOT_RELEVANT == infoGain) return 1;

		return 0;
	}
};

public __gc class DiagResults
{
public:
	ObservationInfo observations __gc[];
	FaultInfo faults __gc[];
};


public __gc class DiagNetwork : public WrappedObject, public IDisposable
{
public:
	__value enum MultiFaultAlgorithmType
	{
		IndependenceAtLeastOne = DSL_DIAG_INDEPENDENCE | DSL_DIAG_PURSUE_ATLEAST_ONE_COMB,
		IndependenceOnlyOne = DSL_DIAG_INDEPENDENCE | DSL_DIAG_PURSUE_ONLY_ONE_COMB,
		IndependenceOnlyAll = DSL_DIAG_INDEPENDENCE | DSL_DIAG_PURSUE_ONLY_ALL_COMB,
		
		DependenceAtLeastOne = DSL_DIAG_DEPENDENCE | DSL_DIAG_PURSUE_ATLEAST_ONE_COMB,
		DependenceOnlyOne = DSL_DIAG_DEPENDENCE | DSL_DIAG_PURSUE_ONLY_ONE_COMB,
		DependenceOnlyAll = DSL_DIAG_DEPENDENCE | DSL_DIAG_PURSUE_ONLY_ALL_COMB,

		Marginal1 = DSL_DIAG_MARGINAL | DSL_DIAG_MARGINAL_STRENGTH1,
		Marginal2 = DSL_DIAG_MARGINAL | DSL_DIAG_MARGINAL_STRENGTH2,
	};

	static const double NotRelevant = DSL_NOT_RELEVANT;
	static const double NotAvailable = DSL_NOT_AVAILABLE;

	DiagNetwork(Network *net)
	{
		if (NULL == net)
		{
			throw new SmileException("Null network reference passed to DiagNetwork constructor");
		}

		multiFaultAlgorithm = IndependenceAtLeastOne;

		this->net = net;
		diagnet = new DIAG_network;

		diagnet->LinkToNetwork(net->_GetDslNet());
		diagnet->CollectNetworkInfo();
		diagnet->SetDefaultStates();
		
		diagnet->UpdateFaultBeliefs();
		int mostLikelyFault = diagnet->FindMostLikelyFault();
		diagnet->SetPursuedFault(mostLikelyFault);
	}

	~DiagNetwork() 
	{ 
		delete diagnet; 
	}

	void Dispose()
	{
		delete diagnet;
		diagnet = NULL;
		GC::SuppressFinalize(this);
	}

	__property MultiFaultAlgorithmType get_MultiFaultAlgorithm()
	{
		return multiFaultAlgorithm;
	}

	__property void set_MultiFaultAlgorithm(MultiFaultAlgorithmType value)
	{
		multiFaultAlgorithm = value;
	}

	/*
	__property bool get_QuickTests()
	{
		return diagnet->AreQuickTestsEnabled();
	}

	__property void set_QuickTests(bool value)
	{
		diagnet->EnableQuickTests(value);
	}
	*/

	__property bool get_DSep()
	{
		return diagnet->IsDSepEnabled();
	}

	__property void set_DSep(bool value)
	{
		diagnet->EnableDSep(value);
	}

	__property double get_EntropyCostRatio()
	{
		return diagnet->GetEntropyCostRatio();
	}

	__property void set_EntropyCostRatio(double value)
	{
		diagnet->SetEntropyCostRatio(value, 10 * value, &diagnet->GetNetwork());
	}


	void SetPursuedFault(int faultIndex)
	{
		CheckFaultIndex(faultIndex);
		int res = diagnet->SetPursuedFault(faultIndex);
		SmileException::CheckSmileStatus("Can't set pursued fault", res);
	}

	void SetPursuedFaults(Int32 faultIndices[])
	{
		DSL_intArray native;
		for (int i = 0; i < faultIndices->Length; i ++)
		{
			native.Add(faultIndices[i]);
		}

		int res = diagnet->SetPursuedFaults(native);
		SmileException::CheckSmileStatus("Can't set multiple pursued faults", res);
	}


	int GetPursuedFault() { return diagnet->GetPursuedFault(); }
	
	Int32 GetPursuedFaults() __gc[]
	{
		return CopyIntArray(diagnet->GetPursuedFaults());
	}
	
	int FindMostLikelyFault() { return diagnet->FindMostLikelyFault(); }

	__property int get_FaultCount() { return diagnet->GetFaults().size(); }
	
	FaultInfo GetFault(int faultIndex)
	{
		CheckFaultIndex(faultIndex);
		FaultInfo fi;
		const DIAG_faultyState &nativeFaultInfo = diagnet->GetFaults()[faultIndex];
		fi.index = faultIndex;
		fi.node = nativeFaultInfo.node;
		fi.outcome = nativeFaultInfo.state;
		fi.isPursued = 
			(DSL_FALSE != const_cast<DSL_intArray &>(diagnet->GetPursuedFaults()).IsInList(faultIndex));

		DSL_nodeValue *value = diagnet->GetNetwork().GetNode(fi.node)->Value();
		if (value->IsValueValid())
		{
			fi.probability = value->GetMatrix()->Subscript(fi.outcome);
		}
		else
		{
			fi.probability = 0;
		}

		return fi;
	}

	int GetFaultNode(int faultIndex) { CheckFaultIndex(faultIndex); return diagnet->GetFaults()[faultIndex].node; }
	String* GetFaultNodeId(int faultIndex) { return net->GetNodeId(GetFaultNode(faultIndex)); }

	int GetFaultOutcome(int faultIndex) { CheckFaultIndex(faultIndex); return diagnet->GetFaults()[faultIndex].state; }
	String* GetFaultOutcomeId(int faultIndex)
	{
		return net->GetOutcomeId(GetFaultNode(faultIndex), diagnet->GetFaults()[faultIndex].state);
	}

	int GetFaultIndex(int nodeHandle, int outcomeIndex)
	{
		return diagnet->FindFault(nodeHandle, outcomeIndex);
	}
	
	int GetFaultIndex(String *nodeId, int outcomeIndex)
	{
		return GetFaultIndex(net->ValidateNodeId(nodeId), outcomeIndex);
	}
	
	int GetFaultIndex(int nodeHandle, String *outcomeId)
	{
		return GetFaultIndex(nodeHandle, net->ValidateOutcomeId(nodeHandle, outcomeId));
	}
	
	int GetFaultIndex(String *nodeId, String *outcomeId)
	{
		return GetFaultIndex(net->ValidateNodeId(nodeId), outcomeId);
	}

	DiagResults* Update()
	{
		DiagResults* results = new DiagResults();

		diagnet->UpdateFaultBeliefs();
		int retCode = diagnet->ComputeTestStrengths(multiFaultAlgorithm);
		SmileException::CheckSmileStatus("Can't compute test strengths", retCode);

		DSL_intArray &unperformed = diagnet->GetUnperformedTests();
		int imax = unperformed.NumItems();
		ObservationInfo ti[] = new ObservationInfo[imax];

		const vector<DIAG_testInfo> &stats = diagnet->GetTestStatistics();
		
		for (int i = 0; i < imax; i ++)
		{
			int handle = unperformed[i];
			const DIAG_testInfo &st = stats[i];
			
			ti[i].node = handle;
			ti[i].infoGain = st.strength;
			ti[i].entropy = st.entropy;
			ti[i].cost = st.cost;
		}

		results->observations = ti;


		const vector<DIAG_faultyState> &faults = diagnet->GetFaults();
		int faultCount = faults.size();
		
		FaultInfo fi[] = new FaultInfo[faultCount];
		for (int i = 0; i < faultCount; i ++)
		{
			double p = NotRelevant;
			const DIAG_faultyState fs = faults[i];
			DSL_nodeValue *value = diagnet->GetNetwork().GetNode(fs.node)->Value();
			if (value->IsValueValid())
			{
				p = value->GetMatrix()->Subscript(fs.state);
			}
			
			fi[i].index = i;
			fi[i].node = fs.node;
			fi[i].outcome = fs.state;
			fi[i].isPursued = false;
			fi[i].probability = p;
		}

		const DSL_intArray &pursuedFaults = diagnet->GetPursuedFaults();
		for (int i = 0; i < pursuedFaults.NumItems(); i ++)
		{
			fi[pursuedFaults[i]].isPursued = true;
		}

		results->faults = fi;

		Array::Sort(results->observations);
		Array::Sort(results->faults);

		return results;
	}

	

	__property int get_UnperformedTestCount() 
	{ 
		return diagnet->GetUnperformedTests().NumItems(); 
	}

	Int32 GetUnperformedObservations() __gc[]
	{
		return CopyIntArray(diagnet->GetUnperformedTests());
	}

	String* GetUnperformedObservationIds() __gc[]
	{
		return net->HandlesToIds(diagnet->GetUnperformedTests());
	}

	bool MandatoriesInstantiated() { return diagnet->MandatoriesInstantiated(); }

	void InstantiateObservation(int nodeHandle, int outcomeIndex)
	{
		net->ValidateNodeHandle(nodeHandle);
		net->ValidateOutcomeIndex(nodeHandle, outcomeIndex);
		int res = diagnet->InstantiateObservation(nodeHandle, outcomeIndex);
		SmileException::CheckSmileStatus("Can't instantiate observation", res);
	}

	void InstantiateObservation(String *nodeId, int outcomeIndex)
	{
		InstantiateObservation(net->ValidateNodeId(nodeId), outcomeIndex);
	}

	void InstantiateObservation(int nodeHandle, String *outcomeId)
	{
		InstantiateObservation(nodeHandle, net->ValidateOutcomeId(nodeHandle, outcomeId));
	}

	void InstantiateObservation(String *nodeId, String *outcomeId)
	{
		InstantiateObservation(net->ValidateNodeId(nodeId), outcomeId);
	}

	void ReleaseObservation(int nodeHandle)
	{
		net->ValidateNodeHandle(nodeHandle);
		int res = diagnet->ReleaseObservation(nodeHandle);
		SmileException::CheckSmileStatus("Can't release observation", res);
	}

	void ReleaseObservation(String *nodeId)
	{
		ReleaseObservation(net->ValidateNodeId(nodeId));
	}


private:
	void CheckFaultIndex(int faultIndex)
	{
		int maxIndex = diagnet->GetFaults().size();
		if (faultIndex < 0 || faultIndex >= maxIndex)
		{
			String *msg = String::Format("Invalid fault index {0}, must be between 0 and {1}", 
				faultIndex.ToString(),
				maxIndex.ToString());
			throw new SmileException(msg);
		}
	}

	Network *net;
	DIAG_network *diagnet;
	MultiFaultAlgorithmType multiFaultAlgorithm;
};

//---------------------------------------------------


namespace Learning
{

public __gc class DataMatch
{
public:
	int column;
	int node;
	int slice;
};

public __gc class DataSet : public WrappedObject, public IDisposable
{
public:
	DataSet() { dsx = new DSL_dataset; }
	~DataSet() { delete dsx; }
	
	void Dispose()
	{
		delete dsx;
		dsx = NULL;
		GC::SuppressFinalize(this);
	}

	static const int DefaultMissingInt = DSL_MISSING_INT;
	static const float DefaultMissingFloat = DSL_MISSING_FLOAT;

    void ReadFile(String *filename)
    {
        ReadFile(filename, NULL);
    }

    void ReadFile(String *filename, String *missingValueToken)
    {
        ReadFile(filename, missingValueToken, DefaultMissingInt, DefaultMissingFloat, true);
    }

    void ReadFile(String *filename, String *missingValueToken, int missingInt, float missingFloat, bool columnIdsPresent)
    {
        DSL_datasetParseParams params;
        if (NULL != missingValueToken && missingValueToken->Length > 0)
        {
            StringToCharPtr szToken(missingValueToken);
            params.missingValueToken = szToken;
        }
        params.missingInt = missingInt;
        params.missingFloat = missingFloat;
		params.columnIdsPresent = columnIdsPresent;

        StringToCharPtr szFile(filename);
        std::string parseError;
        if (DSL_OKAY != dsx->ReadFile(std::string(szFile), &params, &parseError))
        {
            throw new SmileException(String::Format("ReadFile failed - {0}", new String(parseError.c_str())));
        }
    }

	DataMatch* MatchNetwork(Network *net)__gc[]
	{
		vector<DSL_datasetMatch> nativeMatching;
		string errMsg;
		if (DSL_OKAY != dsx->MatchNetwork(*net->_GetDslNet(), nativeMatching, errMsg))
		{
			throw new SmileException(String::Format("MatchNetwork failed - {0}", new String(errMsg.c_str())));
		}

		int count = nativeMatching.size();
		if (count == 0)
		{
			return NULL;
		}

		DataMatch* ar[] = new DataMatch*[count];
		for (int i = 0; i < count; i ++)
		{
			const DSL_datasetMatch &nm = nativeMatching[i];
			DataMatch *m = new DataMatch;
			m->column = nm.column;
			m->node = nm.node;
			m->slice = nm.slice;
			ar[i] = m;
		}

		return ar;
	}

	__property int get_RecordCount() { return _GetDslDataSet()->GetNumberOfRecords(); }
	__property int get_VariableCount() { return _GetDslDataSet()->GetNumberOfVariables(); }

	int GetInt(int variable, int record)
	{
		ValidateVariableRecord(variable, record);
		return _GetDslDataSet()->GetInt(variable, record);
	}

	void SetInt(int variable, int record, int value)
	{
		ValidateVariableRecord(variable, record);
		_GetDslDataSet()->SetInt(variable, record, value);
	}

	float GetFloat(int variable, int record)
	{
		ValidateVariableRecord(variable, record);
		return _GetDslDataSet()->GetFloat(variable, record);
	}

	void SetFloat(int variable, int record, float value)
	{
		ValidateVariableRecord(variable, record);
		_GetDslDataSet()->SetFloat(variable, record, value);
	}

	void AddIntVariable(String *id)
	{
		AddIntVariable(id, DefaultMissingInt);
	}

	void AddIntVariable(String *id, int missingValue)
	{
		StringToCharPtr szId(id);
		int res = _GetDslDataSet()->AddIntVar(string(szId), missingValue);
		SmileException::CheckSmileStatus("AddIntVariable failed", res);
	}

	void AddFloatVariable(String *id)
	{
		AddFloatVariable(id, DefaultMissingFloat);
	}

	void AddFloatVariable(String *id, float missingValue)
	{
		StringToCharPtr szId(id);
		int res = _GetDslDataSet()->AddFloatVar(string(szId), missingValue);
		SmileException::CheckSmileStatus("AddFloatVariable failed", res);
	}

	void AddEmptyRecord()
	{
		DSL_dataset *ds = _GetDslDataSet();
		ds->AddEmptyRecord();
	}

	String* GetVariableId(int variable)
	{
		return new String(_GetDslDataSet()->GetId(variable).c_str());
	}

	String* GetStateNames(int variable)__gc[]
	{
		ValidateVariable(variable);

		const vector<string> & native = _GetDslDataSet()->GetStateNames(variable);
		int count = native.size();
		
		String* ar[] = new String*[count];
		for (int i = 0; i < count; i ++)
		{
			ar[i] = new String(native[i].c_str());
		}
		return ar;		
	}

	void SetStateNames(int variable, String* names[])
	{
		ValidateVariable(variable);

		int count = names->Length;
		vector<string> native(count);

		for (int i = 0; i < count; i ++)
		{
			StringToCharPtr p(names[i]);
			native[i] = p;
		}

		int res = _GetDslDataSet()->SetStateNames(variable, native);
		SmileException::CheckSmileStatus("SetStateNames failed", res);
	}

private public:
	DSL_dataset* _GetDslDataSet()
	{
		return dsx;
	}

private:
	void ValidateVariable(int variable)
	{
		if (variable < 0)
		{
			throw new SmileException("Negative variable index");
		}

		int varCount = _GetDslDataSet()->GetNumberOfVariables();
		if (variable >= varCount)
		{
			if (0 == varCount)
			{
				throw new SmileException("DataSet object has no variables");
			}
			else
			{
				String *msg = String::Format("Invalid variable index {0}, valid range is 0..{1}", 
					variable.ToString(), int(varCount - 1).ToString());
				throw new SmileException(msg);
			}
		}
	}

	void ValidateVariableRecord(int variable, int record)
	{
		ValidateVariable(variable);

		if (record < 0)
		{
			throw new SmileException("Negative record index");
		}

		int recCount = _GetDslDataSet()->GetNumberOfRecords();
		if (record >= recCount)
		{
			if (0 == recCount)
			{
				throw new SmileException("DataSet object has no records");
			}
			else
			{
				String *msg = String::Format("Invalid record index {0}, valid range is 0..{1}", 
					record.ToString(), int(recCount - 1).ToString());
				throw new SmileException(msg);
			}
		}
	}

	DSL_dataset *dsx;
};

//---------------------------------------------------

public __value struct BkArcInfo
{
	int parent;
	int child;
};

public __value struct BkTierInfo
{
	int variable;
	int tier;
};

public __gc class BkKnowledge
{
public:
	BkKnowledge() {}

	BkArcInfo forcedArcs[];
	BkArcInfo forbiddenArcs[];
	BkTierInfo tiers[];

private public:
	typedef pair<int, int> intPair;
	typedef vector<intPair> intPairVec;
	BkKnowledge(const intPairVec& nativeForced, const intPairVec &nativeForbidden, const intPairVec &nativeTiers)
	{
		InitArcs(nativeForced, forcedArcs);
		InitArcs(nativeForbidden, forbiddenArcs);

		int count = nativeTiers.size();
		tiers = new BkTierInfo[count];
		for (int i = 0; i < count; i ++)
		{
			tiers[i].variable = nativeTiers[i].first;
			tiers[i].tier = nativeTiers[i].second;
		}
	}

	void Copy(intPairVec& nativeForced, intPairVec &nativeForbidden, intPairVec &nativeTiers)
	{
		CopyArcs(nativeForced, forcedArcs);
		CopyArcs(nativeForbidden, forbiddenArcs);

		if (NULL != tiers)
		{
			int count = tiers->Length;
			nativeTiers.resize(count);
			for (int i = 0; i < count; i ++)
			{
				nativeTiers[i].first = tiers[i].variable;
				nativeTiers[i].second = tiers[i].tier;
			}
		}
	}

private:
	static void InitArcs(const intPairVec& native, BkArcInfo arcs[])
	{
		int count = native.size();
		arcs = new BkArcInfo[count];
		for (int i = 0; i < count; i ++)
		{
			arcs[i].parent = native[i].first;
			arcs[i].child = native[i].second;
		}
	}

	static void CopyArcs(intPairVec& native, BkArcInfo arcs[])
	{
		if (NULL != arcs)
		{
			int count = arcs->Length;
			native.resize(count);
			for (int i = 0; i < count; i ++)
			{
				native[i].first = arcs[i].parent;
				native[i].second = arcs[i].child;
			}
		}
	}
};
	
//---------------------------------------------------

public __gc class Pattern : public WrappedObject, public IDisposable
{
public:

	__value enum EdgeType
	{
		None = DSL_pattern::None,
		Undirected = DSL_pattern::Undirected,
		Directed = DSL_pattern::Directed
	};


	Pattern() { pat = new DSL_pattern; }
	~Pattern() { delete pat; }
	void Dispose()
	{
		delete pat;
		pat = NULL;
		GC::SuppressFinalize(this);
	}

	int GetSize() { return pat->GetSize(); }
	void SetSize(int size) { pat->SetSize(size); }
	EdgeType GetEdge(int from, int to) { return static_cast<EdgeType>(pat->GetEdge(from, to)); }
	void SetEdge(int from, int to, EdgeType type) { pat->SetEdge(from, to, static_cast<DSL_pattern::EdgeType>(type)); }
	bool HasCycle() { return pat->HasCycle(); }
	bool IsDAG() { return pat->IsDAG(); }
	
	Network* MakeNetwork(DataSet *ds)
	{
		Network *net = new Network();

		if (!pat->ToDAG(*ds->_GetDslDataSet(), *net->_GetDslNet()))
		{
			net->Dispose();
			net = NULL;
			throw new SmileException("Can't convert pattern to network", -1);
		}

		return net;
	}


private public:
	DSL_pattern* _GetDslPattern()
	{
		return pat;
	}

private:
	DSL_pattern *pat;
};

public __gc class GreedyThickThinning : public WrappedObject, public IDisposable
{
public:
	GreedyThickThinning() { gtt = new DSL_greedyThickThinning; }
	~GreedyThickThinning() { delete gtt; }
	void Dispose()
	{
		delete gtt;
		gtt = NULL;
		GC::SuppressFinalize(this);
	}

	Network* Learn(DataSet *ds)
	{
		Network* net = new Network();	
		int res = gtt->Learn(*ds->_GetDslDataSet(), *net->_GetDslNet());
		if (DSL_OKAY != res)
		{
			net->Dispose();
			net = NULL;
			throw new SmileException("Error in GreedyThickThinning algorithm", res);
		}

		return net;
	}

	__value enum PriorsType
	{
		K2 = DSL_greedyThickThinning::K2,
		BDeu = DSL_greedyThickThinning::BDeu
	};

	__property PriorsType get_PriorsMethod() { return PriorsType(gtt->priors); }
	__property void set_PriorsMethod(PriorsType value) { gtt->priors = DSL_greedyThickThinning::PriorsType(value); }

	__property double get_NetWeight() { return gtt->netWeight; }
	__property void set_NetWeight(double value)	{ gtt->netWeight = value; }

	__property int get_MaxParents() { return gtt->maxParents; }
	__property void set_MaxParents(int value) { gtt->maxParents = value; }

	BkKnowledge* GetBkKnowledge() 
	{ 
		return new BkKnowledge(gtt->bkk.forcedArcs, gtt->bkk.forbiddenArcs, gtt->bkk.tiers);
	}

	void SetBkKnowledge(BkKnowledge *bkk)
	{
		bkk->Copy(gtt->bkk.forcedArcs, gtt->bkk.forbiddenArcs, gtt->bkk.tiers);
	}

private:
	DSL_greedyThickThinning *gtt;
};

//---------------------------------------------------

public __gc class BS : public WrappedObject, public IDisposable
{
public:
	BS() { bs = new DSL_bs; }
	~BS() { delete bs; }
	void Dispose()
	{
		delete bs;
		bs = NULL;
		GC::SuppressFinalize(this);
	}

	Network* Learn(DataSet *ds)
	{
		Network* net = new Network();	
		int res = bs->Learn(*ds->_GetDslDataSet(), *net->_GetDslNet());
		if (DSL_OKAY != res)
		{
			net->Dispose();
			net = NULL;
			throw new SmileException("Error in Bayesian Search algorithm", res);
		}

		return net;
	}

	__property int get_MaxParents() { return bs->maxParents; }
	__property void set_MaxParents(int value) { bs->maxParents = value; }

	__property int get_MaxSearchTime() { return bs->maxSearchTime; }
	__property void set_MaxSearchTime(int value) { bs->maxSearchTime = value; }

	__property int get_nrIteration() { return bs->nrIteration; }
	__property void set_nrIteration(int value) { bs->nrIteration = value; }

	__property double get_linkProbability() { return bs->linkProbability; }
	__property void set_linkProbability(double value) { bs->linkProbability = value; }

	__property double get_priorLinkProbability() { return bs->priorLinkProbability; }
	__property void set_priorLinkProbability(double value) { bs->priorLinkProbability = value; }

	__property int get_priorSampleSize() { return bs->priorSampleSize; }
	__property void set_priorSampleSize(int value) { bs->priorSampleSize = value; }

	__property int get_Seed() { return bs->seed; }
	__property void set_Seed(int value) { bs->seed = value; }

	BkKnowledge* GetBkKnowledge() 
	{ 
		return new BkKnowledge(bs->bkk.forcedArcs, bs->bkk.forbiddenArcs, bs->bkk.tiers);
	}

	void SetBkKnowledge(BkKnowledge *bkk)
	{
		bkk->Copy(bs->bkk.forcedArcs, bs->bkk.forbiddenArcs, bs->bkk.tiers);
	}

private:
	DSL_bs *bs;
};

//---------------------------------------------------

public __gc class PC : public WrappedObject, public IDisposable
{
public:
	PC() { pc = new DSL_pc;	}
	~PC() { delete pc; }
	void Dispose()
	{
		delete pc;
		pc = NULL;
		GC::SuppressFinalize(this);
	}

	Pattern* Learn(DataSet *ds)
	{
		Pattern* pat = new Pattern();	

		int res = pc->Learn(*ds->_GetDslDataSet(), *pat->_GetDslPattern());
		if (DSL_OKAY != res)
		{
			pat->Dispose();
			pat = NULL;
			throw new SmileException("Error in PC algorithm", res);
		}

		return pat;
	}

	__property int get_MaxAdjacency() { return pc->maxAdjacency; }
	__property void set_MaxAdjacency(int value) { pc->maxAdjacency = value; }

	__property double get_Significance() { return pc->significance; }
	__property void set_Significance(double value) { pc->significance = value; }

	BkKnowledge* GetBkKnowledge() 
	{ 
		return new BkKnowledge(pc->bkk.forcedArcs, pc->bkk.forbiddenArcs, pc->bkk.tiers);
	}

	void SetBkKnowledge(BkKnowledge *bkk)
	{
		bkk->Copy(pc->bkk.forcedArcs, pc->bkk.forbiddenArcs, pc->bkk.tiers);
	}

private:
	DSL_pc *pc;
};

//---------------------------------------------------

public __gc class NaiveBayes : public WrappedObject, public IDisposable
{
public:
	NaiveBayes() { nb = new DSL_naiveBayes; }
	~NaiveBayes() { delete nb; }
	void Dispose()
	{
		delete nb;
		nb = NULL;
		GC::SuppressFinalize(this);
	}

	Network* Learn(DataSet *ds)
	{
		Network* net = new Network();	
		int res = nb->Learn(*ds->_GetDslDataSet(), *net->_GetDslNet());
		if (DSL_OKAY != res)
		{
			net->Dispose();
			net = NULL;
			throw new SmileException("Error in NaiveBayes algorithm", res);
		}

		return net;
	}

	__value enum PriorsType
	{
		K2 = DSL_naiveBayes::K2,
		BDeu = DSL_naiveBayes::BDeu,
	};

	__property PriorsType get_PriorsMethod() { return PriorsType(nb->priors); }
	__property void set_PriorsMethod(PriorsType value) { nb->priors = DSL_naiveBayes::PriorsType(value); }

	__property double get_NetWeight() { return nb->netWeight; }
	__property void set_NetWeight(double value) { nb->netWeight = value; }

	__property bool get_FeatureSelection() { return nb->featureSelection; }
	__property void set_FeatureSelection(bool value) { nb->featureSelection = value; }

	__property String* get_ClassVariableId() { return new String(nb->classVariableId.c_str()); }
	__property void set_ClassVariableId(String *value)
	{
		StringToCharPtr c(value);
		nb->classVariableId = c;
	}

private:
	DSL_naiveBayes *nb;
};

//---------------------------------------------------

public __gc class EM : public WrappedObject, public IDisposable
{
public:
	EM() { em = new DSL_em;	}
	~EM() { delete em; }
	void Dispose()
	{
		delete em;
		em = NULL;
		GC::SuppressFinalize(this);
	}

	void Learn(DataSet *ds, Network *net, DataMatch* matching[], Int32 fixedNodes[])
	{
		if (NULL == matching || 0 == matching->Length)
		{
			throw new SmileException("No matching specified");
		}
	
		int count = matching->Length;
		vector<DSL_datasetMatch> nativeMatching(count);
		for (int i = 0; i < count; i ++) 
		{
			DSL_datasetMatch &nm = nativeMatching[i];
			nm.column = matching[i]->column;
			nm.node = matching[i]->node;
			nm.slice = matching[i]->slice;
		}

		vector<int> nativeFixedNodes;
		if (NULL != fixedNodes)
		{
			count = fixedNodes->Length;
			nativeFixedNodes.resize(count);
			for (int i = 0; i < count; i ++) nativeFixedNodes[i] = fixedNodes[i];
		}

		SmileException::CheckSmileStatus("Error in EM algorithm", em->Learn(*ds->_GetDslDataSet(), *net->_GetDslNet(), nativeMatching, nativeFixedNodes));
	}

	void Learn(DataSet *ds, Network *net, DataMatch* matching[])
	{
		Learn(ds, net, matching, (Int32[])NULL);
	}


	void Learn(DataSet *ds, Network *net, DataMatch *matching[], String *fixedNodes[])
	{
		int count = fixedNodes->Length;
		Int32 fixedNodeHandles[] = new Int32[count];
		for (int i = 0; i < count; i ++)
		{
			fixedNodeHandles[i] = net->ValidateNodeId(fixedNodes[i]);
		}
		Learn(ds, net, matching, fixedNodeHandles);
	}

	__property int get_EqSampleSize() { return em->GetEquivalentSampleSize(); }
	__property void set_EqSampleSize(int value) { em->SetEquivalentSampleSize(value); }

	__property bool get_RandomizeParameters() { return em->GetRandomizeParameters(); }
	__property void set_RandomizeParameters(bool value) { em->SetRandomizeParameters(value); }

    __property bool get_Relevance() { return em->GetRelevance(); }
	__property void set_Relevance(bool value) { em->SetRelevance(value); }

	__property int get_Seed() { return em->GetSeed(); }
	__property void set_Seed(int value) { em->SetSeed(value); }

    __property bool get_UniformizeParameters() { return em->GetUniformizeParameters(); }
	__property void set_UniformizeParameters(bool value) { em->SetUniformizeParameters(value); }

private:
	DSL_em *em;
};
} // end namespace Learning

} // end namespace Smile
