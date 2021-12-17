#if !defined(INC_TT_MENU_ELEMENTS_FILELIST_H)
#define INC_TT_MENU_ELEMENTS_FILELIST_H


#include <tt/menu/elements/ScrollableList.h>


namespace tt {
namespace save {
	class SaveFile;
}

namespace menu {
namespace elements {

class FileItem;


/*! \brief Custom menu element which provides a list of files. */
class FileList : public ScrollableList
{
public:
	FileList(const std::string&  p_name,
	         const MenuLayout&   p_layout,
	         bool                p_hasEmpty,
	         const std::string&  p_nameVariable,
	         const std::string&  p_typeVariable,
	         const std::string&  p_confirmDialog,
	         const std::string&  p_confirmVariable,
	         const std::wstring& p_filter,
	         const std::string&  p_corruptionDialog,
	         const std::string&  p_parent);
	virtual ~FileList();
	
	virtual void loadResources();
	virtual void unloadResources();
	
	virtual void addChild(value_type* p_child);
	virtual bool doAction(const MenuElementAction& p_action);
	
	virtual void onMenuActivated();
	
	virtual void doLayout(const math::PointRect& p_rect);
	
	virtual FileList* clone() const;
	
protected:
	FileList(const FileList& p_rhs);
	
	virtual bool meetsCriteria(save::SaveFile* p_file) const;
	
private:
	std::string getFileItemName(s32 p_itemIndex) const;
	FileItem*   createFileItem(s32 p_itemIndex, save::SaveFile* p_file);
	void        createContent();
	
	// No assignment
	const FileList& operator=(const FileList&);
	
	
	math::PointRect m_rect;
	std::string     m_nameVariable;
	std::string     m_typeVariable;
	std::string     m_confirmDialog;
	std::string     m_confirmVariable;
	std::wstring    m_filter;
	std::string     m_corruptionDialog;
	std::string     m_parent;
	bool            m_hasEmpty;
	bool            m_resourcesLoaded;
};

// Namespace end
}
}
}


#endif  // !defined(INC_TT_MENU_ELEMENTS_FILELIST_H)
