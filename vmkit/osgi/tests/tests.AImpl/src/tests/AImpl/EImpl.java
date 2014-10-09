package tests.AImpl;

import tests.E.E;

public class EImpl
	implements E
{
	public void e()
	{
		try {
			someProcessing();
		} catch (InterruptedException e1) {
			e1.printStackTrace();
		}
	}
	
	private void someProcessing() throws InterruptedException
	{
		Thread.sleep(10000);
	}
}
