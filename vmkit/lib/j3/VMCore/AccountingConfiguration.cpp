
#include "AccountingConfiguration.h"

#if JAVA_CHARGED_TIER_CALL_STACK


namespace j3 {
namespace OSGi {

//#########################################################################
//
//                        CallConfiguration
//
//#########################################################################

CallConfiguration::CallConfiguration(
	tier_id_t callerTierID, tier_id_t calledTierID,
	const vmkit::UTF8* calledBundleName, const vmkit::UTF8* calledInterfaceName,
	const vmkit::UTF8* calledMethodName)
{
	set(callerTierID, calledTierID,
		calledBundleName, calledInterfaceName, calledMethodName);
}

void CallConfiguration::set(
	tier_id_t callerTierID, tier_id_t calledTierID,
	const vmkit::UTF8* calledBundleName, const vmkit::UTF8* calledInterfaceName,
	const vmkit::UTF8* calledMethodName)
{
	_caller_tier_id = callerTierID;
	_called_tier_id = calledTierID;
	_called_interface_name = calledInterfaceName;
	_called_method_name = calledMethodName;

	// The runtime does not hold a bundle name.
	_called_bundle_name =
		((calledBundleName != nullptr) && (calledTierID != runtimeTierID)) ?
		calledBundleName :
		nullptr;
}

int CallConfiguration::compare(const CallConfiguration& other) const
{
	// Compare based on caller tiers, if they are not equal and not generic
	int diff = 0;

	if (_caller_tier_id != invalidTierID &&
		other._caller_tier_id != invalidTierID)
	{
		diff = (int)(_caller_tier_id - other._caller_tier_id);
		if (diff != 0) return diff;
	}

	// Compare based on called tiers, if they are not equal not generic
	if (_called_tier_id != invalidTierID &&
		other._called_tier_id != invalidTierID)
	{
		diff = (int)(_called_tier_id - other._called_tier_id);
		if (diff != 0) return diff;
	}

	// Compare based on bundleName, if they are not equal
	diff = UTF8::compare(_called_bundle_name, other._called_bundle_name);
	if (diff != 0) return diff;

	// Compare based on interfaceName
	diff = UTF8::compare(_called_interface_name, other._called_interface_name);
	if (diff != 0) return diff;

	return UTF8::compare(_called_method_name, other._called_method_name);
}

bool CallConfiguration::genericize()
{
	if (_called_method_name != nullptr) {
		_called_method_name = nullptr; return true;
	}
	if (_called_interface_name != nullptr) {
		_called_interface_name = nullptr; return true;
	}
	if (_called_bundle_name != nullptr) {
		_called_bundle_name = nullptr; return true;
	}
	return false;
}

std::ostream& operator << (std::ostream& os, const CallConfiguration& obj)
{
	if (obj._caller_tier_id == invalidTierID)
		os << '*';
	else
		os << (int)obj._caller_tier_id;

	os << " => ";

	if (obj._called_tier_id == invalidTierID)
		os << '*';
	else
		os << (int)obj._called_tier_id;

	if (obj._called_bundle_name != nullptr)
		os << " bundle=" << *obj._called_bundle_name;

	if (obj._called_interface_name != nullptr)
		os << " interface=" << *obj._called_interface_name;

	if (obj._called_method_name != nullptr)
		os << " method=" << *obj._called_method_name;
	return os;
}

void CallConfiguration::dump() const
{
	std::cout << *this << std::endl;
}

}
}

#endif

#if OSGI_BUNDLE_TIER_TAGGING

#include <fstream>
#include <cstdlib>
#include <errno.h>

#include "JnjvmClassLoader.h"
#include "JavaThread.h"
#include "Jnjvm.h"
#include "JavaClass.h"

using namespace std;

template<class ContainerType, class ConstIteratorType, class IteratorType>
IteratorType& const_cast_iterator(
	ContainerType& cont, ConstIteratorType& const_iter, IteratorType& iter)
{
	iter = cont.begin();
	std::advance(iter, std::distance<ConstIteratorType>(iter, const_iter));
	return iter;
}

namespace j3 {
namespace OSGi {

//#########################################################################
//
//                     AccountingConfigurationParser
//
//#########################################################################

const std::vector<std::string> AccountingConfigurationParser::_token_name {
	"error",
	"*",
	"integer",
	"identifier",
	"tier",
	"account",
	"caller",
	"called"
};

AccountingConfigurationParser::AccountingConfigurationParser(
	AccountingConfiguration& config, UTF8Map* hashMap) :
	_input(nullptr), _token_start(nullptr), _token(0), _integer_token(0),
	_hash_map(hashMap), _errorDescription("Success"), _config(config)
{
}

int AccountingConfigurationParser::load(
	AccountingConfiguration& config,
	const char* config_file, std::string& errorDescription, UTF8Map* hashMap)
{
	auto r = 0;
	ifstream f;
	f.open(config_file);
	if (!f.is_open()) {
		r = errno;
		errorDescription = strerror(r);
		return r;
	}

	f.seekg(0, fstream::end);
	size_t fsize = f.tellg();
	f.seekg(0, fstream::beg);
	if (fsize == 0) return 0;

	unique_ptr<char[]> buffer(new char[fsize+1]);
	f.read(buffer.get(), fsize);
	f.close();

	buffer[fsize] = '\0';

	AccountingConfigurationParser parser(config, hashMap);
	r = parser.parse(buffer.get());

	errorDescription = parser.getErrorDescription();
	return r;
}

int AccountingConfigurationParser::parseError(int r, const std::string& message)
{
	_errorDescription = message + ". At: '";
	_errorDescription.append(_token_start, 32);
	_errorDescription += "'...";
	return r;
}

int AccountingConfigurationParser::unexpectedToken(int r, const std::string& expected)
{
	return parseError(
		r, "Expecting " + expected + ". Got '" + tokenName(_token) + "'");
}

int AccountingConfigurationParser::parseTier()
{
	if (_token != token_tier) return ENOENT;

	if ((_token = nextToken()) != token_integer)
		return unexpectedToken(EBADF, "tier identifier (integer)");

	tier_id_t tier_id = _integer_token;

	if (tier_id == unspecifiedTierID || tier_id == runtimeTierID)
		return unexpectedToken(EBADF, "non-reserved tier identifier");

	if ((_token = nextToken()) != '{')
		return unexpectedToken(EBADF, "'{'");

	auto& bundles = _config._tier_list.insert(
		AccountingConfiguration::TierListType::value_type(
			tier_id, AccountingConfiguration::TierInformation()))
		.first->second;

	_token = nextToken();
	while (_token != '}') {
		if (_token != token_identifier)
			return unexpectedToken(EBADF, "bundle name (identifier) or '}'");

		bundles.insert(getUTF8(_identifier_token.c_str()));

		if ((_token = nextToken()) == ',')
			_token = nextToken();
	}

	return 0;
}

int AccountingConfigurationParser::parseAccount()
{
	if (_token != token_account) return ENOENT;

	if ((_token = nextToken()) != '{')
		return unexpectedToken(EBADF, "'{'");

	tier_id_t caller_tier_id, called_tier_id;

	_token = nextToken();
	if (_token == token_any_tier)
		caller_tier_id = invalidTierID;
	else if (_token == token_integer)
		caller_tier_id = _integer_token;
	else
		return unexpectedToken(EBADF, "caller tier identifier (integer)");

	if ((_token = nextToken()) != ',')
		return unexpectedToken(EBADF, "','");

	_token = nextToken();
	if (_token == token_any_tier)
		called_tier_id = invalidTierID;
	else if (_token == token_integer)
		called_tier_id = _integer_token;
	else
		return unexpectedToken(EBADF, "called tier identifier (integer)");

	const UTF8 *bundle_name = nullptr, *interface_name = nullptr;
	const UTF8 *method_name = nullptr;

	if ((_token = nextToken()) == '/') {
		if ((_token = nextToken()) != token_identifier)
			return unexpectedToken(EBADF, "bundle name (identifier)");

		if (_identifier_token.length() > 0 &&
			_identifier_token.compare("_") != 0)
		{
			bundle_name = getUTF8(_identifier_token.c_str());
		}

		if ((_token = nextToken()) == '/') {
			if ((_token = nextToken()) != token_identifier)
				return unexpectedToken(EBADF, "interface name (identifier)");

			if (_identifier_token.length() > 0) {
				// Convert interface name to internal name
				std::replace(_identifier_token.begin(), _identifier_token.end(), '.', '/');
				interface_name = getUTF8(_identifier_token.c_str());
			}

			if ((_token = nextToken()) == '/') {
				if ((_token = nextToken()) != token_identifier)
					return unexpectedToken(EBADF, "method name (identifier)");

				if (_identifier_token.length() > 0)
					method_name = getUTF8(_identifier_token.c_str());
				_token = nextToken();
			}
		}
	}

	if (_token != ',')
		return unexpectedToken(EBADF, "'/' or ','");

	bool charge_called;

	_token = nextToken();
	if (_token == token_called)
		charge_called = true;
	else if (_token == token_caller)
		charge_called = false;
	else
		return unexpectedToken(EBADF, "'caller' or 'called'");

	if ((_token = nextToken()) != '}')
		return unexpectedToken(EBADF, "'}'");

	_config._accounting_rule_list.insert(
		AccountingConfiguration::AccountingRuleListType::value_type(
			CallConfiguration(caller_tier_id, called_tier_id,
				bundle_name, interface_name, method_name),
			charge_called));

	return 0;
}

int AccountingConfigurationParser::parse(const char* sourceText)
{
	if (!sourceText || !(*sourceText)) return 0;

	_input = sourceText;

	auto r = 0;

	for (;;) {
		if ((_token = nextToken()) == 0) return 0;	// Done
		if (_token == token_error)
			return unexpectedToken(EBADF, "'tier' or 'account'");

		if ((r = parseTier()) == 0) continue;
		if (r != ENOENT) return r;

		if ((r = parseAccount()) == 0) continue;
		if (r != ENOENT) return r;

		return unexpectedToken(EBADF, "'tier' or 'account'");
	}

	return r;
}

int AccountingConfigurationParser::nextToken()
{
	while (*_input != '\0') {
		for (; isspace(*_input); ++_input);	// Skip spaces and tabs

		if (*_input == '#') {	// Comment
			for (; (*_input != '\n') && (*_input != '\0'); ++_input);
			for (; (*_input == '\n') || (*_input == '\r'); ++_input);
		} else {
			break;
		}
	}
	if (!(*_input)) return 0;

	_token_start = _input;
	if (*_input == '*') {++_input; return token_any_tier;}

	if (isdigit(*_input)) {
		char *endp = nullptr;

		_integer_token = (int)strtol(_input, &endp, 10);
		if (errno == ERANGE)
			return parseError(token_error, "Integer is out of range");

		_input = endp;
		return token_integer;
	}

	if ((*_input == '_') || (*_input == '-') || isalpha(*_input)) {
		const char* start = _input;
		for (++_input; (*_input == '_') || (*_input == '-') ||
			(*_input == '.') || isalnum(*_input);
			++_input);

		size_t count = _input - start;
		if (count == 4 && !strncmp(start, "tier", count))
			return token_tier;
		if (count == 7 && !strncmp(start, "account", count))
			return token_account;
		if (count == 6 && !strncmp(start, "caller", count))
			return token_caller;
		if (count == 6 && !strncmp(start, "called", count))
			return token_called;

		_identifier_token.assign(start, count);
		return token_identifier;
	}

	return *(_input++);
}

//#########################################################################
//
//                        AccountingConfiguration
//
//#########################################################################

bool RuntimeAccountingConfiguration::setFrameworkBundleName()
{
	BundleStateMonitor::BundleInformation bi;
	if (!_bundle_state_monitor.getBundleInfoByID(0, bi)) return false;

	TierInformation ti { bi.name };
	_tier_list[frameworkTierID] = ti;
	return true;
}

tier_id_t AccountingConfiguration::getTierOfBundleName(
	const vmkit::UTF8* bundleName) const
{
	auto e = _tier_list.end();
	auto f = std::find_if(_tier_list.begin(), e,
		[bundleName] (const TierListType::value_type& element) {
			assert(element.first != unspecifiedTierID &&
				element.first != runtimeTierID);
			return element.second.find(bundleName) != element.second.end();
		});

	// Some bundles are not associated with any tiers
	return (f == e) ? unspecifiedTierID : f->first;
}

bool AccountingConfiguration::findNearestCallConfiguration(
	const CallConfiguration& call_config,
	AccountingRuleListType::const_iterator& found) const
{
	if (call_config._called_tier_id == invalidTierID) return false;

	// Look for the rule, from exact form to all possible generic forms.
	auto e = _accounting_rule_list.end();
	auto f = e;
	auto notFound = false;
	auto called(call_config);

	do {
		f = std::find_if(_accounting_rule_list.begin(), e,
			[&called] (const AccountingRuleListType::value_type& element) {
				return element.first == called;
		});
		notFound = (f == e);
	} while (notFound && called.genericize());

	if (!notFound) found = f;
	return !notFound;
}

bool AccountingConfiguration::findNearestCallConfiguration(
	const CallConfiguration& call_config,
	AccountingRuleListType::iterator& found)
{
	AccountingRuleListType::const_iterator i;
	auto r = this->findNearestCallConfiguration(call_config, i);
	if (r) const_cast_iterator(_accounting_rule_list, i, found);
	return r;
}

bool AccountingConfiguration::getDefaultAccountingChargedToCaller(
	const CallConfiguration& call_config) const
{
	// By default, the runtime is not charged for resources allocated
	// in order to serve requests. However, it is charged for independent
	// threads (running only runtime code).

	if (call_config._caller_tier_id == runtimeTierID &&
		call_config._called_tier_id == runtimeTierID)
		return true;

	if (call_config._caller_tier_id == runtimeTierID)
		return false;

	if (call_config._called_tier_id == runtimeTierID)
		return true;

	return true;	// Default behavior: charge to the caller
}

bool AccountingConfiguration::isAccountingChargedToCaller(
	const CallConfiguration& call_config) const
{
	assert(((call_config._caller_tier_id != invalidTierID)
		&& (call_config._called_tier_id != invalidTierID))
		&& "Invalid tierID for caller and called");

	if ((call_config._caller_tier_id != call_config._called_tier_id) ||
		(call_config._called_bundle_name != nullptr) ||
		(call_config._called_interface_name != nullptr) ||
		(call_config._called_method_name != nullptr))
	{
		AccountingRuleListType::const_iterator i;
		if (this->findNearestCallConfiguration(call_config, i))
			return !i->second;
	}

	return getDefaultAccountingChargedToCaller(call_config);
}

std::ostream& operator << (
	std::ostream& os, const AccountingConfiguration::TierInformation& obj)
{
	os << '{';
	for (const auto& i : obj)
		os << *i << ' ';
	return os << '}';
}

std::ostream& operator << (std::ostream& os,
	const AccountingConfiguration::AccountingRuleListType& obj)
{
	os << '{';
	for (const auto& i : obj) {
		os << i.first << (i.second ? ":called " : ":caller ");
	}
	return os << '}';
}

std::ostream& operator << (std::ostream& os, const AccountingConfiguration& obj)
{
	os << "AccountingConfiguration=tiers{";

	for (const auto& i : obj.getTierList())
		os << (int)i.first << '=' << i.second << ',';

	return os << "},rules=" << obj.getAccountingRuleList() << endl;
}

//#########################################################################
//
//                    RuntimeAccountingConfiguration
//
//#########################################################################

tier_id_t RuntimeAccountingConfiguration::getTierOfBundleID(
	bundle_id_t bundleID) const
{
	BundleStateMonitor::BundleInformation bi;
	if (!_bundle_state_monitor.getBundleInfoByID(bundleID, bi))
		return invalidTierID;	// Invalid bundle ID

	return getTierOfBundleName(bi.name);
}

tier_id_t RuntimeAccountingConfiguration::getTierOfCommonClass(
	const j3::CommonClass* ccl) const
{
	BundleStateMonitor::BundleInfoType::const_iterator i;
	if (!_bundle_state_monitor.getBundleInfoByClassLoader(ccl->classLoader, i)){
		// Class loader without a bundle ID: Not a bundle class loader.
		return runtimeTierID;
	}

	return getTierOfBundleName(i->second.name);
}

}
}

#endif
