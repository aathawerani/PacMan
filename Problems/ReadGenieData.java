/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package ciproject.Problems;

import ciproject.AILib.*;
import java.io.IOException;
/**
 *
 * @author aaht14
 */
public class ReadGenieData {
    
    FileIO _GenieFile;
    int _NodesCount, _MaxValue;
    String[][] _Values;
    int[][] _ValueCount;
    Logger _Log;
    public String[] _Header;
    public String[][] _InputData;
    
    public ReadGenieData(FileIO file, Logger log)
    {
        _Log = log;
        _GenieFile = file;
    }
    
    public void ReadHeader(int maxvalue) throws IOException
    {
        _GenieFile.OpenReadFile();
        String[] header = _GenieFile.GetHeader(" ");
        _Header = header;
        if(header == null) return;
        _NodesCount = header.length;

        _Log.Debug("_NodesCount " + _NodesCount);

        _MaxValue = maxvalue;
        
        _Values = new String[_NodesCount][_MaxValue];
        _ValueCount = new int[_NodesCount][_MaxValue];

        for(int i=0; i<_NodesCount; i++)
        {
            for(int j=0; j<_MaxValue; j++)
            {
                _Values[i][j] = "";
                _ValueCount[i][j] = 0;
            }
        }
    }
    
    public void ReadData(int maxrows) throws IOException
    {
        String[] data = _GenieFile.ReadUTF(" ");
        for(int k=0; k<maxrows; k++)
        {
            for(int i=0; i<_NodesCount; i++)
            {
                for(int j=0; j<_MaxValue; j++)
                {
                    if(_Values[i][j].compareTo(data[i].trim()) == 0)
                    {
                        _ValueCount[i][j]++;
                        break;
                    }
                    if(_Values[i][j].compareTo("") == 0)
                    {
                        _Values[i][j] = data[i];
                        _ValueCount[i][j]++;
                        break;
                    }
                }
            }
            data = _GenieFile.ReadUTF(" ");
        }
        _GenieFile.CloseReadFile();
    }
    
    public String[][] GetValues()
    {
        return _Values;
    }
    
    public int[][] GetValueCount()
    {
        return _ValueCount;
    }
    
    public int GetNumberOfNodes()
    {
        return _NodesCount;
    }
    
/*    public int GetTotalTrials()
    {
        int sum = 0;
        for(int i=0; i<_MaxValue; i++)
        {
            sum += _ValueCount[0][i];
        }
        return sum;
    }
*/    
    public void ReadEntireData(int maxvalue) throws IOException
    {
        _InputData = new String[maxvalue][];
        _GenieFile.OpenReadFile();
        String[] header = _GenieFile.GetHeader(" ");
        for(int i=0; i<maxvalue; i++)
        {
            _InputData[i] = _GenieFile.ReadUTF(" ");
            
            //_Log.Debug(_InputData[i]);
        }
        _GenieFile.CloseReadFile();
    }
    
}
