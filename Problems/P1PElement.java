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
public class P1PElement extends PElement {
    
    public P1PElement (double minx, double maxx, double miny, double maxy, double stepx, double stepy)
    {
        _MaxX = maxx;
        _MinX = minx;
        _MaxY = maxy;
        _MinY = miny;
        _StepX = stepx;
        _StepY = stepy;
        super.InitializeElement();
    }
}
