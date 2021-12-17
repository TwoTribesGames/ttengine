#if !defined(INC_TOKI_GAME_SCRIPT_BINDINGS_H)
#define INC_TOKI_GAME_SCRIPT_BINDINGS_H


#include <tt/math/Vector2.h>
#include <tt/script/VirtualMachine.h>
#include <tt/score/Score.h>

#include <toki/game/script/wrappers/EntityWrapper.h>
#include <toki/game/script/wrappers/MusicTrackWrapper.h>
#include <toki/game/fluid/types.h>
#include <toki/input/types.h>
#include <toki/level/types.h>
#include <toki/constants.h>


namespace toki {
namespace game {
namespace script {

/*! \brief Functions globally available in Squirrel scripts. */
class Bindings
{
public:
	// Binds all functions in this class
	static void bindAll(const tt::script::VirtualMachinePtr& p_vm);
	
	/*! \brief Sets the scale for the fixed delta time. NOTE: Use with caution!
	    \param p_scale The scale. Should be > 0 and preferably not much larger than 1.0. */
	static void setFixedDeltaTimeScale(real p_scale);
	
	/*! \brief Returns the scale for the fixed delta time. */
	static real getFixedDeltaTimeScale();
	
	/*! \brief Sets the rendering of the color grading before or after the HUD render pass */
	static void setColorGradingAfterHud(bool p_afterHud);
	
	/*! \brief Gets the rendering of the color grading before or after the HUD render pass */
	static bool isColorGradingAfterHud();
	
	/*! \brief Returns The direction of the left stick. */
	static tt::math::Vector2 getLeftStick();
	
	/*! \brief Returns The direction of the right stick. */
	static tt::math::Vector2 getRightStick();
	
	/*! \brief Returns whether mouse position is valid. */
	static bool isMousePositionValid();
	
	/*! \brief Returns the mouse position in world space.
	    \note Use isMousePositionValid() to see if it's valid. */
	static tt::math::Vector2 getMouseWorldPosition();
	
	/*! \brief Returns whether the left mouse button is down. */
	static bool isMouseLeftDown();
	
	/*! \brief Returns whether the right mouse button is down. */
	static bool isMouseRightDown();
	
	/*! \brief Returns the current display aspect ratio. */
	static real getAspectRatio();
	
	/*! \brief Returns the current display screen width. */
	static s32 getScreenWidth();
	
	/*! \brief Returns the current display screen height. */
	static s32 getScreenHeight();
	
	/*! \brief Returns a sorted array containing all names of the presentation files (without the extension) */
	static tt::str::Strings getPresentationNames();
	
	/*! \brief Returns a sorted array containing all level names */
	static tt::str::Strings getLevelNames();
	
	/*! \brief Returns a sorted array containing all particle effect names */
	static tt::str::Strings getParticleEffectNames();
	
	/*! \brief Returns a sorted array containing all entity names */
	static tt::str::Strings getEntityNames();
	
	/*! \brief Returns a sorted array containing all color grading effect names. */
	static tt::str::Strings getColorGradingNames();
	
	/*! \brief Returns a sorted array containing all light texture names. */
	static tt::str::Strings getLightNames();
	
	/*! \brief Returns a sorted array containing all the level\shoebox_include shoebox names. */
	static tt::str::Strings getShoeboxIncludeNames();
	
	/*! \brief Returns a sorted array containing all sound cue names */
	static tt::str::Strings getSoundCueNames();
	
	/*! \brief Returns a sorted array containing all sound cue names in the specified sound banks. */
	static tt::str::Strings getSoundCueNamesInBanks(const tt::str::Strings& p_soundBankNames);
	
	/*! \brief Returns a sorted array containing all music track names */
	static tt::str::Strings getMusicTrackNames();
	
	/*! \brief Returns a sorted array containing all audio category names. */
	static tt::str::Strings getAudioCategoryNames();
	
	/*! \brief Returns a sorted array containing all reverb effect names. */
	static tt::str::Strings getReverbEffectNames();
	
	/*! \brief Indicates whether the game is running in "level editor mode" (application was started as a level editor). */
	static bool isInLevelEditorMode();
	
	/*! \brief Indicates whether the game is running in demo mode. */
	static bool isInDemoMode();
	
	/*! \brief Indicates whether the game started up with --level. */
	static bool isInLevelMode();
	
	/*! \brief Indicates whether the game started up with --mission. */
	static bool isInMissionMode();
	
	/*! \brief Indicates whether the game is built with Steam features enabled. */
	static bool isSteamBuild();
	
	/*! \brief Returns an array of entities that are registered with the specified tag name.
	    \param p_tag The tag name to find the entities for. */
	static EntityBaseCollection getEntitiesByTag(const std::string& p_tag);
	
	/*! \brief Returns an entity with a specific id. Returns null if entity is not found.
	    \param p_id The entity id as presented in the editor */
	static EntityBase* getEntityByID(s32 p_id);
	
	/*! \brief Returns the type of the entity with an ID
		\param Entity ID [Integer] of the entity (e.g., 90321)
		\param Position [Vector2]
		\param Properties [Table] Contains the properties of this entity (optional)
		\return The entity type [string] or null if entity is not found */
	static SQInteger getEntityTypeByID(HSQUIRRELVM v);
	
	/*! \brief Returns the class of the entity with an ID
		\param Entity ID [Integer] of the entity (e.g., 90321)
		\param Position [Vector2]
		\param Properties [Table] Contains the properties of this entity (optional)
		\return The entity class [Squirrel class] or null if entity is not found */
	static SQInteger getEntityClassByID(HSQUIRRELVM v);
	
	/*! \brief Returns an entity with a specific handle value. Returns null if entity is not found.
	    \param p_handleValue The entity handle value, as returned by entity.getHandleValue() */
	static EntityBase* getEntityByHandleValue(s32 p_handleValue);
	
	/*! \brief Spawns an entity
		\param Type [string] of the entity (e.g., BerryBug)
		\param Position [Vector2]
		\param Properties [Table] Contains the properties of this entity (optional)
		\return The spawned entity */
	static SQInteger spawnEntity(HSQUIRRELVM v);
	
	/*! \brief Respawns an entity at a certain location */
	static EntityBase* respawnEntity(s32 p_id);
	
	/*! \brief Respawns an entity at a certain position  */
	static EntityBase* respawnEntityAtPosition(s32 p_id, const tt::math::Vector2& p_position);
	
	/*! \brief Kills an entity
		NOTE: Only entities that have been initialized can be killed.
		I.e., the to-be-killed entity should have received its onInit() callback.
		\param p_entity The entity that will be killed */
	static void killEntity(EntityBase* p_entity);
	
	/*! \brief Add an entity to the button input listeners. Higher priorities override lower priorities. */
	static void addButtonInputListeningEntity(EntityBase* p_entity, s32 p_priority);
	
	/*! \brief Add a input blocking entity to the button input listeners. Once a blocking button input listener is processed, no other input listeners
	     will be processed. Higher priorities override lower priorities. */
	static void addButtonInputBlockingListeningEntity(EntityBase* p_entity, s32 p_priority);
	
	/*! \brief Remove an entity from the button input listeners. */
	static void removeButtonInputListeningEntity(EntityBase* p_entity);
	
	/*! \brief Remove all entities from the button input listeners. */
	static void removeAllButtonInputListeningEntities();
	
	/*! \brief Add an entity to the mouse input listeners. Higher priorities override lower priorities. */
	static void addMouseInputListeningEntity(EntityBase* p_entity, s32 p_priority);
	
	/*! \brief Add a input blocking entity to the mouse input listeners. Once a blocking mouse input listener is processed, no other mouse input listeners
	     will be processed. Higher priorities override lower priorities. */
	static void addMouseInputBlockingListeningEntity(EntityBase* p_entity, s32 p_priority);
	
	/*! \brief Remove an entity from the mouse input listeners. */
	static void removeMouseInputListeningEntity(EntityBase* p_entity);
	
	/*! \brief Remove all entities from the mouse input listeners. */
	static void removeAllMouseInputListeningEntities();
	
	/*! \brief Add an entity to the keyboard listeners. Higher priorities override lower priorities. */
	static void addKeyboardListeningEntity(EntityBase* p_entity, s32 p_priority);
	
	/*! \brief Add a input blocking entity to the keyboard listeners. Once a blocking keyboard listener is processed, no other keyboard listeners
	     will be processed. Higher priorities override lower priorities. */
	static void addKeyboardBlockingListeningEntity(EntityBase* p_entity, s32 p_priority);
	
	/*! \brief Remove an entity from the keyboard listeners. */
	static void removeKeyboardListeningEntity(EntityBase* p_entity);
	
	/*! \brief Remove all entities from the keyboard listeners. */
	static void removeAllKeyboardListeningEntities();
	
	/*! \brief Returns app time in seconds. */
	static real getAppTimeInSeconds();
	
	/*! \brief Returns game time in seconds. */
	static real getGameTimeInSeconds();
	
	/*! \brief Get the current day in the month. */
	static s32 getDateDay();
	
	/*! \brief Get the current month. */
	static s32 getDateMonth();
	
	/*! \brief Get the current year. */
	static s32 getDateYear();
	
	/*! \brief Get the current date string. */
	static std::string getDateString();
	
	/*! \brief Get the current time string. */
	static std::string getTimeString();
	
	/*! \brief Get the current version string. */
	static std::string getVersion();
	
	/*! \brief Get the current region string. Returns empty string if region is WW. */
	static std::string getRegion();
	
	/*! \brief Returns the collision tile at the specified world position. */
	static level::CollisionType getCollisionType(const tt::math::Vector2& p_worldPos);
	
	/*! \brief Returns the collision tile at the specified world position.
	           This version only looks at the actual level tiles, not any dynamic (entity) collision tiles. */
	static level::CollisionType getCollisionTypeLevelOnly(const tt::math::Vector2& p_worldPos);
	
	/*! \brief Gets a ThemeType from a specified world position. */
	static level::ThemeType getThemeType(const tt::math::Vector2& p_worldPos);
	
	/*! \brief Sets a ThemeType at a specified world position. NOTE: This function must be called before generation of the skin! */
	static void setThemeType(const tt::math::Vector2& p_worldPos, level::ThemeType p_type);
	
	/*! \brief Add ShoeboxPlane to shoebox. !THIS IS NOT SERIALIZED! NOTE: This function must be called before the shoebox is created! */
	static void addShoeboxPlaneToShoebox(const wrappers::ShoeboxPlaneDataWrapper& p_shoeboxPlane);
	
	/*! \brief Add shoebox include to shoebox. This function must be called before the shoebox is created! (onCreate, onInit, onSpawn.) */
	static void addShoeboxInclude(const std::string& p_filename, const tt::math::Vector2& p_offset, real p_offsetZ, s32 p_priority, real p_scale);
	
	/*! \brief Translates a world position into shoebox space. */
	static tt::math::Vector2 getShoeboxSpaceFromWorldPos(const tt::math::Vector2& p_worldPosition);
	
	/*! \brief Sets the visibility of the tile attributes */
	static void setTileAttributesVisible(bool p_visible);
	
	/*! \brief Sends an event to a specific tag name in all shoeboxes.
	    \param p_tag The name of the tag (as specified in the shoebox XML).
	    \param p_event The event to send. For shoebox planes, this can be "show", "hide", "start", "stop", "reset", "pause", "resume", "set-directiontype", "set-timetype", "set-tweentype" 
	    \param p_param The parameter for the event. Only the "set-" events accept paramenters. */
	static void sendShoeboxTagEvent(const std::string& p_tag, const std::string& p_event,
	                                const std::string& p_param);
	
	/*! \brief Returns a sanitized name of the current user */
	static std::string getUsername();
	
	/*! \brief Sends a GET request to a URL.
	    \return -1 if request failed; otherwise it will return the request ID */
	static s32 sendGetRequest(const std::string& p_server, const std::string& p_url, const tt::str::Strings& p_params);
	
	/*! \brief Opens the specified URL. */
	static void openURL(const std::string& p_url);
	
	/*! \brief Opens the steam store page. */
	static void openStorePage();
	
	/*! \brief Returns whether online sessions (leaderboards etc) are allowed */
	static bool allowOnlineSessions();
	
	/*! \brief Returns whether user is part of the RIVE steam group. Returns null if status couldn't be checked */
	static SQInteger isSteamGroupMember(HSQUIRRELVM v);
	
	/*! \brief Let the game know this (user) level is completed. 
	    \return Whether the code handles the further flow. (e.g. If true, script should do nothing.) */
	static bool handleUserLevelCompleted();
	
	/*! \brief Place a pair of warps that are connected. Call this twice with the same name 
	           to create a pair. This will allow for non entities to warp through them (i.e., fluids) 
	    \param p_rect the rect of the warp
	    \param p_name the name of the warp */
	static void addFluidWarp(const tt::math::VectorRect& p_rect, const std::string& p_name);
	
	/*! \brief Remove a previously placed pair of warps that are connected.
	    \param p_name Name used during the placing of the warp pair. */
	static void deleteFluidWarpPair(const std::string& p_name);
	
	/*! \brief Start wave in fluids.
	    \param p_rect The rectangle used for position and width of the wave.
	    \param p_strength The strength of the wave.
	    \param p_duration The duration in seconds. */
	static void startWave(const tt::math::VectorRect& p_rect, real p_strength, real p_duration);
	
	/*! \brief Returns the current progress type. (Which may have been overridden by script.) */
	static ProgressType getCurrentProgressType();
	
	/*! \brief Returns whether or not a position is in light */
	static bool isPositionInLight(const tt::math::Vector2& p_position);
	
	/*! \brief Set the layers in the shoebox background where blur is applied (cumulative)
	    \param p_layers Array of negative Z values that identify the layers */
	static void setBackgroundBlurLayers(const std::vector<real>& p_layers);
	
	/*! \brief Set the layer in the shoebox foreground from where blur is applied (once)
	    \param p_layers Array of positive Z values that identify the layers */
	static void setForegroundBlurLayers(const std::vector<real>& p_layers);
	
	/*! \brief Get the layers in the shoebox background where blur is applied
	    \return Array of negative Z values that identify the layers */
	static std::vector<real> getBackgroundBlurLayers();
	
	/*! \brief Get the layers in the shoebox foreground where blur is applied
	    \return Array of positive Z values that identify the layers */
	static std::vector<real> getForegroundBlurLayers();
	
	/*! \brief Get the default fog color. */
	static const tt::engine::renderer::ColorRGB& getDefaultFogColor();
	
	/*! \brief Set the default fog color.
	    \param p_color The requested color for the fog.
	    \param p_duration Time in seconds for the transition from the current color to the requested color.
	    \param p_easingType the interpolation type that should be used for the transition. */
	static void setDefaultFogColor(const tt::engine::renderer::ColorRGB& p_color, real p_duration,
	                               tt::math::interpolation::EasingType p_easingType);
	
	/*! \brief Get the default fog near. */
	static real getDefaultFogNear();
	
	/*! \brief Get the default fog far. */
	static real getDefaultFogFar();
	
	/*! \brief Set the default fog near and far.
	    \param p_near The fog near plane distance.
	    \param p_far The fog far plane distance.
	    \param duration Time in seconds for the transition from the current color to the requested color.
	    \param p_easingType the interpolation type that should be used for the transition. */
	static void setDefaultFogNearFar(real p_near, real p_far, real p_duration,
	                                 tt::math::interpolation::EasingType p_easingType);
	
	/*! \brief Resets the fog (effectively disabling it) */
	static void resetFogSettings();
	
	/*! \brief Get a texture for color grading.
	    \param p_effectName The requested color grading effect.
	    \return TexturePtr.*/
	static wrappers::TextureWrapper getColorGradingTexture(const std::string& p_effectName);
	
	/*! \brief Get the default color grading effect texture. */
	static wrappers::TextureWrapper getDefaultColorGrading();
	
	/*! \brief Set the default color grading effect.
	    \param p_effectTexture The texture which should be used for the color grading effect
	    \param p_duration Time in seconds for the transition. */
	static void setDefaultColorGrading(const wrappers::TextureWrapper* p_effectTexture, real p_duration);
	
	/*! \brief Returns whether the color grading effects (default color grading is excluded) are enabled. */
	static bool areColorGradingEffectsEnabled();
	
	/*! \brief Set whether or not the color grading effects (default color grading is excluded) are enabled. */
	static void setColorGradingEffectsEnabled(bool p_enabled);
	
	/*! \brief Stores the game state + persistent data to disc.
	    \return true on successful save, false if failed */
	static bool storeGameState(bool p_waitForThreadToFinish);
	
	/*! \brief Modify the TV display gamma
	    \param p_gamma Relative gamma modifier (0.7 - 1.3)
	    NOTE: This value is multiplied by the system preference and applied to the display controller */
	static void setTVDisplayGamma(real p_gamma);
	
	/*! \brief Modify the DRC display gamma
	    \param p_gamma Relative gamma modifier (0.7 - 1.3)
	    NOTE: This value is multiplied by the system preference and applied to the display controller */
	static void setDRCDisplayGamma(real p_gamma);
	
	/*! \brief Is rumble enabled. */
	static bool isRumbleEnabled();
	
	/*! \brief Enable/disable rumble. */
	static void setRumbleEnabled(bool p_enabled);
	
	/*! \brief Make the game controller rumble.
	    \param p_strength The strength of the vibration.
	    \param p_duration For how long to rumble the controller, in seconds. Can be 1.0 (one second) at most.
	    \param p_panning The panning of the vibration. */
	static void startRumble(input::RumbleStrength p_strength, real p_duration, real p_panning);
	
	/*! \brief Stop rumbling the game controller. */
	static void stopRumble();
	
	/*! \brief Sets the lightbar color (PS4 only). */
	static void setLightBarColor(const tt::engine::renderer::ColorRGB& p_color);
	
	/*! \brief Shows the controller applet (NX only). */
	static void showControllerApplet();
	
	/*! \brief Allows a visible hardware pointer/cursor. */
	static bool isPointerAllowed();
	
	/*! \brief Allows a visible hardware pointer/cursor. */
	static void setPointerAllowed(bool p_allow);
	
	/*! \brief Returns if pointer is visible */
	static bool isPointerVisible();
	
	/*! \brief Sets the new player count */
	static void setPlayerCount(s32 p_newCount);
	
	/*! \brief Set the control scheme of the game controller.
	    \param p_scheme the control scheme */
	static void setGamepadControlScheme(input::GamepadControlScheme p_scheme);
	
	/*! \brief Get the control scheme of the game controller.*/
	static input::GamepadControlScheme getGamepadControlScheme();
	
	/*! \brief Gets the custom keybindings table.
		\return The custom keybindings table or null if it doesn't exist.*/
	static SQInteger getKeyBindingsTable(HSQUIRRELVM p_vm);
	
	/*! \brief Sets (and stores) the custom keybindings table.
		\return If succeeded */
	static SQInteger setKeyBindingsTable(HSQUIRRELVM p_vm);
	
	/*! \brief Gets the key name. */
	static std::string getKeyName(tt::input::Key p_key);
	
	/*! \brief Gets the meta data table.
		\return The meta data table or null if no meta data exists.*/
	static SQInteger getMetaData(HSQUIRRELVM p_vm);
	
	/*! \brief Is minimap enabled. (Note we also have isMinimapHidden.)*/
	static bool isMinimapEnabled();
	
	/*! \brief Is minimap hidden. (Note we also have isMinimapEnabled.)*/
	static bool isMinimapHidden();
	
	/*! \brief Enabled minimap. (Note we also have setMinimapHidden.)*/
	static void setMinimapEnabled(bool p_enabled);
	
	/*! \brief Enabled minimap. (Note we also have setMinimapEnabled.)*/
	static void setMinimapHidden( bool p_hidden);
	
	/*! \brief Set minimap Y offset. (screenspace)*/
	static void setMinimapYOffset(real p_offset);
	
	/*! \brief Get minimap Y offset. (screenspace)*/
	static real getMinimapYOffset();
	
	/*! \brief Set minimap side border size. (screenspace)*/
	static void setMinimapSideBorderSize(real p_size);
	
	/*! \brief Get minimap side border size. (screenspace)*/
	static real getMinimapSideBorderSize();
	
	/*! \brief Set largest level width so minimap can used this. */
	static void setMinimapLargestLevelWidth(s32 p_size);
	
	/*! \brief Get the size of the minimap graphics. (Screenspace)*/
	static real getMinimapGraphicsSize();
	
	/*! \brief Set the size of the minimap graphics. (Screenspace)*/
	static void setMinimapGraphicsSize(real p_size);
	
	/*! \brief Get the extra size we add to the sides of the minimap graphics (center quad). (Screenspace)*/
	static real getMinimapExtraWidth();
	
	/*! \brief Set the extra size we add to the sides of the minimap graphics (center quad). (Screenspace)*/
	static void setMinimapExtraWidth(real p_size);
	
	/*! \brief Get the size of the fade quads (to the left and rigth of the center plane) of the minimap graphics (center quad). (Screenspace)*/
	static real getMinimapSideFadeWidth();
	
	/*! \brief Set the size of the fade quads (to the left and rigth of the center plane) of the minimap graphics (center quad). (Screenspace)*/
	static void setMinimapSideFadeWidth(real p_size);
	
	/*! \brief Creates a music track. This preloads the music data, but does NOT play the track yet.
	    \param p_musicName The name of the music to load. */
	static wrappers::MusicTrackWrapper createMusicTrack(const std::string& p_musicName);
	
	/*! \brief Destroys a music track. This stops the track and unloads its data
	           (as well as actually destroying the MusicTrack object). */
	static void destroyMusicTrack(wrappers::MusicTrackWrapper& p_track);
	
	/*! \brief Sets the startup level in case restoring the previous session fails. (Game crashed or updated?)
	           If this level is not set or if the level name doesn't exist (anymore), the game will use the
	           "<startup><restore_failure_level>" property defined in config.xml */
	static void setRestoreFailureLevel(const std::string& p_levelName);
	
	/*! \brief Gets the split priority of the environment and skin shoebox */
	static s32 getShoeboxSplitPriority();
	
	/*! \brief Sets the split priority of the environment and skin.
	           NOTE: This function must be called before generation of the shoeboxes! */
	static void setShoeboxSplitPriority(s32 p_priority);
	
	/*! \brief Resets the state of the demo - countdown is set back to 1 hour and all progress is erased. */
	static void resetDemo();
	
	/*! \brief Callback from script back to game. */
	static SQInteger notifyGame(HSQUIRRELVM p_vm);
	
	/*! \brief Quit the game gracefully */
	static void quitGame();
	
	/*! \brief Toggles the unlimited FPS mode */
	static void toggleUnlimitedFpsMode();
	
	/*! \brief Toggles showing the FPS counter */
	static void toggleFpsCounter();
	
	/*! \brief Crashes the game */
	static void simulateCrash();
	
	/*! \brief Download the leaderboard scores and pass the results to an Entity, using the following callbacks:
	           onDownloadLeaderboardSuccess(p_result) and
	           onDownloadLeaderboardFailed(p_result)
	           where p_result is a table containing the results.
	           \param p_leaderboard Leaderboard name.
	           \param p_rangeType The request range 
	           \param p_rangeMin The lower bound of the range
	           \param p_rangeMax The upper bound of the range
	           \param p_source The source of the request (e.g., DownloadRequestSource_Game or DownloadRequestSource_User)
	           \param p_handlingEntity The entity receiving the callbacks
	           \return A request ID */
	static s32 downloadLeaderboardScores(const std::string& p_leaderboard,
	                                     tt::score::DownloadRequestRangeType p_rangeType, s32 p_rangeMin, s32 p_rangeMax,
	                                     tt::score::DownloadRequestSource p_source, EntityBase* p_handlingEntity);
	
	/*! \brief Upload (async) a score to a leaderboard
	    \return A request ID. */
	static s32 uploadLeaderboardScore(const std::string& p_leaderboard, s32 p_score);
	
	/*! \brief Commit (async) accumulated leaderboard requests (does nothing on Steam, implemetend for PS4) */
	static void commitLeaderboardRequests();
	
	/*! \brief Returns the names of all conversations */
	static tt::str::Strings getAllConversationNames();
	
	/*! \brief Gets a JSON from a string.
		\param Type [string] the JSON in string form
		\return The JSON as a table or null if invalid */
	static SQInteger getJSONFromString(HSQUIRRELVM v);
	
	/*! \brief Gets a JSON from a file.
		\param Type [string] filename of the JSON file
		\return The loaded JSON as a table or null if invalid */
	static SQInteger getJSONFromFile(HSQUIRRELVM v);
	
	/*! \brief Clears the debug console (windows only) */
	static void clearDebugConsole();
	
	/*! \brief Gets the selected language */
	static const std::string& getLanguage();
	
	/*! \brief Sets the game language */
	static void setLanguage(const std::string& p_language);
	
	/*! \brief Sets the game language to platform default */
	static void setLanguageToPlatformDefault();
	
	/*! \brief Gets a localized string in UTF-8 format
	           \param p_locID the localization ID */
	static std::string getLocalizedStringUTF8(const std::string& p_locID);
	
	/*! \brief Gets a localized achievement string in UTF-8 format
	           \param p_locID the localization ID */
	static std::string getLocalizedAchievementStringUTF8(const std::string& p_locID);
};

// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_GAME_SCRIPT_BINDINGS_H)
