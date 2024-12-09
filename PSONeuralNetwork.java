/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package ciproject;

import ciproject.AILib.Utility;
import ciproject.BaseCode.*;
/**
 *
 * @author aaht14
 */
public class PSONeuralNetwork extends AIAlgos{
    
/*    NeuralNetwork[] _NeuralNetworks;
    NeuralNetwork[] _PBNeuralNetworks;
    
    int _NumberOfInputNeurons, _NumberOfHiddenNeurons, _NumberOfOutputNeurons;

    int _NumberOfNeuralNetworks, _GBest;
    
    double[] _Fitness, _PBFitness;
    
    PSO pso;
    
    public void InitializePSONN(int NumberOfNeuralNetworks)
    {
        _NumberOfNeuralNetworks = NumberOfNeuralNetworks;
        _NeuralNetworks = new NeuralNetwork[_NumberOfNeuralNetworks];
        _PBNeuralNetworks = new NeuralNetwork[_NumberOfNeuralNetworks];

        _Fitness = new double[_NumberOfNeuralNetworks];
        _PBFitness = new double[_NumberOfNeuralNetworks];

        for(int i=0; i<_NumberOfNeuralNetworks; i++)
        {
            _Fitness[i] = 0;
            _PBFitness[i] = 0;
        }


        _NumberOfInputNeurons = 14;
        _NumberOfHiddenNeurons = 30;
        _NumberOfOutputNeurons = 4;

        for(int i=0; i<_NumberOfNeuralNetworks; i++)
        {
            _NeuralNetworks[i] = new NeuralNetwork(_NumberOfInputNeurons, _NumberOfHiddenNeurons, _NumberOfOutputNeurons, 
                    Utility.GenerateRandom(0, 0, Utility.DoubleNormal), 
                    Utility.GenerateRandom(0, 0, Utility.DoubleNormal), 
                    Utility.GenerateRandom(0, 0, Utility.DoubleNormal), 
                    Utility.GenerateRandom(0, 0, Utility.DoubleNormal), 
                    Utility.GenerateRandom(0, 0, Utility.DoubleNormal), 
                    Utility.GenerateRandom(0, 0, Utility.DoubleNormal), -5, 5, 1, 1, 0.1, 1);
            _PBNeuralNetworks[i] = new NeuralNetwork(_NumberOfInputNeurons, _NumberOfHiddenNeurons, _NumberOfOutputNeurons, 
                    Utility.GenerateRandom(0, 0, Utility.DoubleNormal), 
                    Utility.GenerateRandom(0, 0, Utility.DoubleNormal), 
                    Utility.GenerateRandom(0, 0, Utility.DoubleNormal), 
                    Utility.GenerateRandom(0, 0, Utility.DoubleNormal), 
                    Utility.GenerateRandom(0, 0, Utility.DoubleNormal), 
                    Utility.GenerateRandom(0, 0, Utility.DoubleNormal), -5, 5, 1, 1, 0.1, 1);
        }
        
        pso = new PSO(_NumberOfInputNeurons + _NumberOfHiddenNeurons + _NumberOfOutputNeurons, 
                Utility.GenerateRandom(0, 0, Utility.DoubleNormal), 
                2, 2, AlgoConfig.Maximization, -1, 1, 1, 1, 0.1, 1);
        
    }
    
    public int PacManTrainer(int trial, double[] input, int fitness)
    {
        /*for(int i=0; i<input.length; i++)
        {
            System.out.println("input " + input[i]);
        }*/
/*        _Fitness[trial] = fitness;
        if(_Fitness[trial] > _PBFitness[trial])
        {
            _PBFitness[trial] = _Fitness[trial];
        }
        
        return _NeuralNetworks[trial].GetOutput(input);
    }

    public int GetOutputFinal(double[] input)
    {
        return _NeuralNetworks[_GBest].GetOutput(input);
    }
    
    public void updateNeuralNetworks()
    {
        
        Population[] P = new Population[_NumberOfNeuralNetworks];
        Population[] PBP = new Population[_NumberOfNeuralNetworks];
        
        for(int j=0; j<_NumberOfNeuralNetworks; j++)
        {
            P[j] = _NeuralNetworks[j].GetAllNeurons();
            PBP[j] = _PBNeuralNetworks[j].GetAllNeurons();
        }

        Population[] newP = new Population[_NumberOfNeuralNetworks];
        
        _GBest = pso.GetUpdatedParticles(_Fitness, _PBFitness, P, PBP, newP);
        
        //System.out.println("_GBest " + _GBest);
        
        for(int j=0; j<_NumberOfNeuralNetworks; j++)
        {
            _NeuralNetworks[j].SetAllNeurons(newP[j]);
        }
    }
    */
}
