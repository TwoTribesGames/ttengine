#if !defined(INC_TT_PRES_TRIGGERFACTORYINTERFACE_H)
#define INC_TT_PRES_TRIGGERFACTORYINTERFACE_H
#include <string>

#include <tt/math/Range.h>

#include <tt/pres/fwd.h>
#include <tt/pres/PresentationValue.h>
#include <tt/str/str_types.h>

namespace tt {
namespace pres {

struct TriggerInfo;

class TriggerFactoryInterface
{
public:
	TriggerFactoryInterface() { }
	virtual ~TriggerFactoryInterface() { }
	
	/*! \brief Creates an Object that implements TriggerInterface
	    \param p_creationInfo TriggerInfo struct with creation info 
	    \param p_object Presentation object where the trigger should be applied 
	    \return an object that inplements TriggerInterface */ 
	virtual TriggerInterfacePtr createTrigger(const TriggerInfo&           p_creationInfo,
	                                          const PresentationObjectPtr& p_object) = 0;
};


//namespace end
}
}

#endif // !defined(INC_TT_PRES_TRIGGERFACTORYINTERFACE_H)
