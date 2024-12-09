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
public abstract class PopulationGenerator {
    
    Population _Population;
    protected double _MinX, _MaxX, _MinY, _MaxY, _StepX, _StepY;
    
    Logger _Log;
    
    public Population GeneratePopulation(int PopulationSize, int ChromosomeSize)
    {
        return _Population;
    }
    
}
