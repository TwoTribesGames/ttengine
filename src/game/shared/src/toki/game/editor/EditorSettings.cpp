#include <json/json.h>

#include <tt/fs/File.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
#include <tt/str/str.h>

#include <toki/game/editor/EditorSettings.h>
#include <toki/game/editor/features.h>
#include <toki/savedata/utils.h>
#include <toki/cfg.h>


namespace toki {
namespace game {
namespace editor {

#if EDITOR_SUPPORTS_SETTINGS_SAVING
static const char* getToolWindowName(EditorSettings::ToolWindow p_window)
{
	switch (p_window)
	{
	case EditorSettings::ToolWindow_EditTools:      return "edit_tools";
	case EditorSettings::ToolWindow_SelectMission:  return "select_mission";
	case EditorSettings::ToolWindow_LoadLevel:      return "load_level";
	case EditorSettings::ToolWindow_SetThemeColors: return "set_theme_colors";
		
	default:
		TT_PANIC("Invalid editor tool window: %d", p_window);
		return "";
	}
}
#endif


//--------------------------------------------------------------------------------------------------
// Public member functions

EditorSettings::EditorSettings()
:
dockWidthLeft(-1),
dockWidthRight(-1),
statusBarVisible(true),
cameraFov(-1.0f),
autoSyncEntityChanges(true),
gameLayersVisible(),
entityRenderFlags(),
loadLevelFilter(LoadLevelFilter_AllLevels),
entityLibraryExpansionState()
{
	setToDefaults();
}


void EditorSettings::setToDefaults()
{
	dockWidthLeft         = 210;
	dockWidthRight        = 220;
	statusBarVisible      = true;
	cameraFov             = cfg()->getRealDirect("toki.camera.editor.fov");
	autoSyncEntityChanges = true;
	
	gameLayersVisible.setAllFlags();
	gameLayersVisible.resetFlag(GameLayer_ShoeboxZero);
	
	entityRenderFlags.setAllFlags();
	entityRenderFlags.resetFlag(level::entity::editor::RenderFlag_AllEntityReferences);
	
	loadLevelFilter = LoadLevelFilter_AllLevels;
	
	entityLibraryExpansionState.clear();
	
	toolWindow[ToolWindow_EditTools].windowRect = tt::math::PointRect(tt::math::Point2(210, 24), 62, 366);
	toolWindow[ToolWindow_EditTools].visible    = true;
	
	toolWindow[ToolWindow_SelectMission].windowRect = tt::math::PointRect(tt::math::Point2(410, 24), 62, 366);
	toolWindow[ToolWindow_SelectMission].visible    = true;
	
	// NOTE: Using default position of (-1, -1) to signal the Editor code to center this window
	toolWindow[ToolWindow_LoadLevel].windowRect = tt::math::PointRect(tt::math::Point2(-1, -1), 300, 600);
	toolWindow[ToolWindow_LoadLevel].visible    = false;
	
	toolWindow[ToolWindow_SetThemeColors].windowRect = tt::math::PointRect(tt::math::Point2(-1, -1), 0, 0);
	toolWindow[ToolWindow_SetThemeColors].visible    = false;
}


bool EditorSettings::save(const std::string& p_relativeFilename) const
{
#if EDITOR_SUPPORTS_SETTINGS_SAVING
	tt::fs::FilePtr file = savedata::createSaveFile(p_relativeFilename);
	if (file == 0)
	{
		return false;
	}
	
	// Create a JSON document containing the settings
	Json::Value rootNode(Json::objectValue);
	
	// - General settings
	rootNode["dockWidthLeft"        ] = dockWidthLeft;
	rootNode["dockWidthRight"       ] = dockWidthRight;
	rootNode["dockWidth"            ] = dockWidthRight;  // for now, keep writing the old setting as well
	rootNode["statusBarVisible"     ] = statusBarVisible;
	rootNode["cameraFov"            ] = cameraFov;
	rootNode["autoSyncEntityChanges"] = autoSyncEntityChanges;
	rootNode["loadLevelFilter"      ] = getLoadLevelFilterName(loadLevelFilter);
	
	{
		std::string flags;
		for (s32 i = 0; i < GameLayer_Count; ++i)
		{
			const GameLayer layer = static_cast<GameLayer>(i);
			if (gameLayersVisible.checkFlag(layer))
			{
				if (flags.empty() == false) flags += ",";
				flags += getGameLayerName(layer);
			}
		}
		
		rootNode["gameLayersVisible"] = flags;
	}
	
	{
		using namespace level::entity::editor;
		
		std::string flags;
		for (s32 i = 0; i < RenderFlag_Count; ++i)
		{
			const RenderFlag flag = static_cast<RenderFlag>(i);
			if (entityRenderFlags.checkFlag(flag))
			{
				if (flags.empty() == false) flags += ",";
				flags += getRenderFlagName(flag);
			}
		}
		
		rootNode["entityRenderFlags"] = flags;
	}
	
	{
		Json::Value& stateNode(rootNode["entityLibraryExpansionState"]);
		
		stateNode["open"  ] = Json::Value(Json::arrayValue);
		stateNode["closed"] = Json::Value(Json::arrayValue);
		
		Json::Value& openNode  (stateNode["open"]);
		Json::Value& closedNode(stateNode["closed"]);
		
		for (tt::gwen::GroupedButtonList::ExpansionState::const_iterator it = entityLibraryExpansionState.begin();
		     it != entityLibraryExpansionState.end(); ++it)
		{
			if ((*it).second)
			{
				openNode.append(Json::Value(static_cast<Json::Value::UInt>((*it).first)));
			}
			else
			{
				closedNode.append(Json::Value(static_cast<Json::Value::UInt>((*it).first)));
			}
		}
	}
	
	// - Tool windows
	Json::Value& toolWindowNode(rootNode["toolWindows"]);
	for (s32 i = 0; i < ToolWindow_Count; ++i)
	{
		const ToolWindow window = static_cast<ToolWindow>(i);
		Json::Value& windowNode(toolWindowNode[getToolWindowName(window)]);
		windowNode["x"      ] = toolWindow[window].windowRect.getPosition().x;
		windowNode["y"      ] = toolWindow[window].windowRect.getPosition().y;
		windowNode["width"  ] = toolWindow[window].windowRect.getWidth();
		windowNode["height" ] = toolWindow[window].windowRect.getHeight();
		windowNode["visible"] = toolWindow[window].visible;
	}
	
	// Write the settings data as nicely formatted JSON
	const std::string jsonText = Json::StyledWriter().write(rootNode);
	const tt::fs::size_type bytesToWrite = static_cast<tt::fs::size_type>(jsonText.length());
	
	if (file->write(jsonText.c_str(), bytesToWrite) != bytesToWrite)
	{
		return false;
	}
	
	file.reset();
	return savedata::commitSaveData();
#else
	// Do not save editor settings in final CAT builds (unnecessary save data usage)
	(void)p_relativeFilename;
	return true;
#endif
}


bool EditorSettings::load(const std::string& p_relativeFilename)
{
	// Start out with default values
	setToDefaults();
	
#if EDITOR_SUPPORTS_SETTINGS_SAVING
	tt::fs::FilePtr file = savedata::openSaveFile(p_relativeFilename);
	if (file == 0 || file->getLength() <= 0)
	{
		return false;
	}
	
	// Parse the entire file as JSON
	tt::code::BufferPtr fileContent = file->getContent();
	if (fileContent == 0)
	{
		return false;
	}
	
	const char* jsonDataBegin = reinterpret_cast<const char*>(fileContent->getData());
	const char* jsonDataEnd   = jsonDataBegin + fileContent->getSize();
	
	Json::Value rootNode;
	Json::Reader reader;
	if (reader.parse(jsonDataBegin, jsonDataEnd, rootNode, false) == false)
	{
		TT_PANIC("Editor settings file could not be parsed as JSON.");
		return false;
	}
	
	// Retrieve the settings from the JSON data
	// - General settings
	if (rootNode.isMember("dockWidth") && rootNode["dockWidth"].isConvertibleTo(Json::intValue))
	{
		// Support legacy "dockWidth" setting: it is the width of the dock on the right side
		dockWidthRight = rootNode["dockWidth"].asInt();
	}
	
	if (rootNode.isMember("dockWidthLeft") && rootNode["dockWidthLeft"].isConvertibleTo(Json::intValue))
	{
		dockWidthLeft = rootNode["dockWidthLeft"].asInt();
	}
	
	if (rootNode.isMember("dockWidthRight") && rootNode["dockWidthRight"].isConvertibleTo(Json::intValue))
	{
		dockWidthRight = rootNode["dockWidthRight"].asInt();
	}
	
	if (rootNode.isMember("statusBarVisible") && rootNode["statusBarVisible"].isConvertibleTo(Json::booleanValue))
	{
		statusBarVisible = rootNode["statusBarVisible"].asBool();
	}
	
	if (rootNode.isMember("cameraFov") && rootNode["cameraFov"].isConvertibleTo(Json::realValue))
	{
		cameraFov = static_cast<real>(rootNode["cameraFov"].asDouble());
	}
	
	if (rootNode.isMember("autoSyncEntityChanges") && rootNode["autoSyncEntityChanges"].isConvertibleTo(Json::booleanValue))
	{
		autoSyncEntityChanges = rootNode["autoSyncEntityChanges"].asBool();
	}
	
	if (rootNode.isMember("loadLevelFilter") && rootNode["loadLevelFilter"].isString())
	{
		const LoadLevelFilter filter = getLoadLevelFilterFromName(rootNode["loadLevelFilter"].asString());
		if (isValidLoadLevelFilter(filter))
		{
			loadLevelFilter = filter;
		}
	}
	
	if (rootNode.isMember("gameLayersVisible") && rootNode["gameLayersVisible"].isString())
	{
		const tt::str::Strings flags(tt::str::explode(rootNode["gameLayersVisible"].asString(), ","));
		
		gameLayersVisible.resetAllFlags();
		
		for (tt::str::Strings::const_iterator it = flags.begin(); it != flags.end(); ++it)
		{
			const GameLayer layer = getGameLayerFromName(*it);
			if (isValidGameLayer(layer))
			{
				gameLayersVisible.setFlag(layer);
			}
		}
	}
	
	if (rootNode.isMember("entityRenderFlags") && rootNode["entityRenderFlags"].isString())
	{
		const tt::str::Strings flags(tt::str::explode(rootNode["entityRenderFlags"].asString(), ","));
		
		entityRenderFlags.resetAllFlags();
		
		for (tt::str::Strings::const_iterator it = flags.begin(); it != flags.end(); ++it)
		{
			using namespace level::entity::editor;
			const RenderFlag flag = getRenderFlagFromName(*it);
			if (isValidRenderFlag(flag))
			{
				entityRenderFlags.setFlag(flag);
			}
		}
	}
	
	if (rootNode.isMember("entityLibraryExpansionState") &&
	    rootNode["entityLibraryExpansionState"].isObject())
	{
		const Json::Value& stateNode(rootNode["entityLibraryExpansionState"]);
		
		if (stateNode.isMember("open") && stateNode["open"].isArray())
		{
			const Json::Value& items(stateNode["open"]);
			for (Json::Value::const_iterator it = items.begin(); it != items.end(); ++it)
			{
				if ((*it).isConvertibleTo(Json::uintValue))
				{
					entityLibraryExpansionState[(*it).asUInt()] = true;
				}
			}
		}
		
		if (stateNode.isMember("closed") && stateNode["closed"].isArray())
		{
			const Json::Value& items(stateNode["closed"]);
			for (Json::Value::const_iterator it = items.begin(); it != items.end(); ++it)
			{
				if ((*it).isConvertibleTo(Json::uintValue))
				{
					entityLibraryExpansionState[(*it).asUInt()] = false;
				}
			}
		}
	}
	
	// - Tool windows
	if (rootNode.isMember("toolWindows") && rootNode["toolWindows"].isObject())
	{
		const Json::Value& toolWindowNode(rootNode["toolWindows"]);
		for (s32 i = 0; i < ToolWindow_Count; ++i)
		{
			const ToolWindow window = static_cast<ToolWindow>(i);
			const char* windowName = getToolWindowName(window);
			
			if (toolWindowNode.isMember(windowName) && toolWindowNode[windowName].isObject())
			{
				const Json::Value& windowNode(toolWindowNode[windowName]);
				
				tt::math::Point2 pos   (toolWindow[window].windowRect.getPosition());
				s32              width (toolWindow[window].windowRect.getWidth());
				s32              height(toolWindow[window].windowRect.getHeight());
				
				if (windowNode.isMember("x") && windowNode["x"].isConvertibleTo(Json::intValue))
				{
					pos.x = windowNode["x"].asInt();
				}
				
				if (windowNode.isMember("y") && windowNode["y"].isConvertibleTo(Json::intValue))
				{
					pos.y = windowNode["y"].asInt();
				}
				
				if (windowNode.isMember("width") && windowNode["width"].isConvertibleTo(Json::intValue))
				{
					width = windowNode["width"].asInt();
				}
				
				if (windowNode.isMember("height") && windowNode["height"].isConvertibleTo(Json::intValue))
				{
					height = windowNode["height"].asInt();
				}
				
				if (windowNode.isMember("visible") && windowNode["visible"].isConvertibleTo(Json::booleanValue))
				{
					toolWindow[window].visible = windowNode["visible"].asBool();
				}
				
				toolWindow[window].windowRect = tt::math::PointRect(pos, width, height);
			}
		}
	}
#else
	(void)p_relativeFilename;
#endif
	
	return true;
}

// Namespace end
}
}
}
