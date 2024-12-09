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
public abstract class AIAlgos {
    
    protected Logger _Log;
        
    public static final int FitnessProportionSelection=0, RankBasedSelection=1, TournamentSelection=2, RandomSelection=3;
    public static final int FitnessProportionSurvival=0, RankBasedSurvival=1, TournamentSurvival=2, TruncationSurvival=3;
    
    protected void Initialize(Logger log)
    {
        _Log = log;
    }
    
    protected void ComputeFitness(Population Generation, AlgoConfig AC)
    {
        double[] input = new double[Generation._PopulationSize * 2 * Generation._ChromosomeSize];
        int count = 0;
        //_Log.Debug("Generation._PopulationSize " + Generation._PopulationSize);
        for(int i=0; i<Generation._PopulationSize; i++)
        {
            double[] temp = Generation.GetInput(i);
            double[] temp2 = Generation.GetWeights(i);
            for(int j=0; j<Generation._ChromosomeSize; j++)
            {
                input[count] = temp[j]; count++;
                input[count] = temp2[j]; count++;
            }
        }
        
        Generation._GenerationFitness = AC.ComputeFitness(Generation._PopulationSize, Generation._ChromosomeSize, input);
        Generation._AverageFitness = Utility.Sum(Generation._GenerationFitness) / Generation._PopulationSize; 
    }
    

    protected void SelectParents(Population Generation, AlgoConfig AC)
    {
        
        switch(AC._ParentSelectionScheme){
            case FitnessProportionSelection: 
                FitnessSelection(Generation, AC);
                break;
            case RankBasedSelection: 
                RankSelection(Generation, AC);
                break;
            case TournamentSelection: 
                TournamentSelection(Generation, AC);
                break;
            case RandomSelection: 
                RandomSelection(Generation, AC);
                break;
        }
    }

    protected void Mutate(Population Generation, AlgoConfig AC)
    {
        Generation.Mutate(AC);
    }
        
    protected void GenerateClones(Population Generation, Population NextGeneration, AlgoConfig AC)
    {
        int Parent = 0, Offspring = 0;
        for(Offspring=0; Offspring<AC._NumberOfOffsprings; Offspring++, Parent++)
        {
            NextGeneration.CloneElement(Offspring, Generation._SelectedElements[Parent], Generation);
        }
        ComputeFitness(NextGeneration, AC);
    }
    
    protected void GenerateOffsprings(Population Generation, Population NextGeneration, AlgoConfig AC)
    {
        if(AC._NumberOfOffsprings*2 > Generation._PopulationSize)
        {
            GenerateClones(Generation, NextGeneration, AC);
        }
        else
        {
            int Parent = 0, Offspring = 0;
            for(Offspring=0; Offspring<AC._NumberOfOffsprings; Offspring++, Parent+=2)
            {
                NextGeneration.GenerateElement(Offspring, Generation._SelectedElements[Parent], 
                        Generation._SelectedElements[Parent+1], Generation);
                if(Parent>=Generation._PopulationSize-2)
                {
                    break;
                }
            }
        }
        _Log.Debug("NextGeneration._PopulationSize " + NextGeneration._PopulationSize);
        ComputeFitness(NextGeneration, AC);
    }
    
    protected void SelectSurvivors(Population Generation, Population NextGeneration, AlgoConfig AC)
    {
        switch(AC._SurvivorSelectionScheme){
            case FitnessProportionSurvival: 
                FitnessSurvival(Generation, NextGeneration, AC);
                break;
            case RankBasedSurvival: 
                RankSurvival(Generation, NextGeneration, AC);
                break;
            case TournamentSurvival: 
                TournamentSurvival(Generation, NextGeneration, AC);
                break;
            case TruncationSurvival: 
                Truncation(Generation, NextGeneration, AC);
                break;
        }
    }

    protected int[] FitnessSelection(double[] _GenerationFitness, double _FitnessSelectionParameter,
            double _AverageFitness, int _PopulationSize, int _ProblemType)
    {
        int[] _SelectedElements = new int[_PopulationSize];
        double[] GenerationProbability = new double[_PopulationSize];
        int[] ParentsRank = new int[_PopulationSize];
        for(int i=0; i<_PopulationSize; i++)
        {
            double t1 = _FitnessSelectionParameter * (_GenerationFitness[i] - _AverageFitness);
            double t2 = _AverageFitness - t1;
            GenerationProbability[i] = _GenerationFitness[i] - t2;
        }
        int Parent = 0;
        Utility.Sort(GenerationProbability, ParentsRank, _PopulationSize, _ProblemType);
        _Log.Debug(_GenerationFitness);
        _Log.Debug("Average Fitness " + _AverageFitness);
        _Log.Debug(GenerationProbability);
        _Log.Debug(ParentsRank);
        for(int i=0; i<_PopulationSize; i++)
        {
            long index = Math.round(Math.abs(GenerationProbability[ParentsRank[i]]));
            for(int j=0; j<index; j++, Parent++)
            {
                if(Parent >= _PopulationSize) return _SelectedElements;
                _SelectedElements[Parent] = ParentsRank[i];
            }
        }
        return _SelectedElements;
    }
    
    protected void FitnessSelection(Population Generation, AlgoConfig AC)
    {
        Generation._SelectedElements = FitnessSelection(Generation._GenerationFitness, 
                AC._FitnessSelectionParameter, Generation._AverageFitness, Generation._PopulationSize, AC._ProblemType);
        
/*        double[] GenerationProbability = new double[AC._NumberOfParents];
        int[] ParentsRank = new int[AC._NumberOfParents];

        for(int i=0; i<AC._NumberOfParents; i++)
        {
            double t1 = AC._FitnessSelectionParameter * (Generation._GenerationFitness[i] - Generation._AverageFitness);
            double t2 = Generation._AverageFitness - t1;
            GenerationProbability[i] = Generation._GenerationFitness[i] - t2;
            //GenerationProbability[i] = Generation._GenerationFitness[i] - Generation._AverageFitness;
            //GenerationProbability[i] = Generation._GenerationFitness[i] / Generation._AverageFitness;
        }
        int Parent = 0;
        Utility.Sort(GenerationProbability, ParentsRank, Generation._PopulationSize, AC._ProblemType);
        _Log.Debug(Generation._GenerationFitness);
        _Log.Debug("Average Fitness " + Generation._AverageFitness);
        _Log.Debug(GenerationProbability);
        _Log.Debug(ParentsRank);
        for(int i=0; i<Generation._PopulationSize; i++)
        {
            long index = Math.round(Math.abs(GenerationProbability[ParentsRank[i]]));
            for(int j=0; j<index; j++, Parent++)
            {
                if(Parent >= AC._NumberOfParents) return;
                Generation._SelectedElements[Parent] = ParentsRank[i];
            }
        }
        */
    }
    
    protected int[] RankSelection(double[] _GenerationFitness, 
            int _PopulationSize, int _ProblemType, int _MaxRank, double _RankSelectionParameter)
    {
        int[] _SelectedElements = new int[_PopulationSize];
        double[] GenerationProbability = new double[_PopulationSize];
        int[] ParentsRank = new int[_PopulationSize];
        double random = 0;
        int maxrank = _MaxRank, elementrank = _MaxRank; 
        Utility.Sort(_GenerationFitness, ParentsRank, _PopulationSize, _ProblemType);
        _Log.Debug(_GenerationFitness);
        _Log.Debug(ParentsRank);
        for(int i=0; i<_PopulationSize; i++)
        {
            //_Log.Debug(" (i % maxrank) " + (i % maxrank));
            if((i % maxrank == 0)) elementrank--;
            double t1 = 2-_RankSelectionParameter, t2 = t1 / maxrank, t3 = 2 * elementrank, 
                    t4 = _RankSelectionParameter-1,
                    t5=t3 * t4, t6 = maxrank-1, t7 = maxrank * t6, t8 = t5/t7, t9=t2+t8;
            GenerationProbability[ParentsRank[i]] = t9;
        }
        _Log.Debug(GenerationProbability);
        double sum = Utility.Sum(GenerationProbability);
        _Log.Debug("sum " + sum);
        for(int i=_PopulationSize - 1; i>=0; i--)
        {
            GenerationProbability[ParentsRank[i]] = GenerationProbability[ParentsRank[i]] / sum;
            if(i<_PopulationSize - 1)
            {
                GenerationProbability[ParentsRank[i]] += GenerationProbability[ParentsRank[i+1]];
            }
        }
        _Log.Debug(GenerationProbability);
        for(int i=0; i<_PopulationSize; i++)
        {
            random = Utility.GenerateRandom(0, 0, Utility.DoubleNormal);
            _Log.Debug("random " + random);
            int j;
            for(j=_PopulationSize-1; j>=0; j--)
            {
                if(random<GenerationProbability[ParentsRank[j]])
                {
                    _SelectedElements[i] = ParentsRank[j];
                    break;
                }
            }
            _Log.Debug("j " + j);
        }
        _Log.Debug(_SelectedElements);
        
        return _SelectedElements;
    }
    
    protected void RankSelection(Population Generation, AlgoConfig AC)
    {
        Generation._SelectedElements = RankSelection(Generation._GenerationFitness, 
                Generation._PopulationSize, AC._ProblemType, AC._MaxRank, AC._RankSelectionParameter);

/*        double[] GenerationProbability = new double[Generation._PopulationSize];
        int[] ParentsRank = new int[Generation._PopulationSize];
        double random = 0;
        int maxrank = AC._MaxRank, elementrank = AC._MaxRank; 
        Utility.Sort(Generation._GenerationFitness, ParentsRank, Generation._PopulationSize, AC._ProblemType);
        _Log.Debug(Generation._GenerationFitness);
        _Log.Debug(ParentsRank);
        for(int i=0; i<Generation._PopulationSize; i++)
        {
            //_Log.Debug(" (i % maxrank) " + (i % maxrank));
            if((i % maxrank == 0)) elementrank--;
            double t1 = 2-AC._RankSelectionParameter, t2 = t1 / maxrank, t3 = 2 * elementrank, 
                    t4 = AC._RankSelectionParameter-1,
                    t5=t3 * t4, t6 = maxrank-1, t7 = maxrank * t6, t8 = t5/t7, t9=t2+t8;
            GenerationProbability[ParentsRank[i]] = t9;//((2-AC._RankSelectionParameter)/maxrank) 
                    //+ (((2 * i)*(AC._RankSelectionParameter-1))/(maxrank * (maxrank-1)));
            /*_Log.Debug("(2-AC._RankSelectionParameter)/maxrank " + ((2-AC._RankSelectionParameter)/maxrank) 
                    + " ((2 * i)*(AC._RankSelectionParameter-1))/(maxrank * (maxrank-1)) " + 
                    (((2 * i)*(AC._RankSelectionParameter-1))/(maxrank * (maxrank-1))) +
                    " ((2-AC._RankSelectionParameter)/maxrank) \n" +
"                    + (((2 * i)*(AC._RankSelectionParameter-1))/(maxrank * (maxrank-1))) " + 
                    (((2-AC._RankSelectionParameter)/maxrank) 
                    + (((2 * i)*(AC._RankSelectionParameter-1))/(maxrank * (maxrank-1)))));*/
            //_Log.Debug("t1 "+t1+" t2 "+t2+" t3 "+t3+" t4 "+t4+" t5 "+t5+" t6 "+t6+" t7 "+t7+" t8 "+t8+" t9 "+t9);
/*        }
        _Log.Debug(GenerationProbability);
        double sum = Utility.Sum(GenerationProbability);
        _Log.Debug("sum " + sum);
        for(int i=Generation._PopulationSize - 1; i>=0; i--)
        {
            GenerationProbability[ParentsRank[i]] = GenerationProbability[ParentsRank[i]] / sum;
            if(i<Generation._PopulationSize - 1)
            {
                GenerationProbability[ParentsRank[i]] += GenerationProbability[ParentsRank[i+1]];
            }
        }
        _Log.Debug(GenerationProbability);
        for(int i=0; i<AC._NumberOfParents; i++)
        {
            random = Utility.GenerateRandom(0, 0, Utility.DoubleNormal);
            _Log.Debug("random " + random);
            int j;
            for(j=Generation._PopulationSize-1; j>=0; j--)
            {
                if(random<GenerationProbability[ParentsRank[j]])
                {
                    Generation._SelectedElements[i] = ParentsRank[j];
                    break;
                }
/*                if(random<GenerationProbability[ParentsRank[j]] && j==Generation._PopulationSize-1)
                {
                    Generation._SelectedElements[i] = ParentsRank[j];
                    break;
                }
                if(j>=1 && random>GenerationProbability[ParentsRank[j]] && random<GenerationProbability[ParentsRank[j-1]])
                {
                    Generation._SelectedElements[i] = ParentsRank[j-1];
                    break;
                }
*/        
/*            }
            _Log.Debug("j " + j);
            if(j < 0)
            {
                Generation._SelectedElements[i] = ParentsRank[0];
            }
        }
        _Log.Debug(Generation._SelectedElements);
*/

    }

    protected int[] TournamentSelection(double[] _GenerationFitness, int _PopulationSize, 
            int _ProblemType, int _MaxRank, double _RankSelectionParameter, int _TParentSampleSize)
    {
        int[] _SelectedElements = new int[_PopulationSize];
        int[] ParentSample = new int[_TParentSampleSize];
        int[] ParentsRank = new int[_TParentSampleSize];
        double random = 0;
        int maxrank = _MaxRank, elementrank = _MaxRank; 
        double[] SampleFitness = new double[_TParentSampleSize];
        for(int p=0; p<_PopulationSize; p++)
        {
            for(int i=0; i<_TParentSampleSize; i++)
            {
                ParentSample[i] = (int)Utility.GenerateRandom(0, _PopulationSize-1, Utility.IntWithRange);
                SampleFitness[i] = _GenerationFitness[ParentSample[i]];
            }
            _Log.Debug(ParentSample);
            _Log.Debug(SampleFitness);
            Utility.Sort(SampleFitness, ParentsRank, _TParentSampleSize, _ProblemType);
            _Log.Debug(ParentsRank);
            double[] ParentProbability = new double[_TParentSampleSize];
            elementrank = _MaxRank;
            for(int i=0; i<_TParentSampleSize; i++)
            {
                if((i % maxrank == 0)) elementrank--;
                double t1 = 2-_RankSelectionParameter, t2 = t1 / maxrank, t3 = 2 * elementrank, 
                        t4 = _RankSelectionParameter-1,
                        t5=t3 * t4, t6 = maxrank-1, t7 = maxrank * t6, t8 = t5/t7, t9=t2+t8;
                ParentProbability[ParentsRank[i]] = t9;
            }
            _Log.Debug(ParentProbability);
            double sum = Utility.Sum(ParentProbability);
            _Log.Debug("sum " + sum);
            for(int i=_TParentSampleSize - 1; i>=0; i--)
            {
                ParentProbability[ParentsRank[i]] = ParentProbability[ParentsRank[i]] / sum;
                if(i<_TParentSampleSize - 1)
                {
                    ParentProbability[ParentsRank[i]] += ParentProbability[ParentsRank[i+1]];
                }
            }
            _Log.Debug(ParentProbability);
            random = Utility.GenerateRandom(0, 0, Utility.DoubleNormal);
            _Log.Debug("random " + random);
            int j;
            for(j=_TParentSampleSize-1; j>=0; j--)
            {
                if(random<ParentProbability[ParentsRank[j]])
                {
                    _SelectedElements[p] = ParentSample[ParentsRank[j]];
                    break;
                }
            }
            _Log.Debug("j " + j);
        }
        _Log.Debug(_SelectedElements);
        
        return _SelectedElements;
    }
    
    protected void TournamentSelection(Population Generation, AlgoConfig AC)
    {
        Generation._SelectedElements = TournamentSelection(Generation._GenerationFitness,  
                Generation._PopulationSize, AC._ProblemType, AC._MaxRank, AC._RankSelectionParameter, AC._TParentSampleSize);
            
/*        int[] ParentSample = new int[AC._TParentSampleSize];
        int[] ParentsRank = new int[AC._TParentSampleSize];
        double random = 0;
        int maxrank = AC._MaxRank, elementrank = AC._MaxRank; 
        double[] SampleFitness = new double[AC._TParentSampleSize];
        for(int p=0; p<AC._NumberOfParents; p++)
        {
            for(int i=0; i<AC._TParentSampleSize; i++)
            {
                ParentSample[i] = (int)Utility.GenerateRandom(0, Generation._PopulationSize-1, Utility.IntWithRange);
                SampleFitness[i] = Generation._GenerationFitness[ParentSample[i]];
            }
            _Log.Debug(ParentSample);
            _Log.Debug(SampleFitness);
            Utility.Sort(SampleFitness, ParentsRank, AC._TParentSampleSize, AC._ProblemType);
            _Log.Debug(ParentsRank);
            double[] ParentProbability = new double[AC._TParentSampleSize];
            elementrank = AC._MaxRank;
            for(int i=0; i<AC._TParentSampleSize; i++)
            {
                //ParentProbability[ParentsRank[i]] = ((2-AC._RankSelectionParameter)/maxrank) 
                //        + (((2 * i)*(AC._RankSelectionParameter-1))/(maxrank * (maxrank-1)));
                if((i % maxrank == 0)) elementrank--;
                double t1 = 2-AC._RankSelectionParameter, t2 = t1 / maxrank, t3 = 2 * elementrank, 
                        t4 = AC._RankSelectionParameter-1,
                        t5=t3 * t4, t6 = maxrank-1, t7 = maxrank * t6, t8 = t5/t7, t9=t2+t8;
                ParentProbability[ParentsRank[i]] = t9;
            }
/*            random = Utility.GenerateRandom(0, 0, Utility.DoubleNormal);
            for(int j=AC._TParentSampleSize-1; j>=0; j--)
            {
                if(random<ParentProbability[ParentsRank[j]] && j==AC._TParentSampleSize-1)
                {
                    Generation._SelectedElements[p] = ParentsRank[j];
                    break;
                }
                if(j>=1 && random>ParentSample[ParentsRank[j]] && random<ParentSample[ParentsRank[j-1]])
                {
                    Generation._SelectedElements[p] = ParentsRank[j-1];
                    break;
                }
            }
        */
 /*           _Log.Debug(ParentProbability);
            double sum = Utility.Sum(ParentProbability);
            _Log.Debug("sum " + sum);
            for(int i=AC._TParentSampleSize - 1; i>=0; i--)
            {
                ParentProbability[ParentsRank[i]] = ParentProbability[ParentsRank[i]] / sum;
                if(i<AC._TParentSampleSize - 1)
                {
                    ParentProbability[ParentsRank[i]] += ParentProbability[ParentsRank[i+1]];
                }
            }
            _Log.Debug(ParentProbability);
            random = Utility.GenerateRandom(0, 0, Utility.DoubleNormal);
            _Log.Debug("random " + random);
            int j;
            for(j=AC._TParentSampleSize-1; j>=0; j--)
            {
                if(random<ParentProbability[ParentsRank[j]])
                {
                    Generation._SelectedElements[p] = ParentSample[ParentsRank[j]];
                    break;
                }
            }
            _Log.Debug("j " + j);
        }
        _Log.Debug(Generation._SelectedElements);
    */
    }
    
    protected void RandomSelection(Population Generation, AlgoConfig AC)
    {
        int randomm = 0;
        for(int i=0; i<Generation._PopulationSize; i++)
        {
            randomm = (int)Utility.GenerateRandom(0, Generation._PopulationSize, Utility.IntWithRange);
            Generation._SelectedElements[i] = randomm;
        }
    }
    protected void Truncation(Population Generation, Population NextGeneration, AlgoConfig AC)
    {
        Utility.Sort(Generation._GenerationFitness, Generation._SelectedElements, Generation._PopulationSize, 
                AC._ProblemType);
        Utility.Sort(NextGeneration._GenerationFitness, NextGeneration._SelectedElements, NextGeneration._PopulationSize, 
                AC._ProblemType);
        int Parent = Generation._PopulationSize - 1;
        for(int i=AC._NumberOfOffsprings-1; i>=0; i--)
        {
            Generation.ReplaceElement(Generation._SelectedElements[Parent], 
                    NextGeneration.GetElement(NextGeneration._SelectedElements[i]));
            Generation._GenerationFitness[Generation._SelectedElements[Parent]] = 
                    NextGeneration._GenerationFitness[NextGeneration._SelectedElements[i]];
            Parent--;
        }
    }
    
    protected void FitnessSurvival(Population Generation, Population NextGeneration, AlgoConfig AC)
    {
        int totalpopulation = Generation._PopulationSize + NextGeneration._PopulationSize;
        
        double[] totalfitness = new double[totalpopulation];
        int[] survivors = new int[totalpopulation];
        
        Utility.CopyArray(totalfitness, Generation._GenerationFitness, 0, 0, Generation._PopulationSize);
        Utility.CopyArray(totalfitness, NextGeneration._GenerationFitness, Generation._PopulationSize, 
                0, NextGeneration._PopulationSize);
        
        double average = Utility.Sum(totalfitness) / totalpopulation;
        
        survivors = FitnessSelection(totalfitness, AC._FitnessSelectionParameter, average, totalpopulation, AC._ProblemType);

        int j=Generation._PopulationSize;
        for(int i=0; i<Generation._PopulationSize; i++)
        {
            if(survivors[i] > Generation._PopulationSize)
            {
                for(; j<totalpopulation; j++)
                {
                    if(survivors[j] < Generation._PopulationSize)
                    {
                        Generation.ReplaceElement(survivors[j], 
                                NextGeneration.GetElement(survivors[i] - Generation._PopulationSize));
                        Generation._GenerationFitness[survivors[j]] = 
                                NextGeneration._GenerationFitness[survivors[i]  - Generation._PopulationSize];
                        break;
                    }
                }
            }
        }
/*        int Parent = Generation._PopulationSize - 1;
        for(int i=AC._NumberOfOffsprings-1; i>=0; i--)
        {
            if(NextGeneration._GenerationFitness[NextGeneration._SelectedElements[i]] > 
                    Generation._GenerationFitness[Generation._SelectedElements[Parent]])
            {
                Generation.ReplaceElement(Generation._SelectedElements[Parent], 
                        NextGeneration.GetElement(NextGeneration._SelectedElements[i]));
                Generation._GenerationFitness[Generation._SelectedElements[Parent]] = 
                        NextGeneration._GenerationFitness[NextGeneration._SelectedElements[i]];
                Parent--;
            }
        }
        */
    }
    protected void RankSurvival(Population Generation, Population NextGeneration, AlgoConfig AC)
    {
        int totalpopulation = Generation._PopulationSize + NextGeneration._PopulationSize;
        
        double[] totalfitness = new double[totalpopulation];
        int[] survivors = new int[totalpopulation];
        
        Utility.CopyArray(totalfitness, Generation._GenerationFitness, 0, 0, Generation._PopulationSize);
        Utility.CopyArray(totalfitness, NextGeneration._GenerationFitness, Generation._PopulationSize, 
                0, NextGeneration._PopulationSize);
        
        survivors = RankSelection(totalfitness, totalpopulation, AC._ProblemType, AC._MaxRank, 
                AC._RankSelectionParameter);

        int j=Generation._PopulationSize;
        for(int i=0; i<Generation._PopulationSize; i++)
        {
            if(survivors[i] > Generation._PopulationSize)
            {
                for(; j<totalpopulation; j++)
                {
                    if(survivors[j] < Generation._PopulationSize)
                    {
                        Generation.ReplaceElement(survivors[j], 
                                NextGeneration.GetElement(survivors[i] - Generation._PopulationSize));
                        Generation._GenerationFitness[survivors[j]] = 
                                NextGeneration._GenerationFitness[survivors[i]  - Generation._PopulationSize];
                        break;
                    }
                }
            }
        }
        
        /*        RankSelection(Generation, AC);
        RankSelection(NextGeneration, AC);

        int Parent = Generation._PopulationSize - 1;
        for(int i=AC._NumberOfOffsprings-1; i>=0; i--)
        {
            Generation.ReplaceElement(Generation._SelectedElements[Parent], 
                    NextGeneration.GetElement(NextGeneration._SelectedElements[i]));
            Generation._GenerationFitness[Generation._SelectedElements[Parent]] = 
                    NextGeneration._GenerationFitness[NextGeneration._SelectedElements[i]];
            Parent--;
        }
  */  
    }
    
    protected void TournamentSurvival(Population Generation, Population NextGeneration, AlgoConfig AC)
    {
        int totalpopulation = Generation._PopulationSize + NextGeneration._PopulationSize;
        
        double[] totalfitness = new double[totalpopulation];
        int[] survivors = new int[totalpopulation];
        
        Utility.CopyArray(totalfitness, Generation._GenerationFitness, 0, 0, Generation._PopulationSize);
        _Log.Debug(totalfitness);
        Utility.CopyArray(totalfitness, NextGeneration._GenerationFitness, Generation._PopulationSize, 
                0, NextGeneration._PopulationSize);
        _Log.Debug(totalfitness);

        _Log.Debug(Generation._GenerationFitness);
        _Log.Debug(NextGeneration._GenerationFitness);
        _Log.Debug(totalfitness);
        
        survivors = TournamentSelection(totalfitness, totalpopulation, AC._ProblemType, 
                AC._MaxRank, AC._RankSelectionParameter, AC._TParentSampleSize);

        _Log.Debug(survivors);
        
        int j=Generation._PopulationSize;
        for(int i=0; i<Generation._PopulationSize; i++)
        {
            if(survivors[i] > Generation._PopulationSize)
            {
                for(; j<totalpopulation; j++)
                {
                    if(survivors[j] < Generation._PopulationSize)
                    {
                        Generation.ReplaceElement(survivors[j], 
                                NextGeneration.GetElement(survivors[i] - Generation._PopulationSize));
                        Generation._GenerationFitness[survivors[j]] = 
                                NextGeneration._GenerationFitness[survivors[i]  - Generation._PopulationSize];
                        break;
                    }
                }
            }
        }

/*        TournamentSelection(Generation, AC);
        TournamentSelection(NextGeneration, AC);

        int Parent = Generation._PopulationSize - 1;
        for(int i=AC._NumberOfOffsprings-1; i>=0; i--)
        {
            Generation.ReplaceElement(Generation._SelectedElements[Parent], 
                    NextGeneration.GetElement(NextGeneration._SelectedElements[i]));
            Generation._GenerationFitness[Generation._SelectedElements[Parent]] = 
                    NextGeneration._GenerationFitness[NextGeneration._SelectedElements[i]];
            Parent--;
        }
        */
    }
    
    public void Execute()
    {
        
    }
    
    public double[] GetGenerationFitness()
    {
        return new double[1];
    }

}
