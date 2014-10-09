package j3.vm;

public class OSGi
{
	public static native void bundleResolved(
		String bundleName, long bundleID, ClassLoader loaderObject);
	public static native void bundleUnresolved(
		String bundleName, long bundleID);
	public static native void dumpClassLoaderBundles();
}
