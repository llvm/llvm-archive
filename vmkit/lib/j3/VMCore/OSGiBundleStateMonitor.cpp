
#include "OSGiBundleStateMonitor.h"

#if OSGI_BUNDLE_STATE_INFO

#include "ClasspathReflect.h"
#include "Jnjvm.h"

#include <iostream>
#include <algorithm>

#define DEBUG_VERBOSE_OSGI_STATE_CHANGE	0

using namespace std;

namespace j3 {
namespace OSGi {

void BundleStateMonitor::BundleInformation::reset()
{
	loader = NULL;
	name = NULL;
	previousLoaders.clear();
}

BundleStateMonitor::BundleStateMonitor() :
	stateEL(NULL), stateEL_user_param(NULL), frameworkFromMainClassSet(false)
{
	stateEL = new BundleStateEventListener;
}

BundleStateMonitor* BundleStateMonitor::get()
{
	vmkit::Thread* th = vmkit::Thread::get();
	assert(th && "Invalid current thread.");
	if (!th) return NULL;

	Jnjvm* vm = static_cast<Jnjvm*>(th->MyVM);
	if (!vm) return NULL;

	return &vm->bundle_state_monitor;
}

void BundleStateMonitor::setStateEventListener(
	void *user_param, BundleStateEventListener& l)
{
	stateEL_user_param = user_param;
	stateEL = &l;
}

bool BundleStateMonitor::getBundleInfoByID(
	bundle_id_t id, BundleInformation& info) const
{
	vmkit::LockGuard lg(lock);
	BundleInfoType::const_iterator i = bundleInfo.find(id), e=bundleInfo.end();
	if (i == e) return false;

	info = i->second;
	return true;
}

bool BundleStateMonitor::getBundleInfoByID(
	bundle_id_t id, BundleInfoType::iterator& iterator)
{
	vmkit::LockGuard lg(lock);
	iterator = bundleInfo.find(id);
	return (iterator != bundleInfo.end());
}

bool BundleStateMonitor::getBundleInfoByClassLoader(
	const j3::JnjvmClassLoader* loader,
	BundleInfoType::const_iterator& iterator) const
{
	vmkit::LockGuard lg(lock);
	BundleInfoType::const_iterator i = bundleInfo.begin(), e = bundleInfo.end();
	for (; i != e; ++i) {
		if (i->second.loader != loader) {
			ClassLoaderQueue::const_iterator
				plb = i->second.previousLoaders.begin(),
				ple = i->second.previousLoaders.end();

			if (std::find(plb, ple, loader) == ple)
				continue;
		}

		iterator = i;
		return true;
	}

	return false;
}

bool BundleStateMonitor::getBundleInfoByClassLoader(
	const j3::JnjvmClassLoader* loader, BundleInfoType::iterator& iterator)
{
	vmkit::LockGuard lg(lock);
	BundleInfoType::iterator i = bundleInfo.begin(), e = bundleInfo.end();
	for (; i != e; ++i) {
		if (i->second.loader != loader) {
			ClassLoaderQueue::const_iterator
				plb = i->second.previousLoaders.begin(),
				ple = i->second.previousLoaders.end();

			if (std::find(plb, ple, loader) == ple)
				continue;
		}

		iterator = i;
		return true;
	}

	return false;
}

bool BundleStateMonitor::getBundleInfoByClassLoader(
	const j3::JnjvmClassLoader* loader,
	bundle_id_t &id, BundleInformation& info) const
{
	vmkit::LockGuard lg(lock);
	BundleInfoType::const_iterator i;
	bool found = const_cast<BundleStateMonitor*>(this)->
		getBundleInfoByClassLoader(loader, i);

	if (found) {
		id = i->first;
		info = i->second;
	} else {
		id = invalidBundleID;
		info.reset();
	}
	return found;
}

size_t BundleStateMonitor::getBundleInfoByName(
	const vmkit::UTF8* name, BundleInfoType& info) const
{
	info.clear();
	size_t count = 0;

	vmkit::LockGuard lg(lock);
	BundleInfoType::const_iterator i = bundleInfo.begin(), e = bundleInfo.end();
	for (; i != e; ++i) {
		if (i->second.name->equals(name)) {
			info.insert(BundleInfoType::value_type(i->first, i->second));
			++count;
		}
	}

	return count;
}

void BundleStateMonitor::onBundleResolved(
	const j3::JavaString* bundleNameObj,
	jlong bundleID, j3::JavaObject* loaderObject)
{
	llvm_gcroot(bundleNameObj, 0);
	llvm_gcroot(loaderObject, 0);

	Jnjvm* vm = JavaThread::get()->getJVM();
	j3::JnjvmClassLoader* loader =
		j3::JnjvmClassLoader::getJnjvmLoaderFromJavaObject(loaderObject, vm);
	assert((loader != NULL) && "Invalid class loader");

	const vmkit::UTF8* bundleName =
		j3::JavaString::toUTF8(bundleNameObj, loader->hashUTF8);

	vmkit::LockGuard lg(lock);

	BundleInfoType::iterator info_iterator;
	if (getBundleInfoByID(bundleID, info_iterator) == false) {
		// Bundle installed and resolved
		BundleInformation info;
		info.loader = loader;
		info.name = bundleName;
		bundleInfo.insert(BundleInfoType::value_type(bundleID, info));

		stateEL->onBundleClassLoaderSet(
			stateEL_user_param, bundleName, bundleID, *loader);

#if DEBUG_VERBOSE_OSGI_STATE_CHANGE
		cerr << "Bundle resolved: ID=" << bundleID
			<< "\tclassLoader=" << loader << "\tname=" << *bundleName << endl;
#endif
	} else {
		BundleInformation& info = info_iterator->second;
		if (info.loader == loader)
			return;	// Same class loader already associated with the bundle

		// Bundle updated
		const j3::JnjvmClassLoader* previous_loader = info.loader;
		if (info.loader == NULL) {
			if (!info.previousLoaders.empty())
				previous_loader = info.previousLoaders.front();
		} else {
			info.previousLoaders.push_front(info.loader);
		}
		info.loader = loader;
		info.name = bundleName;

		stateEL->onBundleClassLoaderUpdated(
			stateEL_user_param, bundleName, bundleID,
			*previous_loader, *loader);

#if DEBUG_VERBOSE_OSGI_STATE_CHANGE
	cerr << "Bundle updated/resolved: ID=" << bundleID
		<< "\tclassLoader=" << loader
		<< "\tpreviousClassLoader=" << previous_loader
		<< "\tname=" << *bundleName << endl;
#endif
	}
}

void BundleStateMonitor::onBundleUnresolved(
	const j3::JavaString* bundleNameObj, jlong bundleID)
{
	llvm_gcroot(bundleNameObj, 0);

	vmkit::LockGuard lg(lock);

	// Uninstalled bundle
	BundleInfoType::iterator info_iterator;
	bool got_state = getBundleInfoByID(bundleID, info_iterator);
	assert(got_state && "Bundle unresolved but never seen.");

	BundleInformation& info = info_iterator->second;
	const vmkit::UTF8* bundleName =
		j3::JavaString::toUTF8(bundleNameObj,
			(info.loader == NULL) ? NULL : info.loader->hashUTF8);

	const j3::JnjvmClassLoader* previous_loader = info.loader;
	info.previousLoaders.push_front(info.loader);
	info.loader = NULL;

	stateEL->onBundleClassLoaderCleared(
		stateEL_user_param, bundleName, bundleID, *previous_loader);

#if DEBUG_VERBOSE_OSGI_STATE_CHANGE
	cerr << "Bundle unresolved: ID=" << bundleID
		<< "\tclassLoader=" << previous_loader
		<< "\tname=" << *bundleName << endl;
#endif
}

void BundleStateMonitor::onClassLoaderUnloaded(
	const j3::JnjvmClassLoader* loader)
{
	vmkit::LockGuard lg(lock);
	BundleInfoType::iterator info_iterator;
	if (!getBundleInfoByClassLoader(loader, info_iterator)) return;

	BundleInformation& info = info_iterator->second;
	assert((info.loader != loader) && "Bundle being unloaded while resolved.");

	info.previousLoaders.remove(loader);

#if DEBUG_VERBOSE_OSGI_STATE_CHANGE
	cerr << "Bundle class loader unloaded: ID=" << info_iterator->first
		<< "\tclassLoader=" << loader
		<< "\tname=" << *info.name << endl;
#endif

	if ((info.loader == NULL) && info.previousLoaders.empty()) {
#if DEBUG_VERBOSE_OSGI_STATE_CHANGE
	cerr << "Bundle unloaded: ID=" << info_iterator->first
		<< "\tname=" << *info.name << endl;
#endif

		bundleInfo.erase(info_iterator);
	}
}

void BundleStateMonitor::dump() const
{
	vmkit::LockGuard lg(lock);
	BundleInfoType::const_iterator i = bundleInfo.begin(), e = bundleInfo.end();
	ClassLoaderQueue::const_iterator cli, cle;

	for (; i != e; ++i) {
		cerr << "Bundle ID=" << i->first
			<< "\tname=" << *i->second.name
			<< "\tclassLoader=" << i->second.loader
			<< "\tpreviousClassLoaders={";

		cli = i->second.previousLoaders.begin();
		cle = i->second.previousLoaders.end();
		for (; cli != cle; ++cli)
			cerr << *cli << ',';

		cerr << '}' << endl;
	}
}

void BundleStateMonitor::setFrameworkFromMainClass(
	const char* mainClassName, const j3::JnjvmClassLoader* loader)
{
	// We suppose the framework bundle is the bundle loaded by the main class.

	const auto lastOccurence = strrchr(mainClassName, '.');
	if (!lastOccurence)
		return;
	string packageName(mainClassName, lastOccurence - mainClassName);

	BundleInformation info;
	info.loader = loader;
	info.name = loader->hashUTF8->lookupOrCreateAsciiz(packageName.c_str());

	vmkit::LockGuard lg(lock);
	bundleInfo.insert(BundleInfoType::value_type(0, info));

	frameworkFromMainClassSet = true;
}

}
}

extern "C" void Java_j3_vm_OSGi_bundleResolved(
	j3::JavaString* bundleName, jlong bundleID, j3::JavaObject* loaderObject)
{
	llvm_gcroot(bundleName, 0);
	llvm_gcroot(loaderObject, 0);

	j3::OSGi::BundleStateMonitor::get()->
		onBundleResolved(bundleName, bundleID, loaderObject);
}

extern "C" void Java_j3_vm_OSGi_bundleUnresolved(
	j3::JavaString* bundleName, jlong bundleID)
{
	llvm_gcroot(bundleName, 0);

	j3::OSGi::BundleStateMonitor::get()->
		onBundleUnresolved(bundleName, bundleID);
}

extern "C" void Java_j3_vm_OSGi_dumpClassLoaderBundles()
{
	j3::OSGi::BundleStateMonitor::get()->dump();
}

#endif
