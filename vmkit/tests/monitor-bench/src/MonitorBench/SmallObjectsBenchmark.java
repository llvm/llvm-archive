package MonitorBench;

public class SmallObjectsBenchmark
	extends Benchmark
{
	int count = 3000000;
	Object[] objects;
	
	public SmallObjectsBenchmark()
	{
		super("Small Objects Benchmark");
		
		objects = new Object[count]; 
	}

	public void run()
	{
		for (int i=0; i < count; ++i) {
			objects[i] = new String();
		}
		
		setResult(0);
	}
}
