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
public class AIAlgos2 {
    
    protected Logger _Log;
        
    
    protected void Initialize(Logger log)
    {
        _Log = log;
    }
    
    protected void ComputeFitness(Population2 Generation, AlgoConfig2 AC)
    {
        Generation._GenerationFitness = AC.ComputeFitness(Generation);
        Generation._AverageFitness = Utility.Sum(Generation._GenerationFitness) / Generation._PopulationSize; 
    }
    

    protected void SelectParents(Population2 Generation, AlgoConfig2 AC)
    {
        Generation._SelectedElements = new int[AC._NumberOfOffsprings * 2];
        _Log.Debug("AC._NumberOfOffsprings " + AC._NumberOfOffsprings + 
                " Generation._SelectedElements.length " + Generation._SelectedElements.length);
        switch(AC._ParentSelectionScheme){
            case Utility.FitnessProportionSelection: 
                FitnessSelection(Generation, AC);
                break;
            case Utility.RankBasedSelection: 
                RankSelection(Generation, AC);
                break;
            case Utility.TournamentSelection: 
                TournamentSelection(Generation, AC);
                break;
            case Utility.RandomSelection: 
                RandomSelection(Generation, AC);
                break;
        }
    }

    protected void Mutate(Population2 Generation, AlgoConfig2 AC)
    {
        Generation.Mutate(AC);
    }
        
    protected void GenerateClones(Population2 Generation, Population2 NextGeneration, AlgoConfig2 AC)
    {
        int Parent = 0, Offspring = 0;
        for(Offspring=0; Offspring<AC._NumberOfOffsprings; Offspring++, Parent++)
        {
            NextGeneration.CloneElement(Offspring, Generation._SelectedElements[Parent], Generation, AC);
        }
        ComputeFitness(NextGeneration, AC);
    }
    
    protected void GenerateOffsprings(Population2 Generation, Population2 NextGeneration, AlgoConfig2 AC)
    {
        /*if(NextGeneration._PopulationSize*2 > Generation._PopulationSize)
        {
            _Log.Debug("generating clones");
            GenerateClones(Generation, NextGeneration, AC);
        }
        else
        {*/
            _Log.Debug("AC._NumberOfOffsprings " + AC._NumberOfOffsprings + 
                    " Generation._SelectedElements.length " + Generation._SelectedElements.length);
            int Parent = 0, Offspring = 0;
            for(Offspring=0; Offspring<AC._NumberOfOffsprings; Offspring++)
            {
                double crossover = Utility.GenerateRandom(0, 0, Utility.DoubleNormal);
                if(crossover < AC._CrossOverRate)
                {
                    NextGeneration.GenerateElement(Offspring, Generation._SelectedElements[Parent], 
                            Generation._SelectedElements[Parent+1], Generation, AC);
                    Parent+=2;
                }
                else
                {
                    NextGeneration.GenerateRandom(Offspring);
                }
                
                /*if(Parent>=Generation._PopulationSize-2)
                {
                    break;
                }*/
            }
            
        //}
        _Log.Debug("NextGeneration._PopulationSize " + NextGeneration._PopulationSize);
        //ComputeFitness(NextGeneration, AC);
    }
    
    protected void SelectSurvivors(Population2 Generation, Population2 NextGeneration, AlgoConfig2 AC)
    {
        switch(AC._SurvivorSelectionScheme){
            case Utility.FitnessProportionSurvival: 
                FitnessSurvival(Generation, NextGeneration, AC);
                break;
            case Utility.RankBasedSurvival: 
                RankSurvival(Generation, NextGeneration, AC);
                break;
            case Utility.TournamentSurvival: 
                TournamentSurvival(Generation, NextGeneration, AC);
                break;
            case Utility.TruncationSurvival: 
                Truncation(Generation, NextGeneration, AC);
                break;
        }
    }

    protected int[] FitnessSelection(double[] _GenerationFitness, double _FitnessSelectionParameter,
            double _AverageFitness, int _PopulationSize, int _ProblemType, int SelectedElementsLength)
    {
        int[] _SelectedElements = new int[SelectedElementsLength];
        double[] GenerationProbability = new double[_PopulationSize];
        int[] ParentsRank = new int[_PopulationSize];
        for(int i=0; i<_PopulationSize; i++)
        {
            double t1 = _FitnessSelectionParameter * (_GenerationFitness[i] - _AverageFitness);
            double t2 = _AverageFitness - t1;
            _Log.Debug("t1 " + t1 + " t2 " + t2 + " _FitnessSelectionParameter " + _FitnessSelectionParameter);
            GenerationProbability[i] = _GenerationFitness[i] - t2;
        }
        int Parent = 0;
        double sum = Utility.Sum(GenerationProbability);
        for(int i=0; i<_PopulationSize; i++)
        {
            GenerationProbability[i] = GenerationProbability[i] / sum;
        }
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
                if(Parent >= _SelectedElements.length) return _SelectedElements;
                _SelectedElements[Parent] = ParentsRank[i];
            }
        }
        return _SelectedElements;
    }
    
    protected void FitnessSelection(Population2 Generation, AlgoConfig2 AC)
    {
        Generation._SelectedElements = FitnessSelection(Generation._GenerationFitness, 
                AC._FitnessSelectionParameter, Generation._AverageFitness, Generation._PopulationSize, AC._ProblemType, 
                Generation._SelectedElements.length);
        
    }
    
    protected int[] RankSelection(double[] _GenerationFitness, 
            int _PopulationSize, int _ProblemType, int _MaxRank, double _RankSelectionParameter, int SelectedElementsLength)
    {
        int[] _SelectedElements = new int[SelectedElementsLength];
        double[] GenerationProbability = new double[_PopulationSize];
        int[] ParentsRank = new int[_PopulationSize];
        double random = 0;
        int maxrank = _MaxRank, elementrank = _MaxRank; 
        Utility.Sort(_GenerationFitness, ParentsRank, _PopulationSize, _ProblemType);
        _Log.Debug(_GenerationFitness);
        _Log.Debug(ParentsRank);
        for(int i=0; i<_PopulationSize; i++)
        {
            double t1 = 2-_RankSelectionParameter, t2 = t1 / maxrank, t3 = 2 * elementrank, 
                    t4 = _RankSelectionParameter-1,
                    t5=t3 * t4, t6 = maxrank-1, t7 = maxrank * t6, t8 = t5/t7, t9=t2+t8;
            GenerationProbability[ParentsRank[i]] = t9;
            if((i % maxrank == 0)) elementrank--;
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
        for(int i=0; i<_SelectedElements.length; i++)
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
    
    protected void RankSelection(Population2 Generation, AlgoConfig2 AC)
    {
        Generation._SelectedElements = RankSelection(Generation._GenerationFitness, 
                Generation._PopulationSize, AC._ProblemType, AC._MaxRank, AC._RankSelectionParameter, 
                Generation._SelectedElements.length);

    }

    protected int[] TournamentSelection(double[] _GenerationFitness, int _PopulationSize, 
            int _ProblemType, int _MaxRank, double _RankSelectionParameter, int _TParentSampleSize, 
            int SelectedElementsLength)
    {
        int[] _SelectedElements = new int[SelectedElementsLength];
        int[] ParentSample = new int[_TParentSampleSize];
        int[] ParentsRank = new int[_TParentSampleSize];
        double random = 0;
        int maxrank = _MaxRank, elementrank = _MaxRank; 
        double[] SampleFitness = new double[_TParentSampleSize];
        for(int p=0; p<_SelectedElements.length; p++)
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
    
    protected void TournamentSelection(Population2 Generation, AlgoConfig2 AC)
    {
        Generation._SelectedElements = TournamentSelection(Generation._GenerationFitness,  
                Generation._PopulationSize, AC._ProblemType, AC._MaxRank, AC._RankSelectionParameter, 
                AC._TParentSampleSize, Generation._SelectedElements.length);
            
    }
    
    protected void RandomSelection(Population2 Generation, AlgoConfig2 AC)
    {
        int randomm = 0;
        for(int i=0; i<Generation._PopulationSize; i++)
        {
            randomm = (int)Utility.GenerateRandom(0, Generation._PopulationSize, Utility.IntWithRange);
            Generation._SelectedElements[i] = randomm;
        }
    }
    
    protected void Truncation(Population2 Generation, Population2 NextGeneration, AlgoConfig2 AC)
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
    
    protected void FitnessSurvival(Population2 Generation, Population2 NextGeneration, AlgoConfig2 AC)
    {
        int totalpopulation = Generation._PopulationSize + NextGeneration._PopulationSize;
        
        double[] totalfitness = new double[totalpopulation];
        int[] survivors = new int[totalpopulation];
        
        Utility.CopyArray(totalfitness, Generation._GenerationFitness, 0, 0, Generation._PopulationSize);
        Utility.CopyArray(totalfitness, NextGeneration._GenerationFitness, Generation._PopulationSize, 
                0, NextGeneration._PopulationSize);
        
        double average = Utility.Sum(totalfitness) / totalpopulation;
        
        survivors = FitnessSelection(totalfitness, AC._FitnessSelectionParameter, average, totalpopulation, 
                AC._ProblemType, survivors.length);

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
                        //Generation._GenerationFitness[survivors[j]] = 
                        //        NextGeneration._GenerationFitness[survivors[i]  - Generation._PopulationSize];
                        break;
                    }
                }
            }
        }
    }
    
    protected void RankSurvival(Population2 Generation, Population2 NextGeneration, AlgoConfig2 AC)
    {
        _Log.Debug("survival --------------------");
        int totalpopulation = Generation._PopulationSize + NextGeneration._PopulationSize;
        
        double[] totalfitness = new double[totalpopulation];
        int[] survivors = new int[totalpopulation];
        
        Utility.CopyArray(totalfitness, Generation._GenerationFitness, 0, 0, Generation._PopulationSize);
        Utility.CopyArray(totalfitness, NextGeneration._GenerationFitness, Generation._PopulationSize, 
                0, NextGeneration._PopulationSize);
        
        _Log.Debug(totalfitness);
        
        survivors = RankSelection(totalfitness, totalpopulation, AC._ProblemType, AC._MaxRank, 
                AC._RankSelectionParameter, survivors.length);

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
                        //Generation._GenerationFitness[survivors[j]] = 
                        //        NextGeneration._GenerationFitness[survivors[i]  - Generation._PopulationSize];
                        break;
                    }
                }
            }
        }
        
    }
    
    protected void TournamentSurvival(Population2 Generation, Population2 NextGeneration, AlgoConfig2 AC)
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
                AC._MaxRank, AC._RankSelectionParameter, AC._TParentSampleSize, survivors.length);

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

    }
    
}
