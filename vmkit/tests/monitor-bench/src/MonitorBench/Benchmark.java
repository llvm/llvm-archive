package MonitorBench;

public abstract class Benchmark
	implements Runnable
{
	String name;
	int result;
	
	protected Benchmark(String name_)
	{
		name = name_;
	}
	
	public String toString() {return name;}
	
	public int getResult() {return result;}
	protected int setResult(int r) {result = r; return result;}
	
	public static String[] listBenchmarkNames()
	{
		int count = 2, i = 0;
		
		String[] r = new String[count];
		r[i++] = MethodCallBenchmark.class.getSimpleName();
		r[i++] = SmallObjectsBenchmark.class.getSimpleName();
		
		return r;
	}
	
	public static boolean isValidBenchmarkName(String name)
	{
		String[] list = listBenchmarkNames();
		for (int i=0; i < list.length; ++i)
			if (name.equalsIgnoreCase(list[i]))
				return true;
		
		return false;
	}
	
	public static Benchmark create(String name)
	{
		if (MethodCallBenchmark.class.getSimpleName().equalsIgnoreCase(name))
			return new MethodCallBenchmark();
		else if (SmallObjectsBenchmark.class.getSimpleName().equalsIgnoreCase(name))
			return new SmallObjectsBenchmark();
		else
			return null;
	}
}
