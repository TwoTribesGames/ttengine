#include <tt/code/helpers.h>
#include <tt/loc/LocStr.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
#include <tt/steam/helpers.h>
#include <tt/system/Language.h>

#include <toki/loc/Loc.h>

namespace toki {
namespace loc {

//--------------------------------------------------------------------------------------------------
// Public member functions

Loc::Loc()
{
	// Detect the currently active language and load the localization strings for it
	//*
#if defined(TT_STEAM_BUILD)
	m_lang = tt::steam::getGameLanguageID();
#endif
	
	if (m_lang.empty())
	{
		// Choose based on regional settings
		m_lang = tt::system::Language::getLanguage();
	}
	// */ m_lang = "en";  // FIXME: Language hard-coded to English until translations are available
	
	TT_Printf("Selected language '%s' for localization strings.\n", m_lang.c_str());
}


Loc::~Loc()
{
	destroyAll();
}


void Loc::createLocStr(SheetID p_sheet)
{
	if (m_sheets.find(p_sheet) != m_sheets.end())
	{
		TT_PANIC("Sheet '%s' (%d) was already created.",
		         getSheetIDName(p_sheet), p_sheet);
		return;
	}
	
	// HACK: The editor should always be in English, so don't use the system language
	const std::string desiredLang((p_sheet == SheetID_Editor) ? "en" : m_lang);
	
	const std::string filename("localization/" + std::string(getSheetIDName(p_sheet)) + ".loc");
	tt::loc::LocStr* locstr = new tt::loc::LocStr(filename, desiredLang);
	
	if (locstr == 0)
	{
		TT_PANIC("Failed to create new locstr (sheet '%s', file '%s').",
		         getSheetIDName(p_sheet), filename.c_str());
		return;
	}
	
	if (locstr->supportsLanguage(desiredLang) == false)
	{
		// Default to english when desired language is not available
		locstr->selectLanguage("en");
	}
	
	m_sheets.insert(Sheets::value_type(p_sheet, locstr));
}


void Loc::destroyLocStr(SheetID p_sheet)
{
	Sheets::iterator it = m_sheets.find(p_sheet);
	if (it == m_sheets.end())
	{
		TT_PANIC("Sheet '%s' (%d) was never created or already destroyed.",
		         getSheetIDName(p_sheet), p_sheet);
		return;
	}
	delete (*it).second;
	m_sheets.erase(p_sheet);
}


void Loc::destroyAll()
{
	tt::code::helpers::freePairSecondContainer(m_sheets);
}


tt::loc::LocStr& Loc::getLocStr(SheetID p_sheet)
{
	Sheets::iterator it = m_sheets.find(p_sheet);
	if (it == m_sheets.end())
	{
		TT_PANIC("Sheet '%s' (%d) was never created or already destroyed.",
		         getSheetIDName(p_sheet), p_sheet);
		// evil, but it should crash anyway
		tt::loc::LocStr* locStr = 0;
		return *locStr;
	}
	return *(*it).second;
}


bool Loc::hasLocStr(SheetID p_sheet)
{
	return m_sheets.find(p_sheet) != m_sheets.end();
}


void Loc::setLanguage(const std::string& p_lang)
{
	if (m_lang == p_lang)
	{
		return;
	}
	
	for (Sheets::iterator it = m_sheets.begin(); it != m_sheets.end(); ++it)
	{
		tt::loc::LocStr* locStr = (*it).second;
		if (locStr->supportsLanguage(p_lang) == false)
		{
			TT_WARN("Sheet '%s' (%d) does not support language '%s'. Defaulting to English.",
			         getSheetIDName((*it).first), (*it).first, p_lang.c_str());
			locStr->selectLanguage("en");
		}
		else
		{
			locStr->selectLanguage(p_lang);
		}
	}
	m_lang = p_lang;
}

// Namespace end
}
}
