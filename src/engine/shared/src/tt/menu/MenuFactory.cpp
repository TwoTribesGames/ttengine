#include <tt/code/ErrorStatus.h>
//#include <tt/memory/HeapMgr.h>
#include <tt/menu/MenuFactory.h>
#include <tt/menu/MenuLayout.h>
#include <tt/menu/MenuDebug.h>
#include <tt/menu/MenuKeyboard.h>
#include <tt/menu/MenuUtils.h>
#include <tt/menu/MenuSoundPlayer.h>
#include <tt/menu/elements/Button.h>
#include <tt/menu/elements/SoftwareKeyboard.h>
#include <tt/menu/elements/Menu.h>
#include <tt/menu/elements/Window.h>
#include <tt/menu/elements/Image.h>
#include <tt/menu/elements/ImageEditor.h>
#include <tt/menu/elements/FATImage.h>
#include <tt/menu/elements/Label.h>
#include <tt/menu/elements/DynamicLabel.h>
#include <tt/menu/elements/MultiPageLabel.h>
#include <tt/menu/elements/Container.h>
#include <tt/menu/elements/Line.h>
#include <tt/menu/elements/Table.h>
#include <tt/menu/elements/ColourItem.h>
#include <tt/menu/elements/MenuElement.h>
#include <tt/menu/elements/ShowOneChild.h>
#include <tt/menu/elements/ShowOneValueChild.h>
#include <tt/menu/elements/ShowOneValueText.h>
#include <tt/menu/elements/ScrollableList.h>
#include <tt/menu/elements/FileList.h>
#include <tt/menu/elements/FileBlocksBar.h>
#include <tt/menu/elements/Scrollbar.h>
#include <tt/menu/elements/SunkenBorderDecorator.h>
#include <tt/menu/elements/BusyBar.h>
#include <tt/menu/elements/ProgressBar.h>
#include <tt/str/parse.h>
#include <tt/xml/XmlFileReader.h>


namespace tt {
namespace menu {

using namespace elements;
using namespace xml;


//------------------------------------------------------------------------------
// Public member functions

MenuFactory::MenuFactory()
:
m_currentlyBuildingMenu(0)
{
}


MenuFactory::~MenuFactory()
{
}


// Create menu from XML
Menu* MenuFactory::createMenuFromXML(const std::string& p_filename)
{
	if (m_currentlyBuildingMenu != 0)
	{
		TT_PANIC("Cannot create menu from XML: another menu ('%s') "
		         "is already being built.",
		         m_currentlyBuildingMenu->getName().c_str());
		return 0;
	}
	
	// Prepare a pointer to the root node
	XmlNode* xmlRoot = 0;
	{
		// Load the xml file
		XmlFileReader xml;
		
		if (xml.loadFile(p_filename) == false)
		{
			TT_PANIC("Failed to load XML file '%s'.", p_filename.c_str());
			return 0;
		}
		
		// Create an XML tree
		xmlRoot = XmlNode::createTree(xml);
		if (xmlRoot == 0)
		{
			TT_PANIC("Creating node tree from XML file '%s' failed.",
			         p_filename.c_str());
			return 0;
		}
	} // Release memory of XmlFileReader
	
	// Make sure a valid menu XML file is loaded
	TT_ASSERT(xmlRoot->getName() == "menu"); // Root node should be "menu"
	TT_ASSERT(xmlRoot->getSibling() == 0);   // No siblings of root allowed
	
	// Use the menu element factory to load the menu recursively
	Menu* menu = static_cast<Menu*>(createMenuElement(Type_Menu, xmlRoot, 0));
	
	// Free memory used by XML hierarchy
	delete xmlRoot;
	xmlRoot = 0;
	
	if (menu == 0)
	{
		TT_PANIC("Creating menu from XML file '%s' failed.",
		         p_filename.c_str());
		m_currentlyBuildingMenu = 0;
		return 0;
	}
	
	// Have the menu perform its layout
	menu->doTopLevelLayout();
	
	// Send event that layout is done
	menu->onLayoutDone();
	
	// Reset the 'currently building menu' pointer
	m_currentlyBuildingMenu = 0;
	
	// Return the created menu
	return menu;
}


/*
void* MenuFactory::operator new(std::size_t p_blockSize)
{
	using memory::HeapMgr;
#ifndef TT_BUILD_FINAL
	u32 foo = 0;
	asm	{    mov     foo, lr}
	return HeapMgr::allocFromHeap(HeapMgr::SINGLETON, p_blockSize, 4, (void*)foo);
#else
	return HeapMgr::allocFromHeap(HeapMgr::SINGLETON, p_blockSize);
#endif
}


void MenuFactory::operator delete(void* p_block)
{
	memory::HeapMgr::freeToHeap(p_block);
}
//*/


//------------------------------------------------------------------------------
// Standard element creation functions (cannot be overridden)

// Base element creating factory function
// @ p_type:     Type of the element to be created (must match root node)
// @ p_node:     Root xml node containing information about element
// @ p_template: Template object in case element is child of template
elements::MenuElementInterface* MenuFactory::createMenuElement(
		int           p_type,
		XmlNode*      p_node,
		MenuTemplate* p_template)
{
	// Return value
	elements::MenuElementInterface* element = 0;
	
	(void)p_template; // Fixes weird compile error on OS X with OSX_Final for device
	
	// Call type specific factory function
	switch (p_type)
	{
	case Type_Menu:
		element = createMenu(p_node, p_template);
		break;
		
	case Type_Window:
		element = createWindow(p_node, p_template);
		break;
		
	case Type_Container:
		element = createContainer(p_node, p_template);
		break;
		
	case Type_Line:
		element = createLine(p_node, p_template);
		break;
		
	case Type_Image:
		element = createImage(p_node, p_template);
		break;
		
	case Type_ImageEditor:
		element = createImageEditor(p_node, p_template);
		break;
		
	case Type_FATImage:
		element = createFATImage(p_node, p_template);
		break;
		
	case Type_Label:
		element = createLabel(p_node, p_template);
		break;
		
	case Type_DynamicLabel:
		element = createDynamicLabel(p_node, p_template);
		break;
		
	case Type_MultiPageLabel:
		element = createMultiPageLabel(p_node, p_template);
		break;
		
	case Type_Button:
		element = createButton(p_node, p_template);
		break;
		
	case Type_SoftwareKeyboard:
		element = createSoftwareKeyboard(p_node, p_template);
		break;
		
	case Type_Table:
		element = createTable(p_node, p_template);
		break;
		
	case Type_ColorItem:
		element = createColorItem(p_node, p_template);
		break;
		
	case Type_ShowOneChild:
		element = createShowOneChild(p_node, p_template);
		break;
		
	case Type_ShowOneValueChild:
		element = createShowOneValueChild(p_node, p_template);
		break;
		
	case Type_ShowOneValueText:
		element = createShowOneValueText(p_node, p_template);
		break;
		
	case Type_ScrollableList:
		element = createScrollableList(p_node, p_template);
		break;
		
	case Type_FileList:
		element = createFileList(p_node, p_template);
		break;
		
	case Type_FileBlocksBar:
		element = createFileBlocksBar(p_node, p_template);
		break;
		
	case Type_Scrollbar:
		element = createScrollbar(p_node, p_template);
		break;
		
	case Type_BusyBar:
		element = createBusyBar(p_node, p_template);
		break;
		
	case Type_ProgressBar:
		element = createProgressBar(p_node, p_template);
		break;
		
	default:
		// Call hook for creating custom elements
		element = createCustomElement(p_type, p_node, p_template);
		if (element == 0)
		{
			TT_PANIC("Unsupported menu element: %d (node '%s')",
			         p_type, p_node->getName().c_str());
			return 0;
		}
		break;
	}
	
	// Get border attribute for element
	{
		std::string borderAttrib(getAttribute(p_node, p_template, "border"));
		if (borderAttrib == "sunken")
		{
			element = new SunkenBorderDecorator(element);
			TT_NULL_ASSERT(element);
		}
	}
	
	// Get the enabled and visible attributes for the element
	{
		std::string enabledAttrib(getAttribute(p_node, p_template, "enabled"));
		if (enabledAttrib.empty() == false)
		{
			TT_ERR_CREATE("enabled");
			bool enabled = str::parseBool(enabledAttrib, &errStatus);
			if (errStatus.hasError())
			{
				TT_PANIC("Element named '%s': Invalid value for attribute "
				         "'enabled': '%s' (must be a Boolean).",
				         element->getName().c_str(), enabledAttrib.c_str());
			}
			else
			{
				MENU_Printf("MenuFactory::createMenuElement: %s element '%s'.\n",
				            enabled ? "Enabling" : "Disabling",
				            element->getName().c_str());
				element->setEnabled(enabled);
			}
		}
		
		std::string visibleAttrib(getAttribute(p_node, p_template, "visible"));
		if (visibleAttrib.empty() == false)
		{
			TT_ERR_CREATE("visible");
			bool visible = str::parseBool(visibleAttrib, &errStatus);
			if (errStatus.hasError())
			{
				TT_PANIC("Element named '%s': Invalid value for attribute "
				         "'visible': '%s' (must be a Boolean).",
				         element->getName().c_str(), visibleAttrib.c_str());
			}
			else
			{
				MENU_Printf("MenuFactory::createMenuElement: %s element '%s'.\n",
				            visible ? "Showing" : "Hiding",
				            element->getName().c_str());
				element->setVisible(visible);
			}
		}
	}
	
	// Check whether this element wants a selection cursor
	{
		std::string wantCursorAttrib(getAttribute(p_node, p_template,
		                                          "wantcursor"));
		if (wantCursorAttrib.empty() == false)
		{
			SelectionCursorType wantCursor =
				stringToSelectionCursorType(wantCursorAttrib);
			element->setWantCursor(wantCursor);
		}
	}
	
	// Check whether this element should only respond to stylus input
	{
		std::string stylusOnlyAttrib(getAttribute(p_node, p_template,
		                                          "stylusonly"));
		if (stylusOnlyAttrib.empty() == false)
		{
			TT_ERR_CREATE("stylusOnly");
			bool stylusOnly = str::parseBool(stylusOnlyAttrib, &errStatus);
			if (errStatus.hasError())
			{
				TT_PANIC("Element '%s': Invalid value for attribute "
				         "'stylusonly': '%s' (must be a Boolean).",
				         element->getName().c_str(), stylusOnlyAttrib.c_str());
			}
			else
			{
				element->setStylusOnly(stylusOnly);
			}
		}
	}
	
	// Check whether this element has selection by default
	{
		std::string defaultSelectAttrib(getAttribute(p_node, p_template,
		                                             "default_selected"));
		if (defaultSelectAttrib.empty() == false)
		{
			TT_ERR_CREATE("stylusOnly");
			bool isDefaultSelected = str::parseBool(defaultSelectAttrib, &errStatus);
			if (errStatus.hasError())
			{
				TT_PANIC("Element '%s': Invalid value for attribute "
				         "'default_selected': '%s' (must be a Boolean).",
				         element->getName().c_str(),
				         defaultSelectAttrib.c_str());
			}
			else
			{
				element->setDefaultSelected(isDefaultSelected);
			}
		}
	}
	
	// Return the element that was created
	return element;
}


Menu* MenuFactory::createMenu(XmlNode* p_node, MenuTemplate* p_template)
{
	// Parse root XML node to get variables
	TT_ASSERT(p_node->getName() == "menu");
	
	// Retrieve name of menu. This is a required attribute
	std::string name = getAttribute(p_node, p_template, "name");
	TT_ASSERTMSG(name.empty() == false,
	             "Element 'menu' is missing required 'name' attribute.");
	
	// Get the optional "can_go_back" attribute (defaults to true)
	std::string canGoBackAttrib = getAttribute(p_node, p_template, "can_go_back");
	bool        canGoBack       = true;
	
	if (canGoBackAttrib.empty() == false)
	{
		TT_ASSERTMSG(canGoBackAttrib == "true" ||
		             canGoBackAttrib == "false",
		             "Menu '%s': Invalid value for attribute 'can_go_back' ('%s'). "
		             "Valid are 'true' and 'false'.",
		             name.c_str(), canGoBackAttrib.c_str());
		canGoBack = (canGoBackAttrib == "true");
	}
	
	
	// Get the optional "render_background" attribute (defaults to true)
	std::string renderBackgroundAttrib = getAttribute(p_node, p_template, "render_background");
	bool        renderBackground       = true;
	
	if (renderBackgroundAttrib.empty() == false)
	{
		TT_ASSERTMSG(renderBackgroundAttrib == "true" ||
		             renderBackgroundAttrib == "false",
		             "Menu '%s': Invalid value for attribute 'render_background' ('%s'). "
		             "Valid are 'true' and 'false'.", name.c_str(),
		             renderBackgroundAttrib.c_str());
		renderBackground = (renderBackgroundAttrib == "true");
	}
	
	// Get the layout options from the XML
	MenuLayout layout(getLayout(p_node, p_template, name));
	
	// Override the width and height (menus always take the entire screen)
	// FIXME: Remove hard-coded dimensions!
	layout.setWidth(256);
	layout.setHeight(192);
	layout.setWidthType(MenuLayout::Size_Absolute);
	layout.setHeightType(MenuLayout::Size_Absolute);
	
	// Set default ordering to vertical, if not defined
	if (layout.getOrder() == MenuLayout::Order_Undefined)
	{
		layout.setOrder(MenuLayout::Order_Vertical);
	}
	
	// Create a new menu
	Menu* menu = new Menu(name, layout);
	
	// Set the menu that's being built to this menu, if none was set yet
	if (m_currentlyBuildingMenu == 0)
	{
		m_currentlyBuildingMenu = menu;
	}
	
	menu->setCanGoBack(canGoBack);
	menu->setRenderBackground(renderBackground);
	
	// Add children to it
	addChildren(*menu, p_node, p_template);
	
	// Add any action sets
	for (XmlNode* child = p_node->getChild();
	     child != 0; child = child->getSibling())
	{
		if (child->getName() == "actionset")
		{
			// Get the name of the set
			std::string setName(getAttribute(child, p_template, "name"));
			TT_ASSERTMSG(setName.empty() == false,
			             "Action set for menu '%s' is missing required 'name' attribute.",
			             name.c_str());
			
			// Add the action set
			menu->addActionSet(setName);
			
			// Add all actions to this set
			for (XmlNode* actionNode = child->getChild(); actionNode != 0;
			     actionNode = actionNode->getSibling())
			{
				TT_ASSERTMSG(actionNode->getName() == "action",
				             "Action sets may only contain actions! "
				             "(encountered element '%s' in set '%s')",
				             actionNode->getName().c_str(), setName.c_str());
				
				// Get the required 'command' attribute
				std::string command(getAttribute(actionNode, p_template, "command"));
				TT_ASSERTMSG(command.empty() == false,
				             "Missing required 'command' attribute for "
				             "action in set '%s'.", setName.c_str());
				
				// Create a menu action for this node
				MenuAction action(command);
				
				// Get all parameters of the action
				for (u32 i = 0; i < actionNode->getAttributeCount(); ++i)
				{
					XmlNode::Attribute attrib(actionNode->getAttribute(i));
					
					if (attrib.first == "param")
					{
						// Get the (possibly templated) value for the parameter
						std::string param(attrib.second);
						
						if (param.empty() == false && param.at(0) == '$')
						{
							TT_ASSERTMSG(p_template != 0,
							             "Attempting to use a template "
							             "variable outside of a template.");
							param = p_template->getVariable(param.c_str());
						}
						
						// Add the parameter to the action
						action.addParameter(param);
					}
					else
					{
						// Only 'param' and 'command' attributes are allowed
						if (attrib.first != "command")
						{
							TT_PANIC("Invalid action attribute: %s",
							         attrib.first.c_str());
						}
					}
				}
				
				// Add the action to the set
				menu->addActionToSet(setName, action);
			}
		}
	}
	
	// Get the optional attribute that specifies the initial child selection
	std::string selChildName(getAttribute(p_node, p_template, "selected_child_name"));
	if (selChildName.empty() == false)
	{
		menu->setInitialChildByName(selChildName);
		menu->selectChildByName(selChildName);
	}
	
	return menu;
}


Window* MenuFactory::createWindow(XmlNode* p_node, MenuTemplate* p_template)
{
	TT_ASSERT(p_node->getName() == "window");
	
	// Retrieve name of window, this is a required attribute
	std::string name(getAttribute(p_node, p_template, "name"));
	
	MenuLayout layout(getLayout(p_node, p_template, name));
	
	if (layout.getOrder() == MenuLayout::Order_Undefined)
	{
		layout.setOrder(MenuLayout::Order_Vertical);
	}
	
	if (layout.getWidthType() == MenuLayout::Size_Undefined)
	{
		layout.setWidthType(MenuLayout::Size_Auto);
		layout.setWidth(0);
	}
	
	if (layout.getHeightType() == MenuLayout::Size_Undefined)
	{
		layout.setHeightType(MenuLayout::Size_Auto);
		layout.setHeight(0);
	}
	
	if (layout.getHorizontalPositionType() == MenuLayout::Position_Undefined)
	{
		layout.setHorizontalPositionType(MenuLayout::Position_Center);
		layout.setLeft(0);
	}
	
	if (layout.getVerticalPositionType() == MenuLayout::Position_Undefined)
	{
		layout.setVerticalPositionType(MenuLayout::Position_Center);
		layout.setTop(0);
	}
	
	// Get caption or wide caption
	std::string caption(getAttribute(p_node, p_template, "caption"));
	std::string wcaption(getAttribute(p_node, p_template, "wcaption"));
	
	TT_ASSERTMSG(caption.empty() || wcaption.empty(),
	             "Window '%s' has both the 'caption' as the 'wcaption' attribute. "
	             "It can only have one or the other, not both.",
	             name.c_str());
	
	bool        hasPoles = true;
	std::string hasPolesAttr(getAttribute(p_node, p_template, "has_poles"));
	
	if (hasPolesAttr.empty() == false)
	{
		TT_ERR_CREATE("has_poles");
		hasPoles = str::parseBool(hasPolesAttr, &errStatus);
		if (errStatus.hasError())
		{
			TT_PANIC("Window '%s' attribute 'has_poles' has invalid value: '%s'",
			         name.c_str(), hasPolesAttr.c_str());
		}
	}
	
	
	Window* wnd = 0;
	if (caption.empty() == false)
	{
		wnd = new Window(name, layout, caption, hasPoles);
	}
	else
	{
		wnd = new Window(name, layout,
		                 MenuUtils::hexToWideString(wcaption),
		                 hasPoles);
	}
	
	// Add children to it
	addChildren(*wnd, p_node, p_template);
	
	// Get the optional attribute that specifies the initial child selection
	std::string selChildName(getAttribute(p_node, p_template, "selected_child_name"));
	if (selChildName.empty() == false)
	{
		wnd->setInitialChildByName(selChildName);
		wnd->selectChildByName(selChildName);
	}
	
	return wnd;
}


Container* MenuFactory::createContainer(XmlNode*      p_node,
                                        MenuTemplate* p_template)
{
	TT_ASSERT(p_node->getName() == "container");
	
	// retrieve name of window, this is a required attribute
	std::string name = getAttribute(p_node, p_template, "name");
	
	MenuLayout layout(getLayout(p_node, p_template, name));
	
	if (layout.getOrder() == MenuLayout::Order_Undefined)
	{
		layout.setOrder(MenuLayout::Order_Vertical);
	}
	
	if (layout.getWidthType() == MenuLayout::Size_Undefined)
	{
		layout.setWidthType(MenuLayout::Size_Auto);
		layout.setWidth(0);
	}
	
	if (layout.getHeightType() == MenuLayout::Size_Undefined)
	{
		layout.setHeightType(MenuLayout::Size_Auto);
		layout.setHeight(0);
	}
	
	if (layout.getHorizontalPositionType() == MenuLayout::Position_Undefined)
	{
		layout.setHorizontalPositionType(MenuLayout::Position_Center);
		layout.setLeft(0);
	}
	
	if (layout.getVerticalPositionType() == MenuLayout::Position_Undefined)
	{
		layout.setVerticalPositionType(MenuLayout::Position_Center);
		layout.setTop(0);
	}
	
	// Get looping attribute
	std::string loopingAttr = getAttribute(p_node, p_template, "looping");
	bool        looping = true; // default looping
	if (loopingAttr.empty() == false)
	{
		TT_ASSERT(loopingAttr == "true" || loopingAttr == "false");
		looping = (loopingAttr == "true");
	}
	
	// Create container
	Container* cont = new Container(name, layout);
	
	// Add children to it
	addChildren(*cont, p_node, p_template);
	
	cont->setUserLoopEnable(looping);
	
	// Get the optional attribute that specifies the initial child selection
	std::string selChildName(getAttribute(p_node, p_template, "selected_child_name"));
	if (selChildName.empty() == false)
	{
		cont->setInitialChildByName(selChildName);
		cont->selectChildByName(selChildName);
	}
	
	return cont;
}


Line* MenuFactory::createLine(XmlNode* p_node, MenuTemplate* p_template)
{
	// Make sure the node name matches the element we're trying to create
	TT_ASSERT(p_node->getName() == "line");
	
	// Retrieve the element name (required)
	std::string name = getAttribute(p_node, p_template, "name");
	
	MenuLayout layout(getLayout(p_node, p_template, name));
	
	if (layout.getWidthType() == MenuLayout::Size_Undefined)
	{
		layout.setWidthType(MenuLayout::Size_Auto);
		layout.setWidth(0);
	}
	
	if (layout.getHeightType() == MenuLayout::Size_Undefined)
	{
		layout.setHeightType(MenuLayout::Size_Auto);
		layout.setHeight(0);
	}
	
	if (layout.getHorizontalPositionType() == MenuLayout::Position_Undefined)
	{
		layout.setHorizontalPositionType(MenuLayout::Position_Center);
		layout.setLeft(0);
	}
	
	if (layout.getVerticalPositionType() == MenuLayout::Position_Undefined)
	{
		layout.setVerticalPositionType(MenuLayout::Position_Center);
		layout.setTop(0);
	}
	
	// Create line element
	Line* line = new Line(name, layout);
	
	// Add children to it
	addChildren(*line, p_node, p_template);
	
	return line;
}


Table* MenuFactory::createTable(XmlNode* p_node, MenuTemplate* p_template)
{
	TT_ASSERT(p_node->getName() == "table");
	
	std::string name = getAttribute(p_node, p_template, "name");
	
	MenuLayout layout(getLayout(p_node, p_template, name));
	
	if (layout.getOrder() == MenuLayout::Order_Undefined)
	{
		layout.setOrder(MenuLayout::Order_Vertical);
	}
	
	if (layout.getWidthType() == MenuLayout::Size_Undefined)
	{
		layout.setWidthType(MenuLayout::Size_Auto);
		layout.setWidth(0);
	}
	
	if (layout.getHeightType() == MenuLayout::Size_Undefined)
	{
		layout.setHeightType(MenuLayout::Size_Auto);
		layout.setHeight(0);
	}
	
	if (layout.getHorizontalPositionType() == MenuLayout::Position_Undefined)
	{
		layout.setHorizontalPositionType(MenuLayout::Position_Center);
		layout.setLeft(0);
	}
	
	if (layout.getVerticalPositionType() == MenuLayout::Position_Undefined)
	{
		layout.setVerticalPositionType(MenuLayout::Position_Center);
		layout.setTop(0);
	}
	
	std::string columnsAttr = getAttribute(p_node, p_template, "columns");
	TT_ASSERTMSG(columnsAttr.empty() == false,
	             "Element 'table' (name '%s') is missing required 'columns' "
	             "attribute.", name.c_str());
	
	s32 columns = static_cast<s32>(atoi(columnsAttr.c_str()));
	
	std::string rowsAttr = getAttribute(p_node, p_template, "rows");
	TT_ASSERTMSG(rowsAttr.empty() == false,
	             "Element 'table' (name '%s') is missing required 'rows' "
	             "attribute.", name.c_str());
	
	s32 rows = static_cast<s32>(atoi(rowsAttr.c_str()));
	
	std::string borderAttr = getAttribute(p_node, p_template, "border_size");
	s32 borderSize = atoi(borderAttr.c_str());
	
	std::string markerAttr = getAttribute(p_node, p_template, "has_marker");
	bool hasMarker = true;
	if (markerAttr.empty() == false)
	{
		TT_ASSERT(markerAttr == "false" || markerAttr == "true");
		hasMarker = (markerAttr == "true");
	}
	
	// Get looping attribute
	std::string loopingAttr(getAttribute(p_node, p_template, "looping"));
	bool looping = true; // default looping
	if (loopingAttr.empty() == false)
	{
		TT_ASSERT(loopingAttr == "true" || loopingAttr == "false");
		looping = (loopingAttr == "true");
	}
	
	// Create the table
	Table* table = new Table(name, layout, columns, rows, borderSize, hasMarker);
	
	// Add children to it
	addChildren(*table, p_node, p_template);
	
	table->setUserLoopEnable(looping);
	
	// Get the optional attribute that specifies the initial child selection
	std::string selChildName(getAttribute(p_node, p_template, "selected_child_name"));
	std::string selChildValue(getAttribute(p_node, p_template, "selected_child_value"));
	
	TT_ASSERTMSG(selChildName.empty() || selChildValue.empty(),
	             "Table '%s': Cannot specify the selected child by both name AND value.",
	             name.c_str());
	if (selChildName.empty() == false)
	{
		table->setInitialChildByName(selChildName);
		table->selectChildByName(selChildName);
	}
	if (selChildValue.empty() == false)
	{
		table->setInitialChildByValue(selChildValue);
		table->selectChildByValue(selChildValue);
	}
	
	return table;
}


ColorItem* MenuFactory::createColorItem(XmlNode* p_node, MenuTemplate* p_template)
{
	TT_ASSERT(p_node->getName() == "coloritem");
	
	std::string name = getAttribute(p_node, p_template, "name");
	
	MenuLayout layout(getLayout(p_node, p_template, name));
	
	if (layout.getOrder() == MenuLayout::Order_Undefined)
	{
		layout.setOrder(MenuLayout::Order_Vertical);
	}
	
	if (layout.getWidthType() == MenuLayout::Size_Undefined)
	{
		layout.setWidthType(MenuLayout::Size_Auto);
		layout.setWidth(0);
	}
	
	if (layout.getHeightType() == MenuLayout::Size_Undefined)
	{
		layout.setHeightType(MenuLayout::Size_Auto);
		layout.setHeight(0);
	}
	
	if (layout.getHorizontalPositionType() == MenuLayout::Position_Undefined)
	{
		layout.setHorizontalPositionType(MenuLayout::Position_Center);
		layout.setLeft(0);
	}
	
	if (layout.getVerticalPositionType() == MenuLayout::Position_Undefined)
	{
		layout.setVerticalPositionType(MenuLayout::Position_Center);
		layout.setTop(0);
	}
	
	std::string color = getAttribute(p_node, p_template, "color");
	TT_ASSERTMSG(color.empty() == false,
	             "Element 'coloritem' (name '%s') is missing required 'color' "
	             "attribute.", name.c_str());
	
	u32 col = static_cast<u32>(atoi(color.c_str()));
	
	ColorItem* colorItem = new ColorItem(name, layout, col);
	
	addChildren(*colorItem, p_node, p_template);
	
	return colorItem;
}


Button* MenuFactory::createButton(XmlNode* p_node, MenuTemplate* p_template)
{
	TT_ASSERT(p_node->getName() == "button");
	
	std::string name(getAttribute(p_node, p_template, "name"));
	
	std::string caption(getAttribute(p_node, p_template, "caption"));
	std::string image(getAttribute(p_node, p_template, "image"));
	std::string pressed_image(getAttribute(p_node, p_template, "pressed_image"));
	std::string selected_image(getAttribute(p_node, p_template, "selected_image"));
	std::string disabled_image(getAttribute(p_node, p_template, "disabled_image"));
	
	TT_ASSERTMSG(caption.empty() == false || image.empty() == false,
	             "Element 'button' (name '%s') is missing required"
	             " 'caption' or 'image' attribute.", name.c_str());
	
	TT_ASSERTMSG(pressed_image.empty() || image.empty() == false,
	             "Element 'button' (name '%s') has 'pressed_image'"
	             " attribute but no 'image' attribute.", name.c_str());
	
	TT_ASSERTMSG(selected_image.empty() || image.empty() == false,
	             "Element 'button' (name '%s') has 'selected_image'"
	             " attribute but no 'image' attribute.", name.c_str());
	
	TT_ASSERTMSG(disabled_image.empty() || image.empty() == false,
	             "Element 'button' (name '%s') has 'disabled_image'"
	             " attribute but no 'image' attribute.", name.c_str());
	
	MenuLayout layout(getLayout(p_node, p_template, name));
	layout.setUndefinedToDefault();
	
	std::string key      = getAttribute(p_node, p_template, "key");
	std::string key_bind = getAttribute(p_node, p_template, "key_bind");
	
	TT_ASSERTMSG(key.empty() || key_bind.empty(),
	             "Button '%s' can not have both the 'key' and 'key_bind' attribute",
	             name.c_str());
	
	// The default key
	MenuKeyboard::MenuKey action_tgr_key = MenuKeyboard::MENU_ACCEPT;
	if (key.empty() == false)
	{
		// Overwrite the default key
		action_tgr_key = MenuKeyboard::getKeyMappingFromString(key);
	}
	else if (key_bind.empty() == false)
	{
		// Overwrite the default key
		action_tgr_key = MenuKeyboard::getKeyMappingFromString(key_bind);
		// Bind key after the button is created.
	}
	
	bool handle_repeat = false;
	std::string handle_repeat_attrib = getAttribute(p_node, p_template, "handle_repeat");
	if (handle_repeat_attrib.empty() == false)
	{
		if (handle_repeat_attrib == "true")
		{
			handle_repeat = true;
		}
		else if (handle_repeat_attrib == "false")
		{
			handle_repeat = false;
		}
		else
		{
			TT_PANIC("Button named '%s': Invalid value for attribute 'handle_repeat': '%s'.",
			            name.c_str(), handle_repeat_attrib.c_str());
		}
	}
	
	// Get optional image dimensions and UV coordinates
	s32  imgW = 0;
	s32  imgH = 0;
	real imgU = 0.0f;
	real imgV = 0.0f;
	
	{
		TT_ERR_CREATE("button");
		s32 val = str::parseS32(getAttribute(p_node, p_template, "imagewidth"), &errStatus);
		if (errStatus.hasError())
		{
			errStatus.resetError();
		}
		else
		{
			imgW = val;
		}
		
		val = str::parseS32(getAttribute(p_node, p_template, "imageheight"), &errStatus);
		if (errStatus.hasError())
		{
			errStatus.resetError();
		}
		else
		{
			imgH = val;
		}
		
		imgU = str::parseReal(getAttribute(p_node, p_template, "u"), 0);
		imgV = str::parseReal(getAttribute(p_node, p_template, "v"), 0);
	}
	
	// Create a new button
	Button* btn = new Button(name,
	                         layout,
	                         caption,
	                         image,
	                         pressed_image,
	                         selected_image,
	                         disabled_image,
	                         imgU,
	                         imgV,
	                         imgW,
	                         imgH,
	                         action_tgr_key,
	                         handle_repeat);
	
	if (key_bind.empty() == false)
	{
		TT_ASSERTMSG(m_currentlyBuildingMenu != 0,
		             "Trying to bind key '%s' to button '%s', but there is no menu.",
		             key_bind.c_str(), name.c_str());
		m_currentlyBuildingMenu->bindKeyToMenuElement(action_tgr_key, btn);
	}
	
	// Check if a sound effect is specified for this button
	std::string sound_effect_attrib(getAttribute(p_node, p_template, "sound"));
	if (sound_effect_attrib.empty() == false)
	{
		btn->setSoundEffect(MenuSoundPlayer::getSoundEnum(sound_effect_attrib));
	}
	
	// Add Actions
	addChildren(*btn, p_node, p_template);
	
	return btn;
}


SoftwareKeyboard* MenuFactory::createSoftwareKeyboard(XmlNode* p_node, MenuTemplate* p_template)
{
	TT_ASSERT(p_node->getName() == "keyboard");
	
	std::string name = getAttribute(p_node, p_template, "name");
	
	MenuLayout layout(getLayout(p_node, p_template, name));
	layout.setUndefinedToDefault();
	
	std::string varname = getAttribute(p_node, p_template, "variable");
	TT_ASSERTMSG(varname.empty() == false, 
	             "Element 'keyboard' is missing required 'variable' attribute.");
	
	std::string label = getAttribute(p_node, p_template, "label");
	std::string okay = getAttribute(p_node, p_template, "okay");
	
	std::string lengthstr = getAttribute(p_node, p_template, "length");
	s32 length = -1;
	if (lengthstr.empty() == false)
	{
		length = std::atoi(lengthstr.c_str());
	}
	
	std::string pixellengthstr = getAttribute(p_node, p_template, "pixellength");
	s32 pixellength = -1;
	if (pixellengthstr.empty() == false)
	{
		pixellength = std::atoi(pixellengthstr.c_str());
	}
	
	std::string default_value(getAttribute(p_node, p_template, "default_value"));
	std::wstring w_default_value = MenuUtils::hexToWideString(default_value);
	
	// Create a new keyboard
	SoftwareKeyboard* kbd = new SoftwareKeyboard(name, layout, varname, label,
		okay, w_default_value, length, pixellength);
	
	// Add Actions
	addChildren(*kbd, p_node, p_template);
	
	return kbd;
}


ShowOneChild* MenuFactory::createShowOneChild(xml::XmlNode* p_node,
                                              MenuTemplate* p_template)
{
	TT_ASSERT(p_node->getName() == "show_one_child");
	
	std::string name = getAttribute(p_node, p_template, "name");
	
	MenuLayout layout(getLayout(p_node, p_template, name));
	
	if (layout.getOrder() == MenuLayout::Order_Undefined)
	{
		layout.setOrder(MenuLayout::Order_Horizontal);
	}
	layout.setUndefinedToDefault();
	
	// Get looping attribute
	std::string loopingAttr = getAttribute(p_node, p_template, "looping");
	bool looping = true; // default looping
	if (loopingAttr.empty() == false)
	{
		TT_ASSERT(loopingAttr == "true" || loopingAttr == "false");
		looping = (loopingAttr == "true");
	}
	
	// Get 'force selection refresh' attribute
	std::string force_selection_refresh_attr(getAttribute(p_node, p_template,
		"force_selection_refresh"));
	bool force_selection_refresh = false;
	if (force_selection_refresh_attr.empty() == false)
	{
		TT_ASSERTMSG(force_selection_refresh_attr == "true" ||
		             force_selection_refresh_attr == "false",
		             "Invalid force_selection_refresh value: '%s' "
		             "(can only be 'true' or 'false').",
		             force_selection_refresh_attr.c_str());
		force_selection_refresh = (force_selection_refresh_attr == "true");
	}
	
	// Create a new ShowOneChild
	ShowOneChild* soc = new ShowOneChild(name, layout);
	soc->setUserLoopEnable(looping);
	
	if (force_selection_refresh_attr.empty() == false)
	{
		soc->setForceSelectionRefresh(force_selection_refresh);
	}
	
	// Add children to it
	addChildren(*soc, p_node, p_template);
	
	// Get the optional attribute that specifies the initial child selection
	std::string selChildName(getAttribute(p_node, p_template, "selected_child_name"));
	if (selChildName.empty() == false)
	{
		soc->setInitialChildByName(selChildName);
		soc->selectChildByName(selChildName);
	}
	
	return soc;
}


ShowOneValueChild* MenuFactory::createShowOneValueChild(xml::XmlNode* p_node,
                                                        MenuTemplate* p_template)
{
	TT_ASSERT(p_node->getName() == "show_one_value_child");
	
	std::string name = getAttribute(p_node, p_template, "name");
	
	MenuLayout layout(getLayout(p_node, p_template, name));
	
	if (layout.getOrder() == MenuLayout::Order_Undefined)
	{
		layout.setOrder(MenuLayout::Order_Horizontal);
	}
	layout.setUndefinedToDefault();
	
	// Get looping attribute
	std::string loopingAttr = getAttribute(p_node, p_template, "looping");
	bool looping = true; // default looping
	if (loopingAttr.empty() == false)
	{
		TT_ASSERT(loopingAttr == "true" || loopingAttr == "false");
		looping = (loopingAttr == "true");
	}
	
	// Get 'force selection refresh' attribute
	std::string force_selection_refresh_attr(getAttribute(p_node, p_template,
		"force_selection_refresh"));
	bool force_selection_refresh = false;
	if (force_selection_refresh_attr.empty() == false)
	{
		TT_ASSERTMSG(force_selection_refresh_attr == "true" ||
		             force_selection_refresh_attr == "false",
		             "Invalid force_selection_refresh value: '%s' "
		             "(can only be 'true' or 'false').",
		             force_selection_refresh_attr.c_str());
		force_selection_refresh = (force_selection_refresh_attr == "true");
	}
	
	// Create a new ShowOneValueChild
	ShowOneValueChild* soc = new ShowOneValueChild(name, layout);
	soc->setUserLoopEnable(looping);
	
	if (force_selection_refresh_attr.empty() == false)
	{
		soc->setForceSelectionRefresh(force_selection_refresh);
	}
	
	// Add children to it
	addChildren(*soc, p_node, p_template);
	
	// Get the optional attribute that specifies the initial child selection
	std::string selChildName(getAttribute(p_node, p_template, "selected_child_name"));
	std::string selChildValue(getAttribute(p_node, p_template, "selected_child_value"));
	
	TT_ASSERTMSG(selChildName.empty() || selChildValue.empty(),
	             "ShowOneValueChild '%s': Cannot specify the selected child by both name AND value.",
	             name.c_str());
	if (selChildName.empty() == false)
	{
		soc->setInitialChildByName(selChildName);
		soc->selectChildByName(selChildName);
	}
	
	if (selChildValue.empty() == false)
	{
		MENU_Printf("MenuFactory::createShowOneValueChild: Creating show one "
		            "value child '%s' with selected_child_value '%s'.\n",
		            name.c_str(), selChildValue.c_str());
		
		soc->setInitialChildByValue(selChildValue);
		soc->selectChildByValue(selChildValue);
	}
	else
	{
		MENU_Printf("MenuFactory::createShowOneValueChild: Creating show one "
		            "value child '%s' without selecting child by value.\n",
		            name.c_str());
	}
	
	return soc;
}


ShowOneValueText* MenuFactory::createShowOneValueText(xml::XmlNode* p_node,
                                                      MenuTemplate* p_template)
{
	TT_ASSERT(p_node->getName() == "show_one_value_text");
	
	std::string name = getAttribute(p_node, p_template, "name");
	
	MenuLayout layout(getLayout(p_node, p_template, name));
	layout.setUndefinedToDefault();
	
	// Get looping attribute
	std::string loopingAttr = getAttribute(p_node, p_template, "looping");
	bool looping = true; // default looping
	if (loopingAttr.empty() == false)
	{
		TT_ASSERT(loopingAttr == "true" || loopingAttr == "false");
		looping = (loopingAttr == "true");
	}
	
	// Get the text alignment attribute
	std::string text_align_attrib(getAttribute(p_node, p_template, "text_align"));
	engine::glyph::GlyphSet::Alignment text_align = engine::glyph::GlyphSet::ALIGN_CENTER;
	if (text_align_attrib.empty() == false)
	{
		if (text_align_attrib == "left")
		{
			text_align = engine::glyph::GlyphSet::ALIGN_LEFT;
		}
		else if (text_align_attrib == "center")
		{
			text_align = engine::glyph::GlyphSet::ALIGN_CENTER;
		}
		else if (text_align_attrib == "right")
		{
			text_align = engine::glyph::GlyphSet::ALIGN_RIGHT;
		}
		else
		{
			TT_PANIC("ShowOneValueText '%s': Invalid text alignment specified: %s",
			         name.c_str(), text_align_attrib.c_str());
		}
	}
	
	// Create a new ShowOneValueText
	ShowOneValueText* sovt = new ShowOneValueText(name, layout);
	sovt->setUserLoopEnable(looping);
	
	if (text_align_attrib.empty() == false)
	{
		sovt->setHorizontalAlign(text_align);
	}
	
	// Add texts to it
	//s32 templateindex = 0;
	for (XmlNode* child = p_node->getChild();
	     child != 0; child = child->getSibling())
	{
		if (child->getName() == "children")
		{
			/*TT_NULL_ASSERT(p_template);
			
			child = p_template->getChild(templateindex);
			++templateindex;
			if (templateindex < p_template->getChildCount())
			{
				--i;
			}*/
		}
		TT_ASSERTMSG(child->getName() == "text",
		             "'%s' found child of type '%s', show_one_value_text"
		             "can only have children of type 'text'.",
		             name.c_str(), child->getName().c_str());
		
		std::string visible(getAttribute(child, p_template, "visible"));
		if (visible == "false")
		{
			continue;
		}
		std::string value = getAttribute(child, p_template, "value");
		std::string caption = getAttribute(child, p_template, "caption");
		std::string wcaption = getAttribute(child, p_template, "wcaption");
		
		TT_ASSERTMSG(value.empty() == false,
		             "'%s' has a text without a value, please specify a value.",
		             name.c_str());
		if (caption.empty() == false)
		{
			TT_ASSERTMSG(wcaption.empty(),
			             "'%s' has a text with both caption and wcaption "
			             "specified, please specify only one.", name.c_str());
			sovt->addText(value, caption);
		}
		else if (wcaption.empty() == false)
		{
			sovt->addText(value, MenuUtils::hexToWideString(wcaption));
		}
		else
		{
			TT_PANIC("'%s' has a text without caption or wcaption specified.",
			         name.c_str());
		}
	}
	
	// Get the optional attribute that specifies the initial child selection
	std::string selected_value(getAttribute(p_node, p_template, "selected_value"));
	
	if (selected_value.empty() == false)
	{
		MENU_Printf("MenuFactory::createShowOneValueText: Creating show one "
		            "value text '%s' with selected_value '%s'.\n",
		            name.c_str(), selected_value.c_str());
		
		sovt->setInitialTextByValue(selected_value);
		sovt->selectTextByValue(selected_value);
	}
	else
	{
		MENU_Printf("MenuFactory::createShowOneValueText: Creating show one "
		            "value text '%s' without selecting text by value.\n",
		            name.c_str());
	}
	
	return sovt;
}


ScrollableList* MenuFactory::createScrollableList(XmlNode* p_node,
                                                  MenuTemplate* p_template)
{
	TT_ASSERT(p_node->getName() == "scrollablelist");
	
	std::string name = getAttribute(p_node, p_template, "name");
	
	MenuLayout layout(getLayout(p_node, p_template, name));
	
	if (layout.getOrder() == MenuLayout::Order_Undefined)
	{
		layout.setOrder(MenuLayout::Order_Vertical);
	}
	
	if (layout.getWidthType() == MenuLayout::Size_Undefined)
	{
		layout.setWidthType(MenuLayout::Size_Auto);
		layout.setWidth(0);
	}
	
	if (layout.getHeightType() == MenuLayout::Size_Undefined)
	{
		layout.setHeightType(MenuLayout::Size_Auto);
		layout.setHeight(0);
	}
	
	if (layout.getHorizontalPositionType() == MenuLayout::Position_Undefined)
	{
		layout.setHorizontalPositionType(MenuLayout::Position_Center);
		layout.setLeft(0);
	}
	
	if (layout.getVerticalPositionType() == MenuLayout::Position_Undefined)
	{
		layout.setVerticalPositionType(MenuLayout::Position_Center);
		layout.setTop(0);
	}
	
	// Get looping attribute
	std::string loopingAttr = getAttribute(p_node, p_template, "looping");
	bool looping = true; // default looping
	if (loopingAttr.empty() == false)
	{
		TT_ASSERT(loopingAttr == "true" || loopingAttr == "false");
		looping = (loopingAttr == "true");
	}
	
	ScrollableList* scroll_list = new ScrollableList(name, layout);
	
	scroll_list->setUserLoopEnable(looping);
	
	// Add children to it
	addChildren(*scroll_list, p_node, p_template);
	
	// Get the optional attribute that specifies the initial child selection
	std::string selChildName(getAttribute(p_node, p_template, "selected_child_name"));
	if (selChildName.empty() == false)
	{
		scroll_list->setInitialChildByName(selChildName);
		scroll_list->selectChildByName(selChildName);
	}
	
	return scroll_list;
}


FileList* MenuFactory::createFileList(XmlNode* p_node, MenuTemplate* p_template)
{
	TT_ASSERT(p_node->getName() == "filelist");
	
	std::string name = getAttribute(p_node, p_template, "name");
	
	MenuLayout layout(getLayout(p_node, p_template, name));
	
	if (layout.getOrder() == MenuLayout::Order_Undefined)
	{
		layout.setOrder(MenuLayout::Order_Vertical);
	}
	
	if (layout.getWidthType() == MenuLayout::Size_Undefined)
	{
		layout.setWidthType(MenuLayout::Size_Auto);
		layout.setWidth(0);
	}
	
	if (layout.getHeightType() == MenuLayout::Size_Undefined)
	{
		layout.setHeightType(MenuLayout::Size_Auto);
		layout.setHeight(0);
	}
	
	if (layout.getHorizontalPositionType() == MenuLayout::Position_Undefined)
	{
		layout.setHorizontalPositionType(MenuLayout::Position_Center);
		layout.setLeft(0);
	}
	
	if (layout.getVerticalPositionType() == MenuLayout::Position_Undefined)
	{
		layout.setVerticalPositionType(MenuLayout::Position_Center);
		layout.setTop(0);
	}
	
	std::string has_empty_attrib(getAttribute(p_node, p_template, "has_empty"));
	bool        has_empty = false;
	if (has_empty_attrib.empty() == false)
	{
		if (has_empty_attrib == "true")
		{
			has_empty = true;
		}
		else if (has_empty_attrib == "false")
		{
			has_empty = false;
		}
		else
		{
			TT_PANIC("Element '%s': Invalid value for 'has_empty' attribute: %s",
			         name.c_str(), has_empty_attrib.c_str());
		}
	}
	
	std::string name_variable(getAttribute(p_node, p_template, "name_variable"));
	std::string type_variable(getAttribute(p_node, p_template, "type_variable"));
	std::string confirm_dialog(getAttribute(p_node, p_template, "confirm_dialog"));
	std::string confirm_variable(getAttribute(p_node, p_template, "confirm_variable"));
	std::string filter(getAttribute(p_node, p_template, "filter"));
	std::string corruption(getAttribute(p_node, p_template, "corruption"));
	std::string parent(getAttribute(p_node, p_template, "parent"));
	TT_ASSERTMSG(name_variable.empty() == false,
	             "FileList '%s': Empty name_variable specified!", name.c_str());
	TT_ASSERTMSG(type_variable.empty() == false,
	             "FileList '%s': Empty type_variable specified!", name.c_str());
	TT_ASSERTMSG(filter.empty() == false,
	             "FileList '%s': Empty filter specified!", name.c_str());
	
	FileList* file_list = new FileList(name, layout, has_empty, name_variable,
		type_variable, confirm_dialog, confirm_variable,
		MenuUtils::hexToWideString(filter), corruption, parent);
	
	// Add children to it
	addChildren(*file_list, p_node, p_template);
	
	// Get the optional attribute that specifies the initial child selection
	std::string selChildName(getAttribute(p_node, p_template, "selected_child_name"));
	if (selChildName.empty() == false)
	{
		file_list->setInitialChildByName(selChildName);
		file_list->selectChildByName(selChildName);
	}
	
	return file_list;
}


FileBlocksBar* MenuFactory::createFileBlocksBar(XmlNode*      p_node,
                                                MenuTemplate* p_template)
{
	TT_ASSERT(p_node->getName() == "fileblocksbar");
	
	std::string name(getAttribute(p_node, p_template, "name"));
	
	// Retrieve the bar display type (free or used)
	std::string type(getAttribute(p_node, p_template, "type"));
	FileBlocksBar::BlocksType displayType = FileBlocksBar::Blocks_Free;
	if (type.empty() || type == "free")
	{
		displayType = FileBlocksBar::Blocks_Free;
	}
	else if (type == "used")
	{
		displayType = FileBlocksBar::Blocks_Used;
	}
	else
	{
		TT_PANIC("File blocks bar '%s' specifies invalid display type '%s'.",
		         name.c_str(), type.c_str());
	}
	
	
	MenuLayout layout(getLayout(p_node, p_template, name));
	layout.setUndefinedToDefault();
	
	// Create a new bar
	FileBlocksBar* fbb = new FileBlocksBar(name, layout, displayType);
	addChildren(*fbb, p_node, p_template);
	return fbb;
}


Scrollbar* MenuFactory::createScrollbar(XmlNode* p_node,
                                        MenuTemplate* p_template)
{
	TT_ASSERT(p_node->getName() == "scrollbar");
	
	std::string name = getAttribute(p_node, p_template, "name");
	
	MenuLayout layout(getLayout(p_node, p_template, name));
	
	if (layout.getOrder() == MenuLayout::Order_Undefined)
	{
		layout.setOrder(MenuLayout::Order_Vertical);
	}
	
	if (layout.getWidthType() == MenuLayout::Size_Undefined)
	{
		layout.setWidthType(MenuLayout::Size_Auto);
		layout.setWidth(0);
	}
	
	if (layout.getHeightType() == MenuLayout::Size_Undefined)
	{
		layout.setHeightType(MenuLayout::Size_Auto);
		layout.setHeight(0);
	}
	
	if (layout.getHorizontalPositionType() == MenuLayout::Position_Undefined)
	{
		layout.setHorizontalPositionType(MenuLayout::Position_Center);
		layout.setLeft(0);
	}
	
	if (layout.getVerticalPositionType() == MenuLayout::Position_Undefined)
	{
		layout.setVerticalPositionType(MenuLayout::Position_Center);
		layout.setTop(0);
	}
	
	Scrollbar* scrbar = new Scrollbar(name, layout);
	addChildren(*scrbar, p_node, p_template);
	return scrbar;
}


Image* MenuFactory::createImage(XmlNode* p_node, MenuTemplate* p_template)
{
	TT_ASSERT(p_node->getName() == "image");
	
	std::string name = getAttribute(p_node, p_template, "name");
	
	// Retrieve filename of image to display. This is a required attribute
	std::string filename = getAttribute(p_node, p_template, "filename");
	TT_ASSERTMSG(filename.empty() == false,
	             "Element 'image' (name '%s') is missing required 'filename' "
	             "attribute.", name.c_str());
	
	MenuLayout layout(getLayout(p_node, p_template, name));
	layout.setUndefinedToDefault();
	
	
	real u = 0.0f;
	real v = 0.0f;
	s32  w = 0;
	s32  h = 0;
	
	{
		TT_ERR_CREATE("image");
		u = str::parseReal(getAttribute(p_node, p_template, "u"), 0);
		v = str::parseReal(getAttribute(p_node, p_template, "v"), 0);
		
		s32 width = str::parseS32(getAttribute(p_node, p_template, "imagewidth"), &errStatus);
		if (errStatus.hasError() == false)
		{
			w = width;
		}
		errStatus.resetError();
		
		s32 height = str::parseS32(getAttribute(p_node, p_template, "imageheight"), &errStatus);
		if (errStatus.hasError() == false)
		{
			h = height;
		}
	}
	
	Image* img = new Image(name, layout, filename, u, v, w, h);
	addChildren(*img, p_node, p_template);
	return img;
}


ImageEditor* MenuFactory::createImageEditor(XmlNode* p_node,
                                            MenuTemplate* p_template)
{
	TT_ASSERT(p_node->getName() == "imageeditor");
	
	std::string name = getAttribute(p_node, p_template, "name");
	
	MenuLayout layout(getLayout(p_node, p_template, name));
	
	if (layout.getOrder() == MenuLayout::Order_Undefined)
	{
		layout.setOrder(MenuLayout::Order_Vertical);
	}
	
	if (layout.getWidthType() == MenuLayout::Size_Undefined)
	{
		layout.setWidthType(MenuLayout::Size_Auto);
		layout.setWidth(0);
	}
	
	if (layout.getHeightType() == MenuLayout::Size_Undefined)
	{
		layout.setHeightType(MenuLayout::Size_Auto);
		layout.setHeight(0);
	}
	
	if (layout.getHorizontalPositionType() == MenuLayout::Position_Undefined)
	{
		layout.setHorizontalPositionType(MenuLayout::Position_Center);
		layout.setLeft(0);
	}
	
	if (layout.getVerticalPositionType() == MenuLayout::Position_Undefined)
	{
		layout.setVerticalPositionType(MenuLayout::Position_Center);
		layout.setTop(0);
	}
	
	std::string image = getAttribute(p_node, p_template, "image");
	TT_ASSERTMSG(image.empty() == false,
	             "Element 'imageeditor' (name '%s') is missing required "
	             "'image' attribute.", name.c_str());
	
	std::string widthstr = getAttribute(p_node, p_template, "image_width");
	TT_ASSERTMSG(widthstr.empty() == false,
	             "Element 'imageeditor' (name '%s') is missing required "
	             "'image_width' attribute.", name.c_str());
	s32 image_width = atoi(widthstr.c_str());
	
	std::string heightstr = getAttribute(p_node, p_template, "image_height");
	TT_ASSERTMSG(heightstr.empty() == false,
	             "Element 'imageeditor' (name '%s') is missing required "
	             "'image_height' attribute.", name.c_str());
	s32 image_height = atoi(heightstr.c_str());
	
	std::string zoomstr = getAttribute(p_node, p_template, "zoomlevel");
	s32 zoomlevel = 1;
	if (zoomstr.empty() == false)
	{
		zoomlevel = atoi(zoomstr.c_str());
	}
	
	
	std::string palette_element = getAttribute(p_node, p_template, "palette_element");
	std::string picker_element = getAttribute(p_node, p_template, "picker_element");
	std::string bucket_element = getAttribute(p_node, p_template, "bucket_element");
	std::string pencil_element = getAttribute(p_node, p_template, "pencil_element");
	std::string template_file = getAttribute(p_node, p_template, "template_file");
	
	std::string colstr = getAttribute(p_node, p_template, "template_columns");
	s32 template_columns = 0;
	if (colstr.empty() == false)
	{
		template_columns = atoi(colstr.c_str());
	}
	
	std::string rowstr = getAttribute(p_node, p_template, "template_rows");
	s32 template_rows = 0;
	if (rowstr.empty() == false)
	{
		template_rows = atoi(rowstr.c_str());
	}
	
	TT_ASSERTMSG((rowstr.empty() ^ colstr.empty()) == false,
	             "ImageEditor '%s' should have template_rows and template_columns "
	             "both defined or both not defined.", name.c_str());
	
	ImageEditor* img_editor = new ImageEditor(
		name, layout, image, image_width, image_height, zoomlevel,
		palette_element, pencil_element, bucket_element, picker_element,
		template_file, template_columns, template_rows);
	
	addChildren(*img_editor, p_node, p_template);
	return img_editor;
}


FATImage* MenuFactory::createFATImage(XmlNode* p_node, MenuTemplate* p_template)
{
	TT_ASSERT(p_node->getName() == "fatimage");
	
	std::string name = getAttribute(p_node, p_template, "name");
	
	MenuLayout layout(getLayout(p_node, p_template, name));
	
	if (layout.getOrder() == MenuLayout::Order_Undefined)
	{
		layout.setOrder(MenuLayout::Order_Vertical);
	}
	
	if (layout.getWidthType() == MenuLayout::Size_Undefined)
	{
		layout.setWidthType(MenuLayout::Size_Auto);
		layout.setWidth(0);
	}
	
	if (layout.getHeightType() == MenuLayout::Size_Undefined)
	{
		layout.setHeightType(MenuLayout::Size_Auto);
		layout.setHeight(0);
	}
	
	if (layout.getHorizontalPositionType() == MenuLayout::Position_Undefined)
	{
		layout.setHorizontalPositionType(MenuLayout::Position_Center);
		layout.setLeft(0);
	}
	
	if (layout.getVerticalPositionType() == MenuLayout::Position_Undefined)
	{
		layout.setVerticalPositionType(MenuLayout::Position_Center);
		layout.setTop(0);
	}
	
	FATImage* img = new FATImage(name, layout);
	addChildren(*img, p_node, p_template);
	return img;
}


Label* MenuFactory::createLabel(XmlNode* p_node, MenuTemplate* p_template)
{
	TT_ASSERT(p_node->getName() == "label");
	
	std::string name(getAttribute(p_node, p_template, "name"));
	
	std::string caption(getAttribute(p_node, p_template, "caption"));
	std::string wcaption(getAttribute(p_node, p_template, "wcaption"));
	
	TT_ASSERTMSG(caption.empty() || wcaption.empty(),
	             "Label '%s' has both the 'caption' as the 'wcaption' attribute. "
	             "It can only have one or the other, not both.", name.c_str());
	
	// Get the layout information
	MenuLayout layout(getLayout(p_node, p_template, name));
	layout.setUndefinedToDefault();
	
	// Get the multiline attribute
	std::string multiline_attrib(getAttribute(p_node, p_template, "multiline"));
	bool        multiline = false;
	
	if (multiline_attrib.empty() == false)
	{
		if (multiline_attrib == "true")
		{
			multiline = true;
		}
		else if (multiline_attrib == "false")
		{
			multiline = false;
		}
		else
		{
			TT_PANIC("Label '%s': Invalid value for 'multiline' attribute: %s",
			         name.c_str(), multiline_attrib.c_str());
		}
	}
	
	if (multiline && layout.getWidthType() == MenuLayout::Size_Auto)
	{
		TT_PANIC("Label '%s': Multi-line labels cannot have automatic width.",
		         name.c_str());
	}
	
	// Get the horizontal text alignment attribute
	std::string text_align_attrib(getAttribute(p_node, p_template, "text_align"));
	engine::glyph::GlyphSet::Alignment text_align = engine::glyph::GlyphSet::ALIGN_LEFT;
	if (text_align_attrib.empty() == false)
	{
		text_align = Label::getAlignmentFromString(text_align_attrib);
	}
	
	// Get the vertical text alignment attribute
	std::string text_valign_attrib(getAttribute(p_node, p_template, "text_valign"));
	engine::glyph::GlyphSet::Alignment text_valign = engine::glyph::GlyphSet::ALIGN_CENTER;
	if (text_valign_attrib.empty() == false)
	{
		text_valign = Label::getAlignmentFromString(text_valign_attrib);
	}
	
	// Create the label
	Label* lbl = 0;
	if (caption.empty() == false)
	{
		lbl = new Label(name, layout, caption, multiline);
	}
	else if (wcaption.empty() == false)
	{
		lbl = new Label(name, layout, MenuUtils::hexToWideString(wcaption), multiline);
	}
	else
	{
		lbl = new Label(name, layout, L"", multiline);
	}
	
	// Set the text alignment, if specified
	if (text_align_attrib.empty() == false)
	{
		lbl->setHorizontalAlign(text_align);
	}
	if (text_valign_attrib.empty() == false)
	{
		lbl->setVerticalAlign(text_valign);
	}
	
	// Add Actions
	addChildren(*lbl, p_node, p_template);
	
	return lbl;
}


DynamicLabel* MenuFactory::createDynamicLabel(XmlNode* p_node, MenuTemplate* p_template)
{
	TT_ASSERT(p_node->getName() == "dynamiclabel");
	
	std::string name = getAttribute(p_node, p_template, "name");
	
	MenuLayout layout(getLayout(p_node, p_template, name));
	layout.setUndefinedToDefault();
	
	std::string caption(getAttribute(p_node, p_template, "caption"));
	std::string wcaption(getAttribute(p_node, p_template, "wcaption"));
	
	TT_ASSERTMSG(caption.empty() || wcaption.empty(),
	             "dynamiclabel '%s' as both the 'caption' as the 'wcaption' attribute."
	             " It can only have one or the other, not both.", name.c_str());
	
	// Get the text alignment attribute
	std::string text_align_attrib(getAttribute(p_node, p_template, "text_align"));
	engine::glyph::GlyphSet::Alignment text_align = engine::glyph::GlyphSet::ALIGN_LEFT;
	if (text_align_attrib.empty() == false)
	{
		if (text_align_attrib == "left")
		{
			text_align = engine::glyph::GlyphSet::ALIGN_LEFT;
		}
		else if (text_align_attrib == "center")
		{
			text_align = engine::glyph::GlyphSet::ALIGN_CENTER;
		}
		else if (text_align_attrib == "right")
		{
			text_align = engine::glyph::GlyphSet::ALIGN_RIGHT;
		}
		else
		{
			TT_PANIC("DynamicLabel '%s': Invalid text alignment specified: %s",
			         name.c_str(), text_align_attrib.c_str());
		}
	}
	
	
	// Create a dynamic label
	DynamicLabel* lbl = 0;
	if (caption.empty() == false)
	{
		lbl = new DynamicLabel(name, layout, caption);
	}
	else if (wcaption.empty() == false)
	{
		lbl = new DynamicLabel(name, layout, MenuUtils::hexToWideString(wcaption));
	}
	else
	{
		lbl = new DynamicLabel(name, layout, L"");
	}
	
	// Set the text alignment, if specified
	if (text_align_attrib.empty() == false)
	{
		lbl->setHorizontalAlign(text_align);
	}
	
	// Add Actions
	addChildren(*lbl, p_node, p_template);
	
	return lbl;
}


MultiPageLabel* MenuFactory::createMultiPageLabel(XmlNode* p_node, MenuTemplate* p_template)
{
	TT_ASSERT(p_node->getName() == "multipagelabel");
	
	std::string name(getAttribute(p_node, p_template, "name"));
	
	std::string caption(getAttribute(p_node, p_template, "caption"));
	std::string wcaption(getAttribute(p_node, p_template, "wcaption"));
	
	TT_ASSERTMSG(caption.empty() || wcaption.empty(),
	             "MultiPageLabel '%s' has both the 'caption' as the 'wcaption' attribute. "
	             "It can only have one or the other, not both.", name.c_str());
	
	// Get the layout information
	MenuLayout layout(getLayout(p_node, p_template, name));
	layout.setUndefinedToDefault();
	
	// Get the text alignment attribute
	std::string text_align_attrib(getAttribute(p_node, p_template, "text_align"));
	engine::glyph::GlyphSet::Alignment text_align = engine::glyph::GlyphSet::ALIGN_LEFT;
	if (text_align_attrib.empty() == false)
	{
		if (text_align_attrib == "left")
		{
			text_align = engine::glyph::GlyphSet::ALIGN_LEFT;
		}
		else if (text_align_attrib == "center")
		{
			text_align = engine::glyph::GlyphSet::ALIGN_CENTER;
		}
		else if (text_align_attrib == "right")
		{
			text_align = engine::glyph::GlyphSet::ALIGN_RIGHT;
		}
		else
		{
			TT_PANIC("Element '%s': Invalid text alignment specified: %s",
			            text_align_attrib.c_str());
		}
	}
	
	std::string prev(getAttribute(p_node, p_template, "prev"));
	std::string next(getAttribute(p_node, p_template, "next"));
	
	// Get the prev_has_other attribute
	std::string prev_has_other_attrib(getAttribute(p_node, p_template, "prev_has_other"));
	bool        prev_has_other = false;
	
	if (prev_has_other_attrib.empty() == false)
	{
		if (prev_has_other_attrib == "true")
		{
			prev_has_other = true;
		}
		else if (prev_has_other_attrib == "false")
		{
			prev_has_other = false;
		}
		else
		{
			TT_PANIC("Element '%s': Invalid value for 'prev_has_other' attribute: %s",
			         name.c_str(), prev_has_other_attrib.c_str());
		}
	}
	
	// Get the next_has_other attribute
	std::string next_has_other_attrib(getAttribute(p_node, p_template, "next_has_other"));
	bool        next_has_other = false;
	
	if (next_has_other_attrib.empty() == false)
	{
		if (next_has_other_attrib == "true")
		{
			next_has_other = true;
		}
		else if (next_has_other_attrib == "false")
		{
			next_has_other = false;
		}
		else
		{
			TT_PANIC("Element '%s': Invalid value for 'next_has_other' attribute: %s",
						name.c_str(), next_has_other_attrib.c_str());
		}
	}
	
	std::string button_container(getAttribute(p_node, p_template, "button_container"));
	
	// Create the label
	MultiPageLabel* lbl = 0;
	if (caption.empty() == false)
	{
		lbl = new MultiPageLabel(name, layout, caption, prev, next,
			prev_has_other, next_has_other, button_container);
	}
	else if (wcaption.empty() == false)
	{
		lbl = new MultiPageLabel(name, layout,
			MenuUtils::hexToWideString(wcaption), prev, next, prev_has_other,
			next_has_other, button_container);
	}
	else
	{
		lbl = new MultiPageLabel(name, layout, L"", prev, next, prev_has_other,
			next_has_other, button_container);
	}
	
	// Set the text alignment, if specified
	if (text_align_attrib.empty() == false)
	{
		lbl->setHorizontalAlign(text_align);
	}
	
	// Add Actions
	addChildren(*lbl, p_node, p_template);
	
	return lbl;
}



elements::BusyBar* MenuFactory::createBusyBar(xml::XmlNode* p_node,
                                              MenuTemplate* p_template)
{
	TT_ASSERT(p_node->getName() == "busybar");
	
	std::string name = getAttribute(p_node, p_template, "name");
	
	MenuLayout layout(getLayout(p_node, p_template, name));
	layout.setUndefinedToDefault();
	
	// Create a dynamic label
	BusyBar* bar = new BusyBar(name, layout);
	
	// Add Actions
	addChildren(*bar, p_node, p_template);
	
	return bar;
}


ProgressBar* MenuFactory::createProgressBar(XmlNode*      p_node,
                                            MenuTemplate* p_template)
{
	TT_ASSERT(p_node->getName() == "progressbar");
	
	std::string name(getAttribute(p_node, p_template, "name"));
	
	MenuLayout layout(getLayout(p_node, p_template, name));
	layout.setUndefinedToDefault();
	
	// Create a new bar
	ProgressBar* pb = new ProgressBar(name, layout);
	
	// Add Actions
	addChildren(*pb, p_node, p_template);
	
	// Return the bar that was created
	return pb;
}



//------------------------------------------------------------------------------
// Helper functions

std::string MenuFactory::getAttribute(XmlNode*      p_node,
                                      MenuTemplate* p_template,
                                      const char*   p_name)
{
	TT_NULL_ASSERT(p_node);
	TT_NULL_ASSERT(p_name);
	
	std::string attribute(p_node->getAttribute(p_name));
	if (attribute.empty())
	{
		return "";
	}
	
	std::string ret(attribute);
	if (ret.at(0) == '$')
	{
		TT_ASSERTMSG(p_template != 0,
		             "Template variable used outside of a template.");
		ret = p_template->getVariable(attribute);
		if (ret.empty() == false && ret.at(0) == '#')
		{
			ret.erase(ret.begin());
			return MenuSystem::getInstance()->getSystemVar(ret);
		}
	}
	else if (ret.at(0) == '#')
	{
		ret.erase(ret.begin());
		return MenuSystem::getInstance()->getSystemVar(ret);
	}
	
	return ret;
}


MenuLayout MenuFactory::getLayout(XmlNode*           p_node,
                                  MenuTemplate*      p_template,
                                  const std::string& p_name)
{
	TT_NULL_ASSERT(p_node);
	
	MenuLayout layout;
	
	// Retrieve order
	std::string orderstr = getAttribute(p_node, p_template, "order");
	if (orderstr.empty())
	{
		// Order not specified
		layout.setOrder(MenuLayout::Order_Undefined);
	}
	else
	{
		if (orderstr == "vertical")
		{
			layout.setOrder(MenuLayout::Order_Vertical);
		}
		else if (orderstr == "horizontal")
		{
			layout.setOrder(MenuLayout::Order_Horizontal);
		}
		else
		{
			TT_PANIC("Element '%s' has invalid order value: '%s'",
			         p_name.c_str(), orderstr.c_str());
		}
	}
	
	std::string widthstr(getAttribute(p_node, p_template, "width"));
	if (widthstr.empty())
	{
		// Width not specified
		layout.setWidthType(MenuLayout::Size_Undefined);
		layout.setWidth(0);
	}
	else
	{
		if (widthstr == "max")
		{
			layout.setWidthType(MenuLayout::Size_Max);
			layout.setWidth(0);
		}
		else if (widthstr == "auto")
		{
			layout.setWidthType(MenuLayout::Size_Auto);
			layout.setWidth(0);
		}
		else
		{
			TT_ERR_CREATE("width");
			s32 width = str::parseS32(widthstr, &errStatus);
			if (errStatus.hasError())
			{
				TT_PANIC("'%s' has invalid width value: '%s'",
				         p_name.c_str(), widthstr.c_str());
			}
			
			layout.setWidthType(MenuLayout::Size_Absolute);
			layout.setWidth(width);
		}
	}
	
	std::string heightstr(getAttribute(p_node, p_template, "height"));
	if (heightstr.empty())
	{
		layout.setHeightType(MenuLayout::Size_Undefined);
		layout.setHeight(0);
	}
	else
	{
		if (heightstr == "max")
		{
			layout.setHeightType(MenuLayout::Size_Max);
			layout.setHeight(0);
		}
		else if (heightstr == "auto")
		{
			layout.setHeightType(MenuLayout::Size_Auto);
			layout.setHeight(0);
		}
		else
		{
			TT_ERR_CREATE("height");
			s32 height = str::parseS32(heightstr, &errStatus);
			if (errStatus.hasError())
			{
				TT_PANIC("'%s' has invalid height value: '%s'",
				         p_name.c_str(), heightstr.c_str());
			}
			
			layout.setHeightType(MenuLayout::Size_Absolute);
			layout.setHeight(height);
		}
	}
	
	std::string horizontalstr(getAttribute(p_node, p_template, "horizontal"));
	if (horizontalstr.empty())
	{
		layout.setHorizontalPositionType(MenuLayout::Position_Undefined);
		layout.setLeft(0);
	}
	else
	{
		if (horizontalstr == "left")
		{
			layout.setHorizontalPositionType(MenuLayout::Position_Left);
			layout.setLeft(0);
		}
		else if (horizontalstr == "center")
		{
			layout.setHorizontalPositionType(MenuLayout::Position_Center);
			layout.setLeft(0);
		}
		else if (horizontalstr == "right")
		{
			layout.setHorizontalPositionType(MenuLayout::Position_Right);
			layout.setLeft(0);
		}
		else
		{
			TT_PANIC("Element '%s' has unknown horizontal alignment value: %s",
			         p_name.c_str(), horizontalstr.c_str());
		}
	}
	
	std::string verticalstr(getAttribute(p_node, p_template, "vertical"));
	if (verticalstr.empty())
	{
		layout.setVerticalPositionType(MenuLayout::Position_Undefined);
		layout.setTop(0);
	}
	else
	{
		if (verticalstr == "top")
		{
			layout.setVerticalPositionType(MenuLayout::Position_Top);
			layout.setTop(0);
		}
		else if (verticalstr == "center")
		{
			layout.setVerticalPositionType(MenuLayout::Position_Center);
			layout.setTop(0);
		}
		else if (verticalstr == "bottom")
		{
			layout.setVerticalPositionType(MenuLayout::Position_Bottom);
			layout.setTop(0);
		}
		else
		{
			TT_PANIC("Element '%s' has unknown vertical alignment value: %s",
			         p_name.c_str(), verticalstr.c_str());
		}
	}
	
	(void)p_name;
	
	return layout;
}


//------------------------------------------------------------------------------
// Functions that need to be overridden for new elements

MenuElementInterface* MenuFactory::createCustomElement(
		int, XmlNode*, MenuTemplate*)
{
	// No unknown elements can be created by default
	return 0;
}


// Finds the correct enum to match the string
int MenuFactory::getElementType(const char* p_typename) const
{
	if (strcmp(p_typename, "menu") == 0)
	{
		return Type_Menu;
	}
	else if (strcmp(p_typename, "window") == 0)
	{
		return Type_Window;
	}
	else if (strcmp(p_typename, "container") == 0)
	{
		return Type_Container;
	}
	else if (strcmp(p_typename, "line") == 0)
	{
		return Type_Line;
	}
	else if (strcmp(p_typename, "label") == 0)
	{
		return Type_Label;
	}
	else if (strcmp(p_typename, "dynamiclabel") == 0)
	{
		return Type_DynamicLabel;
	}
	else if (strcmp(p_typename, "multipagelabel") == 0)
	{
		return Type_MultiPageLabel;
	}
	else if (strcmp(p_typename, "image") == 0)
	{
		return Type_Image;
	}
	else if (strcmp(p_typename, "imageeditor") == 0)
	{
		return Type_ImageEditor;
	}
	else if (strcmp(p_typename, "fatimage") == 0)
	{
		return Type_FATImage;
	}
	else if (strcmp(p_typename, "button") == 0)
	{
		return Type_Button;
	}
	else if (strcmp(p_typename, "keyboard") == 0)
	{
		return Type_SoftwareKeyboard;
	}
	else if (strcmp(p_typename, "table") == 0)
	{
		return Type_Table;
	}
	else if (strcmp(p_typename, "coloritem") == 0)
	{
		return Type_ColorItem;
	}
	else if (strcmp(p_typename, "children") == 0)
	{
		return Type_Children;
	}
	else if (strcmp(p_typename, "action") == 0)
	{
		return Type_Action;
	}
	else if (strcmp(p_typename, "actionset") == 0)
	{
		return Type_ActionSet;
	}
	else if (strcmp(p_typename, "show_one_child") == 0)
	{
		return Type_ShowOneChild;
	}
	else if (strcmp(p_typename, "show_one_value_child") == 0)
	{
		return Type_ShowOneValueChild;
	}
	else if (strcmp(p_typename, "show_one_value_text") == 0)
	{
		return Type_ShowOneValueText;
	}
	else if (strcmp(p_typename, "scrollablelist") == 0)
	{
		return Type_ScrollableList;
	}
	else if (strcmp(p_typename, "filelist") == 0)
	{
		return Type_FileList;
	}
	else if (strcmp(p_typename, "fileblocksbar") == 0)
	{
		return Type_FileBlocksBar;
	}
	else if (strcmp(p_typename, "scrollbar") == 0)
	{
		return Type_Scrollbar;
	}
	else if (strcmp(p_typename, "busybar") == 0)
	{
		return Type_BusyBar;
	}
	else if (strcmp(p_typename, "progressbar") == 0)
	{
		return Type_ProgressBar;
	}
	else
	{
		return Type_Unknown;
	}
}


//------------------------------------------------------------------------------
// Private member functions

void MenuFactory::doAddChildren(Menu*         p_parent,
                                XmlNode*      p_parentNode,
                                MenuTemplate* p_template,
                                menu_element_tag)
{
	// Parse all children
	for (XmlNode* child = p_parentNode->getChild();
	     child != 0; child = child->getSibling())
	{
		if (child->getName() == "var")
		{
			// Get the name of the var (required)
			const std::string var_name = child->getAttribute("name");
			TT_ASSERTMSG(var_name.empty() == false,
			             "Missing required 'name' attribute for var element.");
			
			// Get the init value of the var (required)
			const std::string var_value = child->getAttribute("value");
			TT_ASSERTMSG(var_value.empty() == false,
			             "Missing required 'value' attribute for var element '%s'.",
			             var_name.c_str());
			
			// Add the var
			p_parent->addVar(var_name, var_value);
		}
		else
		{
			// Add all children
			addChild(p_parent, child, p_template);
		}
	}
}


void MenuFactory::doAddChildren(elements::MenuElementInterface* p_parent,
                                XmlNode*                        p_parentNode,
                                MenuTemplate*                   p_template,
                                elements::action_element_tag)
{
	TT_NULL_ASSERT(p_parent);
	TT_NULL_ASSERT(p_parentNode);
	
	for (XmlNode* child = p_parentNode->getChild();
	     child != 0; child = child->getSibling())
	{
		if (child->getName() == "action")
		{
			addAction(p_parent, child, p_template);
		}
		else if (child->getName() == "children")
		{
			TT_NULL_ASSERT(p_template);
			
			for (XmlNode* tplChild = p_template->getChild();
			     tplChild != 0; tplChild = tplChild->getSibling())
			{
				if (tplChild->getName() == "action")
				{
					addAction(p_parent, tplChild, 0);
				}
			}
		}
	}
}


void MenuFactory::addAction(elements::MenuElementInterface* p_parent,
                            XmlNode*                        p_child_node,
                            MenuTemplate*                   p_template)
{
	TT_ASSERTMSG(p_child_node->getName() == "action",
	             "Trying to add an action with an non-action XmlNode.");
	
	// Get the required 'command' attribute
	std::string command = getAttribute(p_child_node, p_template, "command");
	TT_ASSERTMSG(command.empty() == false,
	             "Missing required 'command' attribute for action.");
	
	// Create a menu action for this node
	MenuAction action(command);
	
	
	// FIXME: DOUBLE PARAMETERS NOT SUPPORTED ANYMORE, FIX CODE BELOW...
	
	// Get all parameters of the action
	/*
	for (int a = 0; a < p_child_node.nAttribute(); ++a)
	{
		XMLAttribute attrib(p_child_node.getAttribute(a));
		if (strcmp(attrib.lpszName, "param") == 0)
		{
			// Get the (possibly templated) value for the parameter
			const char* param = attrib.lpszValue;
			
			if (attrib.lpszValue[0] == '$')
			{
				TT_NULL_ASSERT(p_template);
				param = p_template->getVariable(attrib.lpszValue);
			}
			
			// Add the parameter to the action
			action.addParameter(param);
		}
		else
		{
			// Only 'param' and 'command' attributes are allowed
			if (strcmp(attrib.lpszName, "command") != 0)
			{
				TT_PANIC("Invalid action attribute: %s",
				         attrib.lpszName);
			}
		}
	}
	//*/
	
	// return the action
	p_parent->addAction(action);
}

// Namespace end
}
}
