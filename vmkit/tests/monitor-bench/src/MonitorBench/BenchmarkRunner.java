package MonitorBench;

public class BenchmarkRunner
	implements Runnable
{
	volatile Benchmark bench;
	volatile long durationMS;
	volatile int result;
	volatile boolean running;
	volatile Object runningEvent;

	public BenchmarkRunner()
	{
		runningEvent = new Object();
	}
	
	public boolean setBenchmark(Benchmark b)
	{
		if (running) return false;
		
		bench = b;
		return true;
	}
	
	public boolean waitBenchmark() throws InterruptedException
	{
		if (!running) return false;
		
		synchronized(runningEvent) {
			while (running)
				runningEvent.wait();
		}
		
		return true;
	}
	
	public void run()
	{
		running = true;
		
		long startTime;
		durationMS = 0;
		result = 0;
		
		System.out.println("BENCHMARK STARTED: " + bench.toString());
		
		startTime = System.nanoTime();
		bench.run();
		durationMS = System.nanoTime();
		
		durationMS -= startTime;
		durationMS /= 1000000; // ns ==> ms
		
		result = bench.getResult();

		if (result == 0)
			System.out.println("BENCHMARK SUCCEEDED [" + durationMS + " ms]: " + bench.toString());
		else
			System.out.println("BENCHMARK FAILED [" + result + "]: " + bench.toString());
		
		running = false;
		
		synchronized (runningEvent) {
			runningEvent.notify();
		}
	}
	
	public boolean benchmarkRunning() {return running;}
	
	public long getDuration() {return durationMS;}
	
	public long getBenchmarkResult() {return result;}
}
