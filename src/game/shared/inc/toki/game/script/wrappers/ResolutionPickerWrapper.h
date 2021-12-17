#if !defined(INC_TOKITORI_GAME_SCRIPT_WRAPPERS_RESOLUTIONPICKERWRAPPER_H)
#define INC_TOKITORI_GAME_SCRIPT_WRAPPERS_RESOLUTIONPICKERWRAPPER_H


#include <tt/engine/renderer/fwd.h>
#include <tt/script/helpers.h>

#include <toki/game/hud/fwd.h>
#include <toki/game/script/fwd.h>


namespace toki {
namespace game {
namespace script {
namespace wrappers {

/*! \brief 'ResolutionPicker' in Squirrel. Manages the level picker interface for Steam Workshop. */
class ResolutionPickerWrapper
{
public:
	/*! \brief Creates the picker (without showing it). */
	static void create();
	
	/*! \brief Opens/shows the picker. */
	static void open();
	
	/*! \brief Closes/hides the picker. */
	static void close();
	
	/*! \brief Indicates whether the picker is open/visible. */
	static bool isOpen();
	
	/*! \brief Selects the previous element in the list (if available). */
	static void selectPrevious();
	
	/*! \brief Selects the next element in the list (if available). */
	static void selectNext();
	
	/*! \brief Apply the selected resolution. Does nothing if no level is selected. */
	static void applySelectedResolution();
	
	/*! \brief Sets the entity that will receive callbacks from the level picker (such as when the selected level changes).
	           Calls a function 'onWorkshopLevelSelected' when a level is selected / selection changes. */
	static void setCallbackEntity(EntityBase* p_entity);
	
	/*! \brief Sets the entity which will provide the position of the list box. */
	static void setFollowEntity(EntityBase* p_entity);
	
	/*! \brief Sets the size (in tiles) of the list box. */
	static void setSize(real p_width, real p_height);
	
	/*! \brief Gets the color scheme of the list box. */
	static hud::ListBoxColorScheme getColorScheme();
	
	/*! \brief Sets the color scheme of the list box. */
	static void setColorScheme(const hud::ListBoxColorScheme& p_colorScheme);
	
	static void bind(const tt::script::VirtualMachinePtr& p_vm);
};

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKITORI_GAME_SCRIPT_WRAPPERS_RESOLUTIONPICKERWRAPPER_H)
