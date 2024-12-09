/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package ciproject.BaseCode;

import ciproject.AILib.*;
/**
 *
 * @author aaht14
 */
public class AlgoConfig2 {
    public int _ParentSelectionScheme, _SurvivorSelectionScheme,_TParentSampleSize, _ProblemType,
            _MaxRank;
    public int _PopulationCount, _ChromosomeLength, _NumberOfOffsprings, _NumberOfClones, _CrossOverScheme;
    public int _AverageFitness, _DegreeOfMutation, _FitnessProportionate, 
            _MutationVariationPercentage, _MutationScheme;
    public double _MutationRate, _CrossOverRate;
    public double _Phi, _C1, _C2, _RankSelectionParameter, _FitnessSelectionParameter;
    
    public double[] _Output;
    
    protected Logger _Log;
    
    
    public void UpdateConfig(int step)
    {
        
    }
    
    public double[] ComputeFitness(Population2 Input)
    {
        
        return _Output;
    }
}
