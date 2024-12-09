/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package ciproject.Problems;

import ciproject.BaseCode.*;
import ciproject.AILib.*;

/**
 *
 * @author aaht14
 */
public class NNConfig extends AlgoConfig{
    
    public double[] ComputeFitness(int PopulationSize, int ChromosomeSize, double[] input)
    {
        _Input = new double[PopulationSize];
        
        for(int i=0; i<PopulationSize; i++)
        {
            _Input[i] = 0;
        }
        
        int count = 0;
        double _NetSignal = 0, _Sigmoid = 0;

        for(int i=0; i<PopulationSize; i++)
        {
            for(int j=0; j<ChromosomeSize; j++)
            {
                double x = input[count];
                double y = input[count+1];
                _NetSignal += x * y;;
                count+=2;
            }
            double t1 = -1 * _NetSignal, t2 = Math.pow(Math.E, t1), t3 = t2+1, t4 = 1 / t3; 
            _Sigmoid = t4;
            _Input[i] = _Sigmoid;
        }
        
        return _Input;
    }
}
