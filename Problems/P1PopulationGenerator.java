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
public class P1PopulationGenerator extends PopulationGenerator{
    
    public P1PopulationGenerator(double minx, double maxx, double miny, double maxy, double stepx, double stepy)
    {
        _MaxX = maxx;
        _MinX = minx;
        _MaxY = maxy;
        _MinY = miny;
        _StepX = stepx;
        _StepY = stepy;
    
    }
    
    public Population GeneratePopulation(int PopulationSize, int ChromosomeSize)
    {
        P1Population Problem1Population = new P1Population(PopulationSize, _MinX, _MaxX, _MinY, _MaxY, _StepX, _StepY, 
                ChromosomeSize);
        return Problem1Population;
    }
}
