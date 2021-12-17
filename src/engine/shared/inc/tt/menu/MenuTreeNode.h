#if !defined(INC_TT_MENU_MENUTREENODE_H)
#define INC_TT_MENU_MENUTREENODE_H


#include <string>
#include <vector>

//#include <tt/memory/SafeAllocator.h>
//#include <tt/memory/SafeString.h>
#include <tt/menu/elements/MenuElementInterface.h>


namespace tt {
namespace menu {

namespace elements
{
	class Menu;
}


/*! \brief Node for a doubly-linked tree of menu names. */
class MenuTreeNode
{
public:
	/*! \brief Constructs a tree root node. */
	MenuTreeNode(const std::string& p_name);
	
	/*! \brief Frees the memory for all child nodes. */
	~MenuTreeNode();
	
	/*! \brief Searches the direct children of this node
	           for a node with the specified name.
	    \param p_name The name to search for. 
	    \return Whether there is a direct child with the specified name. */
	bool isDirectChild(const std::string& p_name) const;
	
	/*! \brief Searches the tree starting at this node
	           for a node with the specified name,
	    \param p_name The name to search for. */
	MenuTreeNode* findNode(const std::string& p_name);
	
	/*! \brief Creates a node and adds it to this node.
	    \param p_name The name for the child node. */
	MenuTreeNode* createChild(const std::string& p_name);
	
	/*! \brief Returns the name of this node. */
	//inline std::string getName() const { return fromSafeString(m_name); }
	inline std::string getName() const { return m_name; }
	
	/*! \brief Returns the nunmber of direct children this node has. */
	inline int getChildCount() const
	{ return static_cast<int>(m_children.size()); }
	
	/*! \brief Returns the specified direct child of this node. */
	MenuTreeNode* getChild(int p_index);
	
	/*! \brief Returns the specified direct child of this node. */
	const MenuTreeNode* getChild(int p_index) const;
	
	/*! \brief Returns the direct parent of this node. */
	MenuTreeNode* getParent();
	
	/*! \brief Returns the direct parent of this node. */
	const MenuTreeNode* getParent() const;
	
	/*! \brief Creates the menu associated with this node. */
	void createMenu();
	
	/*! \brief Destroys the menu associated with this node. */
	void destroyMenu();
	
	/*! \brief Recursively destroys all menus in the tree starting at this node. */
	void destroyMenuRecursive();
	
	/*! \brief Returns the menu associated with this node. Null if no menu. */
	inline elements::Menu* getMenu() { return m_menu; }
	
	/*! \brief Returns the menu associated with this node. Null if no menu. */
	inline const elements::Menu* getMenu() const { return m_menu; }
	
	/*! \brief Indicates whether this node currently has a menu. */
	inline bool hasMenu() const { return m_menu != 0; }
	
	/*! \brief Clears the selection path. */
	inline void clearSelectionPath() { m_selectionPath.clear(); }
	
	/*! \brief Clears the selection path after the menu has been destroyed. */
	void clearSelectionPathAfterDestroy();
	
	// The menu tree nodes need to be on the safe heap
	/*
	static void* operator new(std::size_t p_blockSize);
	static void operator delete(void* p_block);
	//*/
	
private:
	// Safe heap children container
	//typedef std::vector<MenuTreeNode*,
	//                    memory::SafeAllocator<MenuTreeNode*> > ChildrenContainer;
	typedef std::vector<MenuTreeNode*> ChildrenContainer;
	
	
	/*! \brief Constructs a tree child node. */
	MenuTreeNode(const std::string& p_name, MenuTreeNode* p_parent);
	
	// Tree nodes may not be copied
	MenuTreeNode(const MenuTreeNode&);
	const MenuTreeNode& operator=(const MenuTreeNode&);
	
	
	// Safe heap member variables
	//memory::safestring                            m_name;          //!< This node's name.
	std::string                                   m_name;          //!< This node's name.
	MenuTreeNode*                                 m_parent;        //!< The parent of this tree node.
	ChildrenContainer                             m_children;      //!< All the children of this tree node.
	elements::MenuElementInterface::SelectionPath m_selectionPath; //!< The path to the selected item
	
	// Non-safe heap member variables
	elements::Menu* m_menu; //!< The menu associated with this node.
	
	/*! \brief Whether the selection path should be cleared
	           after the menu has been destroyed
	           (flag gets cleared after each destroy). */
	bool m_clearSelectionPathAfterDestroy;
};

// Namespace end
}
}


#endif  // !defined(INC_TT_MENU_MENUTREENODE_H)
