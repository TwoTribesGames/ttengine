#include <tt/fs/fs.h>
#include <tt/platform/tt_error.h>

#include <toki/cfg.h>


namespace toki {

const tt::cfg::ConfigHivePtr& cfg()
{
	static tt::cfg::ConfigHivePtr hive;
	if (hive == 0)
	{
		std::string filename("config/config.bcfg");
		
		if (tt::fs::fileExists(filename))
		{
			hive = tt::cfg::ConfigHive::load(filename);
			TT_ASSERTMSG(hive != 0, "Loading configuration hive from file '%s' failed.", filename.c_str());
		}
		else
		{
			TT_PANIC("Binary config file not found, please update and run conversion.\nFilename: '%s'",
			         filename.c_str());
		}
	}
	
	return hive;
}

// Namespace end
}
