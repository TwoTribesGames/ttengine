#if !defined(INC_TT_MENU_MENUFACTORY_H)
#define INC_TT_MENU_MENUFACTORY_H


#include <string>

#include <tt/platform/tt_error.h>
#include <tt/menu/MenuTemplate.h>
#include <tt/menu/elements/element_traits.h>
#include <tt/menu/elements/ValueDecorator.h>
#include <tt/xml/XmlNode.h>


namespace tt {
namespace menu {

class MenuTemplate;
class MenuLayout;

namespace elements
{
	class Container;
	template<typename ChildType> class ContainerBase;
	class Line;
	class Label;
	class DynamicLabel;
	class MultiPageLabel;
	class Button;
	class SoftwareKeyboard;
	class Image;
	class ImageEditor;
	class FATImage;
	class Table;
	class ColorItem;
	class Window;
	class Menu;
	class ShowOneChild;
	class ShowOneValueChild;
	class ShowOneValueText;
	class ScrollableList;
	class FileList;
	class FileBlocksBar;
	class Scrollbar;
	class MenuElementInterface;
	class BusyBar;
	class ProgressBar;
}


/*! \brief Parses menu XML files into menu element trees, creating menu elements. */
class MenuFactory
{
public:
	MenuFactory();
	virtual ~MenuFactory();
	
	/*! \brief Creates a fully formed menu based on an XML definition. */
	elements::Menu* createMenuFromXML(const std::string& p_filename);
	
	// All menu factories need to be on the safe heap
	/*
	static void* operator new(std::size_t p_blockSize);
	static void operator delete(void* p_block);
	//*/
	
protected:
	// The basic menu element types
	enum ElementType
	{
		Type_Menu,
		Type_Window,
		Type_Container,
		Type_Line,
		Type_Image,
		Type_ImageEditor,
		Type_FATImage,
		Type_Label,
		Type_DynamicLabel,
		Type_MultiPageLabel,
		Type_Button,
		Type_SoftwareKeyboard,
		Type_Table,
		Type_ColorItem,
		Type_ShowOneChild,
		Type_ShowOneValueChild,
		Type_ShowOneValueText,
		Type_ScrollableList,
		Type_FileList,
		Type_FileBlocksBar,
		Type_Scrollbar,
		Type_BusyBar,
		Type_ProgressBar,
		
		Type_Children,  // Special: the children of a templated tag
		Type_Action,    // Special: the actions of an element
		Type_ActionSet, // Special: action sets for a menu
		Type_Unknown,   // Special: tag may be a template
		
		Type_LastType //!< The last standard menu type (custom types can start here)
	};
	
	
	//--------------------------------------------------------------------------
	// Standard element creation functions (cannot be overridden)
	
	elements::MenuElementInterface* createMenuElement(
			int           p_type,
			xml::XmlNode* p_node,
			MenuTemplate* p_template);
	
	elements::Menu* createMenu(
			xml::XmlNode* p_node, MenuTemplate* p_template);
	
	elements::Window* createWindow(
			xml::XmlNode* p_node, MenuTemplate* p_template);
	
	elements::Image* createImage(
			xml::XmlNode* p_node, MenuTemplate* p_template);
	
	elements::ImageEditor* createImageEditor(
			xml::XmlNode* p_node, MenuTemplate* p_template);
	
	elements::FATImage* createFATImage(
			xml::XmlNode* p_node, MenuTemplate* p_template);
	
	elements::Label* createLabel(
			xml::XmlNode* p_node, MenuTemplate* p_template);
	
	elements::DynamicLabel* createDynamicLabel(
			xml::XmlNode* p_node, MenuTemplate* p_template);
	
	elements::MultiPageLabel* createMultiPageLabel(
			xml::XmlNode* p_node, MenuTemplate* p_template);
	
	elements::Container* createContainer(
			xml::XmlNode* p_node, MenuTemplate* p_template);
	
	elements::Line* createLine(
			xml::XmlNode* p_node, MenuTemplate* p_template);
	
	elements::Button* createButton(
			xml::XmlNode* p_node, MenuTemplate* p_template);
	
	elements::SoftwareKeyboard* createSoftwareKeyboard(
			xml::XmlNode* p_node, MenuTemplate* p_template);
	
	elements::Table* createTable(
			xml::XmlNode* p_node, MenuTemplate* p_template);
	
	elements::ColorItem* createColorItem(
			xml::XmlNode* p_node, MenuTemplate* p_template);
	
	elements::ShowOneChild* createShowOneChild(
			xml::XmlNode* p_node, MenuTemplate* p_template);
	
	elements::ShowOneValueChild* createShowOneValueChild(
			xml::XmlNode* p_node, MenuTemplate* p_template);
	
	elements::ShowOneValueText* createShowOneValueText(
			xml::XmlNode* p_node, MenuTemplate* p_template);
	
	elements::ScrollableList* createScrollableList(
			xml::XmlNode* p_node, MenuTemplate* p_template);
	
	elements::FileList* createFileList(
			xml::XmlNode* p_node, MenuTemplate* p_template);
	
	elements::FileBlocksBar* createFileBlocksBar(
			xml::XmlNode* p_node, MenuTemplate* p_template);
	
	elements::Scrollbar* createScrollbar(
			xml::XmlNode* p_node, MenuTemplate* p_template);
	
	elements::BusyBar* createBusyBar(
			xml::XmlNode* p_node, MenuTemplate* p_template);
	
	elements::ProgressBar* createProgressBar(
			xml::XmlNode* p_node, MenuTemplate* p_template);
	
	
	//--------------------------------------------------------------------------
	// Helper functions
	
	/*! \brief Retrieves an attribute from an XML node and
	           optionally gets the real value from a template. */
	std::string getAttribute(xml::XmlNode* p_node,
	                         MenuTemplate* p_template,
	                         const char*   p_name);
	
	/*! \brief Builds a MenuLayout object based on an XML node. */
	MenuLayout getLayout(xml::XmlNode*       p_node,
	                     MenuTemplate*       p_template,
	                     const std::string&  p_name);
	
	/*! \brief Adds the children of the specified XML node to the specified parent.
	    \param p_parent reference to the parent object. NOTE: This can NOT be a pointer! */
	template<typename ParentType>
	void addChildren(ParentType&   p_parent,
	                 xml::XmlNode* p_parentNode,
	                 MenuTemplate* p_template)
	{
		doAddChildren(&p_parent, p_parentNode, p_template,
			typename elements::element_traits<ParentType>::element_category());
	}
	
	//--------------------------------------------------------------------------
	// Functions that need to be overridden for new elements
	
	/*! \brief Called when encountering an unknown element type.
	           Create custom elements here.
	           Returns null by default, upon which an assertion is triggered. */
	virtual elements::MenuElementInterface* createCustomElement(
			int           p_elementType,
			xml::XmlNode* p_node,
			MenuTemplate* p_template);
	
	/*! \brief Returns the element type identifier based on the element name. */
	virtual int getElementType(const char* p_typeName) const;
	
	
	// Prevent copying (not implemented on purpose)
	MenuFactory(const MenuFactory&);
	const MenuFactory& operator=(const MenuFactory&);
	
	elements::Menu* m_currentlyBuildingMenu;
	
private:
	/*! \brief Adds the children of the specified XML node to the specified menu. */
	void doAddChildren(elements::Menu* p_parent,
	                   xml::XmlNode*   p_parentNode,
	                   MenuTemplate*   p_template,
	                   elements::menu_element_tag);
	
	/*! \brief Adds the children of the specified XML node to the specified container. */
	template<typename ChildType>
	void doAddChildren(elements::ContainerBase<ChildType>* p_parent,
	                   xml::XmlNode*                       p_parentNode,
	                   MenuTemplate*                       p_template,
	                   elements::container_element_tag)
	{
		// Parse all children
		for (xml::XmlNode* child = p_parentNode->getChild(); child != 0;
		     child = child->getSibling()) 
		{
			// Add all children
			addChild(p_parent, child, p_template);
		}
	}
	
	/*! \brief Adds a single child to the specified container based on the specified XML node. */
	template<typename ChildType>
	void addChild(elements::ContainerBase<ChildType>* p_parent,
	              xml::XmlNode*                       p_childNode,
	              MenuTemplate*                       p_template)
	{
		// Retrieve type of child element
		int childType = getElementType(p_childNode->getName().c_str());
		
		if (childType == Type_Unknown)
		{
			// Child is not one of the base elements, see if it is a known template
			MenuTemplate* tpl = MenuTemplate::getTemplate(p_childNode);
			
			// Make sure it is a template
			TT_ASSERTMSG(tpl != 0, "Child node '%s' is not a template.",
			             p_childNode->getName().c_str());
			
			// Template may contain multiple nodes on base level
			// Add each node
			// The template node is now replaced by the contents of the template
			for (xml::XmlNode* node = tpl->getNode();
			     node != 0; node = node->getSibling())
			{
				addChild(p_parent, node, tpl);
			}
			
			// Free template
			delete tpl;
		}
		else if (childType == Type_Children)
		{
			// When the <children /> node is encountered within a template, it
			// needs to be replaced by the children of the used template tag
			
			// Make sure we have template
			TT_ASSERTMSG(p_template != 0,
			             "No template specified, but children need to be added.");
			
			for (xml::XmlNode* child = p_template->getChild();
			     child != 0; child = child->getSibling())
			{
				// Add all children of the template (not to be confused with its nodes) to the parent
				addChild(p_parent, child, 0);
			}
		}
		else if (childType == Type_Action)
		{
			// Add action
			addAction(p_parent, p_childNode, p_template);
		}
		else if (childType == Type_ActionSet)
		{
			// Ignore action sets
		}
		else
		{
			// Child is one of the base menu element types
			
			// Create a new instance of the child element
			elements::MenuElementInterface* childElement =
				createMenuElement(childType, p_childNode, p_template);
			
			// And add it to the parent
			p_parent->addChild(childElement);
		}
	}
	
	
	/*! \brief Loads the value decorators from the XML tree for the specified menu element. */
	template<typename ParentType>
	void doAddChildren(ParentType*   p_parent,
	                   xml::XmlNode* p_parentNode,
	                   MenuTemplate* p_template,
	                   elements::value_element_tag)
	{
		// Parse all children
		for (xml::XmlNode* child = p_parentNode->getChild();
		     child != 0; child = child->getSibling())
		{
			addValueChild(p_parent, child, p_template);
		}
	}
	
	/*! \brief Adds a single ValueDecorator with child to the specified
	           container based on the specified XML node. */
	template<typename ParentType>
	void addValueChild(ParentType*   p_parent,
	                   xml::XmlNode* p_childNode,
	                   MenuTemplate* p_template)
	{
		// Retrieve type of child element
		int childType = getElementType(p_childNode->getName().c_str());
		
		if (childType == Type_Unknown)
		{
			// Child is not one of the base elements, see if it is a known template
			MenuTemplate* tpl = MenuTemplate::getTemplate(p_childNode);
			
			// Make sure it is a template
			TT_ASSERTMSG(tpl != 0,
			             "Child node '%s' is not a template.",
			             p_childNode->getName().c_str());
			
			// Template may contain multiple nodes on base level
			// Add each node
			// The template node is now replaced by the contents of the template
			for (xml::XmlNode* node = tpl->getNode();
			     node != 0; node = node->getSibling())
			{
				addValueChild(p_parent, node, tpl);
			}
			
			// Free template
			delete tpl;
		}
		else if (childType == Type_Children)
		{
			// When the <children /> node is encountered within a template, it
			// needs to be replaced by the children of the used template tag
			
			// Make sure we have template
			TT_ASSERTMSG(p_template != 0,
			             "No template specified, but children need to be added.");
			
			for (xml::XmlNode* child = p_template->getChild();
			     child != 0; child = child->getSibling())
			{
				// Add all children of the template (not to be confused with its nodes) to the parent
				addValueChild(p_parent, child, 0);
			}
		}
		else if (childType == Type_Action)
		{
			// Add action
			addAction(p_parent, p_childNode, p_template);
		}
		else if (childType == Type_ActionSet)
		{
			// Ignore action sets
		}
		else
		{
			// Child is one of the base menu element types
			
			// Create a new instance of the child element
			elements::MenuElementInterface* childElement =
				createMenuElement(childType, p_childNode, p_template);
			
			std::string value = getAttribute(p_childNode, p_template, "value");
			TT_ASSERTMSG(value.empty() == false,
			             "Required attribute 'value' missing for element '%s' named '%s'.",
			             p_childNode->getName().c_str(), childElement->getName().c_str());
			
			// Decorate the element with a value decorator
			using elements::ValueDecorator;
			ValueDecorator* valueDec = new ValueDecorator(childElement, value);
			
			// And add it to the parent
			p_parent->addChild(valueDec);
		}
	}
	
	/*! \brief Loads the actions from the XML tree for the specified menu element. */
	void doAddChildren(elements::MenuElementInterface* p_parent,
	                   xml::XmlNode*                   p_parentNode,
	                   MenuTemplate*                   p_template,
	                   elements::action_element_tag);
	
	/*! \brief adds an action to this menu element.
	    \param p_parent the element to add the action to.
	    \param p_child_node XMLNode representing the action.
	    \param p_template optinal template information. */
	void addAction(elements::MenuElementInterface* p_parent,
	               xml::XmlNode*                   p_childNode,
	               MenuTemplate*                   p_template);
};

// Namespace end
}
}


#endif  // !defined(INC_TT_MENU_MENUFACTORY_H)
