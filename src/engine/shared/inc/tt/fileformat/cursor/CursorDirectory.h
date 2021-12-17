#if !defined(INC_TT_FILEFORMAT_CURSOR_CURSORDIRECTORY_H)
#define INC_TT_FILEFORMAT_CURSOR_CURSORDIRECTORY_H


#include <string>
#include <vector>

#include <tt/fileformat/cursor/fwd.h>


namespace tt {
namespace fileformat {
namespace cursor {

/*! \brief A set of cursors, loaded from a .cur file. */
class CursorDirectory
{
public:
	static CursorDirectoryPtr load(const std::string& p_filename);
	~CursorDirectory();
	
	inline s32 getCursorCount() const { return static_cast<s32>(m_cursors.size()); }
	const CursorData* getCursor(s32 p_index) const;
	
private:
	typedef std::vector<CursorData*> Cursors;
	
	
	CursorDirectory();
	
	// Disable copy and assignment
	CursorDirectory(const CursorDirectory&);
	CursorDirectory& operator=(const CursorDirectory&);
	
	
	Cursors m_cursors;
};

// Namespace end
}
}
}


#endif  // !defined(INC_TT_FILEFORMAT_CURSOR_CURSORDIRECTORY_H)
