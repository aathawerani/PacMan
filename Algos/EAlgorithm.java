/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package ciproject.Algos;

import ciproject.AILib.*;
import ciproject.BaseCode.*;
import java.io.*;

/**
 *
 * @author aaht14
 */
public class EAlgorithm extends AIAlgos{
    
    Population _Generation;
    Population _NextGeneration;
    AlgoConfig _AlgorithmConfiguration;

    PopulationGenerator _PopulationGenerator;
        
    public EAlgorithm(Logger log, PopulationGenerator PG, AlgoConfig AC)
    {
        _Log = log;
        
        _PopulationGenerator = PG;
        _AlgorithmConfiguration = AC;

        _Generation = PG.GeneratePopulation(_AlgorithmConfiguration._PopulationCount, 
                _AlgorithmConfiguration._ChromosomeLength);
        _NextGeneration = PG.GeneratePopulation(_AlgorithmConfiguration._NumberOfOffsprings, 
                _AlgorithmConfiguration._ChromosomeLength);
        
        _Generation.Initialize(_Log, _AlgorithmConfiguration, _AlgorithmConfiguration._PopulationCount, 
                _AlgorithmConfiguration._ChromosomeLength);
        _NextGeneration.Initialize(_Log, _AlgorithmConfiguration, _AlgorithmConfiguration._NumberOfOffsprings, 
                _AlgorithmConfiguration._ChromosomeLength);
    }
    
    public void Execute()
    {
        ComputeFitness(_Generation, _AlgorithmConfiguration);
        SelectParents(_Generation, _AlgorithmConfiguration);
        GenerateOffsprings(_Generation, _NextGeneration, _AlgorithmConfiguration);
        Mutate(_NextGeneration, _AlgorithmConfiguration);
        SelectSurvivors(_Generation, _NextGeneration, _AlgorithmConfiguration);
    }

    public double[] GetGenerationFitness()
    {
        return _Generation._GenerationFitness;
    }
}
