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
public class SLPElement extends PElement2{
    public SLPElement (double minx, double maxx, double miny, double maxy, double stepx, double stepy, int count,
            int valuetype, Logger log)
    {
        _ValueType = valuetype;
        _ElementDepth = count;
        
        _MinX = minx; _MaxX = maxx; _MinY = miny; _MaxY = maxy; _StepX = stepx; _StepY = stepy;

        log.Debug("minx " + minx + " maxx " + maxx + " _MinX " + _MinX + " _MaxX " + _MaxX);
        
        super.InitializeElement(log);
    }
     
    /*protected @Override double GenerateValue (double Min, double Max)
    {
        double temp = Utility.GenerateRandom(0, 0, Utility.DoubleNormal);
        temp = Math.round(temp);
        return temp;
    }*/
}
