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
public class P1Population extends Population {
    
    public P1Population (int PopulationSize, double minx, double maxx, double miny, double maxy, double stepx, double stepy,
            int ChromosomeSize)
    {
        P1PElement[][] Problem1InitialElements = new P1PElement[PopulationSize][ChromosomeSize];
        for(int i=0; i<PopulationSize; i++) 
        {
            for(int j=0; j<ChromosomeSize; j++)
            {
                Problem1InitialElements[i][j] = new P1PElement(minx, maxx, miny, maxy, stepx, stepy);
            }
        }

        _Generation = Problem1InitialElements;
    }
    
}
