#if !defined(INC_TT_PRES_GROUPFACTORY_H)
#define INC_TT_PRES_GROUPFACTORY_H
#include <functional>

#include <tt/math/Range.h>
#include <tt/pres/fwd.h>
#include <tt/pres/GroupFactoryInterface.h>
#include <tt/pres/PresentationGroup.h>


namespace tt {
namespace pres {


template<typename Pred>
class GroupFactory : public GroupFactoryInterface
{
public:
	GroupFactory(){}
	virtual ~GroupFactory(){}

	virtual GroupInterfacePtr create(PresentationMgr* p_mgr, const math::Range& p_zRange)
	{
		return PresentationGroup<Pred>::create(p_zRange, Pred(), p_mgr);
	}

private:
	
	
	
	//Enable copy / disable assignment
	GroupFactory(const GroupFactory& p_rhs);
	GroupFactory& operator=(const GroupFactory&);
};

//namespace end
}
}

#endif // !defined(INC_TT_PRES_GROUPFACTORY_H)

