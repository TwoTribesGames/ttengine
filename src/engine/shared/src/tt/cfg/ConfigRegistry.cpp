#include <tt/cfg/ConfigHive.h>
#include <tt/cfg/ConfigRegistry.h>
#include <tt/code/helpers.h>
#include <tt/platform/tt_error.h>


namespace tt {
namespace cfg {

ConfigRegistry::ConfigHives* ConfigRegistry::ms_allHives = 0;


//--------------------------------------------------------------------------------------------------
// Public member functions

void ConfigRegistry::reloadAllHives()
{
	if (ms_allHives == 0) return;
	
	for (ConfigHives::iterator it = ms_allHives->begin(); it != ms_allHives->end(); ++it)
	{
		(*it)->reload();
	}
}


//--------------------------------------------------------------------------------------------------
// Private member functions

void ConfigRegistry::registerHive(ConfigHive* p_hive)
{
	if (p_hive == 0)
	{
		TT_PANIC("Cannot register null pointers to config hives.");
		return;
	}
	
	if (ms_allHives == 0)
	{
		// First registration: create storage for registry
		ms_allHives = new ConfigHives;
	}
	
	// Ensure hives are only registered once
	if (ms_allHives->find(p_hive) != ms_allHives->end())
	{
		TT_PANIC("Internal consistency error: config hive 0x%08X is already registered.",
		         p_hive);
		return;
	}
	
	ms_allHives->insert(p_hive);
}


void ConfigRegistry::unregisterHive(ConfigHive* p_hive)
{
	if (p_hive == 0)
	{
		TT_PANIC("Cannot register null pointers to config hives.");
		return;
	}
	
	if (ms_allHives == 0)
	{
		TT_PANIC("Internal consistency error: config hive 0x%08X isn't registered, cannot unregister.",
		         p_hive);
		return;
	}
	
	ConfigHives::iterator it = ms_allHives->find(p_hive);
	if (it == ms_allHives->end())
	{
		TT_PANIC("Internal consistency error: config hive 0x%08X isn't registered, cannot unregister.",
		         p_hive);
		return;
	}
	
	ms_allHives->erase(it);
	
	// If the last hive is unregistered, also free the registry storage
	if (ms_allHives->empty())
	{
		code::helpers::safeDelete(ms_allHives);
	}
}

// Namespace end
}
}
