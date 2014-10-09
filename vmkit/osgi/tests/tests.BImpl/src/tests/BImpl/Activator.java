package tests.BImpl;

import tests.A.A;
import tests.B.BEvent;

import org.osgi.framework.BundleActivator;
import org.osgi.framework.BundleContext;
import org.osgi.framework.ServiceEvent;
import org.osgi.framework.ServiceListener;
import org.osgi.util.tracker.ServiceTracker;

public class Activator
	implements BundleActivator, ServiceListener
{
	BundleContext context;
	BImpl b;
	ServiceTracker<A, A> serviceTrackerA;
	ServiceTracker<BEvent, BEvent> serviceTrackerBEvent;

	public void start(BundleContext bundleContext) throws Exception
	{	
		System.out.println(this.getClass().getPackage().getName()
			+ " consumes " + A.class.getName());
		System.out.println(this.getClass().getPackage().getName()
			+ " consumes " + BEvent.class.getName());
		context = bundleContext;

		b = new BImpl();
		b.start();
		
		serviceTrackerA = new ServiceTracker<A, A>(context, A.class, null);
		serviceTrackerA.open();
		
		if (serviceTrackerA.getService() != null) {
			this.serviceChanged(new ServiceEvent(
				ServiceEvent.REGISTERED,
				serviceTrackerA.getServiceReference()));
		}
		
		context.addServiceListener(
			this, "(objectclass=" + A.class.getName() + ")");
		
		serviceTrackerBEvent =
			new ServiceTracker<BEvent, BEvent>(context, BEvent.class, null);
		serviceTrackerBEvent.open();
		
		if (serviceTrackerBEvent.getService() != null) {
			this.serviceChanged(new ServiceEvent(
				ServiceEvent.REGISTERED,
				serviceTrackerBEvent.getServiceReference()));
		}
		
		context.addServiceListener(
			this, "(objectclass=" + BEvent.class.getName() + ")");
	}

	public void stop(BundleContext bundleContext) throws Exception
	{
		System.out.println(this.getClass().getPackage().getName()
			+ " no more consumes " + A.class.getName());
		System.out.println(this.getClass().getPackage().getName()
			+ " no more consumes " + BEvent.class.getName());
		
		b.stop();
		
		serviceTrackerA.close();
		serviceTrackerBEvent.close();
	}

	public void serviceChanged(ServiceEvent event)
	{
		Object service = context.getService(event.getServiceReference());
		String[] objClassList =
			(String[])event.getServiceReference().getProperty("objectclass");
		
		switch(event.getType()) {
		case ServiceEvent.REGISTERED:
			for (String cl : objClassList) {
				if (cl.equals(A.class.getName())) {
					b.setA((A)service);
				}
				
				if (cl.equals(BEvent.class.getName())) {
					b.setBEvent((BEvent)service);
				}
			}
				
			break;
			
		case ServiceEvent.UNREGISTERING:
			for (String cl : objClassList) {
				if (cl.equals(A.class.getName())) {
					b.setA(null);
				}
				
				if (cl.equals(BEvent.class.getName())) {
					b.setBEvent(null);
				}
			}
			break;
		}
	}
}
