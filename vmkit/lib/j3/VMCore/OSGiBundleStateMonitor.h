
#ifndef OSGIGATEWAY_H_
#define OSGIGATEWAY_H_

#include <vmkit/config.h>

#if OSGI_BUNDLE_STATE_INFO

#include "vmkit/Locks.h"
#include "vmkit/UTF8.h"
#include "j3/jni.h"
#include "JavaString.h"

#include <map>
#include <list>

#include <stdint.h>

namespace j3 {

class JnjvmClassLoader;
class JavaObjectClass;
class JavaObject;

namespace OSGi {

typedef int64_t	bundle_id_t;
static const bundle_id_t invalidBundleID = (bundle_id_t)-1;


class BundleStateEventListener
{
public:
	virtual ~BundleStateEventListener() {}

	virtual void onBundleClassLoaderSet(void *user_param,
		const vmkit::UTF8* bundleName, bundle_id_t bundleID,
		const j3::JnjvmClassLoader& loader) {}

	virtual void onBundleClassLoaderUpdated(void *user_param,
		const vmkit::UTF8* bundleName, bundle_id_t bundleID,
		const j3::JnjvmClassLoader& previous_loader,
		const j3::JnjvmClassLoader& new_loader) {}

	virtual void onBundleClassLoaderCleared(void *user_param,
		const vmkit::UTF8* bundleName, bundle_id_t bundleID,
		const j3::JnjvmClassLoader& previous_loader) {}
};


class BundleStateMonitor
{
public:
	typedef std::list<const j3::JnjvmClassLoader*>	ClassLoaderQueue;

	struct BundleInformation {
		const j3::JnjvmClassLoader*	loader;
		const vmkit::UTF8*			name;
		ClassLoaderQueue			previousLoaders;

		BundleInformation() : loader(nullptr), name(nullptr) {}
		void reset();
	};

	typedef std::map<bundle_id_t, BundleInformation> BundleInfoType;

protected:

	mutable vmkit::LockRecursive lock;
	BundleInfoType bundleInfo;
	BundleStateEventListener *stateEL;
	void *stateEL_user_param;
	bool frameworkFromMainClassSet;

public:
	BundleStateMonitor();
	virtual ~BundleStateMonitor() {}

	static BundleStateMonitor* get();

	bool getBundleInfoByID(
		bundle_id_t id, BundleInformation& info) const;
	bool getBundleInfoByID(
		bundle_id_t id, BundleInfoType::iterator& iterator);
	bool getBundleInfoByClassLoader(const j3::JnjvmClassLoader* loader,
		bundle_id_t &id, BundleInformation& info) const;
	bool getBundleInfoByClassLoader(const j3::JnjvmClassLoader* loader,
		BundleInfoType::iterator& iterator);
	bool getBundleInfoByClassLoader(const j3::JnjvmClassLoader* loader,
		BundleInfoType::const_iterator& iterator) const;
	size_t getBundleInfoByName(
		const vmkit::UTF8* name, BundleInfoType& info) const;

	void dump() const;

	void setFrameworkFromMainClass(
		const char* mainClassName, const j3::JnjvmClassLoader* loader);
	bool isFrameworkFromMainClassSet() {return frameworkFromMainClassSet;}

	void setStateEventListener(
		void *user_param, BundleStateEventListener& l);

	void onBundleResolved(const j3::JavaString* bundleName, jlong bundleID,
		j3::JavaObject* loaderObject);
	void onBundleUnresolved(const j3::JavaString* bundleName, jlong bundleID);
	void onClassLoaderUnloaded(const j3::JnjvmClassLoader* loader);
};

}
}

extern "C" void Java_j3_vm_OSGi_bundleResolved(
	j3::JavaString* bundleName, jlong bundleID, j3::JavaObject* loaderObject);
extern "C" void Java_j3_vm_OSGi_bundleUnresolved(
	j3::JavaString* bundleName, jlong bundleID);
extern "C" void Java_j3_vm_OSGi_dumpClassLoaderBundles();

#endif
#endif
