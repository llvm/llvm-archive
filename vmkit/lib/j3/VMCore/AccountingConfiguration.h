
#ifndef ACCOUNTING_CONFIGURATION_H_
#define ACCOUNTING_CONFIGURATION_H_

#include <cstdlib>
#include <cstdint>
#include "vmkit/config.h"
#include "UTF8.h"

#if JAVA_CHARGED_TIER_CALL_STACK

namespace j3 {
namespace OSGi {

typedef uint16_t		tier_id_t;

static const size_t tierCountBits = 16;
static const tier_id_t invalidTierID = (1 << tierCountBits) - 1;
static const tier_id_t unspecifiedTierID = invalidTierID - 1;
static const tier_id_t runtimeTierID = 0;
static const tier_id_t frameworkTierID = 1;


class CallConfiguration
{
public:
	tier_id_t			_caller_tier_id, _called_tier_id;
	const vmkit::UTF8*	_called_bundle_name;
	const vmkit::UTF8*	_called_interface_name;
	const vmkit::UTF8*	_called_method_name;

	CallConfiguration(
		tier_id_t callerTierID = invalidTierID,
		tier_id_t calledTierID = invalidTierID,
		const vmkit::UTF8* calledBundleName = nullptr,
		const vmkit::UTF8* calledInterfaceName = nullptr,
		const vmkit::UTF8* calledMethodName = nullptr);

	void reset() {set(invalidTierID, invalidTierID, nullptr, nullptr, nullptr);}
	void set(
		tier_id_t callerTierID, tier_id_t calledTierID,
		const vmkit::UTF8* calledBundleName,
		const vmkit::UTF8* calledInterfaceName,
		const vmkit::UTF8* calledMethodName);

	int compare(const CallConfiguration&) const;
	bool genericize();

	void dump() const __attribute__((noinline));

	bool operator == (const CallConfiguration& o) const
		{return this->compare(o) == 0;}
	bool operator < (const CallConfiguration& o) const
		{return this->compare(o) < 0;}

	friend std::ostream& operator << (std::ostream&, const CallConfiguration&);
};

}
}

#endif

#if OSGI_BUNDLE_TIER_TAGGING

#include <iostream>
#include <map>
#include <set>
#include <vector>

#include "OSGiBundleStateMonitor.h"

namespace j3 {
namespace OSGi {

class AccountingConfigurationParser
{
protected:
	enum TokenID {
		token_error = -1,
		token_any_tier = -100,
		token_integer,
		token_identifier,
		token_tier,
		token_account,
		token_caller,
		token_called
	};

	const char	*_input, *_token_start;
	std::string	_identifier_token;
	int			_token, _integer_token;
	static const std::vector<std::string> _token_name;
	UTF8Map*	_hash_map;
	std::string	_errorDescription;
	class AccountingConfiguration& _config;

public:
	AccountingConfigurationParser(
		AccountingConfiguration& config, UTF8Map* hashMap);
	virtual ~AccountingConfigurationParser() {}

	static int load(AccountingConfiguration& config,
		const char* config_file, std::string& errorDescription,
		UTF8Map* hashMap);

	const std::string& getErrorDescription() const {return _errorDescription;}

protected:
	int parse(const char* sourceText);
	int parseTier();
	int parseAccount();

	int unexpectedToken(int r, const std::string& expected);
	int parseError(int r, const std::string& message);
	int nextToken();

	const std::string& tokenName(int token) const
		{return _token_name[token - token_error];}

	const UTF8* getUTF8(const char* str)
		{return _hash_map->lookupOrCreateAsciiz(str);}
};

// This configuration is supposed to be immutable before any Java code
// starts running.
class AccountingConfiguration
{
public:
	typedef std::set<
		const vmkit::UTF8* /* bundle name */,
		vmkit::UTF8_Comparator>						TierInformation;
	typedef std::map<tier_id_t, TierInformation>	TierListType;
	typedef std::map<
		const CallConfiguration,
		bool /* charge called? */>					AccountingRuleListType;

protected:
	TierListType			_tier_list;
	AccountingRuleListType	_accounting_rule_list;

public:
	virtual ~AccountingConfiguration() {}

	const TierListType& getTierList() const {return _tier_list;}
	const AccountingRuleListType& getAccountingRuleList() const
		{return _accounting_rule_list;}

	bool isValidTier(tier_id_t tierID) const
		{return (_tier_list.find(tierID) != _tier_list.end());}

	tier_id_t getTierOfBundleName(const vmkit::UTF8* bundleName) const;

	bool findNearestCallConfiguration(const CallConfiguration&,
		AccountingRuleListType::const_iterator&) const;
	bool findNearestCallConfiguration(const CallConfiguration&,
		AccountingRuleListType::iterator&);

	bool getDefaultAccountingChargedToCaller(
		const CallConfiguration& call_config) const;

	bool isAccountingChargedToCaller(
		const CallConfiguration& call_config) const;

	friend std::ostream& operator << (
		std::ostream&, const AccountingConfiguration&);
	friend std::ostream& operator << (
		std::ostream&, const TierInformation&);
	friend std::ostream& operator << (
		std::ostream&, const AccountingRuleListType&);

	friend class AccountingConfigurationParser;
};

// This class adds the variable information to the accounting configuration.
class RuntimeAccountingConfiguration :
	public AccountingConfiguration
{
protected:
	const BundleStateMonitor& _bundle_state_monitor;

public:
	RuntimeAccountingConfiguration(const BundleStateMonitor& bsm) :
		_bundle_state_monitor(bsm) {}

public:
	tier_id_t getTierOfBundleID(bundle_id_t bundleID) const;
	tier_id_t getTierOfCommonClass(const j3::CommonClass* ccl) const;

	bool setFrameworkBundleName();
};

}
}

#endif
#endif
