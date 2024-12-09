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
public class Population2 {
    protected PElement2[][] _Generation;
    public double _AverageFitness;
    public double[] _GenerationFitness; 
    public int[] _SelectedElements;
    public int _PopulationSize, _ChromosomeSize, _MutationProbability;
    
    
    protected Logger _Log;
    
    public void Initialize(Logger log, AlgoConfig2 AC, int PopulationSize, int ChromosomeSize)
    {
        _Log = log;
        
        _PopulationSize = PopulationSize;
        _ChromosomeSize = ChromosomeSize;

        _GenerationFitness = new double[_PopulationSize];
        _SelectedElements = new int[_PopulationSize];
    }

    public void GenerateRandom(int ID)
    {
        for(int j=0; j<_Generation[ID].length; j++)
        {
            _Generation[ID][j].GenerateRandom();
        }
    }
    
    public void GenerateElement(int ID, int P1, int P2, Population2 P, AlgoConfig2 AC)
    {
        PElement2[] PE1 = P._Generation[P1];
        PElement2[] PE2 = P._Generation[P2];

        for(int j=0; j<_Generation[ID].length; j++)
        {
            _Generation[ID][j].CrossOver(PE1[j], PE2[j], AC._CrossOverScheme);
        }
    }

    public void CloneElement(int ID, int PID, Population2 P, AlgoConfig2 AC)
    {
        PElement2[] PE = P._Generation[PID];

        for(int j=0; j<_Generation[ID].length; j++)
        {
            _Generation[ID][j].Clone(PE[j], AC._CrossOverScheme);
        }
    }
    
    public void Mutate(AlgoConfig2 AC)
    {
        double factors = AC._DegreeOfMutation;
        switch(AC._FitnessProportionate)
        {
            case Utility.MutateEvenly: 
                    for(int i=0; i<_Generation.length; i++)
                    {
                        for(int j=0; j<_ChromosomeSize; j++)
                        {
                            double mutate = Utility.GenerateRandom(0, 0, Utility.DoubleNormal);
                            if(mutate < AC._MutationRate)
                            {
                                _Generation[_SelectedElements[i]][j].Mutate(factors, AC._MutationScheme);
                            }
                        }
                    }
                break;
            case Utility.MutateIncreasing: 
                    for(int i=_Generation.length-1; i>=0; i--)
                    {
                        for(int j=0; j<_ChromosomeSize; j++)
                        {
                            double mutate = Utility.GenerateRandom(0, 0, Utility.DoubleNormal);
                            if(mutate < AC._MutationRate)
                            {
                                _Generation[_SelectedElements[i]][j].Mutate(factors, AC._MutationScheme);
                            }
                        }
                        factors = factors * (AC._MutationVariationPercentage / 100);
                    }
                break;
            case Utility.MutateDecreasing: 
                    for(int i=0; i<_Generation.length; i++)
                    {
                        for(int j=0; j<_ChromosomeSize; j++)
                        {
                            double mutate = Utility.GenerateRandom(0, 0, Utility.DoubleNormal);
                            if(mutate < AC._MutationRate)
                            {
                                _Generation[_SelectedElements[i]][j].Mutate(factors, AC._MutationScheme);
                            }
                        }
                        factors = factors * (AC._MutationVariationPercentage / 100);
                    }
                break;
        }
    }
    
    public void ReplaceElement(int ID, PElement2[] PE)
    {
        for(int j=0; j<_Generation[ID].length; j++)
        {
            _Generation[ID][j].SetElement(PE[j]);
        }
    }
    
    public PElement2[] GetElement(int ID)
    {
        return _Generation[ID];
    }
    
/*    public void SetInput(int ID, double[] ChromosomeInput)
    {
        for(int i=0; i<_Generation[ID].length; i++)
        {
            _Generation[ID][i].SetInput(ChromosomeInput[i]);
            
        }
    }

    public void SetWeight(int ID, double[] ChromosomeWeight)
    {
        for(int i=0; i<_Generation[ID].length; i++)
        {
            _Generation[ID][i].SetInput(ChromosomeWeight[i]);
            
        }
    }
    
    public double[] GetInput(int ID)
    {
        double input[] = new double[_Generation[ID].length];
        for(int i=0; i<_Generation[ID].length; i++)
        {
            input[i] = _Generation[ID][i].GetInput();
        }
        
        return input;
    }


    public double[] GetWeights(int ID)
    {
        double weight[] = new double[_Generation[ID].length];
        for(int i=0; i<_Generation[ID].length; i++)
        {
            weight[i] = _Generation[ID][i].GetWeight();
        }
        
        return weight;
    }
   */ 
    
    public double[][] GetValues(int i)
    {
        double[][] values = new double[_ChromosomeSize][];
        
        for(int j=0; j<_ChromosomeSize; j++)
        {
            values[j] = _Generation[i][j].GetValues();
        }
        
        return values;
    }
    
    public double[][] GetWeights(int i)
    {
        double[][] weights = new double[_ChromosomeSize][];
        
        for(int j=0; j<_ChromosomeSize; j++)
        {
            weights[j] = _Generation[i][j].GetWeights();
        }
        
        return weights;
    }

}
