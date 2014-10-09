package tests.AImpl;

import tests.A.A;
import tests.B.BEvent;

import org.osgi.framework.BundleActivator;
import org.osgi.framework.BundleContext;

public class Activator
	implements BundleActivator
{
	BundleContext context;
	AImpl a;
//	EImpl e;

	public void start(BundleContext bundleContext) throws Exception
	{	
		System.out.println(this.getClass().getPackage().getName() + " provides " + A.class.getName());
		context = bundleContext;

		a = new AImpl();
		
		String[] classes = {A.class.getName(), BEvent.class.getName()};
		context.registerService(classes, a, null);

		// e = new EImpl();
		// context.registerService(E.class, e, null);	
	}

	public void stop(BundleContext bundleContext) throws Exception
	{
		System.out.println(this.getClass().getPackage().getName() + " no more provides " + A.class.getName());
	}
}
