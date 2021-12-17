#if !defined(INC_TT_CFG_CONFIGREGISTRY_H)
#define INC_TT_CFG_CONFIGREGISTRY_H


#include <set>


namespace tt {
namespace cfg {

class ConfigHive;


/*! \brief Tracks all existing config hives, so that they can be operated on as a whole. */
class ConfigRegistry
{
public:
	static void reloadAllHives();
	
private:
	typedef std::set<ConfigHive*> ConfigHives;
	
	
	static void registerHive(ConfigHive* p_hive);
	static void unregisterHive(ConfigHive* p_hive);
	
	// No instantiation or copying
	ConfigRegistry();
	~ConfigRegistry();
	ConfigRegistry& operator=(const ConfigRegistry&);
	ConfigRegistry(const ConfigRegistry&);
	
	
	static ConfigHives* ms_allHives;
	
	
	friend class ConfigHive;  // needs access to registration functions
};

// Namespace end
}
}


#endif  // !defined(INC_TT_CFG_CONFIGREGISTRY_H)
