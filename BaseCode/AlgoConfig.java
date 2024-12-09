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
public abstract class AlgoConfig {
    
    public int _ParentSelectionScheme, _SurvivorSelectionScheme,_TParentSampleSize, _ProblemType,
            _MaxRank;
    public int _PopulationCount, _ChromosomeLength, _NumberOfOffsprings, _NumberOfClones;
    public int _AverageFitness, _DegreeOfMutation, _FitnessProportionate, 
            _MutationVariationPercentage;
    public double _Phi, _C1, _C2, _RankSelectionParameter, _FitnessSelectionParameter;
    public int _NumberOfInputNeurons, _NumberOfHiddenNeurons, _NumberOfOutputNeurons;
    
    public double[] _Input;
    
    Logger _Log;
    
    public void Initialize(Logger log)
    {
        _Log = log;
    }
    
    public double[] ComputeFitness(int PopulationSize, int ChromosomeSize, double[] input)
    {
        return _Input;
    }
    
}
