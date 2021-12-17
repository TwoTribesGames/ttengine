#include <toki/script/attributes/ClassAttributes.h>

namespace toki {
namespace script {
namespace attributes {


//--------------------------------------------------------------------------------------------------
// Public member functions

AttributeCollection ClassAttributes::getRootAttributes() const
{
	return getMemberAttributes(std::string());
}


AttributeCollection ClassAttributes::getMemberAttributes(const std::string& p_memberName) const
{
	for (MemberAttributesCollection::const_iterator it = m_memberAttributes.begin();
	     it != m_memberAttributes.end(); ++it)
	{
		if ((*it).first == p_memberName)
		{
			return (*it).second;
		}
	}
	
	return AttributeCollection();
}


// Namespace end
}
}
}
