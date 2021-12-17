#if !defined(TT_BUILD_FINAL)

#include <Gwen/Controls/CheckBox.h>
#include <Gwen/Controls/GroupBox.h>
#include <Gwen/Controls/ScrollControl.h>
#include <Gwen/Controls/WindowControl.h>
#include <json/json.h>

#include <tt/app/Application.h>
#include <tt/engine/particles/ParticleMgr.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/fs/File.h>
#if defined(TT_PLATFORM_WIN)
#include <tt/input/MouseController.h>
#endif
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
#include <tt/profiler/PerformanceProfiler.h>

#include <toki/game/entity/graphics/TextLabelMgr.h>
#include <toki/game/entity/EntityMgr.h>
#include <toki/game/fluid/FluidMgr.h>
#include <toki/game/hud/DebugUI.h>
#include <toki/game/Game.h>
#include <toki/savedata/utils.h>
#include <toki/AppGlobal.h>


namespace toki {
namespace game {
namespace hud {

static const char* const g_debugUiSettingsFile = "debug_ui_settings.json";

static const int g_itemSpacing = 2;


inline Gwen::Controls::GroupBox* createBox(const std::string& p_title, Gwen::Controls::Base* p_parent)
{
	Gwen::Controls::GroupBox* box = new Gwen::Controls::GroupBox(p_parent);
	box->SetText(p_title);
	Gwen::Margin margin(box->GetMargin());
	margin.right  += 3;
	margin.bottom += 10;
	box->SetMargin(margin);
	box->Dock(Gwen::Pos::Top);
	
	return box;
}


//--------------------------------------------------------------------------------------------------
// Public member functions

DebugUIPtr DebugUI::create()
{
	return DebugUIPtr(new DebugUI);
}


DebugUI::~DebugUI()
{
	saveUiSettings();
}


bool DebugUI::update(real /*p_deltaTime*/)
{
	const input::Controller::State&              inputState(AppGlobal::getController(tt::input::ControllerIndex_One).cur);
	const input::Controller::State::EditorState& editorState(inputState.editor);
	
	// Always handle the debug hotkeys (regardless of whether the UI is visible)
	for (DebugRenderHotKeys::iterator it = m_debugRenderHotKeys.begin();
	     it != m_debugRenderHotKeys.end(); ++it)
	{
		const HotKey& hotKey((*it).first);
		if (editorState.keys[hotKey.actionKey      ].pressed                                              &&
		    editorState.keys[tt::input::Key_Control].down == hotKey.modifiers.checkFlag(Modifier_Control) &&
		    editorState.keys[tt::input::Key_Alt    ].down == hotKey.modifiers.checkFlag(Modifier_Alt    ) &&
		    editorState.keys[tt::input::Key_Shift  ].down == hotKey.modifiers.checkFlag(Modifier_Shift  ))
		{
			AppGlobal::modifyDebugRenderMask().toggleFlag((*it).second);
		}
	}
	
	for (GenericHotKeys::iterator it = m_hotKeys.begin(); it != m_hotKeys.end(); ++it)
	{
		const HotKey& hotKey((*it).first);
		if (editorState.keys[hotKey.actionKey      ].pressed                                              &&
		    editorState.keys[tt::input::Key_Control].down == hotKey.modifiers.checkFlag(Modifier_Control) &&
		    editorState.keys[tt::input::Key_Alt    ].down == hotKey.modifiers.checkFlag(Modifier_Alt    ) &&
		    editorState.keys[tt::input::Key_Shift  ].down == hotKey.modifiers.checkFlag(Modifier_Shift  ))
		{
			(this->*((*it).second))();
		}
	}
	
	// Only perform GUI work when the UI is actually visible
	if (m_visible == false)
	{
		return false;
	}
	
	// Update the checkboxes based on the state of the DebugRenderMask
	m_ignoreEvents = true;
	const DebugRenderMask& renderMask(AppGlobal::getDebugRenderMask());
	for (s32 i = 0; i < DebugRender_Count; ++i)
	{
		if (m_debugRenderCheckbox[i] != 0)
		{
			m_debugRenderCheckbox[i]->SetChecked(renderMask.checkFlag(static_cast<DebugRender>(i)));
		}
	}
	
	// Same for game layers
	if (AppGlobal::hasGame())
	{
		const GameLayers& gameLayerVisibility(AppGlobal::getGame()->getLayersVisibility());
		for (s32 i = 0; i < GameLayer_Count; ++i)
		{
			if (m_gameLayerCheckbox[i] != 0)
			{
				m_gameLayerCheckbox[i]->SetChecked(gameLayerVisibility.checkFlag(static_cast<GameLayer>(i)));
			}
		}
	}
	
	m_ignoreEvents = false;
	
	
	const bool handledInput = m_gwenRoot.handleInput(
			editorState.pointer, AppGlobal::getController(tt::input::ControllerIndex_One).prev.editor.pointer,
			editorState.pointerLeft, editorState.pointerRight,
			inputState.wheelNotches * 120, m_rootDock);
	
	return handledInput;
}


void DebugUI::render()
{
	if (m_visible)
	{
		m_gwenRoot.render();
	}
}


void DebugUI::onResetDevice()
{
}


//--------------------------------------------------------------------------------------------------
// Private member functions

DebugUI::DebugUI()
:
Gwen::Event::Handler(),
m_visible(false),
m_gwenRoot("debugUI", "gwen_skin", tt::engine::renderer::ViewPortID_Main),
m_rootDock(0),
m_window(0),
m_debugRenderHotKeys(),
m_hotKeys(),
m_ignoreEvents(false)
{
	for (s32 i = 0; i < DebugRender_Count; ++i)
	{
		m_debugRenderCheckbox[i] = 0;
	}
	
	for (s32 i = 0; i < GameLayer_Count; ++i)
	{
		m_gameLayerCheckbox[i] = 0;
	}
	
	setupUi();
	setupHotKeys();
}


void DebugUI::setupHotKeys()
{
	typedef Modifiers M;  // to keep the lines below a little shorter
	
	// Hotkeys without modifiers
	addDebugRenderHotKey(tt::input::Key_N,        M(),               DebugRender_Light);
	addDebugRenderHotKey(tt::input::Key_N,        M(Modifier_Shift), DebugRender_EntityLightDetectionPoints);
	addDebugRenderHotKey(tt::input::Key_B,        M(),               DebugRender_LevelBorder);
	addDebugRenderHotKey(tt::input::Key_K,        M(),               DebugRender_Disable_PresentationInFrontOfEntities);
	addDebugRenderHotKey(tt::input::Key_M,        M(),               DebugRender_Disable_Fog);
	addDebugRenderHotKey(tt::input::Key_CapsLock, M(),               DebugRender_RenderScreenspace);
	
	addGenericHotKey    (tt::input::Key_Grave, M(), &DebugUI::toggleVisible);
	addGenericHotKey    (tt::input::Key_L,     M(), &DebugUI::hotKeyToggleGameLayerAttributes);
	
	// Shift+key hotkeys
	addDebugRenderHotKey(tt::input::Key_1, M(Modifier_Shift), DebugRender_EntityCollisionRect);
	addDebugRenderHotKey(tt::input::Key_2, M(Modifier_Shift), DebugRender_EntityTileRect);
	addDebugRenderHotKey(tt::input::Key_3, M(Modifier_Shift), DebugRender_EntityRegisteredRect);
	addDebugRenderHotKey(tt::input::Key_4, M(Modifier_Shift), DebugRender_EntityRegisteredTiles);
	addDebugRenderHotKey(tt::input::Key_4, M(Modifier_Shift) | M(Modifier_Control), DebugRender_EntityVibrationDetectionPoints);
	addDebugRenderHotKey(tt::input::Key_5, M(Modifier_Shift), DebugRender_EntitySightSensorShapes);
	addDebugRenderHotKey(tt::input::Key_5, M(Modifier_Shift) | M(Modifier_Control), DebugRender_EntitySightDetectionPoints);
	addDebugRenderHotKey(tt::input::Key_6, M(Modifier_Shift), DebugRender_EntityTouchSensorShapes);
	addDebugRenderHotKey(tt::input::Key_6, M(Modifier_Shift) | M(Modifier_Control), DebugRender_EntityTouchShape);
	addDebugRenderHotKey(tt::input::Key_8, M(Modifier_Shift), DebugRender_EntityPosition);
	addDebugRenderHotKey(tt::input::Key_9, M(Modifier_Shift), DebugRender_EntityCollisionParent);
	addDebugRenderHotKey(tt::input::Key_0, M(Modifier_Shift), DebugRender_Event);
	
	addGenericHotKey    (tt::input::Key_C, M(Modifier_Shift), &DebugUI::hotKeyToggleCameraFollowEntity);
	addGenericHotKey    (tt::input::Key_W, M(Modifier_Shift), &DebugUI::hotKeyToggleFluidGraphicsDebug);
	addGenericHotKey    (tt::input::Key_B, M(Modifier_Shift), &DebugUI::hotKeyToggleGameLayerShoeboxBackground);
	addGenericHotKey    (tt::input::Key_L, M(Modifier_Shift), &DebugUI::hotKeyToggleGameLayerShoeboxZero);
	addGenericHotKey    (tt::input::Key_Numpad5, M(),         &DebugUI::hotKeyToggleTextLabelsBorders);
	
	// Ctrl+key hotkeys
	addDebugRenderHotKey(tt::input::Key_1, M(Modifier_Control), DebugRender_EntityTileSensorShapes);
	addDebugRenderHotKey(tt::input::Key_2, M(Modifier_Control), DebugRender_EntityMissingFrame);
	addDebugRenderHotKey(tt::input::Key_3, M(Modifier_Control), DebugRender_EntitySensorShapesText);
	addDebugRenderHotKey(tt::input::Key_4, M(Modifier_Control), DebugRender_EntityMoveToRect);
	addDebugRenderHotKey(tt::input::Key_5, M(Modifier_Control), DebugRender_DisableDebugRenderForEntityWithParent);
	addDebugRenderHotKey(tt::input::Key_6, M(Modifier_Control), DebugRender_RenderEffectRects);
	addDebugRenderHotKey(tt::input::Key_C, M(Modifier_Control) | M(Modifier_Alt), DebugRender_RenderCullingRects);
	addDebugRenderHotKey(tt::input::Key_P, M(Modifier_Control) | M(Modifier_Alt), DebugRender_RenderParticleRects);
	
	addGenericHotKey    (tt::input::Key_0, M(Modifier_Control), &DebugUI::hotKeyResetCameraFovToDefault);
	addGenericHotKey    (tt::input::Key_O, M(Modifier_Control), &DebugUI::hotKeyShowLoadLevelDialog);
	addGenericHotKey    (tt::input::Key_W, M(Modifier_Control), &DebugUI::hotKeyToggleFluidGraphics);
	addGenericHotKey    (tt::input::Key_F, M(Modifier_Control), &DebugUI::hotKeyCycleSectionProfiler);
	
	// Alt+key hotkeys
	addDebugRenderHotKey(tt::input::Key_1, M(Modifier_Alt), DebugRender_PathMgrAgents);
	addDebugRenderHotKey(tt::input::Key_2, M(Modifier_Alt), DebugRender_PathMgrNavMesh);
	addDebugRenderHotKey(tt::input::Key_3, M(Modifier_Alt), DebugRender_PathMgrContours);
	addDebugRenderHotKey(tt::input::Key_4, M(Modifier_Alt), DebugRender_PathMgrCompactHeightfield);
	addDebugRenderHotKey(tt::input::Key_5, M(Modifier_Alt), DebugRender_PathMgrHeightfield);
	addDebugRenderHotKey(tt::input::Key_6, M(Modifier_Alt), DebugRender_PathMgrPolyMesh);
	addDebugRenderHotKey(tt::input::Key_7, M(Modifier_Alt), DebugRender_PathMgrObstacles);
	// The following keys are reserved for path mgr
	// addDebugRenderHotKey(tt::input::Key_8, M(Modifier_Alt), DebugRender_PathMgr);
	// addDebugRenderHotKey(tt::input::Key_9, M(Modifier_Alt), DebugRender_PathMgr);
	// addDebugRenderHotKey(tt::input::Key_0, M(Modifier_Alt), DebugRender_PathMgr);
	
	addGenericHotKey(tt::input::Key_F,         M(Modifier_Alt), &DebugUI::hotKeyToggleFrameCounter);
	addGenericHotKey(tt::input::Key_F12,       M(Modifier_Alt), &DebugUI::hotKeyToggle30FpsMode);
	addGenericHotKey(tt::input::Key_Backspace, M(Modifier_Alt) | M(Modifier_Shift), &DebugUI::hotKeyCrash);
	addGenericHotKey(tt::input::Key_F3,        M(Modifier_Alt), &DebugUI::hotKeyTakeLevelScreenshot);
	addGenericHotKey(tt::input::Key_C,         M(Modifier_Shift) | M(Modifier_Control) | M(Modifier_Alt),
		&DebugUI::hotKeyToggleEntityCulling);
	
	addGenericHotKey(tt::input::Key_P,         M(Modifier_Shift) | M(Modifier_Control) | M(Modifier_Alt),
		&DebugUI::hotKeyToggleParticleCulling);
}


void DebugUI::setupUi()
{
	UiSettings settings;
	settings.load(g_debugUiSettingsFile);
	
	Gwen::Controls::Canvas* canvas = m_gwenRoot.getCanvas();
	
	m_rootDock = new Gwen::Controls::DockBase(canvas);
	m_rootDock->Dock(Gwen::Pos::Fill);
	
	// Set up a window for the debug render flags
	const UiSettings::WindowSettings& wndSettings(settings.window[UiSettings::Window_DebugRender]);
	
	m_window = new Gwen::Controls::WindowControl(canvas);
	m_window->SetTitle("Debug Rendering (~)");
	m_window->SetMinimumSize(Gwen::Point(50, 50));
	m_window->SetSize(wndSettings.windowRect.getWidth(), wndSettings.windowRect.getHeight());
	m_window->SetPos (wndSettings.windowRect.getPosition().x, wndSettings.windowRect.getPosition().y);
	m_window->SetDeleteOnClose(false);
	m_window->SetClosable(true);
	m_window->SetHidden(m_visible == false);
	m_window->onWindowClosed.Add(this, &DebugUI::onWindowClosed);
	
	Gwen::Controls::ScrollControl* content = new Gwen::Controls::ScrollControl(m_window);
	content->SetScroll(false, true);
	content->SetAutoHideBars(true);
	content->Dock(Gwen::Pos::Fill);
	
	// DebugRender flags
	setupUiDebugRender(content);
	
	// Game Layers
	setupUiGameLayers(content);
}


void DebugUI::setupUiDebugRender(Gwen::Controls::Base* p_parent)
{
	// Create checkboxes for each of the debug render flags
	typedef std::pair<DebugRender, std::string> RenderItem;
	typedef std::vector<RenderItem> RenderItems;
	
	typedef std::pair<std::string, RenderItems> ItemGroup;
	typedef std::vector<ItemGroup> ItemGroups;
	
	ItemGroups groups;
	
	{
		groups.push_back(ItemGroup("Entity Rects", RenderItems()));
		RenderItems& items(groups.back().second);
		
		items.push_back(RenderItem(DebugRender_EntityCollisionRect,                   "Entity Collision Rect (Shift+1)"));
		items.push_back(RenderItem(DebugRender_EntityTileRect,                        "Entity Tile Rect (Shift+2)"));
		items.push_back(RenderItem(DebugRender_EntityRegisteredRect,                  "Entity Registered Rect (Shift+3)"));
		items.push_back(RenderItem(DebugRender_EntityRegisteredTiles,                 "Entity Registered Tiles (Shift+4)"));
	}
	
	{
		groups.push_back(ItemGroup("Sensor Shapes", RenderItems()));
		RenderItems& items(groups.back().second);
		
		items.push_back(RenderItem(DebugRender_EntityVibrationDetectionPoints,        "Vibration Detection Points (Ctrl+Shift+4)"));
		items.push_back(RenderItem(DebugRender_EntitySightSensorShapes,               "Sight Sensor Shapes (Shift+5)"));
		items.push_back(RenderItem(DebugRender_EntitySightDetectionPoints,            "Sight Detection Points (Ctrl+Shift+5)"));
		items.push_back(RenderItem(DebugRender_EntityTouchSensorShapes,               "Touch Sensor Shapes (Shift+6)"));
		items.push_back(RenderItem(DebugRender_EntityTouchShape,                      "Entity Touch Shape (Ctrl+Shift+6)"));
		items.push_back(RenderItem(DebugRender_EntityTileSensorShapes,                "Tile Sensor Shapes (Ctrl+1)"));
		items.push_back(RenderItem(DebugRender_EntitySensorShapesText,                "Entity Sensor Shape Text (Ctrl+3)"));
	}
	
	{
		groups.push_back(ItemGroup("Entity Misc", RenderItems()));
		RenderItems& items(groups.back().second);
		
		items.push_back(RenderItem(DebugRender_EntityPosition,                        "Entity Position (Shift+8)"));
		items.push_back(RenderItem(DebugRender_EntityCollisionParent,                 "Entity Collision Parent (Shift+9)"));
		items.push_back(RenderItem(DebugRender_EntityMissingFrame,                    "Entity Missing Frame (Ctrl+2)"));
		items.push_back(RenderItem(DebugRender_EntityMoveToRect,                      "Entity Move To Rect (Ctrl+4)"));
		items.push_back(RenderItem(DebugRender_DisableDebugRenderForEntityWithParent, "Disable render entities w parent (Ctrl+5)"));
		items.push_back(RenderItem(DebugRender_Disable_PresentationInFrontOfEntities, "Hide Presentation In Front Of Entities (K)"));
	}
	
	{
		groups.push_back(ItemGroup("Path Finding", RenderItems()));
		RenderItems& items(groups.back().second);
		
		items.push_back(RenderItem(DebugRender_PathMgrAgents,                         "PathMgr Agents (Alt+1)"));
		items.push_back(RenderItem(DebugRender_PathMgrNavMesh,                        "PathMgr Nav Mesh (Alt+2)"));
		items.push_back(RenderItem(DebugRender_PathMgrContours,                       "PathMgr Contours (Alt+3)"));
		items.push_back(RenderItem(DebugRender_PathMgrCompactHeightfield,             "PathMgr Compactheightfield (Alt+4)"));
		items.push_back(RenderItem(DebugRender_PathMgrHeightfield,                    "PathMgr Heightfield (Alt+5)"));
		items.push_back(RenderItem(DebugRender_PathMgrPolyMesh,                       "PathMgr Heightfield (Alt+6)"));
		items.push_back(RenderItem(DebugRender_PathMgrObstacles,                      "PathMgr Obstacles (Alt+7)"));
	}
	
	{
		groups.push_back(ItemGroup("Misc", RenderItems()));
		RenderItems& items(groups.back().second);
		
		items.push_back(RenderItem(DebugRender_Event,                                 "Event (Shift+0)"));
		items.push_back(RenderItem(DebugRender_RenderEffectRects,                     "Render Effect Rects (Ctrl+6)"));
		items.push_back(RenderItem(DebugRender_RenderScreenspace,                     "Debug world/screen (Caps off/on)"));
		items.push_back(RenderItem(DebugRender_RenderCullingRects,                    "Entity Cull Rects (Ctrl+Alt+C)"));
		items.push_back(RenderItem(DebugRender_RenderParticleRects,                   "Particle Cull Rects (Ctrl+Alt+P)"));
		items.push_back(RenderItem(DebugRender_Light,                                 "Light (N)"));
		items.push_back(RenderItem(DebugRender_EntityLightDetectionPoints,            "Light Detection Points (Shift+N)"));
		items.push_back(RenderItem(DebugRender_LevelBorder,                           "Level Border (B)"));
		items.push_back(RenderItem(DebugRender_Disable_Fog,                           "Hide Fog (M)"));
		items.push_back(RenderItem(DebugRender_SectionProfiler,                       "Section Profiler Level 1 (Shift+F)"));
		items.push_back(RenderItem(DebugRender_SectionProfilerTwo,                    "Section Profiler Level 2 (Shift+F)"));
	}
	
	const DebugRenderMask& renderMask(AppGlobal::getDebugRenderMask());
	for (ItemGroups::iterator groupIt = groups.begin(); groupIt != groups.end(); ++groupIt)
	{
		const RenderItems& items((*groupIt).second);
		
		if (items.empty())
		{
			TT_WARN("Item group '%s' is empty. Ignoring group.", (*groupIt).first.c_str());
			continue;
		}
		
		Gwen::Controls::GroupBox* box = createBox((*groupIt).first, p_parent);
		
		int boxHeight = 25;  // FIXME: How to get the extra size needed by GroupBox itself?
		
		for (RenderItems::const_iterator it = items.begin(); it != items.end(); ++it)
		{
			Gwen::Controls::CheckBoxWithLabel* checkbox = new Gwen::Controls::CheckBoxWithLabel(box);
			Gwen::Margin margin(checkbox->GetMargin());
			margin.bottom += g_itemSpacing;
			checkbox->SetMargin(margin);
			checkbox->Dock(Gwen::Pos::Top);
			checkbox->Label()->SetText((*it).second);
			
			m_debugRenderCheckbox[(*it).first] = checkbox->Checkbox();
			checkbox->Checkbox()->SetChecked(renderMask.checkFlag((*it).first));
			checkbox->Checkbox()->UserData.Set("flag", (*it).first);
			checkbox->Checkbox()->onCheckChanged.Add(this, &DebugUI::onDebugRenderCheckboxChanged);
			
			checkbox->SizeToChildren();
			boxHeight += checkbox->Height() + margin.top + margin.bottom;
		}
		
		box->SetHeight(boxHeight);
	}
}


void DebugUI::setupUiGameLayers(Gwen::Controls::Base* p_parent)
{
	Gwen::Controls::GroupBox* box = createBox("Game Layers", p_parent);
	
	int boxHeight = 25;  // FIXME: How to get the extra size needed by GroupBox itself?
	
	typedef std::pair<GameLayer, std::string> LayerItem;
	typedef std::vector<LayerItem> LayerItems;
	
	LayerItems layerItems;
	
	layerItems.push_back(LayerItem(GameLayer_ShoeboxBackground, "Shoebox Background (Shift+B)"));
	layerItems.push_back(LayerItem(GameLayer_Attributes,        "Attributes (L)"));
	layerItems.push_back(LayerItem(GameLayer_ShoeboxZero,       "Shoebox Zero (Shift+L)"));
	layerItems.push_back(LayerItem(GameLayer_ShoeboxForeground, "Shoebox Foreground"));
	layerItems.push_back(LayerItem(GameLayer_Notes,             "Notes"));
	layerItems.push_back(LayerItem(GameLayer_EditorWarnings,    "Editor Warings"));
	
	GameLayers gameLayerVisibility;
	gameLayerVisibility.setAllFlags();
	if (AppGlobal::hasGame())
	{
		gameLayerVisibility = AppGlobal::getGame()->getLayersVisibility();
	}
	for (LayerItems::iterator it = layerItems.begin(); it != layerItems.end(); ++it)
	{
		Gwen::Controls::CheckBoxWithLabel* checkbox = new Gwen::Controls::CheckBoxWithLabel(box);
		Gwen::Margin margin(checkbox->GetMargin());
		margin.bottom += g_itemSpacing;
		checkbox->SetMargin(margin);
		checkbox->Dock(Gwen::Pos::Top);
		checkbox->Label()->SetText((*it).second);
		
		m_gameLayerCheckbox[(*it).first] = checkbox->Checkbox();
		checkbox->Checkbox()->SetChecked(gameLayerVisibility.checkFlag((*it).first));
		checkbox->Checkbox()->UserData.Set("layer", (*it).first);
		checkbox->Checkbox()->onCheckChanged.Add(this, &DebugUI::onGameLayerCheckboxChanged);
		
		checkbox->SizeToChildren();
		boxHeight += checkbox->Height() + margin.top + margin.bottom;
	}
	
	box->SetHeight(boxHeight);
}


void DebugUI::saveUiSettings()
{
	UiSettings settings;
	
	if (m_window != 0)
	{
		UiSettings::WindowSettings& settingsWindow(settings.window[UiSettings::Window_DebugRender]);
		settingsWindow.windowRect = tt::math::PointRect(
				tt::math::Point2(m_window->X(), m_window->Y()),
				m_window->Width(), m_window->Height());
	}
	
	settings.save(g_debugUiSettingsFile);
}


void DebugUI::addDebugRenderHotKey(
		tt::input::Key p_actionKey,
		Modifiers      p_modifiers,
		DebugRender    p_debugRenderFlag)
{
	m_debugRenderHotKeys.push_back(DebugRenderHotKey(HotKey(p_actionKey, p_modifiers), p_debugRenderFlag));
}


void DebugUI::addGenericHotKey(
		tt::input::Key p_actionKey,
		Modifiers      p_modifiers,
		ActionHandler  p_actionHandler)
{
	m_hotKeys.push_back(GenericHotKey(HotKey(p_actionKey, p_modifiers), p_actionHandler));
}


void DebugUI::toggleVisible()
{
	setVisible(m_visible == false);
}


void DebugUI::setVisible(bool p_visible)
{
	if (m_visible == p_visible)
	{
		// No change
		return;
	}
	
#if defined(TT_PLATFORM_WIN)
	if (m_visible && p_visible == false)
	{
		// Restore default cursor
		tt::input::MouseController::resetToDefaultCursor();
	}
#endif
	
	m_visible = p_visible;
	
	if (m_visible && m_window != 0)
	{
		m_window->SetHidden(false);
	}
}


void DebugUI::toggleGameLayer(GameLayer p_layer)
{
	if (AppGlobal::hasGame())
	{
		AppGlobal::getGame()->modifyLayersVisibility().toggleFlag(p_layer);
	}
}


void DebugUI::onWindowClosed(Gwen::Controls::Base* /*p_sender*/)
{
	setVisible(false);
}


void DebugUI::onDebugRenderCheckboxChanged(Gwen::Controls::Base* p_sender)
{
	if (m_ignoreEvents) return;
	
	Gwen::Controls::CheckBox* checkbox = gwen_cast<Gwen::Controls::CheckBox>(p_sender);
	TT_NULL_ASSERT(checkbox);
	if (checkbox != 0)
	{
		const DebugRender flag = checkbox->UserData.Get<DebugRender>("flag");
		AppGlobal::modifyDebugRenderMask().setFlag(flag, checkbox->IsChecked());
	}
}


void DebugUI::onGameLayerCheckboxChanged(Gwen::Controls::Base* p_sender)
{
	if (m_ignoreEvents) return;
	
	Gwen::Controls::CheckBox* checkbox = gwen_cast<Gwen::Controls::CheckBox>(p_sender);
	TT_NULL_ASSERT(checkbox);
	if (checkbox != 0 && AppGlobal::hasGame())
	{
		const GameLayer layer = checkbox->UserData.Get<GameLayer>("layer");
		AppGlobal::getGame()->setGameLayerVisible(layer, checkbox->IsChecked());
	}
}


void DebugUI::hotKeyCycleSectionProfiler()
{
#if defined(PERFORMANCE_PROFILER_ENABLED)
	// toggle performance profiler
	if (PROFILE_PERFORMANCE_ISENABLED())
	{
		PROFILE_PERFORMANCE_OFF();
		PROFILE_PERFORMANCE_OUTPUT();
		PROFILE_PERFORMANCE_RESET();
	}
	else
	{
		PROFILE_PERFORMANCE_ON();
	}
#endif
	
	DebugRenderMask& debugRenderMask(AppGlobal::modifyDebugRenderMask());
	if (debugRenderMask.checkFlag(DebugRender_SectionProfiler))
	{
		if (debugRenderMask.checkFlag(DebugRender_SectionProfilerTwo))
		{
			debugRenderMask.resetFlag(DebugRender_SectionProfiler);
			debugRenderMask.resetFlag(DebugRender_SectionProfilerTwo);
		}
		else
		{
			debugRenderMask.setFlag(DebugRender_SectionProfilerTwo);
		}
	}
	else
	{
		debugRenderMask.setFlag(DebugRender_SectionProfiler);
	}
}


void DebugUI::hotKeyToggleCameraFollowEntity()
{
	if (AppGlobal::hasGame())
	{
		static entity::EntityHandle followEntity;
		Camera& cam(AppGlobal::getGame()->getCamera());
		if (cam.isFollowingEntity())
		{
			followEntity = cam.getFollowEntity();
			cam.resetFollowEntity();
		}
		else
		{
			cam.setFollowEntity(followEntity);
		}
	}
}


void DebugUI::hotKeyToggleFluidGraphics()
{
	if (AppGlobal::hasGame())
	{
		AppGlobal::getGame()->getFluidMgr().toggleGraphicsEnabled();
	}
}


void DebugUI::hotKeyToggleFluidGraphicsDebug()
{
	if (AppGlobal::hasGame())
	{
		AppGlobal::getGame()->getFluidMgr().toggleDebugRenderEnabled();
	}
}


void DebugUI::hotKeyToggleTextLabelsBorders()
{
	static bool showTextBorders = false;
	showTextBorders = showTextBorders == false;
	if (AppGlobal::hasGame())
	{
		game::entity::graphics::TextLabelMgr& mgr(
			AppGlobal::getGame()->getEntityMgr().getTextLabelMgr());
		mgr.setShowTextBorders(showTextBorders);
	}
}


void DebugUI::hotKeyShowLoadLevelDialog()
{
	if (AppGlobal::hasGame() && AppGlobal::allowEditorFeatures())
	{
		AppGlobal::getGame()->showLoadLevelDialog();
	}
}


void DebugUI::hotKeyToggleFrameCounter()
{
	AppGlobal::toggleFrameCounter();
}


void DebugUI::hotKeyResetCameraFovToDefault()
{
	if (AppGlobal::hasGame())
	{
		AppGlobal::getGame()->getCamera().setFOVToDefault();
	}
}


void DebugUI::hotKeyToggle30FpsMode()
{
	AppGlobal::setDoubleUpdateMode( AppGlobal::hasDoubleUpdateMode() == false );
	
	// Old code
	//u32 fps = tt::app::getApplication()->getTargetFPS();
	
	//tt::app::getApplication()->setTargetFPS( (fps == 30) ? 0 : 30 );
}


void DebugUI::hotKeyCrash()
{
	u32* crash(0);
	*crash = 42;
}


void DebugUI::hotKeyTakeLevelScreenshot()
{
	ScreenshotSettings settings;
	settings.type = ScreenshotType_FullLevel;
	AppGlobal::takeLevelScreenshot(settings);
}


void DebugUI::hotKeyToggleEntityCulling()
{
	if (AppGlobal::hasGame())
	{
		AppGlobal::getGame()->getEntityMgr().toggleEntityCulling();
	}
}


void DebugUI::hotKeyToggleParticleCulling()
{
	using namespace tt::engine::particles;
	ParticleMgr::getInstance()->toggleParticleCulling();
}


//--------------------------------------------------------------------------------------------------
// DebugUI::UiSettings inner helper class

DebugUI::UiSettings::UiSettings()
{
	setToDefaults();
}


void DebugUI::UiSettings::setToDefaults()
{
	tt::engine::renderer::Renderer* renderer = tt::engine::renderer::Renderer::getInstance();
	
	const s32 wndW = 250;
	const s32 wndH = 435;
	window[Window_DebugRender].windowRect = tt::math::PointRect(
			tt::math::Point2(
				renderer->getScreenWidth()  - wndW - 5,
				renderer->getScreenHeight() - wndH - 5),
			wndW, wndH);
}


bool DebugUI::UiSettings::save(const std::string& p_relativeFilename) const
{
	tt::fs::FilePtr file = savedata::createSaveFile(p_relativeFilename);
	if (file == 0)
	{
		return false;
	}
	
	// Create a JSON document containing the settings
	Json::Value rootNode(Json::objectValue);
	
	// - Windows
	Json::Value& windowsNode(rootNode["windows"]);
	for (s32 i = 0; i < Window_Count; ++i)
	{
		const Window windowID = static_cast<Window>(i);
		Json::Value& windowNode(windowsNode[getWindowName(windowID)]);
		windowNode["x"     ] = window[windowID].windowRect.getPosition().x;
		windowNode["y"     ] = window[windowID].windowRect.getPosition().y;
		windowNode["width" ] = window[windowID].windowRect.getWidth();
		windowNode["height"] = window[windowID].windowRect.getHeight();
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
}


bool DebugUI::UiSettings::load(const std::string& p_relativeFilename)
{
	// Start out with default values
	setToDefaults();
	
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
	// - Windows
	if (rootNode.isMember("windows") && rootNode["windows"].isObject())
	{
		const Json::Value& windowsNode(rootNode["windows"]);
		for (s32 i = 0; i < Window_Count; ++i)
		{
			const Window windowID   = static_cast<Window>(i);
			const char*  windowName = getWindowName(windowID);
			
			if (windowsNode.isMember(windowName) && windowsNode[windowName].isObject())
			{
				const Json::Value& windowNode(windowsNode[windowName]);
				
				tt::math::Point2 pos   (window[windowID].windowRect.getPosition());
				s32              width (window[windowID].windowRect.getWidth());
				s32              height(window[windowID].windowRect.getHeight());
				
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
				
				window[windowID].windowRect = tt::math::PointRect(pos, width, height);
			}
		}
	}
	
	return true;
}


const char* DebugUI::UiSettings::getWindowName(Window p_window)
{
	switch (p_window)
	{
	case DebugUI::UiSettings::Window_DebugRender: return "debugrender";
		
	default:
		TT_PANIC("Invalid debug UI window: %d", p_window);
		return "";
	}
}

// Namespace end
}
}
}


#endif  // !defined(TT_BUILD_FINAL)
