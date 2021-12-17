#include <wchar.h>
#include <sstream>

#include <tt/menu/elements/FileList.h>
#include <tt/menu/MenuSystem.h>
#include <tt/menu/MenuAction.h>
#include <tt/menu/MenuUtils.h>
#include <tt/menu/MenuElementAction.h>
#include <tt/menu/elements/Label.h>
#include <tt/menu/elements/FileItem.h>
//#include <tt/save/SaveFS.h>
//#include <tt/save/SaveFile.h>
#include <tt/str/toStr.h>


namespace tt {
namespace menu {
namespace elements {

using save::SaveFile;


//------------------------------------------------------------------------------
// Public member functions

FileList::FileList(const std::string&  p_name,
                   const MenuLayout&   p_layout,
                   bool                p_hasEmpty,
                   const std::string&  p_nameVariable,
                   const std::string&  p_typeVariable,
                   const std::string&  p_confirmDialog,
                   const std::string&  p_confirmVariable,
                   const std::wstring& p_filter,
                   const std::string&  p_corruptionDialog,
                   const std::string&  p_parent)
:
ScrollableList(p_name, p_layout),
m_rect(math::Point2(0, 0), 1, 1),
m_nameVariable(p_nameVariable),
m_typeVariable(p_typeVariable),
m_confirmDialog(p_confirmDialog),
m_confirmVariable(p_confirmVariable),
m_filter(p_filter),
m_corruptionDialog(p_corruptionDialog),
m_parent(p_parent),
m_hasEmpty(p_hasEmpty),
m_resourcesLoaded(false)
{
	TT_ASSERTMSG((p_confirmVariable.empty() ^ p_confirmDialog.empty()) == false,
	             "Confirm dialog and variable should both be empty or not empty.");
	createContent();
}


FileList::~FileList()
{
}


void FileList::loadResources()
{
	ScrollableList::loadResources();
	m_resourcesLoaded = true;
}


void FileList::unloadResources()
{
	ScrollableList::unloadResources();
	m_resourcesLoaded = false;
}


void FileList::addChild(value_type* /* p_child */)
{
	TT_PANIC("Cannot directly add children to FileList; "
	         "they are managed by the container itself.");
}


void FileList::doLayout(const math::PointRect& p_rect)
{
	createContent();
	m_rect = p_rect;
	ScrollableList::doLayout(p_rect);
}


bool FileList::doAction(const MenuElementAction& p_action)
{
	// Let base handle the action first
	if (ScrollableList::doAction(p_action))
	{
		return true;
	}
	
	
	std::string command(p_action.getCommand());
	
	if (command == "select_file")
	{
		TT_ASSERTMSG(p_action.getParameterCount() == 2,
		             "Command '%s' takes two parameters: "
		             "the name of the file to select and its type.",
		             command.c_str());
		
		std::string filename(p_action.getParameter(0));
		std::string filetype(p_action.getParameter(1));
		
		if (filename.empty())
		{
			// Selected 'new file'; ask for a filename
			MenuAction ma("open_softwarekeyboard");
			ma.addParameter("ENTER_FILENAME_TITLE");
			ma.addParameter(m_nameVariable);
			ma.addParameter("true");
			ma.addParameter(str::toStr(1/*SaveFile::getNameLength()*/));
			
			// Build a list of actions
			MenuActions acts;
			acts.push_back(ma);
			
			MenuAction ma2("menu_element_action");
			ma2.addParameter(getName());
			ma2.addParameter("return_from_keyboard");
			acts.push_back(ma2);
			
			MenuSystem::getInstance()->doActions(acts);
			
			return true;
		}
		else
		{
			MenuSystem::getInstance()->setSystemVar(m_nameVariable, filename);
			MenuSystem::getInstance()->setSystemVar(m_typeVariable, filetype);
			
			if (m_confirmDialog.empty() == false)
			{
				// Confirm file selection
				MenuAction ma("open_menu_and_wait");
				ma.addParameter(m_confirmDialog);
				
				MenuActions acts;
				acts.push_back(ma);
				
				MenuAction ma2("menu_element_action");
				ma2.addParameter(getName());
				ma2.addParameter("confirm");
				
				acts.push_back(ma2);
				
				MenuSystem::getInstance()->doActions(acts);
			}
			else if (m_corruptionDialog.empty() == false)
			{
				// Do corruption check
				/*
				std::wstring name = MenuUtils::hexToWideString(filename);
				std::wstring type = MenuUtils::hexToWideString(filetype);
				
				SaveFile* file = save::SaveFS::openFile(name, type.at(0), false);
				TT_ASSERTMSG(file != 0, "unable to open selected file '%s'\n",
				             str::narrow(name).c_str());
				
				void* buffer = ::operator new(static_cast<size_t>(file->getSize()));
				TT_ASSERTMSG(buffer != 0, "unable to create buffer to read file '%s'\n",
				             str::narrow(name).c_str());
				
				s32 bytes = file->read(buffer, 0, file->getSize());
				
				::operator delete(buffer);
				
				if (bytes != file->getSize())
				{
					// File user selected has been corrupted
					// tell the user what happened and that the file has been deleted
					MenuActions acts;
					
					MenuAction ma("open_menu_and_wait");
					ma.addParameter(m_corruptionDialog);
					
					acts.push_back(ma);
					
					// Then return to the updated filelist
					MenuAction ma2("menu_element_action");
					ma2.addParameter(getName());
					ma2.addParameter("update");
					
					acts.push_back(ma2);
					
					MenuSystem::getInstance()->doActions(acts);
					
					save::SaveFS::deleteFile(file);
					MenuSystem::getInstance()->removeSystemVar(m_nameVariable);
					MenuSystem::getInstance()->removeSystemVar(m_typeVariable);
				}
				else
				{
					performActions();
				}
				//*/
			}
			else
			{
				performActions();
			}
			return true;
		}
	}
	else if (command == "confirm")
	{
		TT_ASSERTMSG(p_action.getParameterCount() == 0,
		             "Command '%s' takes no parameters",
		             command.c_str());
		
		TT_ASSERTMSG(m_confirmVariable.empty() == false,
		             "Cannot handle 'confirm' action without confirm variable.");
		if (m_confirmVariable.empty())
		{
			return false;
		}
		
		if (MenuSystem::getInstance()->hasSystemVar(m_confirmVariable))
		{
			MenuSystem::getInstance()->removeSystemVar(m_confirmVariable);
			performActions();
		}
		else
		{
			if (MenuSystem::getInstance()->hasSystemVar(m_nameVariable))
			{
				MenuSystem::getInstance()->removeSystemVar(m_nameVariable);
			}
			if (MenuSystem::getInstance()->hasSystemVar(m_typeVariable))
			{
				MenuSystem::getInstance()->removeSystemVar(m_typeVariable);
			}
		}
		return true;
	}
	else if (command == "update")
	{
		TT_ASSERTMSG(p_action.getParameterCount() == 0,
		             "Command '%s' takes no parameters", command.c_str());
		
		doLayout(m_rect);
		if (m_resourcesLoaded)
		{
			loadResources();
		}
		
		if (getChildCount() == 0)
		{
			if (m_parent.empty() == false)
			{
				MenuActions acts;
				
				MenuAction ma("menu_element_action");
				ma.addParameter(m_parent);
				ma.addParameter("show_value");
				ma.addParameter("false");
				
				acts.push_back(ma);
				
				MenuSystem::getInstance()->doActions(acts);
			}
		}
		return true;
	}
	else if (command == "return_from_keyboard")
	{
		TT_ASSERTMSG(p_action.getParameterCount() == 0,
		             "Command '%s' takes no parameters", command.c_str());
		
		if (MenuSystem::getInstance()->hasSystemVar(m_nameVariable))
		{
			if (MenuSystem::getInstance()->getSystemVar(m_nameVariable).empty() == false)
			{
				/*
				bool exists = false;
				std::wstring name = MenuUtils::hexToWideString(
					MenuSystem::getInstance()->getSystemVar(m_nameVariable));
				save::SaveFile* file = save::SaveFS::openFile(name, 2, false);
				if (m_confirmDialog.empty() == false && file != 0)
				{
					MenuAction ma("open_menu_and_wait");
					ma.addParameter(m_confirmDialog);
					
					MenuActions acts;
					acts.push_back(ma);
					
					MenuAction ma2("menu_element_action");
					ma2.addParameter(getName());
					ma2.addParameter("confirm");
					
					acts.push_back(ma2);
					
					MenuSystem::getInstance()->doActions(acts);
				}
				else
				{
					performActions();
				}
				//*/
			}
		}
		return true;
	}
	
	return false;
}


void FileList::onMenuActivated()
{
	// Use default scrollable list behavior
	ScrollableList::onMenuActivated();
}


FileList* FileList::clone() const
{
	return new FileList(*this);
}


//------------------------------------------------------------------------------
// Protected member functions

FileList::FileList(const FileList& p_rhs)
:
ScrollableList(p_rhs),
m_rect(p_rhs.m_rect),
m_nameVariable(p_rhs.m_nameVariable),
m_typeVariable(p_rhs.m_typeVariable),
m_confirmDialog(p_rhs.m_confirmDialog),
m_confirmVariable(p_rhs.m_confirmVariable),
m_filter(p_rhs.m_filter),
m_hasEmpty(p_rhs.m_hasEmpty),
m_resourcesLoaded(p_rhs.m_resourcesLoaded)
{
}


bool FileList::meetsCriteria(SaveFile* /* p_file */) const
{
	// By default, all files meet the criteria to be put in the list
	return true;
}


//------------------------------------------------------------------------------
// Private member functions

std::string FileList::getFileItemName(s32 p_itemIndex) const
{
	std::ostringstream oss;
	oss << getName() << "_file_" << p_itemIndex;
	return oss.str();
}


FileItem* FileList::createFileItem(s32 p_itemIndex, SaveFile* p_file)
{
	MenuLayout layout;
	layout.setWidthType(MenuLayout::Size_Max);
	layout.setHeightType(MenuLayout::Size_Auto);
	layout.setOrder(MenuLayout::Order_Horizontal);
	
	return new FileItem(getFileItemName(p_itemIndex),
	                    layout, this, p_file);
}


void FileList::createContent()
{
	ContainerBase<>::removeChildren();
	
	s32 offset = 0;
	
	if (m_hasEmpty)
	{
		FileItem* fi = createFileItem(offset, 0);
		TT_NULL_ASSERT(fi);
		
		++offset;
		
		ScrollableList::addChild(fi);
	}
	
	/*
	using save::SaveFS;
	s32 files = SaveFS::getNumberOfFiles(m_filter);
	
	for (s32 index = 0; index < files; ++index)
	{
		SaveFile* file = SaveFS::openFile(m_filter, index);
		
		if (meetsCriteria(file))
		{
			FileItem* fi = createFileItem(index + offset, file);
			TT_NULL_ASSERT(fi);
			
			// Add the item to the container
			ScrollableList::addChild(fi);
		}
	}
	//*/
}

// Namespace end
}
}
}
