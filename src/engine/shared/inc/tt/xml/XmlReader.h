#if !defined(_XMLREADER_H_)
#define _XMLREADER_H_


#include <tt/platform/tt_types.h>
#include <tt/xml/fast_atof.h>
#include <vector>
#include <string>
#include <string.h>


namespace tt {
namespace xml {

/*! \brief Different types of XML nodes. */
enum EnumNode
{
	//! No xml node. This is usually the node if you did not read anything yet.
	EnumNode_None,
	
	//! A xml element, like <foo>
	EnumNode_Element,
	
	//! End of an xml element, like </foo>
	EnumNode_ElementEnd,
	
	//! Text within a xml element: <foo> this is the text. </foo>
	EnumNode_Text,
	
	//! An xml comment like &lt;!-- I am a comment --&gt; or a DTD definition.
	EnumNode_Comment,
	
	//! An xml cdata section like &lt;![CDATA[ this is some CDATA ]]&gt;
	EnumNode_CData,
	
	//! Unknown element.
	EnumNode_Unknown
};

/*! \brief Structure to hold attribute properties. */
struct Attribute
{
	std::string name;
	std::string value;
	
	inline Attribute()
	:
	name(),
	value()
	{ }
};


//! interface of the XMLReader
class IXmlReader
{
public:
	IXmlReader();
	virtual ~IXmlReader();
	
	//! Reads forward to the next xml node.
	//! \return Returns false, if there was no further node.
	virtual bool read() = 0;
	
	/*! \return The name of the XML source (e.g. filename). */
	virtual std::string getSourceName() const = 0;
	
	//! Returns the type of the current XML node.
	inline EnumNode getNodeType() const { return m_current_node_type; }
	
	//! Returns attribute count of the current XML node.
	inline u32 getAttributeCount() const { return static_cast<u32>(m_current_attributes.size()); }
	
	//! Returns name of an attribute.
	const std::string& getAttributeName(u32 p_index) const;
	
	//! Returns the value of an attribute.
	const std::string& getAttributeValue(u32 p_index) const;
	
	//! Returns the value of an attribute.
	const std::string& getAttributeValue(const std::string& p_name) const;
	
	//! Returns the value of an attribute as integer.
	s32 getAttributeValueAsInt(const std::string& p_name) const;
	
	//! Returns the value of an attribute as integer.
	s32 getAttributeValueAsInt(u32 p_index) const;
	
	//! Returns the value of an attribute as float.
	float getAttributeValueAsFloat(const std::string& p_name) const;
	
	//! Returns the value of an attribute as float.
	float getAttributeValueAsFloat(u32 p_index) const;
	
	//! Returns the name of the current node.
	inline const std::string& getNodeName() const { return m_current_node_name; }
	
	//! Returns data of the current node.
	inline const std::string& getNodeData() const { return m_current_node_name; }
	
	//! Returns if an element is an empty element, like <foo />
	inline bool isEmptyElement() const { return m_current_node_is_empty; }
	
	// Provides an interface to derived classes to access the data
protected:
	inline void setNodeName(const std::string& p_name) { m_current_node_name     = p_name;       }
	inline void setCurrentNodeType(EnumNode p_type)    { m_current_node_type     = p_type;       }
	inline void setIsEmpty(bool p_empty)               { m_current_node_is_empty = p_empty;      }
	inline void addAttribute(const Attribute& p_attr)  { m_current_attributes.push_back(p_attr); }
	inline void clearAttributes()                      { m_current_attributes.clear();           }
	
	// replaces xml special characters in a string
	void replaceSpecialCharacters(std::string& p_src);
	
	//! returns true if a character is whitespace
	inline static bool isWhiteSpace(char p_c)
	{
		return (p_c == ' ' || p_c == '\t' || p_c == '\n' || p_c == '\r');
	}
	
private:
	// finds a current attribute by name, returns 0 if not found
	const Attribute* getAttributeByName(const std::string& p_name) const;
	
	
	// Currently parsed node properties
	EnumNode               m_current_node_type;     // type of the node
	std::string            m_current_node_name;     // name of the node
	std::vector<Attribute> m_current_attributes;    // attributes of node
	bool                   m_current_node_is_empty; // is the node empty?
};

// Namespace end
}
}


#endif  // !defined(_XMLREADER_H_)
