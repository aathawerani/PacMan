/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package ciproject.Problems;

import ciproject.BaseCode.*;

/**
 *
 * @author aaht14
 */
public class PSOConfig extends AlgoConfig{

    public void SetInput(double[] input)
    {
        _Input = new double[input.length];
                
        _Input = input;
    }

}
