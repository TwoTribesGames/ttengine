#if !defined(INC_TOKI_GAME_EDITOR_EDITOR_H)
#define INC_TOKI_GAME_EDITOR_EDITOR_H


#include <map>
#include <vector>

#include <Gwen/Controls/Button.h>
#include <Gwen/Controls/DockBase.h>
#include <Gwen/Controls/ListBox.h>
#include <Gwen/Controls/MenuItem.h>
#include <Gwen/Controls/MenuStrip.h>
#include <Gwen/Controls/StatusBar.h>
#include <Gwen/Controls/TabButton.h>
#include <Gwen/Controls/WindowControl.h>
#include <Gwen/Events.h>
#include <json/json.h>

#include <tt/engine/renderer/fwd.h>
#include <tt/gwen/ButtonList.h>
#include <tt/gwen/GroupedButtonList.h>
#include <tt/gwen/RootCanvasWrapper.h>
#include <tt/input/Button.h>
#include <tt/input/fwd.h>
#if defined(TT_PLATFORM_WIN) || defined(TT_PLATFORM_OSX_MAC) || defined(TT_PLATFORM_LNX)
#include <tt/input/KeyboardController.h>
#if defined(TT_PLATFORM_SDL)
#include <tt/input/MouseController.h>
#endif
#endif
#include <tt/input/KeyList.h>
#include <tt/math/Rect.h>
#include <tt/str/str_types.h>
#include <tt/undo/fwd.h>

#include <toki/game/editor/commands/CommandChangeNoteText.h>
#include <toki/game/editor/hotkey/HotKeyMgr.h>
#include <toki/game/editor/tools/Tool.h>
#include <toki/game/editor/tools/types.h>
#include <toki/game/editor/ui/fwd.h>
#include <toki/game/editor/ui/types.h>
#include <toki/game/editor/EditorSettings.h>
#include <toki/game/editor/features.h>
#include <toki/game/editor/fwd.h>
#include <toki/game/editor/types.h>
#include <toki/game/CameraMgr.h>
#include <toki/game/fwd.h>
#include <toki/game/types.h>
#include <toki/level/entity/editor/fwd.h>
#include <toki/level/entity/EntityInstance.h>
#include <toki/level/skin/types.h>
#include <toki/level/fwd.h>
#include <toki/level/LevelDataObserver.h>
#include <toki/level/LevelSection.h>
#include <toki/level/types.h>


namespace toki {
namespace game {
namespace editor {

class Editor : public Gwen::Event::Handler, public level::LevelDataObserver
{
public:
	enum TileLayerMode
	{
		TileLayerMode_CollisionType,
		TileLayerMode_ThemeType
	};
	
	
	static EditorPtr create(const level::LevelDataPtr& p_levelData);
	~Editor();
	
	void update(real p_deltaTime);
	void updateForRender();
	void render();
	
	void show(bool p_userLevelCompleted = false);
	void hide(bool p_restartLevel       = false);
	
	void setLevelData(const level::LevelDataPtr& p_levelData);
	
	const std::string& getCurrentLevelName() const;
	const StartInfo&   getCurrentLevelInfo() const;
	bool               isUnnamedLevel()      const;  // whether level is new / not saved to disk / untitled
	
	bool hasEditorWarnings() const;
	void editorWarning(s32 p_id, const std::string& p_warningStr);
	void clearEditorWarnings();
	
	void handleUnsavedChangesBeforeLevelLoadFromGame(const std::string& p_levelName);
	void showLoadLevelDialog();
	
	ui::GenericDialogBox* showGenericDialog(const std::wstring& p_title,
	                                        const std::wstring& p_promptText,
	                                        ui::DialogButtons   p_buttons                   = ui::DialogButtons_OK,
	                                        bool                p_autoCloseWhenEditorClosed = false);
	
	void setSelectionRect(const tt::math::PointRect& p_rect, bool p_updateEntitySelection = true);
	void clearSelectionRect(bool p_alsoDeselectEntities);
	/*! \return The rectangle in which tools are allowed to edit. Guaranteed to be a valid section of the level. */
	tt::math::PointRect  getEditableRect()       const;
	tt::math::VectorRect getSelectionWorldRect() const;
	inline const tt::math::PointRect& getSelectionRect() const { return m_selectionRect; }
	inline bool hasSelectionRect() const { return m_selectionRect.hasArea(); }
	
	void createFloatingSectionFromSelection(level::LevelSection::Type p_sectionType);
	void cloneFloatingSection();
	/*! \brief "Un-floats" the floating section, applying its tiles to the level data. */
	void applyFloatingSection();
	bool hasFloatingSection() const;
	void setFloatingSectionPosition(const tt::math::Point2& p_pos);
	
	void onResetDevice();
	void releaseResourcesForReload();
	void onRequestReloadAssets();
	
	void overrideLevelBorderRect(const tt::math::PointRect& p_rect);
	void resetLevelBorderRect();
	tt::math::PointRect getLevelBorderRect() const;
	real                getLevelBorderThickness() const;
	
	void handleLevelResized();
	
	inline const GameLayers& getGameLayersVisible() const { return m_persistentSettings.gameLayersVisible; }
	
	void setCursor(EditCursor p_cursor);
	void restoreDefaultCursor();
	
	level::entity::EntityInstancePtr createEntity(const std::string& p_typeName);
	level::entity::EntityInstances findEntitiesAtWorldPos(const tt::math::Vector2& p_worldPos) const;
	bool hasEntityAtWorldPos(const tt::math::Vector2& p_worldPos) const;
	tt::math::Vector2 getEntitySize(const level::entity::EntityInstancePtr& p_entity) const;
	
	void snapEntityToTile(const level::entity::EntityInstancePtr& p_entity);
	
	/*! \brief Temporarily switches to a different tool until all pointer buttons have been released
	           (the editor then switches back to the previous tool). */
	void switchToToolUntilPointerReleased(tools::ToolID p_tool);
	
	void setPaintTile (level::CollisionType p_tile);
	void setPaintTheme(level::ThemeType p_theme);
	
	void enterEntityPickMode(const level::entity::EntityProperty& p_property,
	                         const level::entity::EntityIDSet&    p_disallowedIDs = level::entity::EntityIDSet());
	void cancelEntityPickMode();
	
	void selectFirstEntityOfType(const std::string& p_type);
	
	void enterNoteTextEditMode(const level::NotePtr& p_noteToEdit, bool p_addToLevelOnAccept);
	void cancelNoteTextEditMode();
	inline void setFocusNote(const level::NotePtr& p_note) { m_focusNote = p_note; }
	inline void resetFocusNote()                           { m_focusNote.reset();  }
	
	void onLevelBackgroundChanged();
	void onLevelThemeChanged();
	void onLevelDefaultMissionChanged();
	void onThemeColorChanged(level::skin::SkinConfigType p_skinConfig, level::ThemeType p_theme);
	
	// Saves the level to the current filename
	bool saveLevel();
	
	bool hasUnsavedChanges() const;
	
	void startPlayTest();
	
	void pushUndoCommand(const tt::undo::UndoCommandPtr& p_command);
	void undo(s32 p_depth = 1);
	s32 getUndoStackSize() const;
	
	void centerInScreen(Gwen::Controls::Base* p_control);
	
	inline       Camera& getEditorCamera()       { return m_cameraMgr.getInputCamera(); }
	inline const Camera& getEditorCamera() const { return m_cameraMgr.getInputCamera(); }
	
	inline       CameraMgr& getCameraMgr()       { return m_cameraMgr; }
	inline const CameraMgr& getCameraMgr() const { return m_cameraMgr; }
	
	inline bool isActive() const { return m_editorActive; }
	inline bool isDoingPlayTest() const { return m_doingPlayTest; }
	inline bool isValidPlayTest() const { return m_doingPlayTest && hasUnsavedChanges() == false; }
	
	inline bool shouldAutoSyncEntityChanges() const { return m_persistentSettings.autoSyncEntityChanges; }
	
	inline TileLayerMode                 getTileLayerMode()   const { return m_tileLayerMode;   }
	inline level::CollisionType          getPaintTile()       const { return m_paintTile;       }
	inline level::ThemeType              getPaintTheme()      const { return m_paintTheme;      }
	inline const std::string&            getPaintEntityType() const { return m_paintEntityType; }
	inline const level::LevelDataPtr&    getLevelData()       const { return m_levelData;       }
	
	inline hotkey::HotKeyMgr& getHotKeyMgr() { return m_hotKeyMgr; }
	
private:
	typedef std::vector<Gwen::Controls::MenuItem*> MenuItems;
	struct UI
	{
		Gwen::Controls::DockBase* editorDock;
		tt::gwen::ButtonList*     listTools;
		tt::gwen::ButtonList*     listTiles;
		tt::gwen::GroupedButtonList* listEntities;
		ui::EntityPropertyList*   listEntityProperties;
		Gwen::Controls::Label*    labelInfoPosX;
		Gwen::Controls::Label*    labelInfoPosY;
		Gwen::Controls::Label*    labelInfoSelectionWidth;
		Gwen::Controls::Label*    labelInfoSelectionHeight;
		Gwen::Controls::Label*    labelInfoHasChanges;
		Gwen::Controls::Label*    labelInfoEntityCount;
		Gwen::Controls::Label*    labelInfoLevelName;
		Gwen::Controls::Label*    labelFocusStatus;  // indicates whether GWEN has keyboard focus
		
		Gwen::Controls::TabButton* tabTiles;
		
		Gwen::Controls::MenuStrip* menuBar;
		
		Gwen::Controls::MenuItem*  subMenuLevelBackground;
		MenuItems                  menuItemsLevelBackground;
		Gwen::Controls::MenuItem*  menuItemLevelTheme[level::ThemeType_Count];
		MenuItems                  menuItemsLevelDefaultMission;
		
		Gwen::Controls::StatusBar* statusBar;
		
		Gwen::Controls::WindowControl* windowTools;
		
		Gwen::Controls::WindowControl* windowMissions;
		
		Gwen::Controls::WindowControl* windowLoadLevel;
		bool                           windowLoadLevelNeedsCentering;
		Gwen::Controls::Button*        windowLoadLevelFilterButton[LoadLevelFilter_Count];
		Gwen::Controls::ListBox*       listLevels;
		Gwen::Controls::ListBox*       listMissions;
		
#if defined(TT_STEAM_BUILD)
		ui::PublishToWorkshopDialog* windowPublishToWorkshop;
#endif
		
		ui::SetThemeColorUI*   windowSetThemeColor;
		
		
		inline UI()
		:
		editorDock(0),
		listTools(0),
		listTiles(0),
		listEntities(0),
		listEntityProperties(0),
		labelInfoPosX(0),
		labelInfoPosY(0),
		labelInfoSelectionWidth(0),
		labelInfoSelectionHeight(0),
		labelInfoHasChanges(0),
		labelInfoEntityCount(0),
		labelInfoLevelName(0),
		labelFocusStatus(0),
		tabTiles(0),
		menuBar(0),
		subMenuLevelBackground(0),
		menuItemsLevelBackground(),
		menuItemsLevelDefaultMission(),
		statusBar(0),
		windowTools(0),
		windowMissions(0),
		windowLoadLevel(0),
		windowLoadLevelNeedsCentering(false),
		listLevels(0),
#if defined(TT_STEAM_BUILD)
		windowPublishToWorkshop(0),
#endif
		windowSetThemeColor(0)
		{
			for (s32 i = 0; i < level::ThemeType_Count; ++i)
			{
				menuItemLevelTheme[i] = 0;
			}
			for (s32 i = 0; i < LoadLevelFilter_Count; ++i)
			{
				windowLoadLevelFilterButton[i] = 0;
			}
		}
	};
	
	enum PointerButton
	{
		PointerButton_Left,
		PointerButton_Middle,
		PointerButton_Right,
		
		PointerButton_Count
	};
	
	enum LockAxis
	{
		LockAxis_None,
		LockAxis_X,
		LockAxis_Y
	};
	
	enum EditorMode
	{
		EditorMode_Normal,
		EditorMode_PickEntity,
		EditorMode_EditNote
	};
	
	enum SaveChangesContinueAction
	{
		SaveChangesContinueAction_None,
		SaveChangesContinueAction_LoadLevel,
		SaveChangesContinueAction_ExitApp,
		SaveChangesContinueAction_PlayTest,
		SaveChangesContinueAction_NewLevel,
		SaveChangesContinueAction_PublishToWorkshop
	};
	
	typedef void (Editor::*ActionFunc)();
	typedef void (Editor::*ActionFuncWithSource)(Gwen::Controls::Base*);
	
	typedef std::vector<ui::DialogBoxBase*> DialogBoxes;
	
	// Entity type name -> QuadSprite for the in-editor representation
	typedef std::map<std::string, tt::engine::renderer::QuadSpritePtr> EntityTypeQuads;
	
	
	explicit Editor(const level::LevelDataPtr& p_levelData);
	
	void scanLevelForEntitiesUpdatedByScript();
	void loadCursors();
	void setupPreviewCamera();
	void setupHotKeys();
	void setupNotifications();
	void setupUi();
	void                  createMenu                  (const EditorSettings& p_settings, Gwen::Controls::Base* p_parent);
	void                  createStatusBar             (const EditorSettings& p_settings, Gwen::Controls::Base* p_parent);
	Gwen::Controls::Base* createToolsWindow           (const EditorSettings& p_settings, Gwen::Controls::Base* p_parent);
	Gwen::Controls::Base* createMissionsWindow        (const EditorSettings& p_settings, Gwen::Controls::Base* p_parent);
	Gwen::Controls::Base* createLoadLevelWindow       (const EditorSettings& p_settings, Gwen::Controls::Base* p_parent);
	Gwen::Controls::Base* createTilesWindow           (const EditorSettings& p_settings, Gwen::Controls::Base* p_parent);
	Gwen::Controls::Base* createEntityLibraryWindow   (const EditorSettings& p_settings, Gwen::Controls::Base* p_parent);
	Gwen::Controls::Base* createEntityPropertiesWindow(const EditorSettings& p_settings, Gwen::Controls::Base* p_parent);
	
	Gwen::Controls::MenuItem* addMenuItem(Gwen::Controls::MenuItem* p_targetMenu,
	                                      const std::wstring&       p_displayName,
	                                      ActionFunc                p_actionFunc,
	                                      const std::string&        p_iconName = std::string());
	Gwen::Controls::MenuItem* addMenuItem(Gwen::Controls::MenuItem* p_targetMenu,
	                                      const std::wstring&       p_displayName,
	                                      ActionFunc                p_actionFunc,
	                                      tt::input::Key            p_actionKey,
	                                      const hotkey::Modifiers&  p_modifiers,
	                                      const std::string&        p_iconName = std::string());
	
	// Adds a menu item with a checkmark
	// Any hotkeys specified will be used for toggling the item: they will not trigger the action function
	Gwen::Controls::MenuItem* addMenuCheckItem(Gwen::Controls::MenuItem* p_targetMenu,
	                                           const std::wstring&       p_displayName,
	                                           bool                      p_checked,
	                                           ActionFunc                p_actionFunc);
	Gwen::Controls::MenuItem* addMenuCheckItem(Gwen::Controls::MenuItem* p_targetMenu,
	                                           const std::wstring&       p_displayName,
	                                           bool                      p_checked,
	                                           ActionFunc                p_actionFunc,
	                                           tt::input::Key            p_actionKey,
	                                           const hotkey::Modifiers&  p_modifiers);
	
	Gwen::Controls::MenuItem* addMenuCheckItem(Gwen::Controls::MenuItem* p_targetMenu,
	                                           const std::wstring&       p_displayName,
	                                           bool                      p_checked,
	                                           ActionFuncWithSource      p_actionFunc);
	Gwen::Controls::MenuItem* addMenuCheckItem(Gwen::Controls::MenuItem* p_targetMenu,
	                                           const std::wstring&       p_displayName,
	                                           bool                      p_checked,
	                                           ActionFuncWithSource      p_actionFunc,
	                                           tt::input::Key            p_actionKey,
	                                           const hotkey::Modifiers&  p_modifiers);
	
	void addHotKey(tt::input::Key            p_actionKey,
	               const hotkey::Modifiers&  p_modifiers,
	               ActionFunc                p_actionFunc);
	void addToggleMenuItemHotKey(tt::input::Key            p_actionKey,
	                             const hotkey::Modifiers&  p_modifiers,
	                             Gwen::Controls::MenuItem* p_menuItem);
	
	void saveEditorSettings();
	void createSelectionIndicator();
	void updateSelectionIndicator();
	void hideEditorGUI();
	void handleToolInput();
	bool isAllowedEntityInstance(const level::entity::EntityInstancePtr& p_instance) const;
	tt::math::PointRect getLevelIntersectedSelectionRect() const;
	void fillSelectionWithTile(level::CollisionType p_tile);
	void fillSelectionWithTile(level::ThemeType     p_tile);
	bool copySelectionToClipboard();
	bool copySelectedEntitiesToClipboard();
	void createFloatingSectionVisual();
	void changeCameraFovBy(real p_deltaDegrees);
	void resetClickLockedToolIDs();
	void refreshEntityLibrary(tt::gwen::GroupedButtonList::ExpansionState* p_savedExpansionState);
	void refreshLevelBackgroundMenu();
	void updateLevelBackgroundMenuChecks();
	void updateLevelThemeMenuChecks();
	void updateDefaultMissionMenuChecks();
	
	void createAttributeViews();
	
	std::wstring getTilesTabTitle() const;
	void refreshTilesList();
	void setTileLayerMode(TileLayerMode p_mode);
	
	void switchToTool(tools::ToolID p_tool, bool p_temporary = false);
	
	void doShowLoadLevelDialog();
	void closeLoadLevelDialog();
	bool isLoadLevelDialogOpen() const;
	void refreshLevelListForCurrentFilter();
	void refreshLevelList(const tt::str::Strings& p_filters, const StartInfo& p_pathInfo);
	void doLoadSelectedLevel(bool p_promptForUnsavedChanges);
	void doLoadLevel(const StartInfo& p_pathInfo, const std::string& p_levelName);
	
	level::entity::EntityInstances getMissionSpecificEntities() const;
	void refreshMissionList();
	
	void setCurrentLevelName(const std::string& p_levelName, bool p_alsoSetForActiveGame);
	
	// Saves the level to an arbitrary filename (and optionally writes a "source" copy as well)
	bool saveLevel(const std::string& p_filename, const std::string& p_internalSourceFilename);
	
	void showSaveChangesDialog(SaveChangesContinueAction p_continueAction);
	void showSaveAsDialog     (SaveChangesContinueAction p_continueAction);
	void showNewLevelDialog();
	
	void openModalDialogBox(ui::DialogBoxBase* p_dialog, bool p_autoCloseWhenEditorClosed);
	
	/*! \brief Returns a quad for the specified entity type. If no visual was available for this yet, an "unknown type" quad is created for this type. */
	tt::engine::renderer::QuadSpritePtr getEntityTypeQuad(const std::string& p_typeName);
	
	void renderEntities();
	void renderFloatingSection();
	void renderNotes();
	void renderEditorWarnings();
	
	void resetNoteEditVariables();
	
#if defined(TT_PLATFORM_WIN) || defined(TT_PLATFORM_OSX_MAC) || defined(TT_PLATFORM_LNX)
	static bool keyboardStringCallback(void*                                          p_userData,
	                                   const std::wstring&                            p_currentString,
	                                   tt::input::KeyboardController::GetStringStatus p_status);
	bool onKeyboardStringCallback(const std::wstring&                            p_currentString,
	                              tt::input::KeyboardController::GetStringStatus p_status);
#endif
	
	// Notification handlers from level data
	virtual void onLevelDataEntitySelectionChanged();
	
	// Hotkey handlers
	void hotKeyShowHelp();
	void hotKeyShowLevelDir();
	void hotKeyUndo();
	void hotKeyRedo();
#if EDITOR_SUPPORTS_ASSETS_SOURCE
	void hotKeySaveLevelSkinShoebox();
#endif
	void hotKeyCut();
	void hotKeyCopy();
	void hotKeyPaste();
	void hotKeySelectAll();
	void hotKeyDeselectAll();
	void hotKeyFillTilesInSelection();
	void hotKeyFillTilesWithAirInSelection();
	void hotKeyEraseTilesInSelection();
	void hotKeyFlipSelectionContentsHorizontally();
	void hotKeyFlipSelectionContentsVertically();
	void hotKeyCancelSelection();
	void hotKeyApplyFloatingSection();
	void hotKeySelectToolBoxSelect();
	void hotKeySelectToolDraw();
	void hotKeySelectToolFloodFill();
	void hotKeySelectToolHand();
	void hotKeySelectToolResize();
	void hotKeySelectToolEntityDraw();
	void hotKeySelectToolEntityMove();
	void hotKeySelectToolNotes();
	void hotKeyZoomIn();
	void hotKeyZoomOut();
	void hotKeyCenterCameraOnLevel();
	void hotKeyFitLevelInScreen();
	void hotKeyEnterPressed();
	void hotKeySelectLevelSettings();
	void hotKeySelectPlayerBot();
	void hotKeyToggleMenuItem(Gwen::Controls::Base* p_sender);
	
	
	// UI handlers
	void onClearTiles();
	void onClearEntities();
	void onClearWholeLevel();
	void onNewLevel();
	void onSaveLevel();
	void onSaveLevelAs();
	void onPublishToWorkshop();
	void onCloseEditor();
	void onReloadAll();
	void onExitApp();
	void selectedTool           (Gwen::Controls::Base* p_sender);
	void selectedMission        (Gwen::Controls::Base* p_sender);
	void selectedPaintTile      (Gwen::Controls::Base* p_sender);
	void selectedEntityInLibrary(Gwen::Controls::Base* p_sender);
	
	void onShowThemeTilesChanged       (Gwen::Controls::Base* p_sender);
	void onShowStatusBarChanged        (Gwen::Controls::Base* p_sender);
	void onGameLayerVisibilityChanged  (Gwen::Controls::Base* p_sender);
	void onEntityRenderFlagChanged     (Gwen::Controls::Base* p_sender);
	void onShowEditToolsChanged        (Gwen::Controls::Base* p_sender);
	void onShowSelectMissionChanged    (Gwen::Controls::Base* p_sender);
	void onAutoSyncEntityChangesChanged(Gwen::Controls::Base* p_sender);
	
	void onSelectLevelBackground(Gwen::Controls::Base* p_sender);
	void onSelectLevelTheme(Gwen::Controls::Base* p_sender);
	void onSelectDefaultMission(Gwen::Controls::Base* p_sender);
	
	void onOpenThemeColorPicker();
	void onOpenPublishedLevelBrowser();
	void onOpenSteamWorkshopTest();
	void onOpenDebugSteamCloudFileManager();
	
	void onLoadLevelDialog_FilterSelected  (Gwen::Controls::Base* p_sender);
	void onLoadLevelDialog_FilterDeselected(Gwen::Controls::Base* p_sender);
	void onLoadLevelDialog_OpenClicked     (Gwen::Controls::Base* p_sender);
	
	void onNewLevelDialogClosed      (Gwen::Controls::Base* p_sender);
	void onSaveChangesDialogClosed   (Gwen::Controls::Base* p_sender);
	void onSaveAsDialogClosed        (Gwen::Controls::Base* p_sender);
	void onSavePlayTestAsDialogClosed(Gwen::Controls::Base* p_sender);
	void onOpenEditorDuringPlayTestDialogClosed(Gwen::Controls::Base* p_sender);
	void onModalDialogClosed         (Gwen::Controls::Base* p_sender);
	void onSaveBeforeLevelLoadFromGameDialogClosed(Gwen::Controls::Base* p_sender);
	
	// No copying
	Editor(const Editor&);
	Editor& operator=(const Editor&);
	
	
	tt_ptr<Editor>::weak m_this;
	
	bool m_editorActive;
	bool m_pointerAutoVisibility;
	bool m_pointerVisible;
	bool m_doingPlayTest; // The level wasn't changed (resaved) after starting a play test.
	
	// Shared state that needs to be restored to original settings when closing the editor
	struct RestoreOnClose
	{
		u32                              targetFps;
		
		inline RestoreOnClose()
		:
		targetFps(0)
		{ }
	};
	RestoreOnClose m_restoreOnClose;
	
	EditorSettings m_persistentSettings;
	
	level::LevelDataPtr m_levelData;
	BorderPtr           m_levelBorder;
	
	tt::gwen::RootCanvasWrapper m_gwenRoot;
	UI                          m_ui;
	hotkey::HotKeyMgr           m_hotKeyMgr;
	ui::SvnCommandsPtr          m_svnCommands;
	DialogBoxes                 m_openDialogBoxes;
	
	bool m_showLoadLevelThisFrame;
	bool m_loadLevelDialogHasBeenOpen;  // whether the dialog was opened at least once this run
	
	// Signalling variable for first editor startup:
	// list of entities whose properties were updated by script
	level::entity::EntityInstanceSet    m_entitiesUpdatedByScript;
	bool                                m_showEntitiesUpdatedDialog;
	tt::engine::renderer::QuadSpritePtr m_propertiesUpdatedOverlay;
	
	tools::ToolPtr       m_tools[tools::ToolID_Count];
	tools::ToolID        m_activePaintTool;
	tools::ToolID        m_previousPaintTool;
	tools::ToolID        m_clickLockedTool[PointerButton_Count];
	bool                 m_switchToPreviousToolOnPointerRelease;
	
	tt::math::Point2     m_axisLockStartScreenPos;
	LockAxis             m_lockAxis;
	tt::input::Button    m_axisLockTrigger;
	
	std::string     m_paintEntityType; //!< Entity type to paint (entities) with.
	
	TileLayerMode        m_tileLayerMode;
	level::CollisionType m_paintTile;     //!< CollisionType to use with tile editing tools.
	level::ThemeType     m_paintTheme;    //!< ThemeType to use with tile editing tools.
	tt::math::PointRect  m_selectionRect; //!< In tiles
	tt::math::PointRect  m_levelRect;     //!< Rectangle for the entire level (in tiles)
	
	AttributeDebugViewPtr               m_collisionTileView;
	AttributeDebugViewPtr               m_themeTileView;
	
	level::LevelSectionPtr              m_floatingSection;
	AttributeDebugViewPtr               m_floatingSectionVisual;
	AttributeDebugViewPtr               m_floatingSectionThemeVisual;
	tt::engine::renderer::QuadSpritePtr m_floatingSectionBackground;
	
	tt::engine::renderer::QuadSpritePtr          m_selectionQuadTop;
	tt::engine::renderer::QuadSpritePtr          m_selectionQuadBottom;
	tt::engine::renderer::QuadSpritePtr          m_selectionQuadLeft;
	tt::engine::renderer::QuadSpritePtr          m_selectionQuadRight;
	tt::engine::renderer::TrianglestripBufferPtr m_selectionOutline;
	
	tt::engine::renderer::QuadSpritePtr m_levelSavedIndicator;
	tt::engine::renderer::QuadSpritePtr m_levelSaveFailedIndicator;
	
	tt::engine::renderer::QuadSpritePtr m_helpTextPickEntity;
	tt::engine::renderer::QuadSpritePtr m_helpTextEditNote;
	
	std::wstring m_statusTextSaved;
	std::wstring m_statusTextUnsaved;
	
	/*! \brief Whether the pointer was on the painting canvas (not the editor GUI or outside the game window). */
	bool m_pointerWasOnCanvas;
	
	tt::input::Button m_spacebarScrollMode;
	
	EditorMode m_editorMode;
	
	tt::str::StringSet         m_entityPickTypeFilter;
	std::string                m_entityPickPropertyName;
	level::entity::EntityIDSet m_entityPickDisallowedIDs;
	BorderPtr                  m_entityPickModeIndicator;
	s32                        m_entityPickCount;
	s32                        m_entityPickMaxCount;
	
	level::NotePtr                     m_focusNote;
	level::NotePtr                     m_noteForTextEdit;
	bool                               m_addTextEditNoteToLevelOnAccept;
	commands::CommandChangeNoteTextPtr m_noteTextChangeCommand;
	
	tt::undo::UndoStackPtr m_undoStack;
	
	std::string m_loadLevelFromGameName;
	
	CameraMgr m_cameraMgr;
	
	// Platform-specific hackery: native OS cursors
#if defined(TT_PLATFORM_SDL)
tt::input::SDLMouseCursorPtr m_editCursors[EditCursor_Count];
#elif defined(TT_PLATFORM_WIN)
	HCURSOR m_editCursors[EditCursor_Count];
#elif defined(TT_PLATFORM_OSX_MAC)
	tt::input::MouseCursorPtr m_editCursors[EditCursor_Count];
#endif
	EditCursor m_activeCursor;
};

// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_GAME_EDITOR_EDITOR_H)
