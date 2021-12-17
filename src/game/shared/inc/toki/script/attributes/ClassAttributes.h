#if !defined(INC_TOKITORI_SCRIPT_ATTRIBUTES_CLASSATTRIBUTES_H)
#define INC_TOKITORI_SCRIPT_ATTRIBUTES_CLASSATTRIBUTES_H

#include <toki/script/attributes/AttributeCollection.h>

namespace toki {
namespace script {
namespace attributes {

class ClassAttributes
{
public:
	ClassAttributes()
	{}
	
	AttributeCollection getRootAttributes() const;
	AttributeCollection getMemberAttributes(const std::string& p_memberName) const;
	
	inline void clear() { m_memberAttributes.clear(); }
	inline MemberAttributesCollection& getMemberAttributesCollection() 
	{
		return m_memberAttributes;
	}
	
	inline const MemberAttributesCollection& getMemberAttributesCollection() const
	{
		return m_memberAttributes;
	}
	
private:
	MemberAttributesCollection m_memberAttributes;
};


// Namespace end
}
}
}

#endif // !defined(INC_TOKITORI_SCRIPT_ATTRIBUTES_CLASSATTRIBUTES_H)
