#if !defined(INC_TT_PRES_CALLBACKTRIGGERINTERFACE_H)
#define INC_TT_PRES_CALLBACKTRIGGERINTERFACE_H


#include <tt/platform/tt_types.h>

#include <tt/pres/fwd.h>


namespace tt {
namespace pres {



class CallbackTriggerInterface
{
public:
	virtual void callback(const std::string& p_data, const tt::pres::PresentationObjectPtr& p_object) = 0;
	
protected:
	CallbackTriggerInterface(){}
	virtual ~CallbackTriggerInterface(){}
};


//namespace end
}
}

#endif // !defined(INC_TT_PRES_CALLBACKTRIGGERINTERFACE_H)
