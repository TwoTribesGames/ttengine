#if !defined(INC_TOKI_GAME_GAME_H)
#define INC_TOKI_GAME_GAME_H


#include <vector>

#include <tt/engine/scene2d/shoebox/fwd.h>
#include <tt/engine/scene2d/shoebox/shoebox_types.h>
#include <tt/engine/scene2d/shoebox/shoebox.h>
#include <tt/input/Button.h>
#include <tt/pres/fwd.h>
#include <tt/thread/thread.h>
#include <tt/thread/Semaphore.h>
#include <tt/thread/Mutex.h>

#include <toki/effects/fwd.h>
#include <toki/game/editor/fwd.h>
#include <toki/game/entity/effect/fwd.h>
#include <toki/game/entity/fwd.h>
#include <toki/game/event/fwd.h>
#include <toki/game/fluid/fwd.h>
#include <toki/game/hud/fwd.h>
#include <toki/game/light/fwd.h>
#include <toki/game/pathfinding/PathMgr.h>
#include <toki/game/CameraMgr.h>
#include <toki/game/fwd.h>
#include <toki/game/types.h>
#include <toki/game/StartInfo.h>
#include <toki/input/Controller.h>
#include <toki/level/entity/fwd.h>
#include <toki/level/skin/fwd.h>
#include <toki/level/fwd.h>
#include <toki/level/types.h>
#include <toki/pres/fwd.h>
#include <toki/serialization/SerializationMgr.h>
#include <toki/utils/SectionProfiler.h>
#include <toki/utils/AssetMonitor.h>
#include <toki/constants.h>
#include <toki/ScreenshotSettings.h>


#define ENABLE_RENDER_SECTIONS 1

#if ENABLE_RENDER_SECTIONS
	#define START_RENDER_FRAME() \
				m_renderSectionProfiler.startFrameUpdate()
	#define START_RENDER_SECTION(p_section) \
				m_renderSectionProfiler.startFrameUpdateSection(p_section)
	#define STOP_RENDER_FRAME() \
				m_renderSectionProfiler.stopFrameUpdate()
	
#else
	#define START_RENDER_FRAME(...)
	#define START_RENDER_SECTION(...)
	#define STOP_RENDER_FRAME(...)
#endif


namespace toki {
namespace game {

class Game
{
public:
	Game();
	~Game();
	
	void init(const StartInfo& p_startInfo, ProgressType p_progressTypeOverride);
	void initOnRenderThread();
	
	static void loadLevel(const std::string& p_levelName, ProgressType p_overrideProgressType = ProgressType_Invalid);
	static void startMission(const std::string& p_missionID);
	
	/*! \brief Called right before the game is first updated/rendered: when the load screen has finished its work. */
	void onLoadScreenComplete();
	
	void update(real p_deltaTime);
	void updateForRender(real p_deltaTime);
	void render();
	
	void onResetDevice();
	void onRequestReloadAssets();
	
	void onPreDestroy();
	
	void handleLevelResized();
	void handleLevelBackgroundPicked();  // called by Editor when a Level Background has been selected
	
	void showLoadLevelDialog();
	
	void onEditorWillOpen(); // Called before editor opens (but after it is created)
	void onEditorClosed(bool p_restartLevel); // Called after editor was closed
	void onEditorReloadAll(); // Editor requests to reload everything
	void clearEditorWarnings();
	void editorWarning(const entity::EntityHandle& p_sourceEntity, const std::string& p_warningStr);
	
	bool handleUserLevelCompleted();
	bool isDoingPlayTest() const;
	
#if defined(TT_PLATFORM_WIN) && !defined(TT_BUILD_FINAL)
	/*! \brief Saves the level skin shoebox as an XML file (in the appropriate auto-determined path).
	    \param p_generateSkin Whether to generate a fresh level skin before saving. */
	bool saveLevelSkinAsXML(bool p_generateSkin) const;
#endif
	
	// 'Game layer' visibility:
	inline const GameLayers& getLayersVisibility() const { return m_layersVisible; }
	inline       GameLayers& modifyLayersVisibility()    { return m_layersVisible; }
	inline void setGameLayerVisible(GameLayer p_layer, bool p_visible) { m_layersVisible.setFlag(p_layer, p_visible); }
	
	inline void forceReload()   { m_forceReload = true;    }
	
	inline Camera& getCamera()                           { return m_cameraMgr.getCamera();             }
	inline bool    isDrcCameraEnabled() const            { return m_cameraMgr.isDrcCameraEnabled();    }
	inline void    setDrcCameraEnabled(bool p_enabled)   { m_cameraMgr.setDrcCameraEnabled(p_enabled); }
	inline bool    isDrcCameraMain() const               { return m_cameraMgr.isDrcCameraMain();       }
	inline void    setDrcCameraAsMain(bool p_enable)     { m_cameraMgr.setDrcCameraAsMain(p_enable);   }
	inline Camera& getDrcCamera()                        { return m_cameraMgr.getDrcCamera();          }
	inline Camera& getInputCamera()                      { return m_cameraMgr.getInputCamera();        }
	inline void setColorGradingAfterHud(bool p_afterHud) { m_colorGradingAfterHud = p_afterHud;        }
	inline bool isColorGradingAfterHud() const           { return m_colorGradingAfterHud;              }
	
	      Minimap& getMinimap();
	const Minimap& getMinimap() const;
	
	inline const StartInfo& getStartInfo() const { return m_startInfo; }
	inline void          setProgressOverride(ProgressType p_progressType) { m_progressTypeOverride = p_progressType; }
	inline ProgressType  getProgressOverride() const { return m_progressTypeOverride; }
	
	// NOTE: Only meant to be used by the editor!
	void setStartInfo(const StartInfo& p_newStartInfo);
	
	void scheduleScreenshot(const ScreenshotSettings& p_settings, real p_delay);
	void unscheduleScreenshot();  // cancel the scheduled screenshot
	inline bool hasScheduledScreenshot() const
	{ return m_scheduledScreenshotDelay > 0.0f && m_scheduledScreenshot.type != ScreenshotType_None; }
	
	inline const level::LevelDataPtr& getLevelData() { TT_NULL_ASSERT(m_levelData); return m_levelData; }
	
	inline const tt::engine::scene2d::shoebox::ShoeboxPtr& getShoebox() const { return m_shoeboxSkinAndEnvironment; }
	
	const level::AttributeLayerPtr&     getAttributeLayer();
	inline level::TileRegistrationMgr&  getTileRegistrationMgr()   { TT_NULL_ASSERT(m_tileRegistrationMgr);   return *m_tileRegistrationMgr;   }
	inline entity::EntityMgr&           getEntityMgr()             { TT_NULL_ASSERT(m_entityMgr);             return *m_entityMgr;             }
	inline bool                         hasEntityMgr()             { return m_entityMgr != 0; }
	// Martijn: not needed for RIVE
	/*
	inline event::EventMgr&             getEventMgr()              { TT_NULL_ASSERT(m_eventMgr);              return *m_eventMgr;              }
	inline event::SoundGraphicsMgr&     getSoundGraphicsMgr()      { TT_NULL_ASSERT(m_soundGraphicsMgr);      return *m_soundGraphicsMgr;    }
	// */
	inline fluid::FluidMgr&             getFluidMgr()              { TT_NULL_ASSERT(m_fluidMgr);              return *m_fluidMgr;              }
	inline light::LightMgr&             getLightMgr()              { TT_NULL_ASSERT(m_lightMgr);              return *m_lightMgr;              }
	inline light::DarknessMgr&          getDarknessMgr()           { TT_NULL_ASSERT(m_darknessMgr);           return *m_darknessMgr;           }
	inline pres::PresentationObjectMgr& getPresentationObjectMgr() { TT_NULL_ASSERT(m_presentationObjectMgr); return *m_presentationObjectMgr; }
	inline pathfinding::PathMgr&        getPathMgr()               { return m_pathMgr; }
	
#if defined(TT_STEAM_BUILD)
	void createWorkshopLevelPicker();
	void openWorkshopLevelPicker();
	void closeWorkshopLevelPicker();
	inline const hud::WorkshopLevelPickerPtr& getWorkshopLevelPicker() { return m_workshopLevelPicker; }
#endif
	
	void createResolutionPicker();
	void openResolutionPicker();
	void closeResolutionPicker();
	inline const hud::ResolutionPickerPtr& getResolutionPicker() { return m_resolutionPicker; }
	
	entity::effect::EffectMgr& getEffectMgr();
	      entity::effect::ColorGradingEffectMgr& getColorGradingEffectMgr();
	const entity::effect::ColorGradingEffectMgr& getColorGradingEffectMgr() const;
	      entity::effect::FogEffectMgr& getFogEffectMgr();
	const entity::effect::FogEffectMgr& getFogEffectMgr() const;
	
	inline bool hasEntityMgr()             const { return m_entityMgr             != 0; }
	inline bool hasPresentationObjectMgr() const { return m_presentationObjectMgr != 0; }
	
	inline const tt::pres::PresentationMgrPtr& getPresentationMgr(ParticleLayer p_layer = ParticleLayer_InFrontOfEntities)
	{ return m_presentationMgr[p_layer]; }
	
#if defined(TT_PLATFORM_WIN)
	inline DebugView&      getDebugView()      { TT_NULL_ASSERT(m_debugView); return *m_debugView; }
#endif
	
	/*! \brief Returns the amount of memory used by the environment shoebox textures (in bytes). */
	inline s32 getEnvironmentShoeboxTexMemSize() const { return m_shoeboxSkinAndEnvironmentTexMemSize; }
	inline s32 getShadowMaskShoeboxTexMemSize()  const { return m_shadowMaskShoeboxTexMemSize;  }
	
	inline real64 getGameTimeInSeconds() const { return m_gameTimeInSeconds; }
	
	void    addScreenSpaceEntity(const entity::EntityHandle& p_controllingEntity);
	void removeScreenSpaceEntity(const entity::EntityHandle& p_controllingEntity);
	
	void    addButtonInputListeningEntity(const entity::EntityHandle& p_entity, s32 p_priority, bool p_isBlocking);
	void removeButtonInputListeningEntity(const entity::EntityHandle& p_entity);
	void removeAllButtonInputListeningEntities();
	
	void    addMouseInputListeningEntity(const entity::EntityHandle& p_entity, s32 p_priority, bool p_isBlocking);
	void removeMouseInputListeningEntity(const entity::EntityHandle& p_entity);
	void removeAllMouseInputListeningEntities();
	
	void    addKeyboardListeningEntity(const entity::EntityHandle& p_entity, s32 p_priority, bool p_isBlocking);
	void removeKeyboardListeningEntity(const entity::EntityHandle& p_entity);
	void removeAllKeyboardListeningEntities();
	
	inline bool shouldForceReload() const { return m_forceReload; }
	
	inline void addShoeboxPlaneToShoebox(const tt::engine::scene2d::shoebox::PlaneData& p_shoeboxPlane)
	{
		TT_NULL_ASSERT(m_shoeboxDataFromScript);
		m_shoeboxDataFromScript->planes.push_back(p_shoeboxPlane);
		// TODO: Add assert to make sure the add time is correct. (Not too late or early.)
	}
	
	inline void addShoeboxInclude(const std::string& p_filename, const tt::math::Vector3& p_offset, s32 p_priority, real p_scale)
	{
		TT_NULL_ASSERT(m_shoeboxDataIncludesFromScript);
		m_shoeboxDataIncludesFromScript->includes.push_back(tt::engine::scene2d::shoebox::IncludeData(p_filename, p_offset, p_priority, p_scale));
		// TODO: Add assert to make sure the add time is correct. (Not too late or early.)
	}
	
	tt::math::Vector3 getShoeboxSpaceFromWorldPos(const tt::math::Vector2& p_worldPosition) const;
	
	level::ThemeType getThemeAtTilePosition(const tt::math::Point2& p_position) const;
	inline void setThemeAtTilePosition(const tt::math::Point2& p_position, level::ThemeType p_type)
	{
		m_levelOverriddenThemeTiles[p_position] = p_type;
	}
	
	const level::skin::TileMaterial& getTileMaterial(const tt::math::Point2& p_tile) const;
	
	/*! \brief Causes the full game state to be serialized at the end of the current frame. */
	void serializeGameState(const std::string& p_id);
	
	void unserializeGameState(const std::string& p_id, bool p_removeCheckpointAfterRestore = false, ProgressType p_progressType = ProgressType_Invalid);
	
	void serializeAll  (      toki::serialization::SerializationMgr& p_serializationMgr) const;
	void unserializeAll(const toki::serialization::SerializationMgr& p_serializationMgr,
	                    const std::string& p_serializationID);
	
	static StartInfo loadStartInfo(const toki::serialization::SerializationMgr& p_serializationMgr,
	                               bool&                                        p_success_OUT);
	
	inline void setStopLightRenderingOnSplit(bool p_stopOnSplit) { m_stopLightRenderingOnSplit = p_stopOnSplit; }
	
	void queueShoeboxTagEvent(const std::string& p_tag, const std::string& p_event, const std::string& p_param);
	
	void setMissionID(const std::string& p_missionID);
	inline const std::string& getMissionID() const { return m_startInfo.getMissionID(); }
	
	void resetAndDisableDebugFOV();
	
	// Callbacks that can be received from the game itself
	void onGameStarted();
	void onGameEnded();
	void onGamePaused();
	void onGameResumed();
	void onGameProgressChanged(s32 p_progress);
	bool onGamePastIntro();
	
private:
	struct EntityPressInfo
	{
		inline explicit EntityPressInfo(const entity::EntityHandle& p_entityHandle = entity::EntityHandle())
		:
		entityHandle(p_entityHandle),
		pressDuration(0.0f)
		{ }
		
		entity::EntityHandle entityHandle;
		real                 pressDuration;
	};
	typedef std::vector<EntityPressInfo> PressingEntities;
	
	struct EditorWarning
	{
		s32         id;
		std::string warningStr;
		
		EditorWarning(s32 p_id, const std::string& p_warningStr)
		:
		id(p_id),
		warningStr(p_warningStr)
		{
		}
	};
	typedef std::vector<EditorWarning> EditorWarnings;
	
	struct InputListener
	{
		InputListener(const entity::EntityHandle& p_entity, s32 p_priority, bool p_isBlocking)
		:
		entity(p_entity),
		priority(p_priority),
		isBlocking(p_isBlocking)
		{}
		
		bool operator < (const InputListener& p_rhs) const
		{
			return (priority < p_rhs.priority);
		}
		
		entity::EntityHandle entity;
		s32                  priority;
		bool                 isBlocking;
	};
	
	typedef std::vector<InputListener> PrioritizedInputListeners;
	
	void setupScriptButtons();
	void loadLevel(const StartInfo& p_startInfo);
	void loadLevelData(const std::string& p_filename);
	void loadUserLevelShoeboxData();
	void loadLevelScript(const StartInfo& p_startInfo);
	void resetParticleMgr();
	void handleLevelChanged();
	tt::engine::scene2d::shoebox::ShoeboxDataPtr generateLevelSkinData() const;
	tt::engine::renderer::EngineIDToTextures createEmptyShoeboxesWithPreviousSettings();
	void createShoeboxesFromDataInclSkin();
	void buildPathFindingData();
	void loadPathFindingData();
	
	tt::engine::scene2d::shoebox::ShoeboxDataPtr loadShoeboxData(const std::string& p_filename);
	void addDataToShoebox(const tt::engine::scene2d::shoebox::ShoeboxPtr& p_shoebox, 
	                      const tt::engine::scene2d::shoebox::ShoeboxDataPtr& p_data,
	                      bool p_useLevelSize);
	
	/*! \param p_overridePositionEntityID If passed (and a non-negative number),
	                                      the position of this entity (identified by level data ID)
	                                      will be overridden during creation.
	    \param p_overridePosition The position to use instead of the position from level data,
	                              if p_overridePositionEntityID is specified. */
	void createLevelEntities(s32                      p_overridePositionEntityID = -1,
	                         const tt::math::Vector2& p_overridePosition         = tt::math::Vector2::zero,
	                         bool                     p_gameReloaded             = false);
	tt::pres::PresentationMgrPtr createPresentationMgr() const;
	
	entity::Entity* getDraggingEntity();
	void releaseDraggingEntity();
	void startDraggingEntity(entity::EntityHandle p_handle);
	
	void updatePressingEntities(real p_deltaTime,
	                            const tt::math::Vector2& p_screenspacePointerPos,
	                            const tt::math::Vector2& p_pointerWorldPos,
	                            bool p_doRelease);
	
	void toggleEditor();
	void setEditorOpen(bool p_open, bool p_userLevelCompleted = false);
	bool isEditorOpen() const;
	
	void syncRuntimeEntitiesWithLevelData();
	
	void reloadGame(bool p_entitiesOnly, bool p_restoreFollowEntityPosition);
	
	void renderHud(  bool p_isRenderingToDRC, bool p_isRenderingMainCam);
	void renderDebug(bool p_isRenderingToDRC) const;
	
	void serialize  (      toki::serialization::SerializationMgr& p_serializationMgr) const;
	void unserialize(const toki::serialization::SerializationMgr& p_serializationMgr);
	void callOnProgressRestoredOnAllEntities(const std::string& p_serializationID) const;
	void callOnReloadRequestedOnAllEntities() const;
	CheckPointMgr& getCheckPointMgr();
	
	// No copying
	Game(const Game&);
	Game& operator=(const Game&);
	
	
	CameraMgr m_cameraMgr;
	
	MinimapPtr m_minimap;
	
	StartInfo             m_startInfo;
	ProgressType          m_progressTypeOverride;
	level::LevelDataPtr   m_levelData;
	AttributeDebugViewPtr m_attribDebugView;
	DebugViewPtr          m_debugView;
	
	tt::engine::scene2d::shoebox::ShoeboxDataPtr m_shadowMaskShoeboxData;
	tt::engine::scene2d::shoebox::ShoeboxDataPtr m_shoeboxDataEnvironment;
	tt::engine::scene2d::shoebox::ShoeboxDataPtr m_shoeboxDataLevelSkin;
	tt::engine::scene2d::shoebox::ShoeboxDataPtr m_shoeboxDataUserLevelBackground;
	tt::engine::scene2d::shoebox::ShoeboxDataPtr m_shoeboxDataFromScript;         // planes   from script are added here. These are NOT serialized.
	tt::engine::scene2d::shoebox::ShoeboxDataPtr m_shoeboxDataIncludesFromScript; // includes from script are added here. These are serialized.
	
	tt::engine::scene2d::shoebox::ShoeboxPtr     m_shadowMaskShoebox;
	s32                                          m_shadowMaskShoeboxTexMemSize;  // in bytes
	tt::engine::scene2d::shoebox::ShoeboxPtr     m_shoeboxSkinAndEnvironment;
	s32                                          m_shoeboxSkinAndEnvironmentTexMemSize;  // in bytes
	
	GameLayers m_layersVisible;
	
	level::skin::SkinContextPtr m_levelSkinContext;
	level::ThemeTiles           m_levelOverriddenThemeTiles;
	
	BorderPtr m_levelBorder;
	
	hud::DebugUIPtr m_debugUI;
#if defined(TT_STEAM_BUILD)
	hud::WorkshopLevelPickerPtr m_workshopLevelPicker;
#endif
	hud::ResolutionPickerPtr m_resolutionPicker;
	
	editor::EditorPtr              m_editor;
	level::entity::EntityInstanceSet m_entitiesInLevelDataOnEditorOpen;
	std::string                      m_levelBackgroundOnEditorOpen;
	level::AttributeLayerPtr         m_levelAttributeLayerOnEditorOpen;
	EditorWarnings                 m_editorWarnings;
	entity::EntityMgrPtr           m_entityMgr;
	// Martijn: not needed for RIVE
	/*
	event::EventMgrPtr             m_eventMgr;
	event::SoundGraphicsMgrPtr     m_soundGraphicsMgr;
	// */
	level::TileRegistrationMgrPtr  m_tileRegistrationMgr;
	tt::pres::PresentationMgrPtr   m_presentationMgr[ParticleLayer_Count];
	fluid::FluidMgrPtr             m_fluidMgr;
	light::LightMgrPtr             m_lightMgr;
	light::DarknessMgrPtr          m_darknessMgr;
	bool                           m_stopLightRenderingOnSplit;
	pathfinding::PathMgr           m_pathMgr;
	
	entity::effect::ColorGradingEffectMgrPtr m_colorGradingEffectMgr;
	entity::effect::FogEffectMgrPtr          m_fogEffectMgr;
	
	pres::PresentationObjectMgrPtr m_presentationObjectMgr;
	
#if !defined(TT_BUILD_FINAL)
	tt::engine::renderer::QuadBufferPtr m_colorTable;
	void createColorTable();
#endif
	
	entity::EntityHandle      m_draggingEntity;
	bool                      m_draggingEntityRestoreSuspendedState;
	bool                      m_directionalControlsEnabled;
	PressingEntities          m_pressingEntities;
	entity::EntityHandles     m_screenSpaceEntities;
	PrioritizedInputListeners m_buttonInputListeningEntities;
	PrioritizedInputListeners m_mouseInputListeningEntities;
	PrioritizedInputListeners m_keyboardListeningEntities;
	
	real64            m_gameTimeInSeconds;
	
	tt::input::Button m_spacebarScrollMode;
	bool              m_forceReload;
	
	bool m_reloadEntities;
	
	ScreenshotSettings m_scheduledScreenshot;
	real               m_scheduledScreenshotDelay;  // delay in seconds before taking screenshot
	
	enum SerializationAction
	{
		SerializationAction_None,        // Do nothing
		SerializationAction_Serialize,   // Serializes the game at the end of the current frame
		SerializationAction_Unserialize  // Unserializes the game at the start of the next frame
	};
	SerializationAction m_serializationAction;
	std::string         m_serializationID;
	ProgressType        m_serializationProgressType;
	bool                m_serializationRemoveAfterUnserialize;
	
	utils::SectionProfiler<utils::FrameUpdateSection, utils::FrameUpdateSection_Count> m_updateSectionProfiler;
	utils::SectionProfiler<utils::FrameUpdateForRenderSection, utils::FrameUpdateForRenderSection_Count> m_updateForRenderSectionProfiler;
#if ENABLE_RENDER_SECTIONS
	mutable utils::SectionProfiler<utils::FrameRenderSection, utils::FrameRenderSection_Count> m_renderSectionProfiler;
#endif
	
	effects::ColorGradingPtr m_colorGrading;
	bool                     m_colorGradingAfterHud;
	
	//////////////////////////////
	// Shoebox update thread
	
	static int staticUpdateShoeboxThread(void* p_arg);
	int  updateShoeboxThread();
	void updateShoebox(real p_deltaTime);
	void applyColorGrading();
	
	tt::thread::handle    m_shoeboxThread;
	tt::thread::Semaphore m_startShoeboxUpdate;
	tt::thread::Semaphore m_finishedShoeboxUpdate;
	bool                  m_threadShouldExit;
	real                  m_deltaTime;
	
	struct ShoeboxTagEvent
	{
		std::string tag;
		std::string event;
		std::string param;
	};
	typedef std::vector<ShoeboxTagEvent> ShoeboxTagEvents;
	
	ShoeboxTagEvents  m_queuedEvents;
	tt::thread::Mutex m_eventsMutex;
	
	//////////////////////////////
	// Asset Monitor
	
	utils::AssetMonitorPtr m_assetMonitor;
	
	//////////////////////////////
	// Debug
	
	real m_debugFOVDelta;
};

// Namespace end
}
}


#endif  // !defined(INC_TOKI_GAME_GAME_H)
