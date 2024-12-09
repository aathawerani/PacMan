/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package ciproject.Problems;

import ciproject.BaseCode.*;
import ciproject.AILib.*;

/**
 *
 * @author aaht14
 */
public class SLAlgoConfig extends AlgoConfig2{

    //public int _TotalTrials, _PriorTrials;
    public int[][] _ValueCount;
    public int[][][] _ParentNodes;
    public String[][] _InputData;
    public String[][] _Values;
    public int[][][] _GraphPaths;
    
    public SLAlgoConfig (Logger log)
    {
        _Log = log;
        
    }
    
    public double[] ComputeFitness(Population2 Input)
    {
        _Output = new double[Input._PopulationSize];
        _GraphPaths = new int[Input._PopulationSize][][];
        for(int i=0; i<Input._PopulationSize; i++) _Output[i] = 0;
        _ParentNodes = new int[Input._PopulationSize][][];
        
        for(int i=0; i<Input._PopulationSize; i++)
        {
            GenerateGraph(Input, i);
            double temp = AnalyzeData(i);
            _Log.Debug("temp " + temp);
            _Output[i] = temp;
        }
        _Log.Debug("output");
        _Log.Debug(_Output);
        return _Output;
    }
    
    public void GenerateGraph(Population2 Input, int i)
    {
        //_Log.Debug(("entered generate graph"));
        double[][] graph = Input.GetValues(i);
        //double[][] order = Input.GetValues(i);
        int chromosomelength = graph.length;
        int chromosomedepth = graph[0].length;
        int possiblegraphpaths = chromosomelength*36;
        int[][] graphpaths = new int[possiblegraphpaths][chromosomedepth];
        
        for(int j=0; j<possiblegraphpaths; j++)
        {
            //_Log.Debug("random values");
            //_Log.Debug(graph[j]);
            for(int k=0; k<chromosomedepth; k++)
            {
                graphpaths[j][k] = -1;
            }
        }
        
        _Log.Debug("final graph");
        for(int j=0; j<chromosomelength; j++)
        {
            for(int k=0; k<chromosomedepth; k++)
            {
                if(j==k) 
                {
                    graph[j][k] = 0;
                }
                else if(Math.round(graph[j][k]) == 1)
                {
                    graph[j][k] = 1;
                    graph[k][j] = 0;
                }
                /*else if(Math.round(graph[j][k]) == 0)
                {
                    graph[j][k] = 0;
                }*/
            }
            _Log.Debug(graph[j]);
        }

        //int[] nodemap = new int[chromosomelength];
        //for(int j=0; j<chromosomelength; j++) nodemap[j] =0;
        int[][] parentnodes = new int[chromosomelength][chromosomedepth];
        int[][] childnodes = new int[chromosomelength][chromosomedepth];
        for(int j=0; j<chromosomelength; j++) 
        {
            for(int k=0; k<chromosomelength; k++) 
            {
                parentnodes[j][k] = -1;
                childnodes[j][k] = -1;
            }
        }
        for(int l=0; l<possiblegraphpaths; l++)
        {
            //_Log.Debug("l " + l);
            int count=0, prevj = 0, prevj2 = -1; boolean circular=false, repeatpath = false;
            for(int j=0; j<chromosomelength; )
            {
                //_Log.Debug("j " + j);
                for(int k=0; k<chromosomedepth; k++)
                {
                    if(graph[j][k] == 2 && count>0)
                    {
                        //_Log.Debug("found 2 ......................... ");
                        repeatpath = true;
                        for(int p=0; p<chromosomedepth; p++)
                        {
                            if(graphpaths[l][p] == -1) break;
                            if(graphpaths[l][p] == j)
                            {
                                //_Log.Debug("l " + l + "---------------------1 j " + j + " k " + k);
                                //_Log.Debug(graphpaths[l]);
                                circular = true;
                                break;
                            }
                        }
                        if(circular == true) break;
                        for(int p=0; p<chromosomedepth; p++)
                        {
                            if(graphpaths[l][p] == -1) break;
                            for(int r=0; r<chromosomedepth; r++)
                            {
                                if(parentnodes[graphpaths[l][p]][r] == -1) break;
                                if(parentnodes[graphpaths[l][p]][r] == j)
                                {
                                    //_Log.Debug("l " + l + "---------------------2");
                                    //_Log.Debug(graphpaths[l]);
                                    circular = true;
                                    break;
                                }
                            }
                            if(circular == true) break; 
                        }
                        if(circular == true) break; 
                        /*for(int p=0; p<chromosomedepth; p++)
                        {
                            if(graphpaths[l][p] == -1) break;
                            for(int r=0; r<chromosomedepth; r++)
                            {
                                if(parentnodes[graphpaths[l][p]][r] == -1) break;
                                for(int s=0; s<chromosomedepth; s++)
                                {
                                    if(childnodes[j][s] == -1) break;
                                    if(parentnodes[graphpaths[l][p]][r] == childnodes[j][s])
                                    {
                                        _Log.Debug("l " + l + "---------------------3");
                                        _Log.Debug(graphpaths[l]);
                                        circular = true;
                                        break;
                                    }
                                }
                                if(circular == true) break;
                            }
                            if(circular==true) break;
                        }*/
                        //if(circular==true) break;
                        //need to check this
                        /*if(graphpaths[l][count] != j)
                        {
                            graphpaths[l][count] = j;
                            _Log.Debug("l " + l + " count " + count + "---------------------4");
                            _Log.Debug(graphpaths[l]);
                            /*for(int b=0; b<chromosomelength; b++)
                            {
                                if(childnodes[j][b] == -1)
                                {
                                    childnodes[j][b] = k;
                                    break;
                                }
                            }*/
                            //_Log.Debug("k " + k);
                            /*for(int a=0; a<chromosomelength; a++)
                            {
                                if(parentnodes[j][a] == -1)
                                {
                                    parentnodes[j][a] = prevj;
                                    break;
                                }
                            }
                            count++;
                        }*/
                    }
                    if(graph[j][k] == 1)
                    {
                        repeatpath = false;
                        //if(circular == false)
                        //{
                            //if(graphpaths[l][count] != j && graphpaths[l][count] == -1)
                            //{
                                //if(count == 0)
                                //{
                                //_Log.Debug("l " + l + "---------------------5 j " + j + " k " + k);
                                    graphpaths[l][count] = j;
                                    circular = false;
                                //}
                                //else if(count>0)
                                //{
                                    //if(graphpaths[l][count-1] != j)
                                    //{
                                        //graphpaths[l][count] = j;
                                        //_Log.Debug("k " + k);
                                    if(count>0)
                                    {
                                        for(int a=0; a<chromosomelength; a++)
                                        {
                                            if(parentnodes[j][a] == -1)
                                            {
                                                parentnodes[j][a] = prevj;
                                                break;
                                            }
                                        }
                                    }
                                //}
                                for(int b=0; b<chromosomelength; b++)
                                {
                                    if(childnodes[j][b] == -1)
                                    {
                                        childnodes[j][b] = k;
                                        break;
                                    }
                                }
                                
                            //}
                            //if(count > 0 && graphpaths[l][count-1] != j)
                            //{
                                count++;
                            //}
                            //if(count == 0) count++;
                            graph[j][k] = 2;
                            //nodemap[j]++;
                            prevj = j;
                            j=k;
                            break;
                        //}
                    }
                }
                if(circular==true)
                {
                    break;
                }
                if(repeatpath == true && circular == false)
                {
                    //_Log.Debug("got here ");
                    graphpaths[l][count] = j;
                    //_Log.Debug("l " + l + "---------------------7 j " + j);
                    //_Log.Debug(graphpaths[l]);
                    for(int a=0; a<chromosomelength; a++)
                    {
                        if(parentnodes[j][a] == prevj) break;
                        if(parentnodes[j][a] == -1)
                        {
                            parentnodes[j][a] = prevj;
                            break;
                        }
                    }
                    break;
                }
                if(j==prevj2) j++;
                prevj2=j;
            }
            //_Log.Debug("graph paths");
            //_Log.Debug(graphpaths[l]);
            //_Log.Debug("and parentnodes");
            //_Log.Debug(parentnodes[l]);
            if(graphpaths[l][0] == -1)
                break;
        }
        

        _GraphPaths[i] = graphpaths;
        
        _Log.Debug("graph paths");
        for(int j=0; j<possiblegraphpaths; j++)
        {
            _Log.Debug(graphpaths[j]);
        }
        
        _Log.Debug("parent nodes");
        for(int j=0; j<chromosomelength; j++)
        {
            _Log.Debug(parentnodes[j]);
        }
        
        _Log.Debug("child nodes");
        for(int j=0; j<chromosomelength; j++)
        {
            _Log.Debug(childnodes[j]);
        }

        /*_Log.Debug("final graph 1");
        for(int j=0; j<chromosomelength; j++)
        {
            _Log.Debug(graph[j]);
        }*/
        
        _ParentNodes[i] = parentnodes;
        
    }
    
    public double AnalyzeData(int i)
    {
        int maxrows = _InputData.length;
/*        for(int j=0; j<maxrows; j++)
        {
            _Log.Debug(_InputData[j]);
        }
*/        
        int chromosomelength = _ParentNodes[i].length;
        int chromosomedepth = _ParentNodes[i][0].length;


        int[][] nodestats = new int[chromosomelength][3];
        int maxval = _ValueCount[0].length;
        for(int d=0; d<chromosomelength; d++)
        {
            nodestats[d][0] = 0;
            nodestats[d][1] = 1;
            for(int e=0; e<chromosomedepth; e++)
            {
                if(_ParentNodes[i][d][e] == -1) break;
                if(_ParentNodes[i][d][e] > -1) nodestats[d][0]++;

                int temp = 0;
                for(int f=0; f<maxval; f++)
                {
                    if(_ValueCount[_ParentNodes[i][d][e]][f] == 0) break;
                    temp++;
                }
                nodestats[d][1] *= temp;
            }
        }
        
        for(int d=0; d<chromosomelength; d++)
        {
            nodestats[d][2] = 1;
            int temp = 0;
            for(int e=0; e<chromosomedepth; e++)
            {
                if(_ValueCount[d][e] == 0) break;
                temp++;
            }
            nodestats[d][2] *= temp;
        }

        int[][][] probabilities = new int[chromosomelength][][];
        for(int g=0; g<chromosomelength; g++)
        {
            _Log.Debug(nodestats[g]);
            probabilities[g] = new int[nodestats[g][2]][];
            for(int q=0; q<nodestats[g][2]; q++)
            {
                probabilities[g][q] = new int[nodestats[g][1]];
                for(int h=0; h<nodestats[g][1]; h++)
                {
                    probabilities[g][q][h] = 0;
                }
            }
        }
                
        for(int a=0; a<maxrows; a++)
        {
            for(int b=0; b<chromosomelength; b++)
            {
                //_Log.Debug(_ParentNodes[i][b]);
                //_Log.Debug("a " + a + " b " + b +" i " + i);
                int[] temp = CheckProbability(_InputData[a], _Values, _ParentNodes[i][b], _ValueCount, b);
                if(temp[1] >= 0 )
                {
                    probabilities[b][temp[0]][temp[1]]++;
                }
            }
        }
        
        for(int c=0; c<chromosomelength; c++)
        {
            for(int d=0; d<nodestats[c][2]; d++)
            {
                _Log.Debug(probabilities[c][d]);
            }
        }
        
        double probability = 0, parentprobability = 0, childprobability = 0;
        int length = 0, numparams = 0, power = 0;;
        
        for(int j=0; j<chromosomelength; j++)
        {
            length = probabilities[j].length; //values of node shorter loop
            int len = probabilities[j][0].length; // parent combo longer loop
            //if(len == 0) numparams++;
            if(probabilities[j][0][0] > 0)
            {
                for(int k=0; k<len; k++) //parent combo longer loop
                {
                    for(int l=0; l<length; l++) //values of node shorter loop
                    {
                        parentprobability += probabilities[j][l][k];
                    }
                    for(int l=0; l<length; l++) //values of node shorter loop
                    {
                        power = probabilities[j][l][k];
                        if(power > 0)
                        {
                            childprobability = power;
                            double temp = Math.log(childprobability) - Math.log(parentprobability);
                            double temp2 = 0;//Math.log(parentprobability) - Math.log(maxrows);
                            _Log.Debug("power " + power + " parentprobability " + parentprobability + " temp " + temp 
                                    + " temp2 " + temp2);
                            temp = temp + temp2;
                            _Log.Debug("temp + temp2 " + temp);
                            temp = (double)power * temp;
                            _Log.Debug("(double)power * (temp) " + temp);
                            probability = probability + temp;
                            _Log.Debug("probability " + probability);
                        }
                    }
                    parentprobability = 0;
                    //numparams++;
                }
                //numparams++;
                numparams = numparams + (int)Math.pow(2, nodestats[j][0]);
            }
            else
            {
                _Log.Debug("case of no parents ");
                for(int u=0; u<_ValueCount[j].length; u++)
                {
                    if(_ValueCount[j][u] == 0) break;
                    parentprobability += _ValueCount[j][u];
                }
                for(int v=0; v<_ValueCount[j].length; v++)
                {
                    power = _ValueCount[j][v];
                    if(power == 0) break;
                    childprobability = power;
                    double temp = Math.log(childprobability) - Math.log(parentprobability);
                    _Log.Debug("power " + power + " parentprobability " + parentprobability + " temp " + temp 
                            );
                    temp = (double)power * temp;
                    _Log.Debug("(double)power * (temp) " + temp);
                    probability = probability + temp;
                    _Log.Debug("probability " + probability);
                }
                parentprobability=0;
                //numparams++;
            }
        }
        
        _Log.Debug("probability " + probability + " numparams " + numparams 
                + " Math.log(maxrows) " + Math.log(maxrows) + 
                " ((numparams / 2) * Math.log(maxrows)) " + ((numparams / 2) * Math.log(maxrows)));
        
        double BICScore = probability - ((numparams / 2) * Math.log(maxrows));
        
        _Log.Debug("BICScore " + BICScore);
                
        return BICScore;
    }
    
    public int[] CheckProbability(String[] Data, String[][] Values, int[] parents, int[][] ValueCounts, int b)
    {
        //_Log.Debug("Check Probability");
        String[] datavalues = Data;
        int length = parents.length;
        int[] combo = new int[2]; combo[0] = 0; combo[1] = 0;
        
        if(parents[0] == -1)
        {
            combo[1] = -1;
            return combo;
        }
        //_Log.Debug(parents);
        for(int i=0; i<length; i++)
        {
            if(parents[i] == -1) {break;}
            //_Log.Debug("reached here");
            
            int depth = Values[i].length;
            for(int j=0; j<depth; j++)
            {
                //_Log.Debug("parents[i] " + parents[i] + " ValueCounts[parents[i]][j] " + ValueCounts[parents[i]][j]);
                if(ValueCounts[parents[i]][j] == 0) {_Log.Debug("second"); break;}
                //_Log.Debug("Values[parents[i]][j] " + Values[parents[i]][j] + 
                //        " datavalues[parents[i] " + datavalues[parents[i]]);
                if(Values[parents[i]][j].compareTo(datavalues[parents[i]]) == 0)
                {
                    //_Log.Debug("match found");
                    combo[1] = combo[1]  + (j * (int)Math.pow(2, i));
                    break;
                }
            }
        }
        
        int depth = Values[b].length;
        for(int j=0; j<depth; j++)
        {
            if(ValueCounts[b][j] == 0) {break;}
            //_Log.Debug("Values[b][j] " + Values[b][j] + 
                    //" datavalues[b] " + datavalues[b]);
            if(Values[b][j].compareTo(datavalues[b]) == 0)
            {
                //_Log.Debug("match found");
                break;
            }
            else
            {
                combo[0]++;
            }
        }
        //combo[1]--;
        //_Log.Debug("combo ");
        //_Log.Debug(combo);

        return combo;
    }
    
    public int[][] GetGraphPath(int i)
    {
        //return _ParentNodes[i];
        return _GraphPaths[i];
        
    }
}
