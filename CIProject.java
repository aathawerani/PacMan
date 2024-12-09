/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package ciproject;

import java.io.*;
import ciproject.AILib.*;
import ciproject.BaseCode.*;
import ciproject.Problems.*;
import ciproject.Algos.*;

/**
 *
 * @author aaht14
 */
public class CIProject {

    static double[][] GenerationResults;
    static double[][] RunsResults;
    static double[][] RunsResults2;

    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) {

        //Assignment1();
        Assignment3();
    }
    
    
    public static void Assignment1()
    {
        int PopulationSize = 10, Offsprings = 10, Iterations = 40, Runs = 10;
        Logger log = new Logger(Logger.LogDebug);
        double minx, maxx, miny, maxy, stepx, stepy;
        P1PopulationGenerator P1PG;
        EAProblem1 EAP1; EAProblem2 EAP2; EAProblem3 EAP3; 
        EAlgorithm Problem1, Problem2, Problem3;

        //FPS and Truncation
        
/*        
        minx = -5; maxx = 0; miny = 0; maxy = 5; stepx = 0.25; stepy = 0.25;
        P1PG = new P1PopulationGenerator(minx, maxx, miny, maxy, stepx, stepy);
        EAP1 = new EAProblem1(); EAP1.Initialize(log);
        EAP1._ChromosomeLength=1; EAP1._NumberOfOffsprings=10; EAP1._PopulationCount=10; 
        EAP1._ProblemType=Utility.Minimization; EAP1._DegreeOfMutation=1; 
        EAP1._FitnessProportionate=Population.MutateEvenly;
        EAP1._ParentSelectionScheme=AIAlgos.FitnessProportionSelection; EAP1._FitnessSelectionParameter=2;
        EAP1._SurvivorSelectionScheme=AIAlgos.TruncationSurvival;  
        Problem1 = new EAlgorithm(log, P1PG, EAP1);
        Execute(Problem1, EAP1, Runs, Iterations);

        minx = -2; maxx = 2; miny = -1; maxy = 3; stepx = 0.25; stepy = 0.25;
        P1PG = new P1PopulationGenerator(minx, maxx, miny, maxy, stepx, stepy);
        EAP2 = new EAProblem2(); EAP2.Initialize(log);
        EAP2._ChromosomeLength=1; EAP2._NumberOfOffsprings=10; EAP2._PopulationCount=10; 
        EAP2._ProblemType=Utility.Maximization; EAP2._DegreeOfMutation=1; 
        EAP2._FitnessProportionate=Population.MutateEvenly;
        EAP2._ParentSelectionScheme=AIAlgos.FitnessProportionSelection; EAP2._FitnessSelectionParameter=2;
        EAP2._SurvivorSelectionScheme=AIAlgos.TruncationSurvival;  
        Problem2 = new EAlgorithm(log, P1PG, EAP2);
        Execute(Problem2, EAP2, Runs, Iterations);
        

        minx = Utility.DoubleMin; maxx = Utility.DoubleMax; miny = Utility.DoubleMin; maxy = Utility.DoubleMax; 
        stepx = 0.25; stepy = 0.25;
        P1PG = new P1PopulationGenerator(minx, maxx, miny, maxy, stepx, stepy);
        EAP3 = new EAProblem3(); EAP3.Initialize(log);
        EAP3._ChromosomeLength=1; EAP3._NumberOfOffsprings=10; EAP3._PopulationCount=10; 
        EAP3._ProblemType=Utility.Maximization; EAP3._DegreeOfMutation=1; 
        EAP3._FitnessProportionate=Population.MutateEvenly;
        EAP3._ParentSelectionScheme=AIAlgos.FitnessProportionSelection; EAP3._FitnessSelectionParameter=2;
        EAP3._SurvivorSelectionScheme=AIAlgos.TruncationSurvival;  
        Problem3 = new EAlgorithm(log, P1PG, EAP3);
        Execute(Problem3, EAP3, Runs, Iterations);
    
        //RBS and Truncation
        
        minx = -5; maxx = 0; miny = 0; maxy = 5; stepx = 0.25; stepy = 0.25;
        P1PG = new P1PopulationGenerator(minx, maxx, miny, maxy, stepx, stepy);
        EAP1 = new EAProblem1(); EAP1.Initialize(log);
        EAP1._ChromosomeLength=1; EAP1._NumberOfOffsprings=10; EAP1._PopulationCount=10; 
        EAP1._ProblemType=Utility.Minimization; EAP1._DegreeOfMutation=1; 
        EAP1._FitnessProportionate=Population.MutateEvenly;
        EAP1._ParentSelectionScheme=AIAlgos.RankBasedSelection; 
        EAP1._SurvivorSelectionScheme=AIAlgos.TruncationSurvival;  
        EAP1._RankSelectionParameter=1.5; EAP1._MaxRank=3; EAP1._FitnessSelectionParameter=2;
        Problem1 = new EAlgorithm(log, P1PG, EAP1);
        Execute(Problem1, EAP1, Runs, Iterations);

        
        minx = -2; maxx = 2; miny = -1; maxy = 3; stepx = 0.25; stepy = 0.25;
        P1PG = new P1PopulationGenerator(minx, maxx, miny, maxy, stepx, stepy);
        EAP2 = new EAProblem2(); EAP2.Initialize(log);
        EAP2._ChromosomeLength=1; EAP2._NumberOfOffsprings=10; EAP2._PopulationCount=10; 
        EAP2._ProblemType=Utility.Maximization; EAP2._DegreeOfMutation=1; 
        EAP2._FitnessProportionate=Population.MutateEvenly;
        EAP2._ParentSelectionScheme=AIAlgos.RankBasedSelection; 
        EAP2._SurvivorSelectionScheme=AIAlgos.TruncationSurvival;  
        EAP2._RankSelectionParameter=1.5; EAP2._MaxRank=3; EAP2._FitnessSelectionParameter=2;
        Problem2 = new EAlgorithm(log, P1PG, EAP2);
        Execute(Problem2, EAP2, Runs, Iterations);
        

        minx = Utility.DoubleMin; maxx = Utility.DoubleMax; miny = Utility.DoubleMin; maxy = Utility.DoubleMax; 
        stepx = 0.25; stepy = 0.25;
        P1PG = new P1PopulationGenerator(minx, maxx, miny, maxy, stepx, stepy);
        EAP3 = new EAProblem3(); EAP3.Initialize(log);
        EAP3._ChromosomeLength=1; EAP3._NumberOfOffsprings=10; EAP3._PopulationCount=10; 
        EAP3._ProblemType=Utility.Maximization; EAP3._DegreeOfMutation=1; 
        EAP3._FitnessProportionate=Population.MutateEvenly;
        EAP3._ParentSelectionScheme=AIAlgos.FitnessProportionSelection; 
        EAP3._SurvivorSelectionScheme=AIAlgos.TruncationSurvival;  
        Problem3 = new EAlgorithm(log, P1PG, EAP3);
        Execute(Problem3, EAP3, Runs, Iterations);

        
        //BT and Truncation
        minx = -5; maxx = 0; miny = 0; maxy = 5; stepx = 0.25; stepy = 0.25;
        P1PG = new P1PopulationGenerator(minx, maxx, miny, maxy, stepx, stepy);
        EAP1 = new EAProblem1(); EAP1.Initialize(log);
        EAP1._ChromosomeLength=1; EAP1._NumberOfOffsprings=10; EAP1._PopulationCount=10; 
        EAP1._ProblemType=Utility.Minimization; EAP1._DegreeOfMutation=1; 
        EAP1._FitnessProportionate=Population.MutateEvenly;
        EAP1._ParentSelectionScheme=AIAlgos.TournamentSelection; 
        EAP1._SurvivorSelectionScheme=AIAlgos.TruncationSurvival;
        EAP1._TParentSampleSize=2;EAP1._MaxRank=2; EAP1._RankSelectionParameter=1.5;
        Problem1 = new EAlgorithm(log, P1PG, EAP1);
        Execute(Problem1, EAP1, Runs, Iterations);

        
        minx = -2; maxx = 2; miny = -1; maxy = 3; stepx = 0.25; stepy = 0.25;
        P1PG = new P1PopulationGenerator(minx, maxx, miny, maxy, stepx, stepy);
        EAP2 = new EAProblem2(); EAP2.Initialize(log);
        EAP2._ChromosomeLength=1; EAP2._NumberOfOffsprings=10; EAP2._PopulationCount=10; 
        EAP2._ProblemType=Utility.Maximization; EAP2._DegreeOfMutation=1; 
        EAP2._FitnessProportionate=Population.MutateEvenly;
        EAP2._ParentSelectionScheme=AIAlgos.TournamentSelection; 
        EAP2._SurvivorSelectionScheme=AIAlgos.TruncationSurvival;
        EAP2._TParentSampleSize=2;EAP2._MaxRank=2; EAP2._RankSelectionParameter=1.5;
        Problem2 = new EAlgorithm(log, P1PG, EAP2);
        Execute(Problem2, EAP2, Runs, Iterations);
        

        minx = Utility.DoubleMin; maxx = Utility.DoubleMax; miny = Utility.DoubleMin; maxy = Utility.DoubleMax; 
        stepx = 0.25; stepy = 0.25;
        P1PG = new P1PopulationGenerator(minx, maxx, miny, maxy, stepx, stepy);
        EAP3 = new EAProblem3(); EAP3.Initialize(log);
        EAP3._ChromosomeLength=1; EAP3._NumberOfOffsprings=10; EAP3._PopulationCount=10; 
        EAP3._ProblemType=Utility.Maximization; EAP3._DegreeOfMutation=1; 
        EAP3._FitnessProportionate=Population.MutateEvenly;
        EAP3._ParentSelectionScheme=AIAlgos.TournamentSelection; 
        EAP3._SurvivorSelectionScheme=AIAlgos.TruncationSurvival;
        EAP3._TParentSampleSize=2;EAP3._MaxRank=2; EAP3._RankSelectionParameter=1.5;
        Problem3 = new EAlgorithm(log, P1PG, EAP3);
        Execute(Problem3, EAP3, Runs, Iterations);
        
        
        //FPS and BT

        minx = -5; maxx = 0; miny = 0; maxy = 5; stepx = 0.25; stepy = 0.25;
        P1PG = new P1PopulationGenerator(minx, maxx, miny, maxy, stepx, stepy);
        EAP1 = new EAProblem1(); EAP1.Initialize(log);
        EAP1._ChromosomeLength=1; EAP1._NumberOfOffsprings=10; EAP1._PopulationCount=10; 
        EAP1._ProblemType=Utility.Minimization; EAP1._DegreeOfMutation=1; 
        EAP1._FitnessProportionate=Population.MutateEvenly;
        EAP1._ParentSelectionScheme=AIAlgos.FitnessProportionSelection; EAP1._FitnessSelectionParameter=2;
        EAP1._SurvivorSelectionScheme=AIAlgos.TournamentSurvival;  
        EAP1._TParentSampleSize=2;EAP1._MaxRank=2; EAP1._RankSelectionParameter=1.5;
        Problem1 = new EAlgorithm(log, P1PG, EAP1);
        Execute(Problem1, EAP1, Runs, Iterations);


        minx = -2; maxx = 2; miny = -1; maxy = 3; stepx = 0.25; stepy = 0.25;
        P1PG = new P1PopulationGenerator(minx, maxx, miny, maxy, stepx, stepy);
        EAP2 = new EAProblem2(); EAP2.Initialize(log);
        EAP2._ChromosomeLength=1; EAP2._NumberOfOffsprings=10; EAP2._PopulationCount=10; 
        EAP2._ProblemType=Utility.Maximization; EAP2._DegreeOfMutation=1; 
        EAP2._FitnessProportionate=Population.MutateEvenly;
        EAP2._ParentSelectionScheme=AIAlgos.FitnessProportionSelection; EAP2._FitnessSelectionParameter=2;
        EAP2._SurvivorSelectionScheme=AIAlgos.TournamentSurvival;  
        EAP2._TParentSampleSize=2;EAP2._MaxRank=2; EAP2._RankSelectionParameter=1.5;
        Problem2 = new EAlgorithm(log, P1PG, EAP2);
        Execute(Problem2, EAP2, Runs, Iterations);



        minx = Utility.DoubleMin; maxx = Utility.DoubleMax; miny = Utility.DoubleMin; maxy = Utility.DoubleMax; 
        stepx = 0.25; stepy = 0.25;
        P1PG = new P1PopulationGenerator(minx, maxx, miny, maxy, stepx, stepy);
        EAP3 = new EAProblem3(); EAP3.Initialize(log);
        EAP3._ChromosomeLength=1; EAP3._NumberOfOffsprings=10; EAP3._PopulationCount=10; 
        EAP3._ProblemType=Utility.Maximization; EAP3._DegreeOfMutation=1; 
        EAP3._FitnessProportionate=Population.MutateEvenly;
        EAP3._ParentSelectionScheme=AIAlgos.FitnessProportionSelection; EAP3._FitnessSelectionParameter=2;
        EAP3._SurvivorSelectionScheme=AIAlgos.TournamentSurvival;  
        EAP3._TParentSampleSize=2;EAP3._MaxRank=2; EAP3._RankSelectionParameter=1.5;
        Problem3 = new EAlgorithm(log, P1PG, EAP3);
        Execute(Problem3, EAP3, Runs, Iterations);
    
        //RBS and BT

        minx = -5; maxx = 0; miny = 0; maxy = 5; stepx = 0.25; stepy = 0.25;
        P1PG = new P1PopulationGenerator(minx, maxx, miny, maxy, stepx, stepy);
        EAP1 = new EAProblem1(); EAP1.Initialize(log);
        EAP1._ChromosomeLength=1; EAP1._NumberOfOffsprings=10; EAP1._PopulationCount=10; 
        EAP1._ProblemType=Utility.Minimization; EAP1._DegreeOfMutation=1; 
        EAP1._FitnessProportionate=Population.MutateEvenly;
        EAP1._ParentSelectionScheme=AIAlgos.RankBasedSelection; 
        EAP1._RankSelectionParameter=1.5; EAP1._MaxRank=3; EAP1._FitnessSelectionParameter=2;
        EAP1._SurvivorSelectionScheme=AIAlgos.TournamentSurvival;  EAP1._TParentSampleSize=2;
        Problem1 = new EAlgorithm(log, P1PG, EAP1);
        Execute(Problem1, EAP1, Runs, Iterations);

        
        minx = -2; maxx = 2; miny = -1; maxy = 3; stepx = 0.25; stepy = 0.25;
        P1PG = new P1PopulationGenerator(minx, maxx, miny, maxy, stepx, stepy);
        EAP2 = new EAProblem2(); EAP2.Initialize(log);
        EAP2._ChromosomeLength=1; EAP2._NumberOfOffsprings=10; EAP2._PopulationCount=10; 
        EAP2._ProblemType=Utility.Maximization; EAP2._DegreeOfMutation=1; 
        EAP2._FitnessProportionate=Population.MutateEvenly;
        EAP2._ParentSelectionScheme=AIAlgos.RankBasedSelection; 
        EAP2._RankSelectionParameter=1.5; EAP2._MaxRank=3; EAP2._FitnessSelectionParameter=2;
        EAP2._SurvivorSelectionScheme=AIAlgos.TournamentSurvival;  EAP2._TParentSampleSize=2;
        Problem2 = new EAlgorithm(log, P1PG, EAP2);
        Execute(Problem2, EAP2, Runs, Iterations);
        

        minx = Utility.DoubleMin; maxx = Utility.DoubleMax; miny = Utility.DoubleMin; maxy = Utility.DoubleMax; 
        stepx = 0.25; stepy = 0.25;
        P1PG = new P1PopulationGenerator(minx, maxx, miny, maxy, stepx, stepy);
        EAP3 = new EAProblem3(); EAP3.Initialize(log);
        EAP3._ChromosomeLength=1; EAP3._NumberOfOffsprings=10; EAP3._PopulationCount=10; 
        EAP3._ProblemType=Utility.Maximization; EAP3._DegreeOfMutation=1; 
        EAP3._FitnessProportionate=Population.MutateEvenly;
        EAP3._ParentSelectionScheme=AIAlgos.RankBasedSelection; 
        EAP3._RankSelectionParameter=1.5; EAP3._MaxRank=3; EAP3._FitnessSelectionParameter=2;
        EAP3._SurvivorSelectionScheme=AIAlgos.TournamentSurvival;  EAP3._TParentSampleSize=2;
        Problem3 = new EAlgorithm(log, P1PG, EAP3);
        Execute(Problem3, EAP3, Runs, Iterations);

        //BT and BT
        
        minx = -5; maxx = 0; miny = 0; maxy = 5; stepx = 0.25; stepy = 0.25;
        P1PG = new P1PopulationGenerator(minx, maxx, miny, maxy, stepx, stepy);
        EAP1 = new EAProblem1(); EAP1.Initialize(log);
        EAP1._ChromosomeLength=1; EAP1._NumberOfOffsprings=10; EAP1._PopulationCount=10; 
        EAP1._ProblemType=Utility.Minimization; EAP1._DegreeOfMutation=1; 
        EAP1._FitnessProportionate=Population.MutateEvenly;
        EAP1._ParentSelectionScheme=AIAlgos.TournamentSelection; 
        EAP1._RankSelectionParameter=1.5; EAP1._MaxRank=3; EAP1._FitnessSelectionParameter=2;
        EAP1._SurvivorSelectionScheme=AIAlgos.TournamentSurvival;  EAP1._TParentSampleSize=2;
        Problem1 = new EAlgorithm(log, P1PG, EAP1);
        Execute(Problem1, EAP1, Runs, Iterations);

        
        minx = -2; maxx = 2; miny = -1; maxy = 3; stepx = 0.25; stepy = 0.25;
        P1PG = new P1PopulationGenerator(minx, maxx, miny, maxy, stepx, stepy);
        EAP2 = new EAProblem2(); EAP2.Initialize(log);
        EAP2._ChromosomeLength=1; EAP2._NumberOfOffsprings=10; EAP2._PopulationCount=10; 
        EAP2._ProblemType=Utility.Maximization; EAP2._DegreeOfMutation=1; 
        EAP2._FitnessProportionate=Population.MutateEvenly;
        EAP2._ParentSelectionScheme=AIAlgos.TournamentSelection; 
        EAP2._RankSelectionParameter=1.5; EAP2._MaxRank=3; EAP2._FitnessSelectionParameter=2;
        EAP2._SurvivorSelectionScheme=AIAlgos.TournamentSurvival;  EAP2._TParentSampleSize=2;
        Problem2 = new EAlgorithm(log, P1PG, EAP2);
        Execute(Problem2, EAP2, Runs, Iterations);
        

        minx = Utility.DoubleMin; maxx = Utility.DoubleMax; miny = Utility.DoubleMin; maxy = Utility.DoubleMax; 
        stepx = 0.25; stepy = 0.25;
        P1PG = new P1PopulationGenerator(minx, maxx, miny, maxy, stepx, stepy);
        EAP3 = new EAProblem3(); EAP3.Initialize(log);
        EAP3._ChromosomeLength=1; EAP3._NumberOfOffsprings=10; EAP3._PopulationCount=10; 
        EAP3._ProblemType=Utility.Maximization; EAP3._DegreeOfMutation=1; 
        EAP3._FitnessProportionate=Population.MutateEvenly;
        EAP3._ParentSelectionScheme=AIAlgos.RankBasedSelection; 
        EAP3._RankSelectionParameter=1.5; EAP3._MaxRank=3; EAP3._FitnessSelectionParameter=2;
        EAP3._SurvivorSelectionScheme=AIAlgos.TournamentSurvival;  EAP3._TParentSampleSize=2;
        Problem3 = new EAlgorithm(log, P1PG, EAP3);
        Execute(Problem3, EAP3, Runs, Iterations);
*/
    }

    public static void Assignment3()
    {
        int PopulationSize = 10, Offsprings = 10, Iterations = 40, Runs = 10;
        Logger log = new Logger(Logger.LogDebug);
        double minx, maxx, miny, maxy, stepx, stepy;
        P1PopulationGenerator P1PG;
        EAProblem1 EAP1; EAProblem2 EAP2; EAProblem3 EAP3; 
        AIS Problem1, Problem2, Problem3;
/*
        minx = -5; maxx = 0; miny = 0; maxy = 5; stepx = 0.25; stepy = 0.25;
        P1PG = new P1PopulationGenerator(minx, maxx, miny, maxy, stepx, stepy);
        EAP1 = new EAProblem1(); EAP1.Initialize(log);
        EAP1._ChromosomeLength=1; EAP1._NumberOfOffsprings=2; EAP1._PopulationCount=10; 
        EAP1._ProblemType=Utility.Minimization; EAP1._DegreeOfMutation=1; 
        EAP1._FitnessProportionate=Population.MutateIncreasing; EAP1._MutationVariationPercentage=80;
        EAP1._ParentSelectionScheme=AIAlgos.FitnessProportionSelection; EAP1._FitnessSelectionParameter=2;
        EAP1._SurvivorSelectionScheme=AIAlgos.TruncationSurvival;  
        EAP1._NumberOfClones=2;
        Problem1 = new AIS(log, P1PG, EAP1);
        Execute(Problem1, EAP1, Runs, Iterations);

        
        minx = -2; maxx = 2; miny = -1; maxy = 3; stepx = 0.25; stepy = 0.25;
        P1PG = new P1PopulationGenerator(minx, maxx, miny, maxy, stepx, stepy);
        EAP2 = new EAProblem2(); EAP2.Initialize(log);
        EAP2._ChromosomeLength=1; EAP2._NumberOfOffsprings=2; EAP2._PopulationCount=10; 
        EAP2._ProblemType=Utility.Maximization; EAP2._DegreeOfMutation=1; 
        EAP2._FitnessProportionate=Population.MutateIncreasing; EAP2._MutationVariationPercentage=80;
        EAP2._ParentSelectionScheme=AIAlgos.FitnessProportionSelection; EAP2._FitnessSelectionParameter=2;
        EAP2._SurvivorSelectionScheme=AIAlgos.TruncationSurvival;  
        EAP2._NumberOfClones=2;
        Problem2 = new AIS(log, P1PG, EAP2);
        Execute(Problem2, EAP2, Runs, Iterations);
        

        minx = Utility.DoubleMin; maxx = Utility.DoubleMax; miny = Utility.DoubleMin; maxy = Utility.DoubleMax; 
        stepx = 0.25; stepy = 0.25;
        P1PG = new P1PopulationGenerator(minx, maxx, miny, maxy, stepx, stepy);
        EAP3 = new EAProblem3(); EAP3.Initialize(log);
        EAP3._ChromosomeLength=1; EAP3._NumberOfOffsprings=2; EAP3._PopulationCount=10; 
        EAP3._ProblemType=Utility.Maximization; EAP3._DegreeOfMutation=1; 
        EAP3._FitnessProportionate=Population.MutateIncreasing; EAP3._MutationVariationPercentage=80;
        EAP3._ParentSelectionScheme=AIAlgos.FitnessProportionSelection; EAP3._FitnessSelectionParameter=2;
        EAP3._SurvivorSelectionScheme=AIAlgos.TruncationSurvival;  
        EAP3._NumberOfClones=2;
        Problem3 = new AIS(log, P1PG, EAP3);
        Execute(Problem3, EAP3, Runs, Iterations);
*/        
    }
    
    public static void Execute(AIAlgos Problem, AlgoConfig AC, int Runs, int Iterations)
    {
        GenerationResults = new double[2][Iterations];
        RunsResults = new double [Runs][Iterations];
        RunsResults2 = new double [Runs][Iterations];
        
        int i=0;
        for(int j=0; j<Runs; j++)
        {
            for(i=0; i<Iterations; i++)
            {
                Problem.Execute();
                SaveResult(Problem, AC, j, i);
            }
            PrintResults(Iterations);
        }
        PrintResults(Runs, Iterations);
    }

    protected static void SaveResult(AIAlgos Problem, AlgoConfig AC, int Runs, int Iteration)
    {
        int[] sorted = new int[AC._PopulationCount];
        double[] fitness = new double[AC._PopulationCount];
        fitness = Problem.GetGenerationFitness();
        Utility.Sort(fitness, sorted, AC._PopulationCount, AC._ProblemType);
        GenerationResults[0][Iteration] = fitness[sorted[0]];
        GenerationResults[1][Iteration] = Utility.Sum(fitness) / AC._PopulationCount;
        RunsResults[Runs][Iteration] = fitness[sorted[0]];
        RunsResults2[Runs][Iteration] = Utility.Sum(fitness) / AC._PopulationCount;
    }
    
    protected static void PrintResults(int Iterations)
    {
        System.out.println("Gen\t\t\t BF\t\t\t\t AF");
        for(int i=0; i<Iterations; i++)
        {
            System.out.println(" "+ i +"\t\t "+ GenerationResults[0][i] +"\t\t "+ GenerationResults[1][i]);
        }
            System.out.println();
    }

    protected static void PrintResults(int Runs, int Iterations)
    {
        System.out.println("Gen\t\t\t R1BSF\t\t\t\t R2BSF\t\t\t\t R3BSF\t\t\t\t R4BSF\t\t\t\t R5BSF\t\t\t\t "
                + "R6BSF\t\t\t\t R7BSF\t\t\t\t R8BSF\t\t\t\t R9BSF\t\t\t\t R10BSF\t\t\t\t ABSF");
        for(int j=0; j<Iterations; j++)
        {
            double average = 0;
            for(int i=0; i<Runs; i++)
            {
                average = average + RunsResults[i][j];
            }
            average = average / Runs;
            System.out.println(j + "\t\t " + RunsResults[0][j] + "\t\t " + RunsResults[1][j] + "\t\t " + RunsResults[2][j]
             + "\t\t " + RunsResults[3][j] + "\t\t " + RunsResults[4][j] + "\t\t " + RunsResults[5][j]
             + "\t\t " + RunsResults[6][j] + "\t\t " + RunsResults[7][j] + "\t\t " + RunsResults[8][j]
             + "\t\t " + RunsResults[9][j] + "\t\t " + average);
        }
            System.out.println();

        System.out.println("Gen\t\t\t R1ASF\t\t\t\t R2ASF\t\t\t\t R3ASF\t\t\t\t R4ASF\t\t\t\t R5ASF\t\t\t\t "
                + "R6ASF\t\t\t\t R7ASF\t\t\t\t R8ASF\t\t\t\t R9ASF\t\t\t\t R10ASF\t\t\t\t AASF");
        for(int j=0; j<Iterations; j++)
        {
            double average = 0;
            for(int i=0; i<Runs; i++)
            {
                average = average + RunsResults2[i][j];
            }
            average = average / Runs;
            System.out.println(j + "\t\t " + RunsResults2[0][j] + "\t\t " + RunsResults2[1][j] + "\t\t " + RunsResults2[2][j]
             + "\t\t " + RunsResults2[3][j] + "\t\t " + RunsResults2[4][j] + "\t\t " + RunsResults2[5][j]
             + "\t\t " + RunsResults2[6][j] + "\t\t " + RunsResults2[7][j] + "\t\t " + RunsResults2[8][j]
             + "\t\t " + RunsResults2[9][j] + "\t\t " + average);
        }
    }
}
