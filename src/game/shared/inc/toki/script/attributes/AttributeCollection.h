#if !defined(INC_TOKITORI_SCRIPT_ATTRIBUTES_ATTRIBUTECOLLECTION_H)
#define INC_TOKITORI_SCRIPT_ATTRIBUTES_ATTRIBUTECOLLECTION_H

#include <map>

#include <toki/script/attributes/Attribute.h>


namespace toki {
namespace script {
namespace attributes {


class AttributeCollection
{
public:
	typedef std::map<std::string, Attribute> AttributeMap;
	
	AttributeCollection() {}
	
	inline void insert(const Attribute& p_attribute) 
	{
		m_attributes.insert(std::make_pair(p_attribute.getName(), p_attribute));
	}
	
	inline bool contains(const std::string& p_attributeName) const 
	{ 
		return m_attributes.find(p_attributeName) != m_attributes.end();
	}
	
	inline Attribute get(const std::string& p_attributeName) const
	{
		AttributeMap::const_iterator it = m_attributes.find(p_attributeName);
		if (it != m_attributes.end())
		{
			return (*it).second;
		}
		
		TT_PANIC("Trying to get a non-existing attribute '%s'", p_attributeName.c_str());
		
		return Attribute();
	}
	
	inline const AttributeMap& getAll() const { return m_attributes; }
	
private:
	AttributeMap m_attributes;
};


typedef std::vector<std::pair<std::string, AttributeCollection> > MemberAttributesCollection;


// Namespace end
}
}
}

#endif // !defined(INC_TOKITORI_SCRIPT_ATTRIBUTES_ATTRIBUTECOLLECTION_H)
