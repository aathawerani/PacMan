/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package ciproject.Algos;

import ciproject.AILib.*;
import ciproject.BaseCode.*;


/**
 *
 * @author aaht14
 */
public class PSO extends AIAlgos{
    
    AlgoConfig _AlgorithmConfiguration;
    PopulationGenerator _PopulationGenerator;
    Population _Generation, _PreviousGeneration;
    int _Iteration;
        
    public PSO(Logger log, PopulationGenerator PG, AlgoConfig AC)
    {
        _Log = log;
        
        _PopulationGenerator = PG;
        _AlgorithmConfiguration = AC;
        
        _Generation = PG.GeneratePopulation(_AlgorithmConfiguration._PopulationCount, 
                _AlgorithmConfiguration._ChromosomeLength);
        _Generation.Initialize(_Log, _AlgorithmConfiguration, _AlgorithmConfiguration._PopulationCount, 
                _AlgorithmConfiguration._ChromosomeLength);
        
        _PreviousGeneration = PG.GeneratePopulation(_AlgorithmConfiguration._PopulationCount, 
                _AlgorithmConfiguration._ChromosomeLength);
        _PreviousGeneration.Initialize(_Log, _AlgorithmConfiguration, _AlgorithmConfiguration._PopulationCount, 
                _AlgorithmConfiguration._ChromosomeLength);

        _Iteration = 0; 
    }

    
    public void Execute()
    {
        ComputeFitness(_Generation, _AlgorithmConfiguration);

        int GBest = 0;
        int[] PBest = new int[_Generation._PopulationSize];

        Utility.Sort(_Generation._GenerationFitness, PBest, _Generation._PopulationSize, 
                _AlgorithmConfiguration._ProblemType);
        
        GBest = PBest[0];

        double[] present = new double[_Generation._ChromosomeSize];
        double[] velocity = new double[_Generation._ChromosomeSize];
        double[] pbest = new double[_Generation._ChromosomeSize];
        double[] gbest = new double[_Generation._ChromosomeSize];
        int count = 0;

        for(int i=0; i<_Generation._PopulationSize; i++)
        {
            if(_Iteration > 0 && _Generation._GenerationFitness[i] < _PreviousGeneration._GenerationFitness[i])
            {
                present = _Generation.GetInput(i);
                velocity = _Generation.GetWeights(i);
                gbest = _Generation.GetInput(GBest);
                pbest = _PreviousGeneration.GetInput(i);
                for(int j=0; j<_Generation._ChromosomeSize; j++)
                {
                    velocity[j] = (_AlgorithmConfiguration._Phi * velocity[j]) + 
                            ((_AlgorithmConfiguration._C1 * Utility.GenerateRandom(0, 0, Utility.DoubleNormal)) * 
                            (pbest[j] - present[j])) 
                            + ((_AlgorithmConfiguration._C2 * Utility.GenerateRandom(0, 0, Utility.DoubleNormal)) * 
                            (gbest[j] - present[j]));
                    present[j] = present[j] + velocity[j];
                }
            }
            else
            {
                double[] input = _Generation.GetInput(i);
                _PreviousGeneration.SetInput(i, input);
                double[] weight = _Generation.GetWeights(i);
                _PreviousGeneration.SetWeight(i, weight);
                _PreviousGeneration._GenerationFitness[i] = _Generation._GenerationFitness[i];
            }
        }
        
        _Iteration++;
        
    }
    
    public void SetInput(int i, double[] input)
    {
        _Generation.SetInput(i, input);
    }
    
    public double[] GetInput(int i)
    {
        return _Generation.GetInput(i);
    }

}
