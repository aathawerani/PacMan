// FaultInfo.java
 
package smile;

import java.util.Comparator;

public class FaultInfo 
{
	public FaultInfo(int index, int node, int outcome, double probability, boolean isPursued) 
	{
		this.index = index;
		this.node = node;
		this.outcome = outcome;
		this.probability = probability;
		this.isPursued = isPursued;
	}
	
	public int index;
	public int node;
	public int outcome;
	public boolean isPursued;
	public double probability;
}
