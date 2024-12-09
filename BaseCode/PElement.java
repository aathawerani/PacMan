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
public abstract class PElement {
    
    protected double _X, _Y;
    protected double _MinX, _MaxX, _MinY, _MaxY, _StepX, _StepY;
    
    public void InitializeElement()
    {
        _X = GenerateValue(_MinX, _MaxX);
        _Y = GenerateValue(_MinY, _MaxY);
        
    }
    
    public void CrossOver(PElement PE1, PElement PE2)
    {
        _X = PE1._X; _Y = PE2._Y;
    }

    public void Clone(PElement PE)
    {
        _X = PE._X; _Y = PE._Y;
    }

    public void Mutate (double[] factor)
    {
        _X = MutateValue(_X, _MinX, _MaxX, _StepX * factor[0]);
        _Y = MutateValue(_Y, _MinY, _MaxY, _StepY * factor[0]);
    }
    
    public void SetElement(PElement PE)
    {
        _X = PE._X; _Y = PE._Y;
    }
    
/*    public double Compute()
    {
        return ((_X * _X) + (_Y * _Y));
    }
*/    
    protected double GenerateValue (double Min, double Max)
    {
        return Utility.GenerateRandom(Min, Max, Utility.DoubleWithRange);
    }
    
    protected double MutateValue (double Value, double Min, double Max, double StepSize)
    {
        double temp = Value;
        temp = temp + Utility.GenerateRandom(StepSize, StepSize, Utility.DoubleWithRange);
        temp = AdjustValue (temp, Min, Max, StepSize);
        //System.out.println("MutateValue temp " + temp);
        return temp;
    }

    protected double AdjustValue (double Value, double Min, double Max, double StepSize)
    {
        double temp = Value;
        temp = Utility.AdjustValue(temp, StepSize, Max, Utility.DoubleWithMax);
        temp = Utility.AdjustValue(temp, StepSize, Min, Utility.DoubleWithMin);
        return temp;
    }
    
    public void SetInput(double input)
    {
        _Y = input;
    }
    
    public void SetWeight(double weight)
    {
        _X = weight;
    }
    
    public double GetInput()
    {
        return _Y;
    }
    
    public double GetWeight()
    {
        return _X;
    }
}
