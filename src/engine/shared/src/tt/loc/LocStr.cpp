#include <stdio.h>
#include <limits>
#include <algorithm>

#include <tt/fs/File.h>
#include <tt/loc/LocStr.h>
#include <tt/loc/LocStrRegistry.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
#include <tt/system/Time.h>
#include <tt/system/Language.h>
#include <tt/str/common.h>


namespace tt {
namespace loc {

/**
 * Constructor
 */
LocStr::LocStr(const std::string& p_filename,
               const std::string& p_language,
               bool               p_supportIndex)
:
m_langs(),
m_selectedLangIdx(0),
m_selectedLangCode(0),
m_isoid(),
m_filename(p_filename),
m_strCache(),
m_orderedHashes(),
m_supportsIndexing(p_supportIndex),
m_strict(false)
{
	// open the loc file
	fs::FilePtr file = fs::open(getFileName(), fs::OpenMode_Read);
	if (file == 0)
	{
		TT_PANIC("Unable to open '%s'", getFileName().c_str());
	}
	
	// Read num languages
	s16 numlangs = 0;
	fs::readInteger(file, &numlangs);
	m_langs.reserve(numlangs);
	
	// Read language identifiers
	for (int c = 0; c < numlangs; ++c)
	{
		u16 langid = 0;
		fs::readInteger(file, &langid);
		m_langs.push_back(static_cast<LangCodes::value_type>(langid));
	}
	
	// Register LocStr in tracker class
	LocStrRegistry::registerLocStr(this);
	
	// Select system language
	if (supportsLanguage(p_language))
	{
		selectLanguage(p_language);
	}
	else
	{
		TT_PANIC("Language '%s' not supported for file '%s', defaulting to 'en'.", p_language.c_str(), p_filename.c_str());
		selectLanguage("en");
	}
}


LocStr::~LocStr()
{
	LocStrRegistry::deRegisterLocStr(this);
}


/**
 * Make sure that the filepointer points to the /next/ string
 *
 * @param p_inf  file in
 */
void LocStr::whiskOverStrings(const tt::fs::FilePtr& p_inf)
{
	for (u32 c = 0; c < getNumLangs(); ++c)
	{
		// Read strlen
		u16 length = 0;
		fs::readInteger(p_inf, &length);
		
		bool result = p_inf->seek(static_cast<fs::pos_type>(length * 2), fs::SeekPos_Cur);
		TT_ASSERTMSG(result, "Seek failed. (LocStr::whiskOverstrings");
	}
}


/**
 *
 * @param p_inf
 *
 * @return only the current selected languages localized string entry
 */
std::wstring LocStr::readLocalizedString(const tt::fs::FilePtr& p_inf)
{
	std::wstring ret;
	
	for (u32 c = 0; c < getNumLangs(); ++c)
	{
		// Read strlen
		u16 length = 0;
		fs::readInteger(p_inf, &length);
		
		if (length == 1)
		{
			// Skip
			p_inf->seek(2, fs::SeekPos_Cur);
			
			// This the current lang string?
			if (m_selectedLangIdx == c)
			{
				// Empty
				return std::wstring();
			}
		}
		else
		{
			if (m_selectedLangIdx == c)
			{
				wchar_t* astr = new wchar_t[length];
				for (u32 i = 0; i < length; ++i)
				{
					u16 wideChar = 0;
					fs::readInteger(p_inf, &wideChar);
					astr[i] = static_cast<wchar_t>(wideChar);
				}
				ret = std::wstring(astr);
				delete[] astr;
				
				return ret;
			}
			else
			{
				p_inf->seek(static_cast<fs::pos_type>(length * 2), fs::SeekPos_Cur);
			}
		}
	}
	
	// Should never happen, this....
	return getErrorString();
}


/**
 * Ask whether a specific language is supported
 *
 * @param p_isoid twoletter country code
 *
 * @return true if country is supported
 */
bool LocStr::supportsLanguage(const std::string& p_isoid) const
{
	// Convert string to 16bit int
	u16 code = getLangCodeFromString(p_isoid);
	
	// Find in vector
	return find(m_langs.begin(),
	            m_langs.end(),
	            static_cast<LangCodes::value_type>(code)) != m_langs.end();
}


/**
 * Set which language to use
 *
 * @param p_isoid two letter country code
 */
void LocStr::selectLanguage(const std::string& p_isoid)
{
	m_isoid = p_isoid;
	
	// Get index code of param
	u16 code = getLangCodeFromString(p_isoid);
	if (m_selectedLangCode == code)
	{
		// language already selected no need to reload it
		return;
	}
	
	LangCodes::iterator it = std::find(m_langs.begin(),
	                                   m_langs.end(),
	                                   static_cast<LangCodes::value_type>(code));
	
	if (it == m_langs.end())
	{
		TT_PANIC("Attempt to set unsupported language '%s' on LocStr "
		        "object loaded from %s.",
		        p_isoid.c_str(), getFileName().c_str());
		return;
	}
	
	// Flush the cache
	m_strCache.clear();
	m_selectedLangIdx  = static_cast<u16>(std::distance(m_langs.begin(), it));
	m_selectedLangCode = code;
	
	// open the loc file
	fs::FilePtr file = fs::open(getFileName(), fs::OpenMode_Read);
	if (file == 0)
	{
		TT_PANIC("Unable to open '%s'", getFileName().c_str());
	}
	
	// Read num languages
	u16 numlangs;
	fs::readInteger(file, &numlangs);
	
	// Skip language identifiers
	for (int c = 0; c < numlangs; ++c)
	{
		u16 dummy;
		fs::readInteger(file, &dummy);
	}
	
	// Now at start of string table, cache strings
	u16 numstrings = 0;
	fs::readInteger(file, &numstrings);
	
	for (int c=0; c < numstrings; ++c)
	{
		WStringVec line;
		
		// Get hash
		u32 hash = 0;
		fs::readInteger(file, &hash);
		
		typedef std::pair<LocStrCache::iterator, bool> InsertResult;
		
		// Now read the localized string and store it in the cache
		tt::fs::pos_type filePos = file->getPosition(); // Remember pos
		std::wstring str(readLocalizedString(file));
		InsertResult result = m_strCache.insert(std::make_pair(hash, str));
		file->seek(filePos, tt::fs::SeekPos_Set); // Restore pos.
		
		if (result.second == false)
		{
			// Insert didn't work!
			TT_PANIC("Couldn't add new string to table. (HASH collision? new: %u, old: %u.)\n"
			         "Old Text: '%s', new Text: '%s'",
			         hash, result.first->first, 
			         tt::str::narrow(result.first->second).c_str(), tt::str::narrow(str).c_str() );
		}
		
		// Store a linear hash if we need to support "by index" access
		if (isGetByIndexSupported())
		{
			m_orderedHashes.push_back(hash);
		}
		
		// read strings, but not store
		whiskOverStrings(file);
	}
	TT_ASSERTMSG(m_strCache.size() == numstrings,
	             "Found a different number of strings then was expected in file: '%s'\n"
	             "Found: %u, expected: %u", getFileName().c_str(), m_strCache.size(), numstrings);
}


/**
 * Tool method for getting a numeric identifer from two-letter countrycode
 *
 * @param p_str
 *
 * @return
 */
u16 LocStr::getLangCodeFromString(const std::string& p_str)
{
	TT_ASSERTMSG(p_str.length() == 2,
	             "The string '%s' is not a properly formatted 2-char language code.",
	             p_str.c_str());
	
	return static_cast<u16>((p_str[1] << 8) | p_str[0]);
}


bool LocStr::hasString(const std::string& p_identifier) const
{
	return m_strCache.find(getHash(p_identifier)) != m_strCache.end();
}


/**
 *
 * @param p_identifier
 *
 * @return string value by identifier
 */
std::wstring LocStr::getString(const std::string& p_identifier)
{
	bool foundstr = false;
	std::wstring ret = getString(LocStr::getHash(p_identifier), foundstr);
	TT_ASSERTMSG(m_strict == false || foundstr,
	             "The locsheet '%s' does not contain the key '%s'.",
	             m_filename.c_str(), p_identifier.c_str());
	if (foundstr)
	{
		if (ret.empty())
		{
			// Found an empty string for this language; string not localized?
			// Return the current language ISO ID along with the non-localized identifier
			// (e.g. "NL<LOC_ID>")
			std::string langIsoId;
			langIsoId += static_cast<std::string::value_type>((m_selectedLangCode)      & 0xFF);
			langIsoId += static_cast<std::string::value_type>((m_selectedLangCode >> 8) & 0xFF);
			ret = tt::str::widen(tt::str::toUpper(langIsoId)) + L"<" + tt::str::widen(p_identifier) + L">";
		}
		return ret;
	}
	
#if !defined(TT_BUILD_FINAL)
	TT_WARN("getString for id: '%s' (hash: 0x%x) failed to find string in language '%s'.",
	        p_identifier.c_str(), LocStr::getHash(p_identifier), m_isoid.c_str());
	
	return tt::str::widen("#" + str::toUpper(m_isoid) + "_<" + p_identifier + ">#");
#else
	return std::wstring();
#endif
}


/**
 *
 * @param p_hash
 *
 * @return a string by its hash value
 */
std::wstring LocStr::getString(u32 p_hash, bool& p_found_OUT)
{
	// Find hash in the cache
	LocStrCache::iterator it = m_strCache.find(p_hash);
	
	// Cache hit? - return known string
	if (it != m_strCache.end())
	{
		p_found_OUT = true;
		return it->second;
	}
	
	p_found_OUT = false;
	return getErrorString();
}


/**
 * Generate oneway hash for a string
 *
 * @param p_hash
 *
 * @return integer hash for input string
 */
u32 LocStr::getHash(const std::string& p_strkey)
{
	u32 b = 378551;
	u32 a = 63689;
	u32 h = 0;
	
	for (u32 c = 0; c < p_strkey.length(); ++c)
	{
		h = (h * a) + p_strkey[c];
		a = a * b;
	}
	
	return (h & 0x7FFFFFFF);
}


/**
 *
 * @param p_pos
 *
 * @return a string by its index, if indexing is supported
 */
std::wstring LocStr::getStringByIndex(u32 p_pos)
{
	TT_WARNING(isGetByIndexSupported(),
	           "Attempt to read string by index, but locstr was not "
	           "constructed with indexing flag set to true!");
	
	TT_ASSERTMSG(p_pos < getNumStrings(),
	             "String index %d is out of range! File %s has only %d strings",
	             p_pos,
	             getFileName().c_str(),
	             getNumStrings());
	
	bool found = false;
	
	return isGetByIndexSupported() ?
	       getString(m_orderedHashes.at(p_pos), found) :
	       getErrorString();
}

// Namespace end
}
}
