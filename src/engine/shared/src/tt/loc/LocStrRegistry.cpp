#include <string>
#include <vector>
#include <algorithm>

#include <tt/loc/LocStrRegistry.h>
#include <tt/loc/LocStr.h>
#include <tt/platform/tt_error.h>


namespace tt {
namespace loc {

// Alloc space for registry
LocStrRegistry::LocStrVec* LocStrRegistry::m_locstr_objects = 0L;


/**
 * Register a LocStr
 * 
 * Only the LocStr constructor should call this.
 * 
 * @param p_locstr pointer to LocStr object
 */
void LocStrRegistry::registerLocStr(LocStr* p_locstr)
{
	if (m_locstr_objects == 0)
	{
		m_locstr_objects = new LocStrVec;
		m_locstr_objects->reserve(16);
	}
	
	m_locstr_objects->push_back(p_locstr);
}


/**
 * De-register a LocStr from registry
 * 
 * LocStr constructor should be only caller of this method
 * 
 * @param p_locstr
 */
void LocStrRegistry::deRegisterLocStr(LocStr* p_locstr)
{
	// Find param in vector
	LocStrVec::iterator itr = find(
			m_locstr_objects->begin(),
			m_locstr_objects->end(),
			p_locstr );
	
	// If found, remove from vector
	if ( itr != m_locstr_objects->end() )
	{
		m_locstr_objects->erase(itr);
		
		// All gone? erase and zero
		if (m_locstr_objects->empty())
		{
			delete m_locstr_objects;
			m_locstr_objects = 0;
		}
	}
	else
	{
		TT_WARNING(false, "Attempt to deRegister un-registered LocStr");
	}
}


/**
 * Apply language setting to all the currently live LocStr objects
 * 
 * @param p_lang 2-byte language code
 */
void LocStrRegistry::setLanguageOnAll( const std::string& p_lang )
{
	// Allowed to happen
	if ( m_locstr_objects == 0L ) return;
	
	
	for (LocStrVec::iterator it = m_locstr_objects->begin();
	     it != m_locstr_objects->end(); ++it)
	{
		// LocStr error handles itself
		(*it)->selectLanguage( p_lang );
	}
}

// Namespace end
}
}

