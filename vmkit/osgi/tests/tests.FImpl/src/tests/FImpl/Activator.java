package tests.FImpl;

import java.util.ArrayList;

import org.osgi.framework.BundleActivator;
import org.osgi.framework.BundleContext;
import org.osgi.util.tracker.ServiceTracker;

import tests.E.E;

public class Activator
	implements BundleActivator
{
	BundleContext context;
	ServiceTracker<E, E> serviceTrackerE;

	public void start(BundleContext bundleContext) throws Exception
	{	
		context = bundleContext;
		
		serviceTrackerE = new ServiceTracker<E, E>(context, E.class, null);
		serviceTrackerE.open();
		
		heavyInitialization();
	}

	public void stop(BundleContext bundleContext) throws Exception
	{
		System.out.println(this.getClass().getPackage().getName()
			+ " consumes " + E.class.getName());
		
		serviceTrackerE.close();
	}
	
	void heavyInitialization() throws Exception
	{
		E e = serviceTrackerE.getService();
		if (e == null)
			throw new Exception("Service not found: " + E.class.getName());
		
		ArrayList<Double> T = new ArrayList<Double>();
		for (int i=0; i < 499; ++i)
			T.add(new Double(Math.random()));
		
		e.e();	
	}
}
