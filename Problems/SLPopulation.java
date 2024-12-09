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
public class SLPopulation extends Population2{
    
    public SLPopulation (int PopulationSize, double[] minx, double[] maxx, double[] miny, double[] maxy, double stepx, 
            double stepy, int ChromosomeSize, int ElementDepth, int valuetype, Logger log)
    {
        _Log = log;
        
        _Log.Debug(minx); _Log.Debug(maxx); 

        SLPElement[][] SLPElements = new SLPElement[PopulationSize][ChromosomeSize];
        for(int i=0; i<PopulationSize; i++) 
        {
            for(int j=0; j<ChromosomeSize; j++)
            {
                SLPElements[i][j] = new SLPElement(minx[j], maxx[j], miny[j], maxy[j], stepx, stepy, 
                ChromosomeSize, valuetype, _Log);
            }
        }
        
        _Generation = SLPElements;
    }
    
}
