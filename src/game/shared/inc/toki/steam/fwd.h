#if !defined(INC_TOKI_STEAM_FWD_H)
#define INC_TOKI_STEAM_FWD_H


#if defined(TT_STEAM_BUILD)  // Steam Workshop functionality is only available in Steam builds


#include <vector>

#include <steam/isteamremotestorage.h>

#include <tt/platform/tt_types.h>


namespace toki {
namespace steam {

class Workshop;
class WorkshopObserver;

typedef tt_ptr<WorkshopObserver>::shared WorkshopObserverPtr;
typedef tt_ptr<WorkshopObserver>::weak   WorkshopObserverWeakPtr;


typedef std::vector<PublishedFileId_t> PublishedFileIds;

// Namespace end
}
}


#endif  // defined(TT_STEAM_BUILD)

#endif  // !defined(INC_TOKI_STEAM_FWD_H)
