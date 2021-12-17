#include <stdlib.h>
#include <tt/fs/fs.h>
#include <tt/fs/utils/utils.h>
#include <tt/platform/tt_error.h>
#include <tt/system/utils.h>
#include <tt/str/manip.h>

#include <fstream>

namespace tt {
namespace system {

bool openWithDefaultApplication(const std::string& p_item)
{
	std::string shellCmd("xdg-open ");
	shellCmd += p_item;

	return std::system(shellCmd.c_str()) >= 0;
}


bool editWithDefaultApplication(const std::string& p_item)
{
	return openWithDefaultApplication(p_item);
}


bool showFileInFileNavigator(const std::string& p_item)
{
	// Show the file path in the dile manager
	return openWithDefaultApplication(fs::utils::getDirectory(p_item));
}

#define ensureTrailingSlash(s) if (*s.rbegin() != '/') s += "/"


static std::string find_xdg_config_dir()
{
	std::string path;
	const char* env = getenv("XDG_CONFIG_HOME");
	if (env) {
		path = env;
		ensureTrailingSlash(path);
	} else {
		env = getenv("HOME");
		if (!env) {
			return "";
		}
		path = env;
		ensureTrailingSlash(path);
		path += ".config/";
	}
	return path;
}

static std::string find_xdg_user_dir(const std::string name)
{
	std::string path;
	std::string match = "XDG_" + name + "_DIR";

	std::string cfg_dir = find_xdg_config_dir();

	if (!cfg_dir.empty())
	{
		std::string cfg_file = cfg_dir + "user-dirs.dirs";
		if (fs::fileExists(cfg_file))
		{
			std::ifstream fin(cfg_file.c_str());
			if (fin)
			{
				std::string line;
				while (std::getline(fin, line))
				{
					if (line.empty() || line[0] == '#') continue;
					str::Strings parts = str::explode(line, "=");
					if (parts.size() == 2)
					{
						std::string key = str::trim(parts[0]);
						if (key == match)
						{
							path = str::trim(parts[1]);
							break;
						}
					}
				}
			}
		}
	}
	const char* home =  getenv("HOME");
	if (home)
	{
		size_t pos = path.find("$HOME");
		if (pos != path.npos)
		{
			path.replace(pos, pos + 5, home);
		}
		pos = path.find("${HOME}");
		if (pos != path.npos)
		{
			path.replace(pos, pos + 5, home);
		}
	}
	if (path.empty() || !fs::dirExists(path))
	{
		if (home)
		{
			path = home;
			ensureTrailingSlash(path);
			if (name == "DESKTOP") {
				if (fs::dirExists(path + "Desktop")) path += "Desktop/";
			}
		} else {
			path = "";
		}
	}
	ensureTrailingSlash(path);
	return path;
}

std::string getDesktopPath()
{
	static std::string s_desktopDir = find_xdg_user_dir("DESKTOP");
	
	return s_desktopDir;
}


// Namespace end
}
}
