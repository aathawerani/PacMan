/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package ciproject.Problems;

import ciproject.AILib.*;
import ciproject.BaseCode.*;

/**
 *
 * @author aaht14
 */
public class SLPopulationGenerator extends PopulationGenerator2{
    
    public SLPopulationGenerator(double[] minx, double[] maxx, double[] miny, double[] maxy, 
            double stepx, double stepy, int valuetype, int ElementDepth, int ChromosomeSize, Logger log)
    {
        

        _Log = log;
        
        _ValueType = valuetype;
        _ElementDepth = ElementDepth;
        _ChromosomeSize = ChromosomeSize;
        
        _MinX = minx; _MaxX = maxx; _MinY = miny; _MaxY = maxy; _StepX = stepx; _StepY = stepy;
        
        _Log.Debug(minx); _Log.Debug(maxx); _Log.Debug(_MinX); _Log.Debug(_MaxX);
    }

    public @Override Population2 GeneratePopulation(int PopulationSize)
    {
        
        SLPopulation SLPop = new SLPopulation(PopulationSize, _MinX, _MaxX, _MinY, _MaxY, _StepX, _StepY, 
                _ChromosomeSize, _ElementDepth, _ValueType, _Log);
                
        return SLPop;
    }
}
