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
public abstract class Population {
    protected PElement[][] _Generation;
    public double _AverageFitness;
    public double[] _GenerationFitness; 
    public int[] _SelectedElements;
    public int _PopulationSize, _ChromosomeSize;
    
    public static final int MutateEvenly=0, MutateDecreasing=1, MutateIncreasing=2; 
    
    Logger _Log;
    
    public void Initialize(Logger log, AlgoConfig AC, int PopulationSize, int ChromosomeSize)
    {
        _Log = log;
        
        _PopulationSize = PopulationSize;
        _ChromosomeSize = ChromosomeSize;
        _GenerationFitness = new double[_PopulationSize];
        _SelectedElements = new int[_PopulationSize];
        /*for(int i=0; i<PopulationSize; i++)
        {
            for(int j=0; j<ChromosomeSize; j++)
            {
                _Generation[i][j].InitializeElement();
            }
        }*/
    }

/*    public void Compute(double[] Fitness)
    {
        for(int i=0; i<_Generation.length; i++)
        {
            for(int j=0; j<_Generation[i].length; j++)
            {
                Fitness[i] += _Generation[i][j].Compute();
            }
        }
    }
*/    
    public void GenerateElement(int ID, int P1, int P2, Population P)
    {
        PElement[] PE1 = P._Generation[P1];
        PElement[] PE2 = P._Generation[P2];

        for(int j=0; j<_Generation[ID].length; j++)
        {
            _Generation[ID][j].CrossOver(PE1[j], PE2[j]);
        }
        //double[] temp = new double[2];
        //_Generation[ID].Mutate(temp);
    }

    public void CloneElement(int ID, int PID, Population P)
    {
        PElement[] PE = P._Generation[PID];

        for(int j=0; j<_Generation[ID].length; j++)
        {
            _Generation[ID][j].Clone(PE[j]);
        }
        //double[] temp = new double[2];
        //_Generation[ID].Mutate(temp);
    }
    
    public void Mutate(AlgoConfig AC)
    {
        double[] factors = new double[2];
        factors[0] = AC._DegreeOfMutation;
        switch(AC._FitnessProportionate)
        {
            case MutateEvenly: 
                    for(int i=0; i<_Generation.length; i++)
                    {
                        for(int j=0; j<_ChromosomeSize; j++)
                        {
                            _Generation[_SelectedElements[i]][j].Mutate(factors);
                        }
                    }
                break;
            case MutateIncreasing: 
                    for(int i=_Generation.length-1; i>=0; i--)
                    {
                        for(int j=0; j<_ChromosomeSize; j++)
                        {
                            _Generation[_SelectedElements[i]][j].Mutate(factors);
                        }
                        factors[0] = factors[0] * (AC._MutationVariationPercentage / 100);
                    }
                break;
            case MutateDecreasing: 
                    for(int i=0; i<_Generation.length; i++)
                    {
                        for(int j=0; j<_ChromosomeSize; j++)
                        {
                            _Generation[_SelectedElements[i]][j].Mutate(factors);
                        }
                        factors[0] = factors[0] * (AC._MutationVariationPercentage / 100);
                    }
                break;
        }
    }
    
    public void ReplaceElement(int ID, PElement[] PE)
    {
        for(int j=0; j<_Generation[ID].length; j++)
        {
            _Generation[ID][j].SetElement(PE[j]);
        }
    }
    public PElement[] GetElement(int ID)
    {
        return _Generation[ID];
    }
    
/*    protected void CopyPopulation(Population P)
    {
        for(int i=0; i<_Generation.length; i++)
        {
            for(int j=0; j<_Generation[i].length; j++)
            {
                _Generation[i][j].SetElement(P.GetElement(i)[j]);
            }
        }
    }
    protected void CopyPopulation(Population P, int srcstartindex, int desstartindex, int count)
    {
        /*System.out.println("_Generation.length " + _Generation.length);
        System.out.println("P._Generation.length " + P._Generation.length);
        System.out.println("srcstartindex " + srcstartindex);
        System.out.println("desstartindex " + desstartindex);
        System.out.println("count " + count);
        */
/*        for(int i=0; i<count; i++, srcstartindex++, desstartindex++)
        {
            for(int j=0; j<_Generation[desstartindex].length; j++)
            {
                _Generation[desstartindex][j].SetElement(P.GetElement(srcstartindex)[j]);
            }
        }
    }
*/    
    public void SetInput(int ID, double[] ChromosomeInput)
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
}
