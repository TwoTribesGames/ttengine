#include <sstream>
#include <string>
#include <wchar.h>

//#include <tt/save/SaveFile.h>

#include <tt/menu/elements/FileItem.h>
#include <tt/menu/elements/Button.h>
#include <tt/menu/elements/Label.h>
#include <tt/menu/elements/FileList.h>
#include <tt/menu/MenuKeyboard.h>
#include <tt/menu/MenuSystem.h>
#include <tt/menu/MenuUtils.h>


namespace tt {
namespace menu {
namespace elements {

using save::SaveFile;


//------------------------------------------------------------------------------
// Public member functions

FileItem::FileItem(const std::string&  p_name,
                   const MenuLayout&   p_layout,
                   FileList*           p_fileList,
                   SaveFile*           p_file)
:
Line(p_name, p_layout),
m_fileList(p_fileList),
m_file(p_file)
{
	TT_ASSERTMSG(m_fileList != 0, "Valid file list pointer required.");
	
	// This element is selectable
	setCanHaveFocus(true);
	
	// Create the elements for the item information
	MenuLayout layout;
	
	if (p_file != 0)
	{
		// Item name label
		layout.setWidthType(MenuLayout::Size_Absolute);
		layout.setWidth(170); // FIXME: Remove hard-coded dimensions!
		layout.setHeightType(MenuLayout::Size_Auto);
		layout.setHorizontalPositionType(MenuLayout::Position_Left);
		layout.setVerticalPositionType(MenuLayout::Position_Center);
		Button* btn = new Button("", layout, /*m_file->getName()*/L"FIXME");
		MenuAction ma("menu_element_action");
		ma.addParameter(m_fileList->getName());
		ma.addParameter("select_file");
		ma.addParameter(MenuUtils::wideStringToHex(/*m_file->getName()*/L"FIXME"));
		std::wstring filetype;
		//filetype += m_file->getFileType();
		ma.addParameter(MenuUtils::wideStringToHex(filetype));
		btn->addAction(ma);
		
		Line::addChild(btn);
		
		layout.setWidthType(MenuLayout::Size_Absolute);
		layout.setWidth(30);
		layout.setHeightType(MenuLayout::Size_Auto);
		layout.setHorizontalPositionType(MenuLayout::Position_Right);
		layout.setVerticalPositionType(MenuLayout::Position_Center);
		std::wostringstream woss;
		//woss << m_file->getDisplayBlockSize();
		Label* lbl = new Label("", layout, woss.str());
		lbl->setVerticalAlign(engine::glyph::GlyphSet::ALIGN_BOTTOM);
		Line::addChild(lbl);
	}
	else
	{
		// Item name label
		layout.setWidthType(MenuLayout::Size_Absolute);
		layout.setWidth(170);
		layout.setHeightType(MenuLayout::Size_Auto);
		layout.setHorizontalPositionType(MenuLayout::Position_Left);
		layout.setVerticalPositionType(MenuLayout::Position_Center);
		// FIXME: Do not assume availability of localization identifiers.
		Button* btn = new Button("", layout, "NEW_FILE");
		MenuAction ma("menu_element_action");
		ma.addParameter(m_fileList->getName());
		ma.addParameter("select_file");
		ma.addParameter("");
		ma.addParameter("");
		btn->addAction(ma);
		
		Line::addChild(btn);
		
		layout.setWidthType(MenuLayout::Size_Absolute);
		layout.setWidth(30);
		layout.setHeightType(MenuLayout::Size_Auto);
		layout.setHorizontalPositionType(MenuLayout::Position_Right);
		layout.setVerticalPositionType(MenuLayout::Position_Center);
		Label* lbl = new Label("", layout, L"");
		lbl->setVerticalAlign(engine::glyph::GlyphSet::ALIGN_BOTTOM);
		Line::addChild(lbl);
	}
}


FileItem::~FileItem()
{
}


void FileItem::addChild(FileItem::value_type* /* p_child */)
{
	TT_PANIC("FileItem cannot have any children "
	         "(they are managed automatically by the container).");
}


FileItem* FileItem::clone() const
{
	return new FileItem(*this);
}


//------------------------------------------------------------------------------
// Protected member functions

FileItem::FileItem(const FileItem& p_rhs)
:
Line(p_rhs),
m_fileList(p_rhs.m_fileList),
m_file(p_rhs.m_file)
{
}

// Namespace end
}
}
}
