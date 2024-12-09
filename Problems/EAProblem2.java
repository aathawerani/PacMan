/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package ciproject.Problems;

import ciproject.BaseCode.AlgoConfig;

/**
 *
 * @author aaht14
 */
public class EAProblem2 extends AlgoConfig{
    
    public double[] ComputeFitness(int PopulationSize, int ChromosomeSize, double[] input)
    {
        _Input = new double[PopulationSize];
        
        for(int i=0; i<PopulationSize; i++)
        {
            _Input[i] = 0;
        }
        
        int count = 0;

        for(int i=0; i<PopulationSize; i++)
        {
            for(int j=0; j<ChromosomeSize; j++)
            {
                double x = input[count];
                double y = input[count+1];
                _Input[i] = _Input[i] + ((100 * (x*x) - (y*y)) + (1 - (x*x)));
                count+=2;
            }
        }
        return _Input;
    }
}
