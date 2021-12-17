#if !defined(INC_TOKITORI_GAME_SCRIPT_WRAPPERS_WORKSHOPLEVELPICKERWRAPPER_H)
#define INC_TOKITORI_GAME_SCRIPT_WRAPPERS_WORKSHOPLEVELPICKERWRAPPER_H


#include <tt/engine/renderer/fwd.h>
#include <tt/script/helpers.h>

#include <toki/game/entity/graphics/types.h>
#include <toki/game/hud/fwd.h>
#include <toki/game/script/fwd.h>
#include <toki/game/script/sqbind_bindings.h>


namespace toki {
namespace game {
namespace script {
namespace wrappers {

/*! \brief 'WorkshopLevelPicker' in Squirrel. Manages the level picker interface for Steam Workshop. */
class WorkshopLevelPickerWrapper
{
public:
	/*! \brief Creates the level picker (without showing it). */
	static void create();
	
	/*! \brief Opens/shows the level picker. */
	static void open();
	
	/*! \brief Closes/hides the level picker. */
	static void close();
	
	/*! \brief Indicates whether the level picker is open/visible. */
	static bool isOpen();
	
	/*! \brief Returns the global score (rating based on votes) for the selected level.
	           Returns 0 if no level is selected or if an error occurred. */
	static float getSelectedLevelScore();
	
	/*! \brief Indicates whether the selected level has been downloaded (and as such can be played). */
	static bool isSelectedLevelDownloaded();
	
	/*! \brief Indicates whether any level in the level list is selected. */
	static bool isAnyLevelSelected();
	
	/*! \brief Selects the previous level in the level list (if available). */
	static void selectPrevious();
	
	/*! \brief Selects the next level in the level list (if available). */
	static void selectNext();
	
	/*! \brief Plays (loads) the selected level. Does nothing if no level is selected. */
	static void playSelectedLevel();
	
	/*! \brief Opens the Steam overlay to browse the game's Steam Workshop page. */
	static void browseWorkshop();
	
	/*! \brief Opens the Steam overlay to view the selected level's Steam Workshop page.
	    \note Does nothing if no level is selected. */
	static void showSelectedLevelWorkshopPage();
	
	/*! \brief Debug feature to show outlines of text elements, to help with positioning. */
	static void setShowTextBorders(bool p_show);
	
	/*! \brief Sets the entity that will receive callbacks from the level picker (such as when the selected level changes).
	           Calls a function 'onWorkshopLevelSelected' when a level is selected / selection changes. */
	static void setCallbackEntity(EntityBase* p_entity);
	
	/*! \brief Sets the entity which will provide the position for the 'level list' element. */
	static void setFollowEntityLevelList(EntityBase* p_entity);
	
	/*! \brief Sets the entity which will provide the position for the 'title' element. */
	static void setFollowEntityTitle(EntityBase* p_entity);
	
	/*! \brief Sets the entity which will provide the position for the 'description' element. */
	static void setFollowEntityDescription(EntityBase* p_entity);
	
	/*! \brief Sets the entity which will provide the position for the 'preview image' element. */
	static void setFollowEntityPreviewImage(EntityBase* p_entity);
	
	/*! \brief Sets the entity which will provide the position for the 'author avatar' element. */
	static void setFollowEntityAuthorAvatar(EntityBase* p_entity);
	
	/*! \brief Sets the entity which will provide the position for the 'author name' element. */
	static void setFollowEntityAuthorName(EntityBase* p_entity);
	
	/*! \brief Sets the size (in tiles) of the 'level list' element. */
	static void setSizeLevelList(real p_width, real p_height);
	
	/*! \brief Sets the size (in tiles) of the 'title' element. */
	static void setSizeTitle(real p_width, real p_height);
	
	/*! \brief Sets the size (in tiles) of the 'description' element. */
	static void setSizeDescription(real p_width, real p_height);
	
	/*! \brief Sets the size (in tiles) of the 'preview image' element. */
	static void setSizePreviewImage(real p_width, real p_height);
	
	/*! \brief Sets the size (in tiles) of the 'author avatar' element. */
	static void setSizeAuthorAvatar(real p_width, real p_height);
	
	/*! \brief Sets the size (in tiles) of the 'author name' element. */
	static void setSizeAuthorName(real p_width, real p_height);
	
	/*! \brief Sets the color of the title text. */
	static void setColorTitle(const tt::engine::renderer::ColorRGBA& p_color);
	
	/*! \brief Sets the color of the description text. */
	static void setColorDescription(const tt::engine::renderer::ColorRGBA& p_color);
	
	/*! \brief Sets the color of the author name text. */
	static void setColorAuthorName(const tt::engine::renderer::ColorRGBA& p_color);
	
	/*! \brief Sets the vertical alignment of the title text. */
	static void setElementVerticalAlignmentTitle(      entity::graphics::VerticalAlignment     p_verticalAlignment);
	
	/*! \brief Sets the vertical alignment of the description text. */
	static void setElementVerticalAlignmentDescription(entity::graphics::VerticalAlignment     p_verticalAlignment);
	
	/*! \brief Sets the vertical alignment of the author name text. */
	static void setElementVerticalAlignmentAuthorName( entity::graphics::VerticalAlignment     p_verticalAlignment);
	
	/*! \brief Sets the horizontal alignment of the title text. */
	static void setElementHorizontalAlignmentTitle(      entity::graphics::HorizontalAlignment p_horizontalAlignment);
	
	/*! \brief Sets the horizontal alignment of the description text. */
	static void setElementHorizontalAlignmentDescription(entity::graphics::HorizontalAlignment p_horizontalAlignment);
	
	/*! \brief Sets the horizontal alignment of the author name text. */
	static void setElementHorizontalAlignmentAuthorName( entity::graphics::HorizontalAlignment p_horizontalAlignment);
	
	/*! \brief Gets the color scheme of the level list box. */
	static hud::ListBoxColorScheme getLevelListColorScheme();
	
	/*! \brief Sets the color scheme of the level list box. */
	static void setLevelListColorScheme(const hud::ListBoxColorScheme& p_colorScheme);
	
	
	static void bind(const tt::script::VirtualMachinePtr& p_vm);
};

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKITORI_GAME_SCRIPT_WRAPPERS_WORKSHOPLEVELPICKERWRAPPER_H)
