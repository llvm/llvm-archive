package MonitorBench;

public class MethodCallBenchmark
	extends Benchmark
{
	private interface DummyInterface
	{
		public void dummy();
	}
	
	private class DummyClass implements DummyInterface
	{
		public void dummy()
		{
			try {
				System.nanoTime();
			} catch (Exception e) {
				e.toString();
			}
		}
	}

	private class DummyClass2 implements DummyInterface
	{
		public void dummy()
		{
			System.out.println("Avoid some heuristics");
		}
	}
	
	DummyInterface dummy_int;
	
	public MethodCallBenchmark()
	{
		super("Method Call Benchmark");
		
		if (System.nanoTime() % 2 == 3)
			dummy_int = new DummyClass2();	// Unreachable (see condition above)
		else
			dummy_int = new DummyClass();
	}
	
	public void run()
	{
		for (int count = 100000000; count != 0; --count) {
			dummy_int.dummy();
		}
		
		setResult(0);
	}
}
