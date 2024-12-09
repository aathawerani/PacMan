// smile_learning_PC.cpp

#include "jsmile.h"

extern "C"
{
JSMILE_IMPLEMENT_WRAPPER(learning_Pattern, DSL_pattern)

//-------------------------------------------------------------------------------

JNIEXPORT jint JNICALL Java_smile_learning_Pattern_getSize(JNIEnv *env, jobject obj)
{
	JSMILE_ENTER;
    JSMILE_RETURN(GetPtr(env, obj)->GetSize());
}

JNIEXPORT void JNICALL Java_smile_learning_Pattern_setSize(JNIEnv *env, jobject obj, jint size)
{
	JSMILE_ENTER;
	GetPtr(env, obj)->SetSize(size);
	JSMILE_LEAVE;
}

//-------------------------------------------------------------------------------

JNIEXPORT jint JNICALL Java_smile_learning_Pattern_getEdge(JNIEnv *env, jobject obj, jint from, jint to)
{
	JSMILE_ENTER;
	JSMILE_RETURN(GetPtr(env, obj)->GetEdge(from, to));
}

JNIEXPORT void JNICALL Java_smile_learning_Pattern_setEdge(JNIEnv *env, jobject obj, jint from, jint to, jint type)
{
	JSMILE_ENTER;
	GetPtr(env, obj)->SetEdge(from, to, static_cast<DSL_pattern::EdgeType>(type));
	JSMILE_LEAVE;
}

//-------------------------------------------------------------------------------

JNIEXPORT jboolean JNICALL Java_smile_learning_Pattern_hasCycle(JNIEnv *env, jobject obj)
{
	JSMILE_ENTER;
	JSMILE_RETURN(GetPtr(env, obj)->HasCycle());
}

JNIEXPORT jboolean JNICALL Java_smile_learning_Pattern_isDAG(JNIEnv *env, jobject obj)
{
	JSMILE_ENTER;
	JSMILE_RETURN(GetPtr(env, obj)->IsDAG());
}

//-------------------------------------------------------------------------------

JNIEXPORT jobject JNICALL Java_smile_learning_Pattern_makeNetwork(JNIEnv *env, jobject obj, jobject jDataset)
{
	JSMILE_ENTER;

    jobject jNetwork = CreateNewNetwork(env);
	DSL_network *net = GetNetworkPtr(env, jNetwork);	
	DSL_dataset *ds = GetDataSet(env, jDataset);

	if (!GetPtr(env, obj)->ToNetwork(*ds, *net))
	{
		ThrowSmileException(env, "MakeNetwork", -1);
	}
	
	JSMILE_RETURN(jNetwork);
}

//-------------------------------------------------------------------------------

} // end extern "C"
