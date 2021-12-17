#if !defined(INC_TT_MENU_ELEMENTS_FILEITEM_H)
#define INC_TT_MENU_ELEMENTS_FILEITEM_H


#include <tt/menu/elements/Line.h>


namespace tt {
namespace save {
class SaveFile;
}

namespace menu {
namespace elements {

class FileList;


/*! \brief Custom menu element which displays file information. */
class FileItem : public Line
{
public:
	FileItem(const std::string& p_name,
	         const MenuLayout&  p_layout,
	         FileList*          p_fileList,
	         save::SaveFile*    p_file);
	virtual ~FileItem();
	
	virtual void addChild(value_type* p_child);
	
	virtual FileItem* clone() const;
	
protected:
	FileItem(const FileItem& p_rhs);
	
private:
	// No assignment
	const FileItem& operator=(const FileItem&);
	
	
	FileList*       m_fileList;
	save::SaveFile* m_file;
};

// Namespace end
}
}
}


#endif  // !defined(INC_TT_MENU_ELEMENTS_FILEITEM_H)
