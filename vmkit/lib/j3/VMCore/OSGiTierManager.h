
#ifndef OSGITIER_H_
#define OSGITIER_H_

#include "vmkit/config.h"
#include "j3/jni.h"

class gc;


namespace vmkit {

class UTF8Map;
class Thread;

}

namespace j3 {

class JnjvmClassLoader;
class CommonClass;
class JavaObject;

}

#if OSGI_BUNDLE_TIER_TAGGING

#include "AccountingConfiguration.h"

#include <set>
#include <map>
#include <atomic>

namespace j3 {
namespace OSGi {

class AccountingStatsDataBase
{
public:
	struct Record {
		size_t	object_count;

#if OSGI_STACK_SPACE_TIER_TAGGING
		size_t stack_size;
#endif

#if OSGI_OBJECT_TIER_TAGGING
		size_t heap_virtual_size, heap_static_size;
		std::set<const j3::CommonClass*> classes;
#endif
	};

	std::atomic<bool>			_now_collecting_stats;
	std::map<tier_id_t, Record>	_db;

#if OSGI_OBJECT_TIER_TAGGING
	bool _mark_state;
#endif

	AccountingStatsDataBase();
	virtual ~AccountingStatsDataBase() {}

	bool isCollectingStats() const
		{return _now_collecting_stats.load(std::memory_order_acquire);}
	void collectStats();

	friend std::ostream& operator << (
		std::ostream&, const AccountingStatsDataBase&);
};


class TierManager
{
protected:
	const BundleStateMonitor&		_bundle_state_monitor;
	AccountingStatsDataBase			_stats_data_base;
	RuntimeAccountingConfiguration	_accounting_config;
	vmkit::UTF8Map*					_hash_map;

public:
	TierManager(const BundleStateMonitor& bsm, UTF8Map* hashMap);
	virtual ~TierManager() {}

	int loadAccountingRules(
		const char* config_file, std::string& errorDescription);

	const RuntimeAccountingConfiguration& getAccountingConfig() const
		{return _accounting_config;}

	bool isCollectingStats() const
		{return _stats_data_base.isCollectingStats();}
	void dumpStats();

#if OSGI_OBJECT_TIER_TAGGING
	static void updateTierChargedToNewlyCreatedObject(j3::JavaObject* obj);
	static void accountForLiveObject(j3::JavaObject* obj);
#endif

#if OSGI_STACK_SPACE_TIER_TAGGING
	static void accountForThreadStack(vmkit::Thread* thread);
#endif
};

}
}

#if OSGI_STACK_SPACE_TIER_TAGGING

extern "C" void OSGi_TierManager_accountForThreadStack(vmkit::Thread* th);

#endif

extern "C" void Java_j3_vm_Tier_dumpTierStats();

#endif
#endif

#if OSGI_OBJECT_TIER_TAGGING

extern "C" void OSGi_TierManager_updateTierChargedToNewlyCreatedObject(gc*);
extern "C" void OSGi_TierManager_accountForLiveObject(void* obj);
extern "C" jlong Java_j3_vm_Tier_getTierChargedForObject(
	const j3::JavaObject* obj);

#endif
