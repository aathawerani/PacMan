package smile.learning;

import smile.Wrapper;
import smile.Network;

public class DataSet extends Wrapper 
{
	public final static int DefaultMissingInt = -1;
	public final static float DefaultMissingFloat = (float) Math.sqrt(-1.0);
	
	public class DiscretizationAlgorithmType 
	{
		public static final int Hierarchical = 0;
		public static final int UniformWidth = 1;
		public static final int UniformCount = 2;
	}

	
    public native void readFile(String filename, String missingValueToken, int missingInt, float missingFloat, boolean columnIdsPresent);
    public void readFile(String filename, String missingValueToken) 
    { 
        readFile(filename, missingValueToken, DefaultMissingInt, DefaultMissingFloat, true);
    }
    public void readFile(String filename) { readFile(filename, null); }

	public native void writeFile(String filename, char separator, String missingValueToken, boolean columnIdsPresent);
	public void writeFile(String filename) { writeFile(filename, '\t', null, true); }

	public native DataMatch[] matchNetwork(Network net);

	public native double[] discretize(int variable, int algorithm, int intervals, String statePrefix);

	public native int getRecordCount();
	public native int getVariableCount();
	
	public native void addEmptyRecord();

	public native String getVariableId(int variableIndex);
	public native int findVariable(String variableId);
	
	public native int getInt(int variable, int record);
	public native void setInt(int variable, int record, int value);
	
	public native float getFloat(int variable, int record);
	public native void setFloat(int variable, int record, float value);
	
	public native boolean isMissing(int variable, int record);
	public native boolean setMissing(int variable, int record);
		
	public native void addIntVariable(String id, int missingValue);
	public native void addFloatVariable(String id, float missingValue);
	public void addFloatVariable(String id) { addFloatVariable(id, DefaultMissingFloat); }
	public void addIntVariable(String id) { addIntVariable(id, DefaultMissingInt); }
	
	public native boolean isDiscrete(int variableIndex);
	public native boolean isDiscrete(String variableId);
	
	public native String[] getStateNames(int variable);
	public native void setStateNames(int variable, String[] names);

    protected native long createNative(Object param);
	protected native void deleteNative(long nativePtr);
}
