/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package ciproject.AILib;

import java.lang.String;
/**
 *
 * @author aaht14
 */
public class Logger {
    
    int _LogLevel;
    public static final int LogDebug=0, LogInfo=1, LogError=2;
    
    public Logger(int level)
    {
        _LogLevel = level;
    }
    
    public void Debug(String message)
    {
        if(_LogLevel == LogDebug)
        {
            System.out.println(message);
        }
    }

    public void Debug(double[] array)
    {
        if(_LogLevel == LogDebug)
        {
            for(int i=0; i<array.length; i++)
            {
                System.out.print(" " + array[i]);
            }
            System.out.println();
        }
    }
    
    public void Debug(int[] array)
    {
        if(_LogLevel == LogDebug)
        {
            for(int i=0; i<array.length; i++)
            {
                System.out.print(" " + array[i]);
            }
            System.out.println();
        }
    }

    public void Info(String message)
    {
        if(_LogLevel == LogInfo)
        {
            System.out.println(message);
        }
    }

    public void Error(String message)
    {
        if(_LogLevel == LogError)
        {
            System.out.println(message);
        }
    }
}
