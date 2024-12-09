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
public class NeuralNetwork extends AIAlgos{
    
    AlgoConfig _AlgorithmConfiguration;
    PopulationGenerator _PopulationGenerator;
    Population _InputNeurons, _HiddenNeurons, _OutputNeurons;
    

    public NeuralNetwork(Logger log, PopulationGenerator PG, AlgoConfig AC)
    {
        _Log = log;
        
        _PopulationGenerator = PG;
        _AlgorithmConfiguration = AC;
        
        _InputNeurons = PG.GeneratePopulation(_AlgorithmConfiguration._PopulationCount, 
                _AlgorithmConfiguration._NumberOfInputNeurons);
        _InputNeurons.Initialize(log, _AlgorithmConfiguration, _AlgorithmConfiguration._PopulationCount, 
                _AlgorithmConfiguration._NumberOfInputNeurons);

        _HiddenNeurons = PG.GeneratePopulation(_AlgorithmConfiguration._PopulationCount, 
                _AlgorithmConfiguration._NumberOfHiddenNeurons);
        _HiddenNeurons.Initialize(log, _AlgorithmConfiguration, _AlgorithmConfiguration._PopulationCount, 
                _AlgorithmConfiguration._NumberOfHiddenNeurons);

        _OutputNeurons = PG.GeneratePopulation(_AlgorithmConfiguration._PopulationCount, 
                _AlgorithmConfiguration._NumberOfOutputNeurons);
        _OutputNeurons.Initialize(log, _AlgorithmConfiguration, _AlgorithmConfiguration._PopulationCount, 
                _AlgorithmConfiguration._NumberOfOutputNeurons);
    }

    public void Execute()
    {
        for(int i=0; i<_InputNeurons._PopulationSize; i++)
        {
            _Log.Debug(_InputNeurons.GetInput(i));
        }
        ComputeFitness(_InputNeurons, _AlgorithmConfiguration);
        _Log.Debug(_InputNeurons._GenerationFitness);
        
        double[] temp = _InputNeurons._GenerationFitness;
        double[] input = new double[_AlgorithmConfiguration._NumberOfHiddenNeurons];
        
        for(int j=0; j<_InputNeurons._PopulationSize; j++)
        {
            for(int i=0; i<_AlgorithmConfiguration._NumberOfHiddenNeurons; i++)
            {
                input[i] = temp[j];
            }
            _HiddenNeurons.SetInput(j, input);
        }
        for(int i=0; i<_HiddenNeurons._PopulationSize; i++)
        {
            _Log.Debug(_HiddenNeurons.GetInput(i));
        }
        ComputeFitness(_HiddenNeurons, _AlgorithmConfiguration);
        _Log.Debug(_HiddenNeurons._GenerationFitness);
        
        input = new double[_AlgorithmConfiguration._NumberOfOutputNeurons];
        
        for(int j=0; j<_InputNeurons._PopulationSize; j++)
        {
            for(int i=0; i<_AlgorithmConfiguration._NumberOfOutputNeurons; i++)
            {
                input[i] = temp[j];
            }
            _OutputNeurons.SetInput(j, input);
        }
        for(int i=0; i<_OutputNeurons._PopulationSize; i++)
        {
            _Log.Debug(_OutputNeurons.GetInput(i));
        }
        ComputeFitness(_OutputNeurons, _AlgorithmConfiguration);
        _Log.Debug(_OutputNeurons._GenerationFitness);
    }
    
    public void SetInput(int i, double[] input)
    {
        _InputNeurons.SetInput(i, input);
    }
    
    public void SetWeights(int i, double[] input)
    {
        double[] weights = new double[_InputNeurons._ChromosomeSize];
        Utility.CopyArray(weights, input, 0, 0, _InputNeurons._ChromosomeSize);
        _InputNeurons.SetWeight(i, weights);
        
        weights = new double[_HiddenNeurons._ChromosomeSize];
        Utility.CopyArray(weights, input, 0, _InputNeurons._ChromosomeSize, _HiddenNeurons._ChromosomeSize);
        _HiddenNeurons.SetWeight(i, weights);
        
        weights = new double[_OutputNeurons._ChromosomeSize];
        Utility.CopyArray(weights, input, 0, _InputNeurons._ChromosomeSize + _HiddenNeurons._ChromosomeSize, 
                _OutputNeurons._ChromosomeSize);
        _OutputNeurons.SetWeight(i, weights);
    }
    
    public double[] GetOutput(int i)
    {
        return _OutputNeurons.GetInput(i);
    }
    
    public double[] GetWeights(int i)
    {
        double[] weights = new double[
                _InputNeurons._ChromosomeSize + _HiddenNeurons._ChromosomeSize + _OutputNeurons._ChromosomeSize];
        
        Utility.CopyArray(weights, _InputNeurons.GetWeights(i), 0, 0, _InputNeurons._ChromosomeSize);
        Utility.CopyArray(weights, _HiddenNeurons.GetWeights(i), _InputNeurons._ChromosomeSize, 
                0, _HiddenNeurons._ChromosomeSize);
        Utility.CopyArray(weights, _OutputNeurons.GetWeights(i), _InputNeurons._ChromosomeSize 
                + _HiddenNeurons._ChromosomeSize, 0, _OutputNeurons._ChromosomeSize);
        
        return weights;
    }
}
