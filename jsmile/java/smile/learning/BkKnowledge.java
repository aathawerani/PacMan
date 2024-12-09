package smile.learning;

import smile.Wrapper;

public class BkKnowledge extends Wrapper
{
    public native void readFile(String filename);
	public native void writeFile(String filename);
	
	public native void matchData(DataSet dataset);

    public native void addVariable(String id);
    
    public native int getVariableCount();
    public native int findVariable(String id);
	public native String getVariableId(int variable);
    
    public native int getTier(int var);
    public native int getTier(String var);
    
    public native void setTier(int var, int tier);
    public native void setTier(String var, int tier);
    
    public native void addForcedArc(int from, int to);
    public native void addForcedArc(String from, String to);

    public native void addForbiddenArc(int from, int to);
    public native void addForbiddenArc(String from, String to);

	public native int[] getForbiddenChildren(int var);
	public native int[] getForcedChildren(int var);
    
    protected native long createNative(Object param);
	protected native void deleteNative(long nativePtr);
}
