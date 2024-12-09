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
public class PElement2 {
    
    protected double _X[], _Y[];
    protected double _MinX, _MaxX, _MinY, _MaxY, _StepX, _StepY;
    protected int _ElementDepth, _ValueType;
    
    Logger _Log;
            
    public void InitializeElement(Logger log)
    {
        _Log = log;
        
        _X = new double[_ElementDepth];
        _Y = new double[_ElementDepth];

        for(int i=0; i<_ElementDepth; i++)
        {
            _X[i] = GenerateValue(_MinX, _MaxX);
            _Y[i] = GenerateValue(_MinY, _MaxY);
        }
        _Log.Debug("_MinX " + _MinX + " _MaxX " + _MaxX);
        _Log.Debug(_X);
    }
    
    public void GenerateRandom()
    {
        for(int i=0; i<_ElementDepth; i++)
        {
            _X[i] = GenerateValue(_MinX, _MaxX);
            _Y[i] = GenerateValue(_MinY, _MaxY);
        }
    }
    
    public void CrossOver(PElement2 PE1, PElement2 PE2, int scheme)
    {
        _Log.Debug("scheme " + scheme);
        _Log.Debug(PE1._X);
        _Log.Debug(PE2._X);
        switch(scheme)
        {
            case Utility.TwoPointCrossover: 
                int point1 = (int)Utility.GenerateRandom(0, _ElementDepth, Utility.IntWithRange);
                int point2 = (int)Utility.GenerateRandom(0, _ElementDepth, Utility.IntWithRange);
                int start, end;

                if(point1 < point2) {start = point1; end = point2;}
                else {start = point2; end = point1;}
                
                for(int i=0; i<point1; i++)
                {
                    _X[i] = PE1._X[i];
                    _Y[i] = PE1._Y[i];
                }
                for(int i=start; i<end; i++)
                {
                    _X[i] = PE2._X[i];
                    _Y[i] = PE2._Y[i];
                }
                for(int i=point2; i<_ElementDepth; i++)
                {
                    _X[i] = PE1._X[i];
                    _Y[i] = PE1._Y[i];
                }
                break;
            case Utility.OnePointCrossover: 
                int point = (int)Utility.GenerateRandom(0, _ElementDepth, Utility.IntWithRange);

                for(int i=0; i<point; i++)
                {
                    _X[i] = PE1._X[i];
                    _Y[i] = PE1._Y[i];
                }
                for(int i=point; i<_ElementDepth; i++)
                {
                    _X[i] = PE2._X[i];
                    _Y[i] = PE2._Y[i];
                }
                break;
            case Utility.RandomCrossOver:
                for(int i=0; i<_ElementDepth; i++)
                {
                    point = (int)Utility.GenerateRandom(0, 1, Utility.IntWithRange);
                    if(point > 0)
                    {
                        _X[i] = PE1._X[i];
                        _Y[i] = PE1._Y[i];
                    }
                    else
                    {
                        _X[i] = PE2._X[i];
                        _Y[i] = PE2._Y[i];
                    }
                }
                break;

        }
        _Log.Debug(_X);
    }

    public void Clone(PElement2 PE, int scheme)
    {
        CrossOver(PE, PE, scheme);
    }

    public void Mutate (double factor, int scheme)
    {
        switch(scheme)
        {
            case Utility.RandomMutation:
                for(int i=0; i<_ElementDepth; i++)
                {
                    //int random = (int)Utility.GenerateRandom(0, 1, Utility.IntWithRange);
                    //if(random > 0)
                    //{
                    _X[i] = MutateValue(_X[i], _MinX, _MaxX, _StepX * factor);
                    _Y[i] = MutateValue(_Y[i], _MinY, _MaxY, _StepY * factor);
                    //}
                }
                break;
            case Utility.SwapMutation:
                for(int i=0; i<_ElementDepth; i++)
                {
                    int swap = (int)Utility.GenerateRandom(0, _ElementDepth, Utility.IntWithRange);
                    if(swap > i)
                    {
                        double temp = _X[i];
                        _X[i] = _X[swap];
                        _X[swap] = temp;
                        temp = _Y[i];
                        _Y[i] = _Y[swap];
                        _Y[swap] = temp;
                    }
                    _X[i] = MutateValue(_X[i], _MinX, _MaxX, _StepX * factor);
                    _Y[i] = MutateValue(_Y[i], _MinY, _MaxY, _StepY * factor);
                }
                break;
            case Utility.InsertMutation:
                for(int i=0; i<_ElementDepth; i++)
                {
                    int insert = (int)Utility.GenerateRandom(0, _ElementDepth, Utility.IntWithRange);
                    if(insert > i)
                    {
                        double tempx = _X[insert];
                        double tempy = _Y[insert];
                        for(int j=insert; j>i; j--)
                        {
                            _X[j] = _X[j-1];
                            _Y[j] = _Y[j-1];
                        }
                        _X[i+1] = tempx;
                        _Y[i+1] = tempy;
                    }
                    _X[i] = MutateValue(_X[i], _MinX, _MaxX, _StepX * factor);
                    _Y[i] = MutateValue(_Y[i], _MinY, _MaxY, _StepY * factor);
                }
                break;
            case Utility.BitFlipMutation:
                for(int i=0; i<_ElementDepth; i++)
                {
                    _X[i] = _MaxX - _X[i];
                    _Y[i] = _MaxY - _Y[i];
                    _X[i] = MutateValue(_X[i], _MinX, _MaxX, _StepX * factor);
                    _Y[i] = MutateValue(_Y[i], _MinY, _MaxY, _StepY * factor);
                }
                break;
            case Utility.InversionMutation:
                int point1 = (int)Utility.GenerateRandom(0, _ElementDepth, Utility.IntWithRange);
                int point2 = (int)Utility.GenerateRandom(0, _ElementDepth, Utility.IntWithRange);
                int start, end;

                if(point1 < point2) {start = point1; end = point2;}
                else {start = point2; end = point1;}
                
                for(int i=start, j=end; i<=end/2; i++, j--)
                {
                    double temp = _X[i];
                    _X[i] = _X[j];
                    _X[j] = temp;
                    temp = _Y[i];
                    _Y[i] = _Y[j];
                    _Y[j] = temp;
                    
                    _X[i] = MutateValue(_X[i], _MinX, _MaxX, _StepX * factor);
                    _Y[i] = MutateValue(_Y[i], _MinY, _MaxY, _StepY * factor);
                }
                break;
        }
    }
    
    public void SetElement(PElement2 PE)
    {
        for(int i=0; i<_ElementDepth; i++)
        {
            _X[i] = PE._X[i]; _Y[i] = PE._Y[i];
        }
    }
    
    protected double GenerateValue (double Min, double Max)
    {
        double value = 0;
        switch(_ValueType)
        {
            case Utility.UseInt: value = Utility.GenerateRandom(Min, Max, Utility.IntWithRange); break;
            case Utility.UseDouble: value = Utility.GenerateRandom(Min, Max, Utility.DoubleWithRange); break;
            default: return Utility.GenerateRandom(Min, Max, Utility.DoubleWithRange); 
        }

        return value;
    }
    
    protected double MutateValue (double Value, double Min, double Max, double StepSize)
    {
        double temp = Value;
        switch(_ValueType)
        {
            case Utility.UseInt: temp = temp + Utility.GenerateRandom(StepSize, StepSize, Utility.IntWithRange); break;
            case Utility.UseDouble: temp = temp + Utility.GenerateRandom(StepSize, StepSize, Utility.DoubleWithRange); 
            break;
            default: temp = temp + Utility.GenerateRandom(StepSize, StepSize, Utility.DoubleWithRange); break;
        }
        temp = AdjustValue (temp, Min, Max, StepSize);
        return temp;
    }

    protected double AdjustValue (double Value, double Min, double Max, double StepSize)
    {
        double temp = Value;
        switch(_ValueType)
        {
            case Utility.UseInt: 
                temp = Utility.AdjustValue(temp, StepSize, Max, Utility.IntWithMax);
                temp = Utility.AdjustValue(temp, StepSize, Min, Utility.IntWithMin);
                break;
            case Utility.UseDouble: 
                temp = Utility.AdjustValue(temp, StepSize, Max, Utility.DoubleWithMax);
                temp = Utility.AdjustValue(temp, StepSize, Min, Utility.DoubleWithMin);
                break;
            default: 
                temp = Utility.AdjustValue(temp, StepSize, Max, Utility.DoubleWithMax);
                temp = Utility.AdjustValue(temp, StepSize, Min, Utility.DoubleWithMin);
                break;
        }
        return temp;
    }
    
/*    public void SetInput(double input)
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
    */
    
    public double[] GetValues()
    {
        double[] clonex = new double[_ElementDepth];
        
        for(int i=0; i<_ElementDepth; i++)
        {
            clonex[i] = _X[i];
        }
        return clonex;
    }

    public double[] GetWeights()
    {
        double[] cloney = new double[_ElementDepth];
        
        for(int i=0; i<_ElementDepth; i++)
        {
            cloney[i] = _Y[i];
        }
        return cloney;
    }

}
