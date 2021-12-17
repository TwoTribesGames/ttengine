#if !defined(INC_TT_MENU_ELEMENTS_FILEBLOCKSBAR_H)
#define INC_TT_MENU_ELEMENTS_FILEBLOCKSBAR_H


#include <tt/menu/elements/ProgressBar.h>


namespace tt {
namespace menu {
namespace elements {

/*! \brief Element that displays a bar of free or in-use blocks. */
class FileBlocksBar : public ProgressBar
{
public:
	enum BlocksType
	{
		Blocks_Free,
		Blocks_Used
	};
	
	
	FileBlocksBar(const std::string& p_name,
	              const MenuLayout&  p_layout,
	              BlocksType         p_displayType);
	virtual ~FileBlocksBar();
	
	virtual bool doAction(const MenuElementAction& p_action);
	
	virtual FileBlocksBar* clone() const;
	
protected:
	FileBlocksBar(const FileBlocksBar& p_rhs);
	
private:
	void updateValues();
	
	// No assignment
	const FileBlocksBar& operator=(const FileBlocksBar&);
	
	
	//! Which type of overlay to display: used or free blocks
	BlocksType m_displayType;
};

// Namespace end
}
}
}


#endif  // !defined(INC_TT_MENU_ELEMENTS_FILEBLOCKSBAR_H)
