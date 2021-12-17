#include <algorithm>
#include <tt/platform/tt_error.h>
#include <tt/xml/XmlNode.h>


namespace tt {
namespace xml {

#if defined(TT_PLATFORM_WIN)
#pragma warning (disable:4592)
#endif

const std::string        g_emptyString;
const XmlNode::Attribute g_emptyAttribute;


//--------------------------------------------------------------------------------------------------
// Public member functions

XmlNode::XmlNode(const std::string& p_name)
:
m_firstChild(0),
m_nextSibling(0),
m_name(p_name),
m_data(),
m_childCount(0),
m_attributes(),
m_attributeList()
{
}


XmlNode::~XmlNode()
{
	// Recursively clean up tree structure
	// Delete all children
	XmlNode* child(m_firstChild);
	while (child != 0)
	{
		XmlNode* sibling = child->getSibling();
		delete child;
		child = sibling;
	}
}


XmlNode* XmlNode::createTree(IXmlReader& p_xml, bool p_isSubTree)
{
	// Create pointer
	XmlNode* root = 0;
	
	// Parse xml file
	u32 depth = 0;
	u32 oldDepth = 0;
	
	// Store current node for each depth
	std::vector<XmlNode*> current;

	// Keep parsing while this flag is true
	bool keepParsing = true;
	
	// Loop until entire file has been read
	while(keepParsing && p_xml.read())
	{
		switch(p_xml.getNodeType())
		{
			case EnumNode_Element:
			{
				// Create new node
				XmlNode* temp = new XmlNode(p_xml.getNodeName());
				
				if(temp == 0)
				{
					TT_PANIC("File '%s': Failed to create node [%s]. Out of memory?",
					         p_xml.getSourceName().c_str(), p_xml.getNodeName().c_str());
					
					delete root;
					return 0;
				}
				
				// Store all attributes for this node
				for (u32 i = 0; i < p_xml.getAttributeCount(); ++i)
				{
					const std::string& attribName(p_xml.getAttributeName(i));
					if (temp->hasAttribute(attribName))
					{
						TT_PANIC("Invalid XML: element '%s' specifies attribute '%s' more than once.\nFile: '%s'",
						         temp->getName().c_str(), attribName.c_str(), p_xml.getSourceName().c_str());
						delete temp;
						delete root;
						return 0;
					}
					
					temp->setAttribute(attribName,
					                   p_xml.getAttributeValue(i));
				}
				
				// If this is the root...
				if(depth == 0)
				{
					if(root == 0)
					{
						// Point root to this node
						root = temp;
						
						// Store pointer to root
						current.push_back(temp);
					}
					else
					{
						// If parsing a subtree,seeing a rootnode sibling
						// will stop parsing. 
						if ( p_isSubTree )
						{
							keepParsing = false;
						}
						else
						{
							// Sibling of root (invalid XML)
							TT_PANIC("Invalid XML: file has more than one root element. Found both '%s' and '%s'.\nFile: '%s'",
							         root->getName().c_str(), temp->getName().c_str(), p_xml.getSourceName().c_str());
							delete temp;
							delete root;
							return 0;
						}
					}
				}
				// We have travelled one level deeper, insert as child
				else if(depth > oldDepth)
				{
					// Link as child of previous level
					current[depth-1]->setChild(temp);
					current[depth-1]->setChildCount(1);
					
					// Add or update current pointer for this level
					if(current.size()-1 < depth)
					{
						current.push_back(temp);
					}
					else
					{
						current[depth] = temp;
					}
					
				}
				// Level stayed the same or is higher, insert as sibling
				else
				{
					// Link as sibling of current level
					current[depth]->setSibling(temp);
					current[depth-1]->setChildCount(
						current[depth-1]->getChildCount() + 1 );
					
					// Update current pointer
					current[depth] = temp;
				}
				
				// Store depth of this node
				oldDepth = depth;
				
				// Increase depth if element isn't closed
				if(p_xml.isEmptyElement() == false)
				{
					++depth;
				}
				break;
			}
			case EnumNode_ElementEnd:
			{
				if (depth == 0)
				{
					TT_PANIC("Invalid XML: encountered close tag ('</%s>'), but all elements were already closed.\nFile: '%s'",
					         p_xml.getNodeName().c_str(), p_xml.getSourceName().c_str());
					delete root;
					return 0;
				}
				
				// Decrease depth
				--depth;
				
				// Validate name of element that was closed; must be same as open tag!
				if (current[depth]->getName() != p_xml.getNodeName())
				{
					TT_PANIC("Invalid XML: encountered '</%s>', expected '</%s>'.\nFile: '%s'",
					         p_xml.getNodeName().c_str(), current[depth]->getName().c_str(), p_xml.getSourceName().c_str());
					delete root;
					return 0;
				}
				break;
			}
			case EnumNode_Text:
			{
				if(current.size() > oldDepth && current[oldDepth] != 0)
				{
					// Store text data
					current[oldDepth]->setData(p_xml.getNodeData());
				}
				break;
			}
			default:
			{
				break;
			}
		}
	}
	
	// Return pointer to root of hierarchy
	return root;
}


XmlNode* XmlNode::createSubTree(IXmlReader& p_reader)
{
	return XmlNode::createTree(p_reader, true);
}


const XmlNode* XmlNode::getChild(s32 p_childIndex) const
{
	if (p_childIndex < 0) return 0;
	
	const XmlNode* retVal = getChild();
	while (p_childIndex > 0)
	{
		retVal = retVal->getSibling();
		if (retVal == 0) break;
		--p_childIndex;
	}
	
	return retVal;
}


XmlNode* XmlNode::getFirstChild(const std::string& p_name)
{
	XmlNode* child = m_firstChild;
	
	while (child != 0)
	{
		if (child->getName() == p_name)
		{
			return child;
		}
		child = child->getSibling();
	}
	return 0;
}


const XmlNode* XmlNode::getFirstChild(const std::string& p_name) const
{
	const XmlNode* child = m_firstChild;
	
	while (child != 0)
	{
		if (child->getName() == p_name)
		{
			return child;
		}
		child = child->getSibling();
	}
	return 0;
}


u32 XmlNode::getChildCount(const std::string& p_name) const
{
	u32 childCount = 0;
	
	for (const XmlNode* child = m_firstChild; child != 0; child = child->getSibling())
	{
		if (child->getName() == p_name)
		{
			++childCount;
		}
	}
	
	return childCount;
}


void XmlNode::addChild(XmlNode* p_child)
{
	if (m_firstChild == 0)
	{
		m_firstChild = p_child;
		return;
	}
	
	XmlNode* child = m_firstChild->getSibling();
	if (child == 0)
	{
		m_firstChild->setSibling(p_child);
		return;
	}
	
	while (child->getSibling() != 0)
	{
		child = child->getSibling();
	}
	
	child->setSibling(p_child);
}


void XmlNode::setAttribute(const std::string& p_name, const std::string& p_value)
{
	if (m_attributes.find(p_name) == m_attributes.end())
	{
		m_attributeList.push_back(p_name);
	}
	m_attributes[p_name] = p_value;
}


const std::string& XmlNode::getAttribute(const std::string& p_name) const
{
	AttributeMap::const_iterator it = m_attributes.find(p_name);
	if (it != m_attributes.end())
	{
		return it->second;
	}
	return g_emptyString;
}


bool XmlNode::hasAttribute(const std::string& p_name) const
{
	return m_attributes.find(p_name) != m_attributes.end();
}


void XmlNode::removeAttribute(const std::string& p_name)
{
	// Remove attribute from both containers (the key/value mapping and the name order)
	AttributeMap::iterator mapIt = m_attributes.find(p_name);
	if (mapIt != m_attributes.end())
	{
		m_attributes.erase(mapIt);
	}
	
	AttributeList::iterator listIt = std::find(
		m_attributeList.begin(), m_attributeList.end(), p_name);
	if (listIt != m_attributeList.end())
	{
		m_attributeList.erase(listIt);
	}
}


const XmlNode::Attribute& XmlNode::getAttribute(u32 p_index) const
{
	AttributeList::size_type listIndex = static_cast<AttributeList::size_type>(p_index);
	if (listIndex >= m_attributeList.size())
	{
		return g_emptyAttribute;
	}
	
	const std::string& attr = m_attributeList[listIndex];
	
	AttributeMap::const_iterator it = m_attributes.find(attr);
	if (it == m_attributes.end())
	{
		return g_emptyAttribute;
	}
	
	return *it;
}

// Namespace end
}
}
