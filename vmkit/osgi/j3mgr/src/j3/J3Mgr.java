package j3;

public interface J3Mgr
{
	// THE FOLLOWING METHODS ARE DEBUGGING HELPERS
	// THEY SHOULD BE REMOVED IN PRODUCTION
	
	public void dumpClassLoaderBundles() throws Throwable;
	public void dumpTierStats(String delayedSec) throws Throwable;
}
