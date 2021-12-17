#include <algorithm>

#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
//#include <tt/save/SaveFAT.h>
//#include <tt/save/SaveFS.h>

#include <tt/menu/elements/FileBlocksBar.h>
#include <tt/menu/MenuElementAction.h>
#include <tt/menu/MenuSystem.h>
#include <tt/menu/MenuSkin.h>
#include <tt/menu/elements/SkinElementIDs.h>


namespace tt {
namespace menu {
namespace elements {

using math::PointRect;
using math::Vector2;


//------------------------------------------------------------------------------
// Public member functions

FileBlocksBar::FileBlocksBar(const std::string&        p_name,
                             const MenuLayout&         p_layout,
                             FileBlocksBar::BlocksType p_displayType)
:
ProgressBar(p_name, p_layout),
m_displayType(p_displayType)
{
	updateValues();
}


FileBlocksBar::~FileBlocksBar()
{
}


bool FileBlocksBar::doAction(const MenuElementAction& p_action)
{
	// Allow base to handle action first
	if (MenuElement::doAction(p_action))
	{
		return true;
	}
	
	if (p_action.getCommand() == "update")
	{
		TT_ASSERTMSG(p_action.getParameterCount() == 0,
		             "FileBlocksBar '%s': "
		             "Command 'update' takes no parameters.",
		             getName().c_str());
		updateValues();
		return true;
	}
	
	return false;
}


FileBlocksBar* FileBlocksBar::clone() const
{
	return new FileBlocksBar(*this);
}


//------------------------------------------------------------------------------
// Protected member functions

FileBlocksBar::FileBlocksBar(const FileBlocksBar& p_rhs)
:
ProgressBar(p_rhs),
m_displayType(p_rhs.m_displayType)
{
}


//------------------------------------------------------------------------------
// Private member functions

void FileBlocksBar::updateValues()
{
	/*
	// Update the free, used and total block count
	using save::SaveFAT;
	using save::SaveFS;
	s32 blocks_free  = SaveFAT::getFreeBlockCount();
	s32 blocks_used  = SaveFS::getFileTypeUsage(2);
	s32 blocks_total = SaveFAT::getTotalBlockCount() - SaveFAT::getFatBlockCount();
	blocks_total -= SaveFS::getFileTableUsage();
	blocks_total -= SaveFS::getFileTypeUsage(1);
	
	TT_Printf("FileBlocksBar::updateOverlayBar: Free:  %d blocks\n", blocks_free);
	TT_Printf("FileBlocksBar::updateOverlayBar: Used:  %d blocks\n", blocks_used);
	TT_Printf("FileBlocksBar::updateOverlayBar: Total: %d blocks\n", blocks_total);
	
	setValue(m_displayType == Blocks_Free ? blocks_free : blocks_used);
	setMaxValue(blocks_total);
	//*/
}

// Namespace end
}
}
}
