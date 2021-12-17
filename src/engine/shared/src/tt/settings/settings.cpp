#include <tt/platform/tt_error.h>
#include <tt/settings/settings.h>
#include <tt/system/Language.h>


namespace tt {
namespace settings {

static Region       s_region = Region_Invalid;
static std::wstring s_applicationName;


Region getRegion()
{
	TT_ASSERTMSG(s_region != Region_Invalid, "getRegion called before setRegion!");
	return s_region;
}


std::string getRegionName()
{
	switch (tt::settings::getRegion())
	{
	case tt::settings::Region_EU: return "eu";
	case tt::settings::Region_US: return "us";
	case tt::settings::Region_JP: return "jp";
	case tt::settings::Region_WW: break; // No specific region
	default: break;
	}
	
	return std::string();
}


void setRegion(Region p_region)
{
	s_region = p_region;
}


std::wstring getApplicationName()
{
	TT_ASSERTMSG(s_applicationName.empty() == false,
	             "getApplicationName called before setApplicationName!");
	return s_applicationName;
}


void setApplicationName(const std::wstring& p_name)
{
	s_applicationName = p_name;
}


std::string getRegionLanguage()
{
	std::string lang = system::Language::getLanguage();
	Region region = getRegion();
	if (region == Region_US)
	{
		if (lang == "en") return "us";
		if (lang == "es") return "ue";
		if (lang == "fr") return "uf";
		TT_WARN("Unknown language '%s' for region US, defaulting to 'us'.", lang.c_str());
		return "us"; // default to US English
	}
	else if (region == Region_EU)
	{
		if (lang == "en" ||
			lang == "fr" ||
			lang == "it" ||
			lang == "es" ||
			lang == "de" ||
			lang == "nl")
		{
			return lang;
		}
		TT_WARN("Unknown language '%s' for region EU, defaulting to 'en'.", lang.c_str());
		return "en"; // default to the queen's English
	}
	else if (region == Region_JP)
	{
		if (lang == "jp") return lang;
		TT_WARN("Unknown language '%s' for region JP, defaulting to 'jp'.", lang.c_str());
		return "jp"; // default to Japanese
	}
	else
	{
		TT_WARN("Unknown region %d, defaulting to 'en'.");
		return "en";
	}
}

// namespace end
}
}
