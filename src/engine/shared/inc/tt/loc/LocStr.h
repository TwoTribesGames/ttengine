#ifndef INC_TT_LOC_LOCSTR_H
#define INC_TT_LOC_LOCSTR_H

#include <map>
#include <vector>
#include <string>

#include <tt/fs/types.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace loc {

/*! \brief Localized string pool. */
class LocStr
{
public:
	explicit LocStr(const std::string& p_filename,
	                const std::string& p_language        = "en",
	                bool               p_supportIndex    = false);
	
	~LocStr();
	
	bool supportsLanguage(const std::string& p_isoid) const;
	void selectLanguage(const std::string& p_isoid);
	
	/*! \brief Indicates whether this LocStr object contains the specified localization identifier. */
	bool hasString(const std::string& p_identifier) const;
	
	std::wstring getString(const std::string& p_identifier);
	std::wstring getString(u32 p_hash, bool& p_error_OUT);
	std::wstring getStringByIndex(u32 p_pos);
	
	static u32 getHash(const std::string& p_hash);
	
	/*! \return Number of strings. */
	inline u32 getNumStrings() const { return static_cast<u32>(m_strCache.size()); }
	
	/*! \return number of languages defined in file. */
	inline u32 getNumLangs() const { return static_cast<u32>(m_langs.size()); }
	
	/*! \return locstr filename. */
	inline const std::string& getFileName() const { return m_filename; }
	
	/*! \return true if the hashes were indexed. */
	inline bool isGetByIndexSupported() const { return m_supportsIndexing; }
	
	/*! \brief Tell the LocStr to assert if someone tries querying a nonexistant identifier.
	    \param p_onoff Set to true to enable strict behavior. */
	inline void setStrict(bool p_onoff) { m_strict = p_onoff; }
	
	/*! \return The string used in case a loc ID was not found. */
	inline std::wstring getErrorString() const { return L"ERR"; }
	
	static u16 getLangCodeFromString(const std::string& p_str);
	
private:
	typedef std::vector<std::wstring>   WStringVec;
	typedef std::map<u32, std::wstring> LocStrCache;
	typedef std::vector<int>            LangCodes;
	
	// No copying
	LocStr(const LocStr&);
	LocStr& operator=(const LocStr&);
	
	void whiskOverStrings(const fs::FilePtr& p_inf);
	std::wstring readLocalizedString(const fs::FilePtr& p_inf);
	
	
	LangCodes m_langs;
	
	u16            m_selectedLangIdx;
	u16            m_selectedLangCode;
	std::string    m_isoid;
	std::string    m_filename;
	
	LocStrCache m_strCache;
	
	std::vector<u32> m_orderedHashes;
	bool             m_supportsIndexing;
	
	bool m_strict;
};

// Namespace end
}
}


#endif  // !defined(INC_TT_LOC_LOCSTR_H)
