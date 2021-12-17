#if !defined(INC_TOKI_LOC_LOC_H)
#define INC_TOKI_LOC_LOC_H


#include <map>

#include <toki/loc/SheetList.h>


namespace tt {
namespace loc {
	class LocStr;
}
}

namespace toki {
namespace loc {

/*! \brief Localization management class. */
class Loc
{
public:
	Loc();
	~Loc();
	
	void createLocStr(SheetID p_sheet);
	void destroyLocStr(SheetID p_sheet);
	
	void destroyAll();
	
	tt::loc::LocStr& getLocStr(SheetID p_sheet);
	bool             hasLocStr(SheetID p_sheet);
	
	void setLanguage(const std::string& p_lang);
	inline const std::string& getLanguage() const { return m_lang; }
	
private:
	typedef std::map<SheetID, tt::loc::LocStr*> Sheets;
	
	
	Loc(const Loc&);
	const Loc& operator=(const Loc&);
	
	
	Sheets      m_sheets;
	std::string m_lang;
};

// Namespace end
}
}


#endif  // !defined(INC_TOKI_LOC_LOC_H)
