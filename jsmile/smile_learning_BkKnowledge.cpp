// smile_learning_EM.cpp

#include "jsmile.h"

using namespace std;

namespace
{
	void ValidateVarIndex(const DSL_bkgndKnowledge *bkk, jint var)
	{
		int varCount = int(bkk->vars.size());
		if (var < 0 || var >= varCount)
		{
			string msg = "Invalid variable index: ";
			DSL_appendInt(msg, var);
			throw invalid_argument(msg);
		}
	}

	jint ValidateVarId(JNIEnv *env, jobject obj, jstring id)
	{
		const DSL_bkgndKnowledge *bkk = static_cast<const DSL_bkgndKnowledge *>(GetNativePtr(env, obj));
		string nativeId = string(NativeString(env, id));
		int idx = bkk->FindVariable(nativeId);
		if (idx < 0)
		{
			string msg = "Invalid variable id: ";
			msg += nativeId;
			throw invalid_argument(msg);
		}
		return idx;
	}

	void DoAddArc(JNIEnv *env, jobject obj, DSL_bkgndKnowledge::IntPairVector DSL_bkgndKnowledge::* pv, int from, int to)
	{
		DSL_bkgndKnowledge *bkk = static_cast<DSL_bkgndKnowledge *>(GetNativePtr(env, obj));
		int varCount = int(bkk->vars.size());
		if (from < 0 || from >= varCount)
		{
			string msg = "Invalid variable index for arc start: ";
			DSL_appendInt(msg, from);
			throw invalid_argument(msg);
		}
		
		if (to < 0 || to >= varCount)
		{
			string msg = "Invalid variable index for arc end: ";
			DSL_appendInt(msg, to);
			throw invalid_argument(msg);
		}

		DSL_bkgndKnowledge::IntPairVector &v = bkk->*pv;
		int arcCount = int(v.size());
		for (int i = 0; i < arcCount; i ++)
		{
			if (v[i].first == from && v[i].second == to)
			{
				string msg = "Arc between variables ";
				DSL_appendInt(msg, from);
				msg += " and ";
				DSL_appendInt(msg, to);
				msg += " already exists.";
				throw invalid_argument(msg);
			}
		}

		v.push_back(make_pair(from, to));
	}

	jintArray DoGetChildren(JNIEnv *env, jobject obj, DSL_bkgndKnowledge::IntPairVector DSL_bkgndKnowledge::* pv, int var)
	{
		const DSL_bkgndKnowledge *bkk = static_cast<const DSL_bkgndKnowledge *>(GetNativePtr(env, obj));
		ValidateVarIndex(bkk, var);
		vector<int> children;
		children.reserve(8);

		const DSL_bkgndKnowledge::IntPairVector &v = bkk->*pv;
		int arcCount = int(v.size());
		for (int i = 0; i < arcCount; i ++)
		{
			if (v[i].first == var)
			{
				children.push_back(v[i].second);
			}
		}
		int childCount = int(children.size());

		jintArray a = env->NewIntArray(childCount);
		env->SetIntArrayRegion(a, 0, childCount, (jint *)&children.front());

		return a;
	}
}

extern "C"
{
JSMILE_IMPLEMENT_WRAPPER(learning_BkKnowledge, DSL_bkgndKnowledge);

JNIEXPORT void JNICALL Java_smile_learning_BkKnowledge_readFile(JNIEnv *env, jobject obj, jstring filename)
{
	JSMILE_ENTER;
    DSL_bkgndKnowledge *bkk = GetPtr(env, obj);
	string errMsg;
    int res = bkk->ReadFile(string(NativeString(env, filename)), &errMsg);
    if (DSL_OKAY != res) 
	{
		ThrowSmileException(env, runtime_error(errMsg));
	}
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_learning_BkKnowledge_writeFile(JNIEnv *env, jobject obj, jstring filename)
{
	JSMILE_ENTER;
    DSL_bkgndKnowledge *bkk = GetPtr(env, obj);
	string errMsg;
    int res = bkk->WriteFile(string(NativeString(env, filename)), &errMsg);
    if (DSL_OKAY != res) 
	{
		ThrowSmileException(env, runtime_error(errMsg));
	}
    JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_learning_BkKnowledge_matchData(JNIEnv *env, jobject obj, jobject dataset)
{
	JSMILE_ENTER;
    DSL_bkgndKnowledge *bkk = GetPtr(env, obj);
	const DSL_dataset *nativeDs = static_cast<DSL_dataset *>(GetNativePtr(env, dataset));
	string errMsg;
    int res = bkk->MatchData(*nativeDs, &errMsg);
    if (DSL_OKAY != res) 
	{
		ThrowSmileException(env, runtime_error(errMsg));
	}
    JSMILE_LEAVE;

}

JNIEXPORT jstring JNICALL Java_smile_learning_BkKnowledge_getVariableId(JNIEnv *env, jobject obj, jint var)
{
	JSMILE_ENTER;
    const DSL_bkgndKnowledge *bkk = GetPtr(env, obj);
	ValidateVarIndex(bkk, var);
	JSMILE_RETURN(env->NewStringUTF(bkk->vars[var].id.c_str()));
}


JNIEXPORT jint JNICALL Java_smile_learning_BkKnowledge_findVariable(JNIEnv *env, jobject obj, jstring id)
{
	JSMILE_ENTER;
	JSMILE_RETURN(GetPtr(env, obj)->FindVariable(string(NativeString(env, id))));
}


JNIEXPORT void JNICALL Java_smile_learning_BkKnowledge_addVariable(JNIEnv *env, jobject obj, jstring id)
{
}

JNIEXPORT jint JNICALL Java_smile_learning_BkKnowledge_getVariableCount(JNIEnv *env, jobject obj)
{
	JSMILE_ENTER;
	JSMILE_RETURN(jint(GetPtr(env, obj)->vars.size()));
}

JNIEXPORT jint JNICALL Java_smile_learning_BkKnowledge_getTier__I(JNIEnv *env, jobject obj, jint var)
{
	JSMILE_ENTER;
    const DSL_bkgndKnowledge *bkk = GetPtr(env, obj);
	ValidateVarIndex(bkk, var);
	JSMILE_RETURN(bkk->GetTier(var));
}

JNIEXPORT jint JNICALL Java_smile_learning_BkKnowledge_getTier__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring var)
{
	JSMILE_ENTER;
	JSMILE_RETURN(Java_smile_learning_BkKnowledge_getTier__I(env, obj, ValidateVarId(env, obj, var)));
}

JNIEXPORT void JNICALL Java_smile_learning_BkKnowledge_setTier__II(JNIEnv *env, jobject obj, jint var, jint tier)
{
	JSMILE_ENTER;

	DSL_bkgndKnowledge *bkk = GetPtr(env, obj);
	ValidateVarIndex(bkk, var);

	DSL_bkgndKnowledge::IntPairVector &v = bkk->tiers;
	int count = int(v.size());
	bool found = false;
	for (int i = 0; i < count; i ++)
	{
		if (v[i].first == var) 
		{
			v[i].second = tier;
			found = true;
			break;
		}
	}

	if (!found)
	{
		v.push_back(make_pair(var, tier));
	}

	JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_learning_BkKnowledge_setTier__Ljava_lang_String_2I(JNIEnv *env, jobject obj, jstring var, jint tier)
{
	JSMILE_ENTER;
	Java_smile_learning_BkKnowledge_setTier__II(env, obj, ValidateVarId(env, obj, var), tier);
	JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_learning_BkKnowledge_addForcedArc__II(JNIEnv *env, jobject obj, jint from, jint to)
{
	JSMILE_ENTER;
	DoAddArc(env, obj, &DSL_bkgndKnowledge::forcedArcs, from, to);
	JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_learning_BkKnowledge_addForcedArc__Ljava_lang_String_2Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring from, jstring to)
{
	JSMILE_ENTER;
	DoAddArc(env, obj, &DSL_bkgndKnowledge::forcedArcs, 
		ValidateVarId(env, obj, from),
		ValidateVarId(env, obj, to)); 
	JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_learning_BkKnowledge_addForbiddenArc__II(JNIEnv *env, jobject obj, jint from, jint to)
{
	JSMILE_ENTER;
	DoAddArc(env, obj, &DSL_bkgndKnowledge::forbiddenArcs, from, to);
	JSMILE_LEAVE;
}

JNIEXPORT void JNICALL Java_smile_learning_BkKnowledge_addForbiddenArc__Ljava_lang_String_2Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring from, jstring to)
{
	JSMILE_ENTER;
	DoAddArc(env, obj, &DSL_bkgndKnowledge::forbiddenArcs, 
		ValidateVarId(env, obj, from),
		ValidateVarId(env, obj, to)); 
	JSMILE_LEAVE; 
}


JNIEXPORT jintArray JNICALL Java_smile_learning_BkKnowledge_getForbiddenChildren(JNIEnv *env, jobject obj, jint var)
{
	JSMILE_ENTER;
	JSMILE_RETURN(DoGetChildren(env, obj, &DSL_bkgndKnowledge::forbiddenArcs, var));
}

JNIEXPORT jintArray JNICALL Java_smile_learning_BkKnowledge_getForcedChildren(JNIEnv *env, jobject obj, jint var)
{
	JSMILE_ENTER;
	JSMILE_RETURN(DoGetChildren(env, obj, &DSL_bkgndKnowledge::forcedArcs, var));
}

} // end extern "C"


