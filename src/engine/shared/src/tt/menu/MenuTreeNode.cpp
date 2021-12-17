#include <tt/platform/tt_error.h>
//#include <tt/memory/HeapMgr.h>

#include <tt/menu/MenuTreeNode.h>
#include <tt/menu/MenuFactory.h>
#include <tt/menu/MenuSystem.h>
#include <tt/menu/elements/Menu.h>


namespace tt {
namespace menu {

/*
using memory::safestring;
using memory::toSafeString;
using memory::fromSafeString;
//*/


//------------------------------------------------------------------------------
// Public member functions

MenuTreeNode::MenuTreeNode(const std::string& p_name)
:
//m_name(toSafeString(p_name)),
m_name(p_name),
m_parent(0),
m_menu(0),
m_clearSelectionPathAfterDestroy(false)
{
	// Name cannot be empty
	TT_ASSERTMSG(m_name.empty() == false,
	             "Menu tree node name cannot be empty.");
}


MenuTreeNode::~MenuTreeNode()
{
	// Free memory for all children
	for (ChildrenContainer::iterator it = m_children.begin();
	     it != m_children.end(); ++it)
	{
		delete *it;
	}
	
	// Destroy the menu associated with this node
	destroyMenu();
}


bool MenuTreeNode::isDirectChild(const std::string& p_name) const
{
	// Search all child nodes
	for (ChildrenContainer::const_iterator it = m_children.begin();
	     it != m_children.end(); ++it)
	{
		if ((*it)->getName() == p_name)
		{
			// Found a child with this name.
			return true;
		}
	}
	
	// No node with the specified name was found
	return false;
}


MenuTreeNode* MenuTreeNode::findNode(const std::string& p_name)
{
	// Check if this is the node we're searching for
	{
		//std::string ourName(fromSafeString(m_name));
		std::string ourName(m_name);
		if (ourName == p_name)
		{
			return this;
		}
	}
	
	// Search all child nodes
	MenuTreeNode* node;
	for (ChildrenContainer::iterator it = m_children.begin();
	     it != m_children.end(); ++it)
	{
		node = (*it)->findNode(p_name);
		if (node != 0)
		{
			return node;
		}
	}
	
	// No node with the specified name was found
	return 0;
}


MenuTreeNode* MenuTreeNode::createChild(const std::string& p_name)
{
	// Create a new child node and add it to the tree
	MenuTreeNode* child = new MenuTreeNode(p_name, this);
	TT_ASSERTMSG(child != 0, "Creating child node failed (out of memory?).");
	m_children.push_back(child);
	
	// Return the newly created node
	return child;
}


MenuTreeNode* MenuTreeNode::getChild(int i)
{
	TT_ASSERTMSG(i >= 0 && i < static_cast<int>(m_children.size()),
	             "Menu tree child index out of range.");
	return m_children.at(static_cast<ChildrenContainer::size_type>(i));
}


const MenuTreeNode* MenuTreeNode::getChild(int i) const
{
	TT_ASSERTMSG(i >= 0 && i < static_cast<int>(m_children.size()),
	             "Menu tree child index out of range.");
	return m_children.at(static_cast<ChildrenContainer::size_type>(i));
}


MenuTreeNode* MenuTreeNode::getParent()
{
	return m_parent;
}


const MenuTreeNode* MenuTreeNode::getParent() const
{
	return m_parent;
}


void MenuTreeNode::createMenu()
{
	TT_ASSERTMSG(m_menu == 0, "Cannot create menu: menu already exists.");
	
	// Load the menu using MenuSystem
	//m_menu = MenuSystem::getInstance()->getMenu(fromSafeString(m_name));
	m_menu = MenuSystem::getInstance()->getMenu(m_name);
	if (m_selectionPath.empty() == false)
	{
		m_menu->setSelectionPath(m_selectionPath);
	}
}


void MenuTreeNode::destroyMenu()
{
	if (m_menu != 0)
	{
		m_menu->getSelectionPath(m_selectionPath);
		// Have the menu system destroy the menu
		MenuSystem::getInstance()->destroyMenu(m_menu);
		m_menu = 0;
	}
	
	if (m_clearSelectionPathAfterDestroy)
	{
		clearSelectionPath();
		m_clearSelectionPathAfterDestroy = false;
	}
}


void MenuTreeNode::destroyMenuRecursive()
{
	// Destroy the menu associated with this node
	destroyMenu();
	
	// Recurse into all children
	for (ChildrenContainer::iterator it = m_children.begin();
	     it != m_children.end(); ++it)
	{
		(*it)->destroyMenuRecursive();
	}
}


void MenuTreeNode::clearSelectionPathAfterDestroy()
{
	m_clearSelectionPathAfterDestroy = true;
}


/*
void* MenuTreeNode::operator new(size_t p_blockSize)
{
	using memory::HeapMgr;
#ifndef TT_BUILD_FINAL
	u32 foo = 0;
	asm {    mov     foo, lr}
	return HeapMgr::allocFromHeap(HeapMgr::SINGLETON, p_blockSize, 4, (void*)foo);
#else
	return HeapMgr::allocFromHeap(HeapMgr::SINGLETON, p_blockSize);
#endif
}


void MenuTreeNode::operator delete(void* p_block)
{
	memory::HeapMgr::freeToHeap(p_block);
}
//*/


//------------------------------------------------------------------------------
// Private member functions

MenuTreeNode::MenuTreeNode(const std::string& p_name, MenuTreeNode* p_parent)
:
//m_name(toSafeString(p_name)),
m_name(p_name),
m_parent(p_parent),
m_menu(0),
m_clearSelectionPathAfterDestroy(false)
{
	TT_ASSERTMSG(m_parent != 0,
	             "Child menu tree nodes must have valid parent pointer.");
}

// Namespace end
}
}
