#if !defined(INC_TT_XML_XMLNODE_H)
#define INC_TT_XML_XMLNODE_H


#include <map>
#include <string>
#include <vector>

#include <tt/platform/tt_types.h>
#include <tt/xml/XmlReader.h>


namespace tt {
namespace xml {

/*! \brief XML Node class. */
class XmlNode
{
public:
	typedef std::map<std::string, std::string>  AttributeMap;
	typedef std::vector<std::string>            AttributeList;
	typedef AttributeMap::value_type            Attribute;
	
	
	explicit XmlNode(const std::string& p_name = std::string());
	~XmlNode();
	
	/*! \brief Builds a tree structure and returns a pointer to the root. */
	static XmlNode* createTree(IXmlReader& p_reader, bool p_isSubTree = false);
	static XmlNode* createSubTree(IXmlReader& p_reader);
	
	// Access functions:
	inline void setChild(XmlNode* p_child) { m_firstChild = p_child; }
	inline XmlNode*       getChild()       { return m_firstChild;    }
	inline const XmlNode* getChild() const { return m_firstChild;    }
	const XmlNode* getChild(s32 p_childIndex) const;
	
	// Get first child that matches that name
	XmlNode* getFirstChild(const std::string& p_name);
	const XmlNode* getFirstChild(const std::string& p_name) const;
	
	/*! \brief Returns number of direct children of this node with the specified name.
	    \param p_name Name to search for. */
	u32 getChildCount(const std::string& p_name) const;
	
	/* \brief Addes XmlNode as child or sibling of child depending on current 
	          children. This function does not overwrite child or child's 
	          sibling but finds the first empty spot.
	   \param p_child XmlNode which is to be added as child. */
	void addChild(XmlNode* p_child);
	
	inline void setSibling(XmlNode* p_sibling) { m_nextSibling = p_sibling; }
	inline XmlNode*       getSibling()         { return m_nextSibling;      }
	inline const XmlNode* getSibling() const   { return m_nextSibling;      }
	
	inline void               setName(const std::string& p_name) { m_name = p_name; }
	inline const std::string& getName() const                    { return m_name;   }
	
	void setAttribute(const std::string& p_name, const std::string& p_value);
	const std::string& getAttribute(const std::string& p_name) const;
	bool hasAttribute(const std::string& p_name) const;
	
	void removeAttribute(const std::string& p_name);
	
	inline void               setData(const std::string& p_data) { m_data = p_data; }
	inline const std::string& getData() const                    { return m_data;   }
	
	inline void setChildCount(u32 p_count) { m_childCount = p_count; }
	inline u32  getChildCount() const      { return m_childCount;    }
	
	inline u32  getAttributeCount() const { return static_cast<u32>(m_attributes.size()); }
	const Attribute& getAttribute(u32 p_index) const;
	inline const AttributeMap& getAttributes() const { return m_attributes; }
	
private:
	// No copying
	XmlNode(const XmlNode&);
	XmlNode& operator=(const XmlNode&);
	
	
	XmlNode*    m_firstChild;
	XmlNode*    m_nextSibling;
	
	std::string m_name;
	std::string m_data;
	u32         m_childCount;
	
	AttributeMap  m_attributes;
	AttributeList m_attributeList;
};

// Namespace end
}
}


#endif  // !defined(INC_TT_XML_XMLNODE_H)
