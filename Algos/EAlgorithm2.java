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
public class EAlgorithm2 extends AIAlgos2{
    
    Population2 _Generation;
    Population2 _NextGeneration;
    AlgoConfig2 _AlgorithmConfiguration;

    PopulationGenerator2 _PopulationGenerator;
        
    public EAlgorithm2(Logger log, PopulationGenerator2 PG, AlgoConfig2 AC)
    {
        _Log = log;
        
        _PopulationGenerator = PG;
        _AlgorithmConfiguration = AC;

        _Generation = PG.GeneratePopulation(_AlgorithmConfiguration._PopulationCount);
        _NextGeneration = PG.GeneratePopulation(_AlgorithmConfiguration._NumberOfOffsprings);
        
        _Generation.Initialize(_Log, _AlgorithmConfiguration, _AlgorithmConfiguration._PopulationCount, 
                _AlgorithmConfiguration._ChromosomeLength);
        _NextGeneration.Initialize(_Log, _AlgorithmConfiguration, _AlgorithmConfiguration._NumberOfOffsprings, 
                _AlgorithmConfiguration._ChromosomeLength);

        ComputeFitness(_Generation, _AlgorithmConfiguration);
        _Log.Info(_Generation._GenerationFitness);
    }
    
    public void Execute()
    {
        SelectParents(_Generation, _AlgorithmConfiguration);
        //_Log.Info(_Generation._SelectedElements);
        GenerateOffsprings(_Generation, _NextGeneration, _AlgorithmConfiguration);
        Mutate(_NextGeneration, _AlgorithmConfiguration);
        ComputeFitness(_NextGeneration, _AlgorithmConfiguration);
        _Log.Info(_NextGeneration._GenerationFitness);
        SelectSurvivors(_Generation, _NextGeneration, _AlgorithmConfiguration);
        ComputeFitness(_Generation, _AlgorithmConfiguration);
        _Log.Info(_Generation._GenerationFitness);
    }

    public int BestFitness()
    {
        _Log.Debug(_Generation._GenerationFitness);
        int max = Utility.Max(_Generation._GenerationFitness, _Generation._PopulationSize);
        return max;
    }
    
    public int WorstFitness()
    {
        _Log.Debug(_Generation._GenerationFitness);
        int min = Utility.Min(_Generation._GenerationFitness, _Generation._PopulationSize);
        return min;
    }
    
    public double Fitness(int id)
    {
        return _Generation._GenerationFitness[id];
    }
}
