
package j3mgr;

import j3.J3Mgr;

import org.osgi.framework.BundleActivator;
import org.osgi.framework.BundleContext;

public class Activator
	implements BundleActivator
{
	J3MgrImpl j3mgr;
	
	public void start(BundleContext context) throws Exception
	{
		j3mgr = new J3MgrImpl();
		try {
			j3mgr.open(context);
		} catch (Throwable e) {throw new Exception(e);}
		
		context.registerService(J3Mgr.class, j3mgr, null);
	}
	
	public void stop(BundleContext context) throws Exception
	{
		j3mgr.close();
	}
}