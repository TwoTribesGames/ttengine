#if !defined(INC_TOKI_STATELIST_STATELIST_H)
#define INC_TOKI_STATELIST_STATELIST_H


#include <string>

#include <tt/code/StateID.h>


namespace toki {
namespace statelist {

// Special helper state to ensure all resources get cleaned up
extern const tt::code::StateID StateID_ExitApp;

extern const tt::code::StateID StateID_LoadApp;

extern const tt::code::StateID StateID_LoadGame;
extern const tt::code::StateID StateID_Game;

// Debug viewers (FIXME: should probably disable this for final builds)
extern const tt::code::StateID StateID_PresentationViewer;



const char*       getStateName(const tt::code::StateID& p_state);
tt::code::StateID getStateFromName(const std::string& p_stateName);

// Namespace end
}
}


#endif  // !defined(INC_TOKI_STATELIST_STATELIST_H)
