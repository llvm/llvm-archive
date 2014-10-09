
package j3mgr;

public class Main {

	public static void main(String[] args) throws Throwable
	{
		TierConfigParser parser = new TierConfigParser();
		
		parser.parse(
			"/home/koutheir/PhD/VMKit/vmkit-monitor/tests/tier.description");
	}
}
