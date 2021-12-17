#include <string>

#include <tt/platform/tt_error.h>
#include <tt/xml/XmlFileReader.h>
#include <tt/menu/MenuTemplate.h>
#include <tt/menu/MenuDebug.h>


namespace tt {
namespace menu {

//------------------------------------------------------------------------------
// Public member functions

MenuTemplate::~MenuTemplate()
{
	// This is managed externally, just set to 0
	m_data = 0;
	
	// This one is managed by MenuTemplate, free memory
	delete m_template;
	m_template = 0;
}


MenuTemplate* MenuTemplate::getTemplate(xml::XmlNode* p_node)
{
	// Build a filename from the node name
	std::string templateFilename("menu/templates/");
	templateFilename += p_node->getName();
	templateFilename += ".xml";
	
	MENU_Printf("MenuTemplate::getTemplate: Attempting to open template file: %s\n",
	            templateFilename.c_str());
	
	// Prepare a pointer to the root node
	xml::XmlNode* xmlRoot = 0;
	{
		// Load the xml file
		xml::XmlFileReader xml;
		
		if (xml.loadFile(templateFilename) == false)
		{
			TT_PANIC("Failed to load XML file '%s'.", templateFilename.c_str());
			return 0;
		}
		
		// Create a hierarchy
		xmlRoot = xml::XmlNode::createTree(xml);
		TT_ASSERT(xmlRoot != 0);
	} // Release memory of XmlFileReader
	
	// Make sure a valid menu XML file is loaded
	TT_ASSERT(xmlRoot->getName() == "template"); // Root node should be "template"
	TT_ASSERT(xmlRoot->getSibling() == 0);       // No siblings of root allowed
	
	// Template name must match the node name
	const std::string templateName(xmlRoot->getAttribute("name"));
	TT_ASSERTMSG(templateName.empty() == false,
	             "Missing required 'name' attribute.");
	
	if (p_node->getName() != templateName)
	{
		TT_PANIC("Template name '%s' must match filename '%s'.",
		         templateName.c_str(), p_node->getName().c_str());
		return 0;
	}
	
	// Create a new menu template based on the root node
	return new MenuTemplate(p_node, xmlRoot);
}


const std::string& MenuTemplate::getVariable(std::string p_name)
{
	TT_ASSERTMSG(p_name.empty() == false && p_name.at(0) == '$',
	             "Specified name '%s' is not a valid variable name.",
	             p_name.c_str());
	p_name.erase(p_name.begin());
	return m_data->getAttribute(p_name);
}


//------------------------------------------------------------------------------
// Private member functions

MenuTemplate::MenuTemplate(xml::XmlNode* p_data, xml::XmlNode* p_template)
:
m_template(p_template),
m_data(p_data)
{
	// Template can have at most one <children> node
	int childNodeCount = getChildrenTagCount(p_template);
	TT_ASSERTMSG(childNodeCount <= 1,
	             "At most one <children /> node is allowed in a template.");
	
	m_hasChildrenNode = (childNodeCount >= 1);
}


int MenuTemplate::getChildrenTagCount(xml::XmlNode* p_node) const
{
	int           childNodeCount = 0;
	xml::XmlNode* childrenNode   = 0;
	
	for (xml::XmlNode* child = p_node->getChild();
	     child != 0; child = child->getSibling())
	{
		if (child->getName() == "children")
		{
			++childNodeCount;
			childrenNode = child;
		}
	}
	
	// Only allow for 1 children tag
	TT_ASSERTMSG(childNodeCount <= 1,
	             "At most one <children /> node is allowed in a template.");
	
	if (childNodeCount > 0)
	{
		// Child node should not have any children or attributes
		TT_ASSERTMSG(childrenNode->getChildCount() == 0,
		             "The <children /> node may not have any children.");
		TT_ASSERTMSG(childrenNode->getAttributeCount() == 0,
		             "The <children /> node may not have any attributes.");
	}
	
	// Check for other child nodes to contain "children" tags
	for (xml::XmlNode* child = p_node->getChild();
	     child != 0; child = child->getSibling())
	{
		if (child->getName() != "children")
		{
			childNodeCount += getChildrenTagCount(child);
		}
	}
	
	return childNodeCount;
}

// Namespace end
}
}
