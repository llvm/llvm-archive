package MonitorBench;

import java.security.InvalidParameterException;

import javax.naming.InvalidNameException;

public class Main
{
	static String benchmarkName;
	
	public static void main(String[] args) throws Exception
	{
		if (!parseCommandLine(args)) return;
		
		BenchmarkRunner br = new BenchmarkRunner();
		Benchmark bench = Benchmark.create(benchmarkName);
		
		br.setBenchmark(bench);
		br.run();
	}
	
	static boolean parseCommandLine(String[] args) throws Exception
	{
		if (args.length != 1)
			throw new InvalidParameterException("Usage: monitor-bench {-h | --help | -l | --list | benchmark_name}");
		
		if (args[0].startsWith("-")) {
			if (args[0].equals("-h") || args[0].equals("--help")) {
				System.out.println("Usage: monitor-bench {-h | --help | -l | --list | benchmark_name}");
				return false;
			} else if (args[0].equals("-l") || args[0].equals("--list")) {
				String[] list = Benchmark.listBenchmarkNames();
				for (int i = 0; i < list.length; ++i)
					System.out.println(list[i]);
				return false;
			} else {
				throw new InvalidParameterException("Unrecognized parameter: " + args[0]);
			}
		} else {
			if (!Benchmark.isValidBenchmarkName(args[0]))
				throw new InvalidNameException("Unrecognized benchmark name: " + args[0]);
			benchmarkName = args[0];
		}
		
		return true; // Proceed
	}
}
