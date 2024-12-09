/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package ciproject.AILib;

import ciproject.BaseCode.AlgoConfig;
import java.util.Random;
import java.lang.Integer;

/**
 *
 * @author aaht14
 */
public class Utility {
    
    public static final double DoubleMax = Integer.MAX_VALUE;
    public static final double DoubleMin = Integer.MIN_VALUE;
    public static final int DoubleWithRange = 0, DoubleNormal = 1, IntNormal = 2, IntWithRange = 3, DoubleWithMax = 4,
            DoubleWithMin = 5, Minimization=0, Maximization=1;
    
    public static double GenerateRandom(double min, double max, int RT)
    {
        Random random = new Random();
        double temp = 0;//, rnum = 0, absmin = 0, t1 = 0, t2 = 0;
        switch(RT)
        {
            case DoubleWithRange: 
                temp = (random.nextDouble() * (max + Math.abs(min))) - Math.abs(min);
                //rnum = random.nextDouble();
                //absmin = Math.abs(min);
                //t1 = max + Math.abs(min);
                //t2 = rnum * (t1);
                //temp = t2 - absmin;
                //System.out.println("GenerateRandom max " + max + " min " + min + " rnum " + rnum 
                //        + " t2 " + t2 + " t1 " + t1 + " absmin " + absmin + " temp " + temp);
                //System.out.println("GenerateRandom  temp " + temp);
                break;
            case DoubleNormal: temp = random.nextDouble(); break;
            case IntNormal: temp = random.nextInt(); break;
            case IntWithRange: temp = random.nextInt((int)max); break;
        }
        return temp;
    }

    public static double AdjustValue(double value, double stepsize, double limit, int RT)
    {
        Random random = new Random();
        double newval = value;
        if(RT == DoubleWithMax)
        {
            while(newval > limit)
            {
                newval = newval - (random.nextDouble() * stepsize);
            }
            //System.out.println("AdjustValue Max" + newval);
        }
        else if(RT == DoubleWithMin)
        {
            while(newval < limit)
            {
                newval = newval + (random.nextDouble() * stepsize);
            }
            //System.out.println("AdjustValue Max" + newval);
        }
        return newval;
    }
    
    public static int Max(double[] Unsorted, int Count)
    {
        /*for(int i=0; i<Unsorted.length; i++)
        {
            System.out.println("Unsorted " + Unsorted[i]);
        }*/
        int max = 0;
        for(int i=0; i<Count; i++)
        {
            if(Unsorted[max]<Unsorted[i])
            {
                max=i;
            }
        }
            //System.out.println("max " + max);
        return max;
    }

    public static int Min(double[] Unsorted, int Count)
    {
        /*for(int i=0; i<Unsorted.length; i++)
        {
            System.out.println("Unsorted " + Unsorted[i]);
        }*/
        int min = 0;
        for(int i=0; i<Count; i++)
        {
            if(Unsorted[min]>Unsorted[i])
            {
                min=i;
            }
        }
            //System.out.println("max " + max);
        return min;
    }
    public static void Sort(double[] Unsorted, int[] Sorted, int Count, int Order)
    {
        //System.out.println("printing unsorted array");
        int max = 0, next =0;
        for(int k=0; k<Sorted.length; k++) Sorted[k] = 0;
        //for(int k=0; k<Sorted.length; k++) System.out.print(Unsorted[k] + " ");
        //System.out.println();
        /*for(int i=0; i<Count; i++)
        {
            if(Order == 1)
            {
                if(Unsorted[max]<Unsorted[i])
                {
                    max=i;
                }
            }
            else if(Order == 2)
            {
                if(Unsorted[max]>Unsorted[i])
                {
                    max=i;
                }
            }
        }
        Sorted[0] = max;*/
        //Sorted[0] = 0;
        boolean valueset = false;
        for(int j=1; j<Count; j++)
        {
            //System.out.println("printing sorted array");
            //for(int k=0; k<Sorted.length; k++) System.out.print(Sorted[k] + " ");
            
            for(int i=0; i<=j; i++)
            {
                if(Order == Minimization)
                {
                    /*System.out.print("max " + max + " ");
                    System.out.print("i " + i + " ");
                    System.out.print("next " + next + " ");
                    System.out.print("Unsorted[next] " + Unsorted[next] + " ");
                    System.out.print("Unsorted[i] " + Unsorted[i] + " ");
                    System.out.print("Unsorted[max] " + Unsorted[max] + " ");
                    System.out.println();*/
                    if(Unsorted[j]<=Unsorted[Sorted[i]] && i!=j)
                    {
                        for(int k=j; k>i; k--)
                        {
                            Sorted[k] = Sorted[k-1];
                        }
                        Sorted[i] = j;
                        valueset = true;
                        break;
                    }
                    
                }
                else if(Order == Maximization)
                {
                    /*System.out.print("max " + max + " ");
                    System.out.print("i " + i + " ");
                    System.out.print("next " + next + " ");
                    System.out.print("Unsorted[next] " + Unsorted[next] + " ");
                    System.out.print("Unsorted[i] " + Unsorted[i] + " ");
                    System.out.print("Unsorted[max] " + Unsorted[max] + " ");
                    System.out.println();
                    if(Unsorted[next]>Unsorted[i] && Unsorted[max]<Unsorted[i])
                    {
                        next=i;
                    }*/
                    if(Unsorted[j]>=Unsorted[Sorted[i]] && i!=j)
                    {
                        for(int k=j; k>i; k--)
                        {
                            Sorted[k] = Sorted[k-1];
                        }
                        Sorted[i] = j;
                        valueset = true;
                        break;
                    }
                }
            }
            if(valueset == false) Sorted[j] = j;
            valueset = false;
            //max = next;
            //if(next == 0) next = 1; else next = 0;
            //next = 0;
        }
        //System.out.println("printing sorted array");
        //for(int k=0; k<Sorted.length; k++) System.out.print(Sorted[k] + " ");
    }
    
    public static double Sum(double[] array)
    {
        double sum = 0;
        for(int i=0; i<array.length; i++)
        {
            sum = sum + array[i];
        }
        return sum;
    }
    
    public static int[] CopyArray(int[] destination, int[] source, int destinationstartindex, int sourcestartindex, 
            int length)
    {
        for(int i=0; i<length; i++)
        {
            destination[destinationstartindex + i] = source[sourcestartindex + i];
        }
        
        return destination;
    }

    public static double[] CopyArray(double[] destination, double[] source, int destinationstartindex, int sourcestartindex, 
            int length)
    {
        for(int i=0; i<length; i++)
        {
            destination[destinationstartindex + i] = source[sourcestartindex + i];
        }
        
        return destination;
    }
}
