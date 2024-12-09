package smile;

public abstract class Wrapper 
{
	public Wrapper() 
	{ 
	    ptrNative = createNative(null); 
	}
	
	public Wrapper(Object param)
	{
	    ptrNative = createNative(param);
	}
	
	public void dispose() 
	{
		deleteNative(ptrNative);
		ptrNative = 0;
	}
	
	
	protected void finalize() 
	{ 
	    deleteNative(ptrNative); 
    }
	
	protected abstract long createNative(Object param);
	protected abstract void deleteNative(long nativePtr);
	private static native void nativeStaticInit();

	protected long ptrNative = 0;
	
	static 
	{
	    System.loadLibrary("jsmile");
	    nativeStaticInit();
    }
}
