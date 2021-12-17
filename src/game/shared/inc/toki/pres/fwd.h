#if !defined(INC_TOKI_PRES_FWD_H)
#define INC_TOKI_PRES_FWD_H

#include <tt/code/Handle.h>

namespace toki {
namespace pres {

class PresentationObjectMgr;
typedef tt_ptr<PresentationObjectMgr>::shared PresentationObjectMgrPtr;

class PresentationStartSettings;

class PresentationObject;
typedef tt::code::Handle<PresentationObject> PresentationObjectHandle;


enum StartType
{
	StartType_Normal,
	StartType_Script,
	StartType_TransitionMove
};


// Namespace end
}
}


#endif  // !defined(INC_TOKI_PRES_FWD_H)
