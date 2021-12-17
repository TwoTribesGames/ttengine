#if !defined(INC_TOKI_GAME_EDITOR_EDITORSETTINGS_H)
#define INC_TOKI_GAME_EDITOR_EDITORSETTINGS_H


#include <string>

#include <tt/gwen/GroupedButtonList.h>
#include <tt/math/Rect.h>
#include <tt/platform/tt_types.h>

#include <toki/game/editor/types.h>
#include <toki/game/types.h>
#include <toki/level/entity/editor/fwd.h>


namespace toki {
namespace game {
namespace editor {

struct EditorSettings
{
	enum ToolWindow
	{
		ToolWindow_EditTools,
		ToolWindow_SelectMission,
		ToolWindow_LoadLevel,
		ToolWindow_SetThemeColors,
		
		ToolWindow_Count
	};
	
	struct ToolWindowSettings
	{
		tt::math::PointRect windowRect;
		bool                visible;
		
		inline ToolWindowSettings()
		:
		windowRect(),
		visible(false)
		{ }
	};
	
	
	ToolWindowSettings toolWindow[ToolWindow_Count];
	s32                dockWidthLeft;   // width of the dock on the left side
	s32                dockWidthRight;  // width of the dock on the right side
	bool               statusBarVisible;
	real               cameraFov;
	bool               autoSyncEntityChanges;
	
	GameLayers                         gameLayersVisible;
	level::entity::editor::RenderFlags entityRenderFlags;
	LoadLevelFilter                    loadLevelFilter;  // selected filter in the Load Level window
	
	tt::gwen::GroupedButtonList::ExpansionState entityLibraryExpansionState;
	
	
	EditorSettings();
	
	void setToDefaults();
	
	bool save(const std::string& p_relativeFilename) const;
	bool load(const std::string& p_relativeFilename);
};

// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_GAME_EDITOR_EDITORSETTINGS_H)
