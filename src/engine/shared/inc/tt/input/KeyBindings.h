#if !defined(INC_TT_INPUT_KEYBINDINGS_H)
#define INC_TT_INPUT_KEYBINDINGS_H


#include <list>
#include <map>
#include <string>
#include <vector>

#include <tt/fs/types.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace input {

typedef u8 KeyCode;
typedef std::list<KeyCode> KeyList;
typedef std::map<std::string, KeyList> ActionMap;
typedef std::map<std::string, ActionMap> ControllerBindings;
// Required actions are actions that are not having a permanent binding and these MUST be defined in the keybindings.xml
// for the game to work properly. If any of these actions miss, the creation fails.
typedef std::vector<std::string> RequiredActions;

static const char* const keyboardID = "keyboard";
static const char* const mouseID    = "mouse";
static const char* const xinputID   = "xinput";

class KeyBindings
{
public:
	static bool createFromJSON(const fs::FilePtr& p_file, const RequiredActions& p_requiredActions);
	static bool saveAsJSON(const fs::FilePtr& p_file);
	
	// Only for legacy support
	static bool createFromXML(const fs::FilePtr& p_file);
	
	static ActionMap getPermanentControllerBindings(const std::string& p_controllerID);
	static ActionMap getCustomControllerBindings(const std::string& p_controllerID);
	static ActionMap getCombinedControllerBindings(const std::string& p_controllerID);
	
	static void setPermanentControllerBindings(const std::string& p_controllerID, const ActionMap& p_bindings);
	static void setCustomControllerBindings(const std::string& p_controllerID, const ActionMap& p_bindings);
	
private:
	static void addActionToCustomBindings(const std::string& p_controllerID, const std::string& p_action, KeyCode p_code,
	                                      RequiredActions& p_requiredActions);
	
	static ControllerBindings ms_permanentBindings;
	static ControllerBindings ms_customBindings;
};

// Namespace end
}
}


#endif  // !defined(INC_TT_INPUT_KEYBINDINGS_H)
