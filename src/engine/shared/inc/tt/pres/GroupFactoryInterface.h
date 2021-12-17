#if !defined(INC_TT_PRES_GROUPFACTORYINTERFACE_H)
#define INC_TT_PRES_GROUPFACTORYINTERFACE_H
#include <functional>

#include <tt/math/Range.h>
#include <tt/pres/fwd.h>


namespace tt {
namespace pres {


class GroupFactoryInterface
{
public:
	virtual ~GroupFactoryInterface(){}
	
	virtual GroupInterfacePtr create(PresentationMgr* p_mgr, const math::Range& p_zRange) = 0;
};


//namespace end
}
}

#endif // !defined(INC_TT_PRES_GROUPFACTORYINTERFACE_H)

