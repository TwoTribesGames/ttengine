#if !defined(INC_TOKI_PRES_TRIGGERFACTORY_H)
#define INC_TOKI_PRES_TRIGGERFACTORY_H


#include <tt/pres/TriggerFactoryInterface.h>


namespace toki {
namespace pres {

class TriggerFactory : public tt::pres::TriggerFactoryInterface
{
public:
	TriggerFactory()          { }
	virtual ~TriggerFactory() { }
	
	virtual tt::pres::TriggerInterfacePtr createTrigger(
	      const tt::pres::TriggerInfo&           p_creationInfo,
	      const tt::pres::PresentationObjectPtr& p_object);
	
private:
	// No copying
	TriggerFactory(const TriggerFactory&);
	TriggerFactory& operator=(const TriggerFactory&);
};

// Namespace end
}
}


#endif  // !defined(INC_TOKI_PRES_TRIGGERFACTORY_H)
