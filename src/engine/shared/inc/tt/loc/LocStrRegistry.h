#ifndef INC_TT_LOC_LOCSTR_REGISTRY_H
#define INC_TT_LOC_LOCSTR_REGISTRY_H


#include <vector>
#include <tt/code/Uncopyable.h>


namespace tt {
namespace loc {

class LocStr;

/** 
 * Keeps a list of all current live LocStr objects
 */
class LocStrRegistry  : private code::Uncopyable
{
public:
	static void registerLocStr(LocStr* p_locstr);
	static void deRegisterLocStr(LocStr* p_locstr);
	
	static void setLanguageOnAll(const std::string& p_lang);
	
private:
	typedef std::vector<LocStr*> LocStrVec;
	
	LocStrRegistry();
	~LocStrRegistry();
	
	static LocStrVec* m_locstr_objects;
};

// Namespace end
}
}


#endif  // !defined(INC_TT_LOC_LOCSTR_REGISTRY_H)
