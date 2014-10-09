
#include "OSGiTierManager.h"
#include "vmkit/StackEmbeddedList.h"

#if OSGI_BUNDLE_TIER_TAGGING

#include "JavaThread.h"
#include "Jnjvm.h"
#include "VMStaticInstance.h"

#include <iostream>
#include <stack>

#include <dlfcn.h>

using namespace std;

#define DEBUG_VERBOSE_ACCOUNTING_OBJECTS	0
#define DEBUG_VERBOSE_ACCOUNTING_STACK		1

namespace j3 {
namespace OSGi {

//#########################################################################
//
//                         AccountingStatsDataBase
//
//#########################################################################

AccountingStatsDataBase::AccountingStatsDataBase() :
	_now_collecting_stats(false)

#if OSGI_OBJECT_TIER_TAGGING
	, _mark_state(false)
#endif
{
}

void AccountingStatsDataBase::collectStats()
{
	// Initialize counters
	_db.clear();

#if OSGI_OBJECT_TIER_TAGGING
	_mark_state = !_mark_state;
#endif

	// Perform a garbage collection to stop all threads and count memory usage
	_now_collecting_stats.store(true, memory_order_release);

	vmkit::Collector::collect();

	_now_collecting_stats.store(false, memory_order_release);
}

std::ostream& operator << (std::ostream& os, const AccountingStatsDataBase& db)
{
	os << "Tier memory dump:" << endl;

	for (const auto& i : db._db) {
		os << "Tier " << (int)i.first;
#if OSGI_STACK_SPACE_TIER_TAGGING
		os << "\tstackSize=" << i.second.stack_size;
#endif

#if OSGI_OBJECT_TIER_TAGGING
		os << "\tobjectCount=" << i.second.object_count <<
			"\tclassCount=" << i.second.classes.size() <<
			"\theapStaticSize=" << i.second.heap_static_size <<
			"\theapVirtualSize=" << i.second.heap_virtual_size;
#endif
		os << endl;
	}
	return os << "Tier memory dump [done]." << endl;
}

//#########################################################################
//
//                            TierManager
//
//#########################################################################

TierManager::TierManager(const BundleStateMonitor& bsm, UTF8Map* hashMap) :
	_bundle_state_monitor(bsm), _accounting_config(bsm), _hash_map(hashMap)
{
}

int TierManager::loadAccountingRules(
	const char* config_file, std::string& errorDescription)
{
	_accounting_config.setFrameworkBundleName();

	return AccountingConfigurationParser::load(
		_accounting_config, config_file, errorDescription, _hash_map);
}

void TierManager::dumpStats()
{
	_stats_data_base.collectStats();
	cout << _stats_data_base << endl;
}

#if OSGI_OBJECT_TIER_TAGGING

void TierManager::updateTierChargedToNewlyCreatedObject(j3::JavaObject* obj)
{
	llvm_gcroot(obj, 0);

	auto thread = j3::JavaThread::get();

	obj->setTierID(
		(thread->chargedTierID == (uint32_t)-1) ?
			runtimeTierID :
			(tier_id_t)thread->chargedTierID);

	obj->setTierObjectMarkState(
		thread->getJVM()->tier_manager._stats_data_base._mark_state);
}

void TierManager::accountForLiveObject(j3::JavaObject* obj)
{
	llvm_gcroot(obj, 0);

	auto& tm = j3::JavaThread::get()->getJVM()->tier_manager;
	if (!tm.isCollectingStats()) return;

	if (j3::VMClassLoader::isVMClassLoader(obj) ||
		j3::VMStaticInstance::isVMStaticInstance(obj))
		return;

	if (obj->getTierObjectMarkState() == tm._stats_data_base._mark_state)
		return;	// Already accounted for

	obj->setTierObjectMarkState(tm._stats_data_base._mark_state);

	const auto cl = j3::JavaObject::getClass(obj)->asClass();
	if (!cl) return;

	tier_id_t tierID = (tier_id_t)obj->getTierID();
	auto& tier_iter = tm._stats_data_base._db[tierID];

#if DEBUG_VERBOSE_ACCOUNTING_OBJECTS
	if (tierID != 0)
		std::cerr << "Tier " << (int)tierID << " object " << *obj << std::endl;
#endif

	++tier_iter.object_count;
	tier_iter.heap_virtual_size += cl->getVirtualSize();

	if (tier_iter.classes.insert(cl).second)
		tier_iter.heap_static_size += cl->getStaticSize();
}

#endif

#if OSGI_STACK_SPACE_TIER_TAGGING

void TierManager::accountForThreadStack(vmkit::Thread* thread)
{
	/* x86 stack frame:

|-| (last method called)
| |     ...
| | BP(i-2)
| |  |  ret addr(i-2)
| |  |  args(i-2)
|a|  |  saved regs(i-1)
|d|  V  locals(i-1)
|d| BP(i-1)
|r|  |  ret addr(i-1)
|e|  |  args(i-1)
|s|  |  saved regs(i)
|s|  V  locals(i)
| | BP(i  )
| |    ret addr(i)
| |    args(i)
| |    ...
|+| (first caller method)

    BP := call frame address (base pointer).
*/

	auto& tm = j3::JavaThread::get()->getJVM()->tier_manager;
	if (!tm.isCollectingStats()) return;

	auto& db = tm._stats_data_base._db;
	vmkit::StackWalker stackWalker(thread);
	std::stack<vmkit::StackWalker> stackFrames;
	auto lastFrameAddress = stackWalker.callFrameAddress;
	size_t frameSize;

	vmkit::StackEmbeddedListNode *current_node =
		thread->stackEmbeddedListHead[vmkit::StackEmbeddedListChargedTier];

	for (; current_node != nullptr; current_node = current_node->callerNode) {
		// Look for the method frame holding the node
		auto nodeStart = reinterpret_cast<word_t>(current_node);
		auto nodeEnd = nodeStart + sizeof(*current_node);
		auto callFrameStart = stackWalker.callFrameAddress;
		auto callFrameEnd = callFrameStart;
		auto checkPointFrameAddress = stackWalker.callFrameAddress;
		auto found = false;

		for (;
			!found && stackWalker.get() != nullptr;
			stackFrames.push(stackWalker), ++stackWalker)
		{
			callFrameEnd = stackWalker.callFrameAddress;
			found = (nodeStart >= callFrameStart) && (nodeEnd <= callFrameEnd);
			callFrameStart = callFrameEnd;
		}

		if (!found) {
			// Incomplete stack :-(
			while (stackFrames.size() != 0 &&
				stackFrames.top().callFrameAddress != checkPointFrameAddress)
			{
				stackFrames.pop();
			}

			stackWalker = stackFrames.top();
			stackFrames.pop();
			continue;
		}

		if (stackFrames.size() < 2) {
			// Incomplete stack :-(
			continue;
		}

		// Method frame found
		callFrameEnd = stackFrames.top().callFrameAddress;
		frameSize = callFrameEnd - lastFrameAddress;

		stackFrames.pop();
		stackWalker = stackFrames.top();
		stackFrames.pop();

#if DEBUG_VERBOSE_ACCOUNTING_STACK
		cout << "# [" << current_node->data[0] << " += "
			<< frameSize << "]\t";

		if (const auto FI = stackWalker.get()) {
			if (!FI->Metadata) {
				Dl_info si = {};
				auto ra =reinterpret_cast<void*>(stackWalker.returnAddress);
				if (dladdr(ra, &si) != 0 && si.dli_sname != nullptr)
					cout << si.dli_sname << endl;
				else
					cout << "<native>" << endl;
			} else {
				cout << *reinterpret_cast<const JavaMethod*>(FI->Metadata)
					<< endl;
			}
		}
#endif

		db[current_node->data[0]].stack_size += frameSize;
		lastFrameAddress = callFrameEnd;
	}

	frameSize = thread->baseSP - lastFrameAddress;
	db[runtimeTierID].stack_size += frameSize;

#if DEBUG_VERBOSE_ACCOUNTING_STACK
	cout << "# [0 += " << frameSize << "]\t<native>" << endl << endl;
#endif
}

#endif

}
}

#if OSGI_STACK_SPACE_TIER_TAGGING

extern "C" void OSGi_TierManager_accountForThreadStack(vmkit::Thread* thread)
{
	if (!thread) return;
	j3::OSGi::TierManager::accountForThreadStack(thread);
}

#endif

extern "C" void Java_j3_vm_Tier_dumpTierStats()
{
	auto& tm = j3::JavaThread::get()->getJVM()->tier_manager;
	tm.dumpStats();
}

#endif

#if OSGI_OBJECT_TIER_TAGGING

extern "C" void OSGi_TierManager_updateTierChargedToNewlyCreatedObject(gc* obj)
{
#if OSGI_BUNDLE_TIER_TAGGING
	llvm_gcroot(obj, 0);

	if (!obj) return;
	j3::OSGi::TierManager::updateTierChargedToNewlyCreatedObject(
		static_cast<j3::JavaObject*>(obj));
#endif
}

extern "C" void OSGi_TierManager_accountForLiveObject(void* obj)
{
#if OSGI_BUNDLE_TIER_TAGGING
	llvm_gcroot(obj, 0);

	if (!obj) return;
	j3::OSGi::TierManager::accountForLiveObject(
		static_cast<j3::JavaObject*>(obj));
#endif
}

extern "C" jlong Java_j3_vm_Tier_getTierChargedForObject(
	const j3::JavaObject* obj)
{
#if OSGI_BUNDLE_TIER_TAGGING
	llvm_gcroot(obj, 0);
	return obj->getTierID();
#else
	return 0;
#endif
}

#endif
