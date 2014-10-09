package tests.GImpl;

import org.osgi.framework.BundleActivator;
import org.osgi.framework.BundleContext;

import tests.E.E;

public class Activator
	implements BundleActivator
{
	BundleContext context;
	EImpl e;

	public void start(BundleContext bundleContext) throws Exception
	{
		context = bundleContext;
		
		System.out.println(this.getClass().getPackage().getName()
			+ " provides " + E.class.getName());

		e = new EImpl(context);
		context.registerService(E.class, e, null);	
	}

	public void stop(BundleContext bundleContext) throws Exception
	{
		System.out.println(this.getClass().getPackage().getName()
			+ " no more provides " + E.class.getName());
	}
}
