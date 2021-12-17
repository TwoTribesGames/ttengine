#include <tt/platform/tt_types.h>

#include <toki/statelist/statelist.h>


namespace toki {
namespace statelist {

const tt::code::StateID StateID_ExitApp;

const tt::code::StateID StateID_LoadApp;

const tt::code::StateID StateID_LoadGame;
const tt::code::StateID StateID_Game;

const tt::code::StateID StateID_PresentationViewer;


const char* getStateName(const tt::code::StateID& p_state)
{
	if      (p_state == StateID_ExitApp)            return "exit_app";
	else if (p_state == StateID_LoadApp)            return "load_app";
	else if (p_state == StateID_LoadGame)           return "load_game";
	else if (p_state == StateID_Game)               return "game";
	else if (p_state == StateID_PresentationViewer) return "presentation_viewer";
	
	TT_PANIC("Invalid state ID: %d", p_state.getValue());
	return "";
}


tt::code::StateID getStateFromName(const std::string& p_stateName)
{
	for (int i = 0; i < tt::code::StateID::getCount(); ++i)
	{
		tt::code::StateID id(tt::code::StateID::createFromValue(i));
		if (getStateName(id) == p_stateName)
		{
			return id;
		}
	}
	
	return tt::code::StateID::invalid;
}

// Namespace end
}
}
