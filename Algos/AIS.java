/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package ciproject.Algos;

import ciproject.AILib.*;
import ciproject.BaseCode.*;

/**
 *
 * @author aaht14
 */
public class AIS extends AIAlgos{
    
    double[][] GenerationResults;
    double[][] RunsResults;
    double[][] RunsResults2;

    Population _Generation;
    Population _Clones;
    Population _NextGeneration;

    PopulationGenerator _PopulationGenerator;
    AlgoConfig _AlgorithmConfiguration;
    

    public AIS(Logger log, PopulationGenerator PG, AlgoConfig AC)
    {
        _Log = log;
        
        _PopulationGenerator = PG;
        _AlgorithmConfiguration = AC;
        
        _Generation = PG.GeneratePopulation(_AlgorithmConfiguration._PopulationCount, 
                _AlgorithmConfiguration._ChromosomeLength);
        _NextGeneration = PG.GeneratePopulation(_AlgorithmConfiguration._NumberOfOffsprings, 
                _AlgorithmConfiguration._ChromosomeLength);
        _Clones = PG.GeneratePopulation(_AlgorithmConfiguration._NumberOfClones, _AlgorithmConfiguration._ChromosomeLength);
        
        _Generation.Initialize(_Log, _AlgorithmConfiguration, _AlgorithmConfiguration._PopulationCount, 
                _AlgorithmConfiguration._ChromosomeLength);
        _NextGeneration.Initialize(_Log, _AlgorithmConfiguration, _AlgorithmConfiguration._NumberOfOffsprings, 
                _AlgorithmConfiguration._ChromosomeLength);
        _Clones.Initialize(_Log, _AlgorithmConfiguration, _AlgorithmConfiguration._NumberOfClones, 
                _AlgorithmConfiguration._ChromosomeLength);
        
    }
    
    public void Execute()
    {
/*        GenerationResults = new double[2][Iterations];
        RunsResults = new double [Runs][Iterations];
        RunsResults2 = new double [Runs][Iterations];

        int i=0;
        for(int j=0; j<Runs; j++)
        {
            for(i=0; i<Iterations; i++)
            {
*/
        ComputeFitness(_Generation, _AlgorithmConfiguration);
        //SaveResult(j, i);
        SelectParents(_Generation, _AlgorithmConfiguration);
        GenerateClones(_Generation, _Clones, _AlgorithmConfiguration);
        Mutate(_Clones, _AlgorithmConfiguration);
        ComputeFitness(_Clones, _AlgorithmConfiguration);
        SelectParents(_Clones, _AlgorithmConfiguration);
        GenerateOffsprings(_Clones, _NextGeneration, _AlgorithmConfiguration);
        SelectSurvivors(_Generation, _Clones, _AlgorithmConfiguration);
        SelectSurvivors(_Generation, _NextGeneration, _AlgorithmConfiguration);
/*            }
            PrintResults(Iterations);
        }
        PrintResults(Runs, Iterations);
*/    }

/*    protected void SaveResult(int Runs, int Iteration)
    {
        Utility.Sort(_Generation._GenerationFitness, _Generation._SelectedElements, _Generation._PopulationSize, 
                _AlgorithmConfiguration._ProblemType);
        GenerationResults[0][Iteration] = _Generation._GenerationFitness[_Generation._SelectedElements[0]];
        GenerationResults[1][Iteration] = _Generation._AverageFitness;
        RunsResults[Runs][Iteration] = _Generation._GenerationFitness[_Generation._SelectedElements[0]];
        RunsResults2[Runs][Iteration] = _Generation._AverageFitness;
    }
    
    protected void PrintResults(int Iterations)
    {
        System.out.println("Gen\t\t\t BF\t\t\t\t AF");
        for(int i=0; i<Iterations; i++)
        {
            System.out.println(" "+ i +"\t\t "+ GenerationResults[0][i] +"\t\t "+ GenerationResults[1][i]);
        }
            System.out.println();
    }
    protected void PrintResults(int Runs, int Iterations)
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
    */
    public double[] GetGenerationFitness()
    {
        return _Generation._GenerationFitness;
    }
    
}
