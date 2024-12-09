#ifndef JSMILE_JSMILE_H
#define JSMILE_JSMILE_H

// jsmile.h

#include <jni.h>
#include <vector>
#include <stdexcept>
#include "smile.h"
#include "smilearn.h"
#include "defequation.h"
#include "valequationevaluation.h"

class DSL_dataset;

extern "C" jfieldID FID_ptrNative; 

inline void* GetNativePtr(JNIEnv *env, jobject obj)
{
    return reinterpret_cast<void *>(env->GetLongField(obj, FID_ptrNative));
}

// specialize this template if nontrivial creation is required 
// (e.g. parameters must be passed to c'tor)
template <class T> 
T* CreateWrapped(JNIEnv *env, jobject obj, jobject param) { return new T; }

#define JSMILE_ENTER try {
#define JSMILE_LEAVE } catch (std::exception &ex) { ThrowSmileException(env, ex); }
#define JSMILE_RETURN(r) return (r); } catch (std::exception &ex) { ThrowSmileException(env, ex); return 0; }
#define JSMILE_METHOD_NAME(jcls, jmeth) Java_smile_##jcls##_##jmeth

#define JSMILE_IMPLEMENT_WRAPPER(jcls, ncls) \
JNIEXPORT jlong JNICALL JSMILE_METHOD_NAME(jcls, createNative)(JNIEnv *env, jobject obj, jobject param) \
{ JSMILE_ENTER; JSMILE_RETURN(reinterpret_cast<jlong>(CreateWrapped<ncls>(env, obj, param))); } \
JNIEXPORT void JNICALL JSMILE_METHOD_NAME(jcls, deleteNative)(JNIEnv *env, jobject obj, jlong nativePtr) \
{ JSMILE_ENTER; delete reinterpret_cast<ncls *>(nativePtr); JSMILE_LEAVE; } \
inline ncls* GetPtr(JNIEnv *env, jobject obj) { return static_cast<ncls *>(GetNativePtr(env, obj)); }

class NativeString
{
public:
    NativeString(JNIEnv *env, jstring js)
    {
        this->env = env;
        this->js = js;
        native = env->GetStringUTFChars(js, 0);
    }
    ~NativeString() { env->ReleaseStringUTFChars(js, native); }
    operator const char*() const { return native; }
    
    // SMILE isn't full const-correct, sometimes non-const ptr is needed
    // (but the string contents aren't changed)
    char* GetChars() { return const_cast<char *>(native); }

private:
    NativeString(const NativeString &);
    NativeString& operator=(const NativeString &);

    JNIEnv *env;
    jstring js;
    const char *native;
};

void DSL_appendInt(std::string &s, int i);

void ThrowSmileException(JNIEnv *env, const char *function, int errCode);
inline void ThrowSmileException(JNIEnv *env, const std::exception &ex)
{
    env->ThrowNew(env->FindClass("smile/SMILEException"), ex.what()); 
}

inline DSL_network* GetNetworkPtr(JNIEnv *env, jobject obj)
{
    return static_cast<DSL_network *>(GetNativePtr(env, obj));
}
DSL_network* GetRelatedNetworkPtr(JNIEnv *env, jobject obj);

jobject CreateNewNetwork(JNIEnv *env);

DSL_node* ValidateNodeHandle(DSL_network *net, int handle);
inline DSL_node* ValidateNodeHandle(JNIEnv *env, jobject obj, int handle)
{
    return ValidateNodeHandle(GetNetworkPtr(env, obj), handle);
}

int ValidateNodeId(JNIEnv *env, DSL_network *net, jstring nodeId);
inline int ValidateNodeId(JNIEnv *env, jobject obj, jstring nodeId)
{
    return ValidateNodeId(env, GetNetworkPtr(env, obj), nodeId);
}

DSL_node* ValidateOutcomeIndex(DSL_network *net, int nodeHandle, int outcomeIndex);
inline DSL_node* ValidateOutcomeIndex(JNIEnv *env, jobject obj, int nodeHandle, int outcomeIndex)
{
    return ValidateOutcomeIndex(GetNetworkPtr(env, obj), nodeHandle, outcomeIndex);
}

int ValidateOutcomeId(JNIEnv *env, DSL_network *net, int nodeHandle, jstring outcomeId);
inline int ValidateOutcomeId(JNIEnv *env, jobject obj, int nodeHandle, jstring outcomeId)
{
    return ValidateOutcomeId(env, GetNetworkPtr(env, obj), nodeHandle, outcomeId);
}

jintArray CopyIntArray(JNIEnv *env, const DSL_intArray &native);
jdoubleArray CopyDoubleArray(JNIEnv *env, DSL_doubleArray &native);
jobjectArray CopyStringArray(JNIEnv *env, const DSL_stringArray &native);
jobjectArray HandlesToIds(JNIEnv *env, DSL_network *net, const DSL_intArray &native);

DSL_dataset* GetDataSet(JNIEnv *env, jobject obj);

void ConvertMatching(JNIEnv *env, jobjectArray jMatching, std::vector<DSL_datasetMatch> &nativeMatching);

jobject GetBkKnowledge(JNIEnv *env, const DSL_bkgndKnowledge &bkk);

template <class T>
jobject Learn(T * learner, JNIEnv *env, jobject jNetwork, jobject jDataset)
{
    jNetwork = CreateNewNetwork(env);
	DSL_network *net = GetNetworkPtr(env, jNetwork);	
	DSL_dataset *ds = GetDataSet(env, jDataset);
		
    int res = learner->Learn(*ds, *net);
	if (0 != res) ThrowSmileException(env, "Learn", res);
    return jNetwork;
}



class DefLocaleCtx
{
public:
	DefLocaleCtx();
	~DefLocaleCtx();
private:
	std::string defLocale;
};

#endif //!JSMILE_JSMILE_H
