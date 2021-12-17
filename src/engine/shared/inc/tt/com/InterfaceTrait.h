#if !defined(INC_TT_COM_INTERFACETRAIT_H)
#define INC_TT_COM_INTERFACETRAIT_H


#include <tt/platform/tt_types.h>
#include <tt/com/types.h>


namespace tt {
namespace com {

template<class InterfaceType>
class InterfaceTrait
{
public:
	typedef InterfaceType ValueType;
	typedef typename tt_ptr<ValueType>::shared Ptr;
	
	InterfaceID id;
	
	
	// Provide a default constructor, so that static const objects of this class can be instantiated
	InterfaceTrait() { }
};

// Namespace end
}
}


#endif  // !defined(INC_TT_COM_INTERFACETRAIT_H)
