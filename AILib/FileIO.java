/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package ciproject.AILib;

import java.io.*;

/**
 *
 * @author aaht14
 */
public class FileIO {
        
    BufferedReader _BR;
    PrintWriter _PW;
    Logger _Log;
    String _Header;
    String _ReadFileName, _WriteFileName;
        
    public FileIO(String filename1, String filename2, Logger log) 
    {
        _Log = log;
        _ReadFileName = filename1;
        _WriteFileName = filename2;
        //_BR = new BufferedReader(new FileReader(new File(filename1).getAbsoluteFile()));
        //String line = _BR.readLine();
        //_Header = line;
    }

    public void OpenReadFile() throws IOException
    {
        _BR = new BufferedReader(new FileReader(new File(_ReadFileName).getAbsoluteFile()));
    }

    public void CloseReadFile() throws IOException
    {
        _BR.close();
    }
    
    public String[] GetHeader(String textseparator) throws IOException
    {
        String line = _BR.readLine();
        _Header = line;
        String[] values = _Header.split(textseparator);
        return values;
    }
    
    public String[] ReadUTF(String textseparator) throws IOException
    {
        String line = _BR.readLine();
        if(line == null) {_Log.Debug("null line"); return null;}
        String[] values = line.split(textseparator);
        return values;
    }

    public double[] ReadLineDouble(String textseparator) throws IOException
    {
        String line = _BR.readLine();
        String[] values = line.split(textseparator);
        double[] convertedvalues = new double[values.length];
        for(int i=0; i<values.length; i++)
        {
            convertedvalues[i] = Double.parseDouble(values[i]);
        }
        return convertedvalues;
    }

    public int[] ReadInt(String textseparator) throws IOException
    {
        String line = _BR.readLine();
        String[] values = line.split(textseparator);
        int[] convertedvalues = new int[values.length];
        for(int i=0; i<values.length; i++)
        {
            convertedvalues[i] = Integer.parseInt(values[i]);
        }
        return convertedvalues;
    }
    
    public void OpenWriteFile()
    {
        
    }

    public void CloseWriteFile()
    {
        
    }

    public void WriteDouble(double[] values, String textseparator) throws IOException
    {
        String line = "";
        for(int i=0; i<values.length; i++)
        {
            line += new Double(values[i]).toString() + textseparator;
        }
        _PW.println(line);
    }

    public void WriteInt(int[] values, String textseparator) throws IOException
    {
        String line = "";
        for(int i=0; i<values.length; i++)
        {
            line += new Integer(values[i]).toString() + textseparator;
        }
        _PW.println(line);
    }

    public void WriteUTF(String[] values, String textseparator) throws IOException
    {
        String line = "";
        for(int i=0; i<values.length; i++)
        {
            line += values[i] + textseparator;
        }
        _PW.println(line);
    }
}
