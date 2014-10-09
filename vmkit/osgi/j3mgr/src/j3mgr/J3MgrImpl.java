package j3mgr;

import j3.J3Mgr;

import org.osgi.framework.Bundle;
import org.osgi.framework.BundleContext;

public class J3MgrImpl
	implements J3Mgr
{
	class dumpTierStatsRunner implements Runnable
	{
		final long delayedMillisec;
		
		public dumpTierStatsRunner(long delayedMillisec)
		{
			this.delayedMillisec = delayedMillisec;
		}
		
		public void run() {
			try {
				Thread.sleep(delayedMillisec);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
			
			j3.vm.Tier.dumpTierStats();
		}
	}

	BundleContext context;
	
	public void open(BundleContext bundleContext) throws Throwable
	{
		context = bundleContext;
	}

	public void close()
	{
		context = null;
	}
		
	// THE FOLLOWING METHODS ARE DEBUGGING HELPERS
	// THEY SHOULD BE REMOVED IN PRODUCTION

	long getBundleID(String symbolicNameOrID)
	{
		try {
			long bundleID = Long.parseLong(symbolicNameOrID);
			
			if (context.getBundle(bundleID) == null) {
				System.out.println(
					"WARNING: bundleID=" + bundleID +
					" is invalid, probably already uninstalled.");
			}
			
			return (bundleID < 0) ? -1 : bundleID;
		} catch (NumberFormatException e) {
			// This is not a bundle ID, it must be a symbolic name
		}
		
		Bundle[] bundles = context.getBundles();
		for (int i=0; i < bundles.length; ++i) {
			if (symbolicNameOrID.equals(bundles[i].getSymbolicName()))
				return bundles[i].getBundleId();
		}
		return -1;
	}

	public void dumpClassLoaderBundles() throws Throwable
	{
		j3.vm.OSGi.dumpClassLoaderBundles();
	}
			
	public void dumpTierStats(String delayedSec) throws Throwable
	{
		long delay = Long.parseLong(delayedSec);
		Runnable runner = new dumpTierStatsRunner(delay * 1000);
		
		if (delay <= 0) {
			runner.run();
		} else {
			System.out.println("dumpTierStats runs in delayed mode...");
			new Thread(runner, "dumpTierStats").start();
		}
	}
}
