package smile.learning;

import smile.Network;
import smile.Wrapper;

public class TAN extends Wrapper
{
    public native Network learn(DataSet data);

	public native void setClassVariableId(String id);
	public native String getClassVariableId();
	
	public native void setRandSeed(int seed);
	public native int getRandSeed();

	public native void setMaxSearchTime(int searchTime);
	public native int getMaxSearchTime();

	protected native long createNative(Object param);
	protected native void deleteNative(long nativePtr);
}
