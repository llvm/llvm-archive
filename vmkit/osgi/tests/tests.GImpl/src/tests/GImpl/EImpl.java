package tests.GImpl;

import java.util.ArrayList;

import org.osgi.framework.BundleContext;
import org.osgi.framework.ServiceReference;

import tests.E.E;
import tests.B.BEvent;

public class EImpl
	implements E
{
	BundleContext context;
	
	public EImpl(BundleContext context)
	{
		this.context = context;
	}
	
	public void e()
	{
		try {
			someProcessing();
		} catch (Exception e1) {
			e1.printStackTrace();
		}
	}
	
	private void someProcessing() throws Exception
	{
		ServiceReference<BEvent> refBEvent =
			context.getServiceReference(BEvent.class);
		
		if (refBEvent == null)
			throw new Exception("Service not found: " + BEvent.class.getName());
		
		BEvent b = context.getService(refBEvent);
		if (b == null)
			throw new Exception("Service not found: " + BEvent.class.getName());

		ArrayList<Double> T = new ArrayList<Double>();
		for (int i=0; i < 4999; ++i)
			T.add(new Double(Math.random()));

		b.handler();
	}
}
