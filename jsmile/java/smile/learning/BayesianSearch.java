package smile.learning;

import smile.Network;
import smile.Wrapper;

public class BayesianSearch extends Wrapper
{
	public native Network learn(DataSet data);
	
	public double getLastScore() {
		return lastScore;
	}

	public native void setMaxParents(int count);
	public native int getMaxParents();

	public native void setMaxSearchTime(int searchTime);
	public native int getMaxSearchTime();

	public native void setIterationCount(int count);
	public native int getIterationCount();

	public native void setLinkProbability(double prob);
	public native double getLinkProbability();

	public native void setPriorLinkProbability(double prob);
	public native double getPriorLinkProbability();
	
	public native void setPriorSampleSize(int sampleSize);
	public native int getPriorSampleSize();

	public native void setRandSeed(int seed);
	public native int getRandSeed();
	
	public native BkKnowledge getBkKnowledge();
	public native void setBkKnowledge(BkKnowledge bkk);
	
	protected native long createNative(Object param);
	protected native void deleteNative(long nativePtr);
	
	private double lastScore;
}
