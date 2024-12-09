package smile.learning;

import smile.Network;
import smile.Wrapper;

public class Validator extends Wrapper
{
	public Validator(Network net, DataSet dataset, DataMatch matching[]) 
	{
		super(new Object[] { net, dataset, matching });
		this.net = net;
		this.dataset = dataset;
		this.matching = matching;
	}
	
	public native void addClassNode(int nodeHandle);
	public native void addClassNode(String nodeId);
	
	public native void test();
	public native void leaveOneOut(EM em);
	public native void kFold(EM em, int foldCount, int foldingRandSeed);
	public void kFold(EM em, int foldCount)
	{
		kFold(em, foldCount, 0);
	}
	
	public native DataSet getResultDataSet();

	public native int[][] getConfusionMatrix(int nodeHandle);
	public native int[][] getConfusionMatrix(String nodeId);
	
	public native double getAccuracy(int nodeHandle, int outcomeIndex);
	public native double getAccuracy(String nodeId, int outcomeIndex);
	public native double getAccuracy(int nodeHandle, String outcomeId);
	public native double getAccuracy(String nodeId, String outcomeId);

	protected native long createNative(Object param);
	protected native void deleteNative(long nativePtr);

	private Network net;
	private DataSet dataset;
	private DataMatch matching[];
}