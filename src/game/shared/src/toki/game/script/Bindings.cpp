#include <tt/app/Application.h>
#include <tt/args/CmdLine.h>
#include <tt/engine/scene2d/shoebox/shoebox.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/fs/utils/utils.h>
#include <tt/http/HttpConnectMgr.h>
#include <tt/input/KeyBindings.h>
#include <tt/loc/LocStr.h>
#include <tt/script/helpers.h>
#include <tt/stats/stats.h>
#include <tt/steam/helpers.h>
#include <tt/system/Language.h>
#if defined(TT_STEAM_BUILD)
#include <tt/steam/Leaderboards.h>
#elif defined(TT_PLATFORM_WIN)
#include <Lmcons.h>
#endif
#include <tt/system/Calendar.h>
#include <tt/version/Version.h>

#include <toki/game/entity/effect/ColorGradingEffectMgr.h>
#include <toki/game/entity/effect/FogEffectMgr.h>
#include <toki/game/entity/Entity.h>
#include <toki/game/entity/EntityMgr.h>
#include <toki/game/fluid/FluidMgr.h>
#include <toki/game/light/LightMgr.h>
#include <toki/game/script/EntityScriptClass.h>
#include <toki/game/script/Bindings.h>
#include <toki/game/script/Registry.h>
#include <toki/game/script/sqbind_bindings.h>
#include <toki/game/script/wrappers/TextureWrapper.h>
#include <toki/game/script/wrappers/ShoeboxPlaneDataWrapper.h>
#include <toki/game/Game.h>
#include <toki/game/Minimap.h>
#include <toki/input/Controller.h>
#include <toki/level/AttributeLayer.h>
#include <toki/level/helpers.h>
#include <toki/level/LevelData.h>
#include <toki/savedata/utils.h>
#include <toki/script/ScriptMgr.h>
#include <toki/utils/types.h>
#include <toki/AppGlobal.h>
#include <toki/loc/Loc.h>
#include <toki/AppGlobal.h>

#if !defined(TT_BUILD_FINAL)
#include <tt/http/HttpResponseHandler.h>
class RIVEHttpResponseHandler : public tt::http::HttpResponseHandler
{
	virtual void handleHttpResponse(const tt::http::HttpResponse& /*p_response*/)
	{
		// FIXME: Disabled for now; I need to fix the Game loading thread as that one initializes the VM and this callback is
		// handled on the main thread; so threads can clash, causing the VM to crash
		//tt::script::VirtualMachinePtr vmPtr = toki::script::ScriptMgr::getVM();
		//if (vmPtr->isInitialized())
		//{
		//	vmPtr->callSqFun("onHTTPResponseReceived", p_response.requestID, p_response.statusCode, p_response.data);
		//}
	}
	
	virtual void handleHttpError(const std::wstring& /*p_errorMessage*/)
	{
		//TT_Printf("ERROR: %s\n", p_errorMessage.c_str());
	}
};
RIVEHttpResponseHandler g_httpResponseHandler;
#endif

namespace toki {
namespace game {
namespace script {

//--------------------------------------------------------------------------------------------------
// Callbacks

void downloadLeaderboardScoresCallback(s32 p_requestID, const tt::score::DownloadRequestResults& p_results)
{
	using namespace tt::score;
	
	// Fetch target entity first
	entity::EntityHandle handle(game::entity::EntityHandle::createFromRawValue(p_results.userData));
	entity::Entity *target = handle.getPtr();
	if (target == 0 || target->getEntityScript() == 0)
	{
		// Target already died?
		return;
	}
	script::EntityBasePtr script(target->getEntityScript());
	
	if (p_results.status == DownloadRequestStatus_Failed)
	{
		// Callback to script onDownloadLeaderboardFailed
		script->callSqFun("onDownloadLeaderboardFailed", p_requestID, p_results.leaderboard);
		return;
	}
	
	// Convert to JSON format
	Json::Value rootNode(Json::objectValue);
	rootNode["leaderboard"]             = p_results.leaderboard;
	rootNode["leaderboardTotalEntries"] = p_results.leaderboardTotalEntries;
	rootNode["rangeType"]               = static_cast<s32>(p_results.rangeType);
	rootNode["currentUserIndex"]        = p_results.currentUserIndex;
	rootNode["entries"]                 = Json::Value(Json::arrayValue);
	
	Json::Value& entriesNode(rootNode["entries"]);
	
	for (DownloadRequestResultEntries::const_iterator it = p_results.entries.begin();
		it != p_results.entries.end(); ++it)
	{
		Json::Value entryNode(Json::objectValue);
		entryNode["name"]        = it->name;
		entryNode["rank"]        = it->rank;
		entryNode["score"]       = it->score;
		entryNode["isDeveloper"] = it->isDeveloper;
		entriesNode.append(entryNode);
	}
	
	// FIXME: I'm mixing the ScriptMgr and EntityMgr VMs here; are we sure they are the same?
	// Why do all these managers even have the option to have different VMs?
	
	tt::script::VirtualMachinePtr vm(toki::script::ScriptMgr::getVM());
	{
		tt::script::SqTopRestorerHelper stackRestorer(vm->getVM());
	
		// Manually handle callback as EntityBase doesn't support calling sq methods with an agrument that is already
		// on the stack. pushJSON pushed
		const std::string methodName("onDownloadLeaderboardSuccess");
		if (vm->prepareMethodOnStack(methodName, script->getSqState(), script->getSqInstance()) == false)
		{
			return;
		}
		
		// Parameters here.
		sq_pushinteger(vm->getVM(), p_requestID);
		toki::script::ScriptMgr::pushJSON(rootNode);
		
		// Call the method
		vm->callFunction(methodName, 3);
	}
}


//--------------------------------------------------------------------------------------------------
// Public methods

void Bindings::bindAll(const tt::script::VirtualMachinePtr& p_vm)
{
	TT_SQBIND_SETVM(p_vm);
	
	// Bind the registry
	Registry::bind(p_vm);
	
	// Bind the global functions defined in this class
	TT_SQBIND_FUNCTION(setFixedDeltaTimeScale);
	TT_SQBIND_FUNCTION(getFixedDeltaTimeScale);
	TT_SQBIND_FUNCTION(setColorGradingAfterHud);
	TT_SQBIND_FUNCTION(isColorGradingAfterHud);
	TT_SQBIND_FUNCTION(getLeftStick);
	TT_SQBIND_FUNCTION(getRightStick);
	TT_SQBIND_FUNCTION(isMousePositionValid);
	TT_SQBIND_FUNCTION(getMouseWorldPosition);
	TT_SQBIND_FUNCTION(isMouseLeftDown);
	TT_SQBIND_FUNCTION(isMouseRightDown);
	TT_SQBIND_FUNCTION(getAspectRatio);
	TT_SQBIND_FUNCTION(getScreenWidth);
	TT_SQBIND_FUNCTION(getScreenHeight);
	TT_SQBIND_FUNCTION(getPresentationNames);
	TT_SQBIND_FUNCTION(getLevelNames);
	TT_SQBIND_FUNCTION(getParticleEffectNames);
	TT_SQBIND_FUNCTION(getEntityNames);
	TT_SQBIND_FUNCTION(getColorGradingNames);
	TT_SQBIND_FUNCTION(getLightNames);
	TT_SQBIND_FUNCTION(getShoeboxIncludeNames);
	TT_SQBIND_FUNCTION(getSoundCueNames);
	TT_SQBIND_FUNCTION(getSoundCueNamesInBanks);
	TT_SQBIND_FUNCTION(getMusicTrackNames);
	TT_SQBIND_FUNCTION(getAudioCategoryNames);
	TT_SQBIND_FUNCTION(getReverbEffectNames);
	TT_SQBIND_FUNCTION(isInLevelEditorMode);
	TT_SQBIND_FUNCTION(isInDemoMode);
	TT_SQBIND_FUNCTION(isInLevelMode);
	TT_SQBIND_FUNCTION(isInMissionMode);
	TT_SQBIND_FUNCTION(isSteamBuild);
	TT_SQBIND_FUNCTION(getEntitiesByTag);
	TT_SQBIND_FUNCTION(getEntityByID);
	TT_SQBIND_SQUIRREL_FUNCTION(getEntityTypeByID);
	TT_SQBIND_SQUIRREL_FUNCTION(getEntityClassByID);
	TT_SQBIND_FUNCTION(getEntityByHandleValue);
	TT_SQBIND_SQUIRREL_FUNCTION(spawnEntity);
	TT_SQBIND_FUNCTION(respawnEntity);
	TT_SQBIND_FUNCTION(respawnEntityAtPosition);
	TT_SQBIND_FUNCTION(killEntity);
	TT_SQBIND_FUNCTION(addButtonInputListeningEntity);
	TT_SQBIND_FUNCTION(addButtonInputBlockingListeningEntity);
	TT_SQBIND_FUNCTION(removeButtonInputListeningEntity);
	TT_SQBIND_FUNCTION(removeAllButtonInputListeningEntities);
	TT_SQBIND_FUNCTION(addMouseInputListeningEntity);
	TT_SQBIND_FUNCTION(addMouseInputBlockingListeningEntity);
	TT_SQBIND_FUNCTION(removeMouseInputListeningEntity);
	TT_SQBIND_FUNCTION(removeAllMouseInputListeningEntities);
	TT_SQBIND_FUNCTION(addKeyboardListeningEntity);
	TT_SQBIND_FUNCTION(addKeyboardBlockingListeningEntity);
	TT_SQBIND_FUNCTION(removeKeyboardListeningEntity);
	TT_SQBIND_FUNCTION(removeAllKeyboardListeningEntities);
	TT_SQBIND_FUNCTION(getAppTimeInSeconds);
	TT_SQBIND_FUNCTION(getGameTimeInSeconds);
	TT_SQBIND_FUNCTION(getDateDay);
	TT_SQBIND_FUNCTION(getDateMonth);
	TT_SQBIND_FUNCTION(getDateYear);
	TT_SQBIND_FUNCTION(getDateString);
	TT_SQBIND_FUNCTION(getTimeString);
	TT_SQBIND_FUNCTION(getVersion);
	TT_SQBIND_FUNCTION(getRegion);
	TT_SQBIND_FUNCTION(getCollisionType);
	TT_SQBIND_FUNCTION(getCollisionTypeLevelOnly);
	TT_SQBIND_FUNCTION(getThemeType);
	TT_SQBIND_FUNCTION(setThemeType);
	TT_SQBIND_FUNCTION(addShoeboxPlaneToShoebox);
	TT_SQBIND_FUNCTION(getShoeboxSpaceFromWorldPos);
	TT_SQBIND_FUNCTION(addShoeboxInclude);
	TT_SQBIND_FUNCTION(openURL);
	TT_SQBIND_FUNCTION(openStorePage);
	TT_SQBIND_FUNCTION(allowOnlineSessions);
	TT_SQBIND_SQUIRREL_FUNCTION(isSteamGroupMember);
	TT_SQBIND_FUNCTION(handleUserLevelCompleted);
	TT_SQBIND_FUNCTION(setTileAttributesVisible);
	TT_SQBIND_FUNCTION(sendShoeboxTagEvent);
	TT_SQBIND_FUNCTION(getUsername);
	TT_SQBIND_FUNCTION(sendGetRequest);
	TT_SQBIND_FUNCTION(addFluidWarp);
	TT_SQBIND_FUNCTION(deleteFluidWarpPair);
	TT_SQBIND_FUNCTION(startWave);
	TT_SQBIND_FUNCTION(getCurrentProgressType);
	TT_SQBIND_FUNCTION(isPositionInLight);
	TT_SQBIND_FUNCTION(setBackgroundBlurLayers);
	TT_SQBIND_FUNCTION(setForegroundBlurLayers);
	TT_SQBIND_FUNCTION(getBackgroundBlurLayers);
	TT_SQBIND_FUNCTION(getForegroundBlurLayers);
	TT_SQBIND_FUNCTION(getDefaultFogColor);
	TT_SQBIND_FUNCTION(setDefaultFogColor);
	TT_SQBIND_FUNCTION(getDefaultFogNear);
	TT_SQBIND_FUNCTION(getDefaultFogFar);
	TT_SQBIND_FUNCTION(setDefaultFogNearFar);
	TT_SQBIND_FUNCTION(resetFogSettings);
	TT_SQBIND_FUNCTION(getColorGradingTexture);
	TT_SQBIND_FUNCTION(getDefaultColorGrading);
	TT_SQBIND_FUNCTION(setDefaultColorGrading);
	TT_SQBIND_FUNCTION(areColorGradingEffectsEnabled);
	TT_SQBIND_FUNCTION(setColorGradingEffectsEnabled);
	TT_SQBIND_FUNCTION(storeGameState);
	TT_SQBIND_FUNCTION(setTVDisplayGamma);
	TT_SQBIND_FUNCTION(setDRCDisplayGamma);
	TT_SQBIND_FUNCTION(isPointerAllowed);
	TT_SQBIND_FUNCTION(setPointerAllowed);
	TT_SQBIND_FUNCTION(isPointerVisible);
	TT_SQBIND_FUNCTION(isRumbleEnabled);
	TT_SQBIND_FUNCTION(setRumbleEnabled);
	TT_SQBIND_FUNCTION(startRumble);
	TT_SQBIND_FUNCTION(stopRumble);
	TT_SQBIND_FUNCTION(setPlayerCount);
	TT_SQBIND_FUNCTION(setGamepadControlScheme);
	TT_SQBIND_FUNCTION(getGamepadControlScheme);
	TT_SQBIND_SQUIRREL_FUNCTION(getKeyBindingsTable);
	TT_SQBIND_SQUIRREL_FUNCTION(setKeyBindingsTable);
	TT_SQBIND_FUNCTION(getKeyName);
	TT_SQBIND_SQUIRREL_FUNCTION(getMetaData);
	TT_SQBIND_FUNCTION(isMinimapEnabled);
	TT_SQBIND_FUNCTION(isMinimapHidden);
	TT_SQBIND_FUNCTION(setMinimapEnabled);
	TT_SQBIND_FUNCTION(setMinimapHidden);
	TT_SQBIND_FUNCTION(setMinimapYOffset);
	TT_SQBIND_FUNCTION(getMinimapYOffset);
	TT_SQBIND_FUNCTION(setMinimapSideBorderSize);
	TT_SQBIND_FUNCTION(getMinimapSideBorderSize);
	TT_SQBIND_FUNCTION(setMinimapLargestLevelWidth);
	TT_SQBIND_FUNCTION(getMinimapGraphicsSize);
	TT_SQBIND_FUNCTION(setMinimapGraphicsSize);
	TT_SQBIND_FUNCTION(getMinimapExtraWidth);
	TT_SQBIND_FUNCTION(setMinimapExtraWidth);
	TT_SQBIND_FUNCTION(getMinimapSideFadeWidth);
	TT_SQBIND_FUNCTION(setMinimapSideFadeWidth);
	TT_SQBIND_FUNCTION(createMusicTrack);
	TT_SQBIND_FUNCTION(destroyMusicTrack);
	TT_SQBIND_FUNCTION(setRestoreFailureLevel);
	TT_SQBIND_FUNCTION(getShoeboxSplitPriority);
	TT_SQBIND_FUNCTION(setShoeboxSplitPriority);
	TT_SQBIND_FUNCTION(resetDemo);
	TT_SQBIND_SQUIRREL_FUNCTION(notifyGame);
	TT_SQBIND_FUNCTION(quitGame);
	TT_SQBIND_FUNCTION(toggleFpsCounter);
	TT_SQBIND_FUNCTION(simulateCrash);
	TT_SQBIND_FUNCTION(uploadLeaderboardScore);
	TT_SQBIND_FUNCTION(downloadLeaderboardScores);
	TT_SQBIND_FUNCTION(commitLeaderboardRequests);
	TT_SQBIND_FUNCTION(getAllConversationNames);
	TT_SQBIND_SQUIRREL_FUNCTION(getJSONFromString);
	TT_SQBIND_SQUIRREL_FUNCTION(getJSONFromFile);
	TT_SQBIND_FUNCTION(clearDebugConsole);
	TT_SQBIND_FUNCTION(getLanguage);
	TT_SQBIND_FUNCTION(setLanguage);
	TT_SQBIND_FUNCTION(setLanguageToPlatformDefault);
	TT_SQBIND_FUNCTION(getLocalizedStringUTF8);
	TT_SQBIND_FUNCTION(getLocalizedAchievementStringUTF8);
	
	// Bind all Entity wrappers
	using namespace wrappers;
	EntityWrapper::bind(p_vm);
	
	// Bind Direction enum values
	using namespace movement;
	TT_SQBIND_CONSTANT(Direction_Up);
	TT_SQBIND_CONSTANT(Direction_Down);
	TT_SQBIND_CONSTANT(Direction_Left);
	TT_SQBIND_CONSTANT(Direction_Right);
	TT_SQBIND_CONSTANT(Direction_None);
	
	// Bind Direction functions.
	TT_SQBIND_FUNCTION(isValidDirection);
	TT_SQBIND_FUNCTION(getDirectionFromName);
	TT_SQBIND_FUNCTION(getInverseDirection);
	TT_SQBIND_FUNCTION_NAME(getDirectionNameAsString, "getDirectionName");
	
	// Bind SurveyResults
	TT_SQBIND_CONSTANT(SurveyResult_OnRightEdge);
	TT_SQBIND_CONSTANT(SurveyResult_OnTopEdge);
	TT_SQBIND_CONSTANT(SurveyResult_OnLeftEdge);
	TT_SQBIND_CONSTANT(SurveyResult_OnBottomEdge);
	TT_SQBIND_CONSTANT(SurveyResult_WallRight);
	TT_SQBIND_CONSTANT(SurveyResult_Ceiling);
	TT_SQBIND_CONSTANT(SurveyResult_WallLeft);
	TT_SQBIND_CONSTANT(SurveyResult_Floor);
	TT_SQBIND_CONSTANT(SurveyResult_TwoRight);
	TT_SQBIND_CONSTANT(SurveyResult_TwoUp);
	TT_SQBIND_CONSTANT(SurveyResult_TwoLeft);
	TT_SQBIND_CONSTANT(SurveyResult_TwoDown);
	TT_SQBIND_CONSTANT(SurveyResult_OneUpRight);
	TT_SQBIND_CONSTANT(SurveyResult_OneUpLeft);
	TT_SQBIND_CONSTANT(SurveyResult_OneDownLeft);
	TT_SQBIND_CONSTANT(SurveyResult_OneDownRight);
	TT_SQBIND_CONSTANT(SurveyResult_TopRight);
	TT_SQBIND_CONSTANT(SurveyResult_TopLeft);
	TT_SQBIND_CONSTANT(SurveyResult_BottomLeft);
	TT_SQBIND_CONSTANT(SurveyResult_BottomRight);
	TT_SQBIND_CONSTANT(SurveyResult_DropRight);
	TT_SQBIND_CONSTANT(SurveyResult_DropRight_fromRightAsDown);
	TT_SQBIND_CONSTANT(SurveyResult_DropRight_fromUpAsDown);
	TT_SQBIND_CONSTANT(SurveyResult_DropRight_fromLeftAsDown);
	TT_SQBIND_CONSTANT(SurveyResult_DropLeft);
	TT_SQBIND_CONSTANT(SurveyResult_DropLeft_fromRightAsDown);
	TT_SQBIND_CONSTANT(SurveyResult_DropLeft_fromUpAsDown);
	TT_SQBIND_CONSTANT(SurveyResult_DropLeft_fromLeftAsDown);
	TT_SQBIND_CONSTANT(SurveyResult_InsideCollision);
	TT_SQBIND_CONSTANT(SurveyResult_InsideCollisionRaw);
	TT_SQBIND_CONSTANT(SurveyResult_StandOnSolid);
	TT_SQBIND_CONSTANT(SurveyResult_OnParent);
	TT_SQBIND_CONSTANT(SurveyResult_OnParentVertical);
	TT_SQBIND_CONSTANT(SurveyResult_OnWaterLeft);
	TT_SQBIND_CONSTANT(SurveyResult_OnWaterRight);
	TT_SQBIND_CONSTANT(SurveyResult_OnWaterStatic);
	TT_SQBIND_CONSTANT(SurveyResult_NotInWater);
	TT_SQBIND_CONSTANT(SurveyResult_SubmergedInWater);
	TT_SQBIND_CONSTANT(SurveyResult_WaterFlowLeft);
	TT_SQBIND_CONSTANT(SurveyResult_WaterFlowRight);
	TT_SQBIND_CONSTANT(SurveyResult_WaterFlowStatic);
	TT_SQBIND_CONSTANT(SurveyResult_OnLavaLeft);
	TT_SQBIND_CONSTANT(SurveyResult_OnLavaRight);
	TT_SQBIND_CONSTANT(SurveyResult_OnLavaStatic);
	TT_SQBIND_CONSTANT(SurveyResult_NotInLava);
	TT_SQBIND_CONSTANT(SurveyResult_SubmergedInLava);
	
	using namespace entity;
	PresStartSettings::bind(p_vm);
	
	TT_SQBIND_CONSTANT(LocalDir_Forward);
	TT_SQBIND_CONSTANT(LocalDir_Up);
	TT_SQBIND_CONSTANT(LocalDir_Back);
	TT_SQBIND_CONSTANT(LocalDir_Down);
	TT_SQBIND_CONSTANT(LocalDir_None);
	
	// Bind Direction functions.
	TT_SQBIND_FUNCTION(isValidLocalDir);
	TT_SQBIND_FUNCTION(getLocalDirFromName);
	TT_SQBIND_FUNCTION(getInverseLocalDir);
	TT_SQBIND_FUNCTION_NAME(getLocalDirNameAsString, "getLocalDirName");
	
	// Bind the CollisionType and ThemeType values and helpers
	{
		using namespace level;
		
		TT_SQBIND_FUNCTION(isValidCollisionType);
		TT_SQBIND_FUNCTION(getCollisionTypeFromName);
		TT_SQBIND_FUNCTION_NAME(getCollisionTypeNameAsString, "getCollisionTypeName");
		TT_SQBIND_FUNCTION_NAME(getThemeTypeNameAsString, "getThemeTypeName");
		TT_SQBIND_FUNCTION(isSolid);
		
		// Only bind a subset of the collision types. (These may be set as entity collision tiles.)
		TT_SQBIND_CONSTANT(CollisionType_Air);
		TT_SQBIND_CONSTANT(CollisionType_Solid);
		TT_SQBIND_CONSTANT(CollisionType_Solid_Allow_Pathfinding);
		TT_SQBIND_CONSTANT(CollisionType_Water_Still);
		
		TT_SQBIND_CONSTANT(ThemeType_UseLevelDefault);
		TT_SQBIND_CONSTANT(ThemeType_DoNotTheme);
		TT_SQBIND_CONSTANT(ThemeType_Sand);
		TT_SQBIND_CONSTANT(ThemeType_Rocks);
		TT_SQBIND_CONSTANT(ThemeType_Beach);
		TT_SQBIND_CONSTANT(ThemeType_DarkRocks);
		
		TT_SQBIND_CONSTANT(Placeable_Hidden);
		TT_SQBIND_CONSTANT(Placeable_Developer);
		TT_SQBIND_CONSTANT(Placeable_Everyone);
		TT_SQBIND_CONSTANT(Placeable_UserLevelEditor);
	}
	
	using namespace fluid;
	TT_SQBIND_CONSTANT(FluidType_Water);
	TT_SQBIND_CONSTANT(FluidType_Lava);
	
	TT_SQBIND_FUNCTION(isValidFluidType);
	
	// PowerBeamType and alignment
	{
		using namespace entity::graphics;
		TT_SQBIND_CONSTANT(PowerBeamType_Hack);
		TT_SQBIND_CONSTANT(PowerBeamType_Electricity);
		TT_SQBIND_CONSTANT(PowerBeamType_PreviewLaser);
		TT_SQBIND_CONSTANT(PowerBeamType_Laser);
		TT_SQBIND_CONSTANT(PowerBeamType_Sight);
		
		TT_SQBIND_CONSTANT(HorizontalAlignment_Left);
		TT_SQBIND_CONSTANT(HorizontalAlignment_Center);
		TT_SQBIND_CONSTANT(HorizontalAlignment_Right);
		
		TT_SQBIND_CONSTANT(VerticalAlignment_Top);
		TT_SQBIND_CONSTANT(VerticalAlignment_Center);
		TT_SQBIND_CONSTANT(VerticalAlignment_Bottom);
	}
	
	// EffectRectTarget
	{
		using namespace entity::effect;
		TT_SQBIND_CONSTANT(EffectRectTarget_CameraPos);
		TT_SQBIND_CONSTANT(EffectRectTarget_ControllingEntityPos);
	}
	
	// Bind the ParticleLayer values and helpers
	{
		TT_SQBIND_CONSTANT(ParticleLayer_BehindShoeboxZero);
		TT_SQBIND_CONSTANT(ParticleLayer_BehindEntities);
		TT_SQBIND_CONSTANT(ParticleLayer_BehindEntitiesSubOnly);
		TT_SQBIND_CONSTANT(ParticleLayer_InFrontOfEntities);
		TT_SQBIND_CONSTANT(ParticleLayer_InFrontOfEntitiesSubOnly);
		TT_SQBIND_CONSTANT(ParticleLayer_InFrontOfWater);
		TT_SQBIND_CONSTANT(ParticleLayer_InFrontOfStillWater);
		TT_SQBIND_CONSTANT(ParticleLayer_InFrontOfSplit);
		TT_SQBIND_CONSTANT(ParticleLayer_InFrontOfShoeboxZeroOne);
		TT_SQBIND_CONSTANT(ParticleLayer_InFrontOfShoeboxZeroTwo);
		TT_SQBIND_CONSTANT(ParticleLayer_BehindHud);
		TT_SQBIND_CONSTANT(ParticleLayer_BehindHudTvOnly);
		TT_SQBIND_CONSTANT(ParticleLayer_BehindHudDrcOnly);
		TT_SQBIND_CONSTANT(ParticleLayer_Hud);
		TT_SQBIND_CONSTANT(ParticleLayer_HudTvOnly);
		TT_SQBIND_CONSTANT(ParticleLayer_HudDrcOnly);
		TT_SQBIND_CONSTANT(ParticleLayer_Minimap);
		TT_SQBIND_CONSTANT(ParticleLayer_InFrontOfHud);
		TT_SQBIND_CONSTANT(ParticleLayer_UseLayerFromParticleEffect);
		
		TT_SQBIND_FUNCTION(isValidParticleLayer);
		TT_SQBIND_FUNCTION(getParticleLayerFromName);
		TT_SQBIND_FUNCTION_NAME(getParticleLayerNameAsString, "getParticleLayerName");
	}
	
	// Input presets
	{
		using namespace input;
		TT_SQBIND_CONSTANT(GamepadControlScheme_A1);
		TT_SQBIND_CONSTANT(GamepadControlScheme_A2);
		TT_SQBIND_CONSTANT(GamepadControlScheme_B1);
		TT_SQBIND_CONSTANT(GamepadControlScheme_B2);
		TT_SQBIND_CONSTANT(RumbleStrength_Low);
		TT_SQBIND_CONSTANT(RumbleStrength_Medium);
		TT_SQBIND_CONSTANT(RumbleStrength_High);
	}
	
	{
		TT_SQBIND_CONSTANT(ProgressType_Main);
		TT_SQBIND_CONSTANT(ProgressType_UserLevel);
		TT_SQBIND_CONSTANT(ProgressType_Invalid);
	}
	
	// GlyphSetID
	{
		using namespace utils;
		
		TT_SQBIND_CONSTANT(GlyphSetID_Title);
		TT_SQBIND_CONSTANT(GlyphSetID_Header);
		TT_SQBIND_CONSTANT(GlyphSetID_Text);
		TT_SQBIND_CONSTANT(GlyphSetID_Notes);
		TT_SQBIND_CONSTANT(GlyphSetID_EditorHelpText);
	}
}


void Bindings::setFixedDeltaTimeScale(real p_scale)
{
	AppGlobal::setFixedDeltaTimeScale(p_scale);
}


real Bindings::getFixedDeltaTimeScale()
{
	return AppGlobal::getFixedDeltaTimeScale();
}


void Bindings::setColorGradingAfterHud(bool p_afterHud)
{
	if (AppGlobal::hasGame() == false)
	{
		return;
	}
	
	AppGlobal::getGame()->setColorGradingAfterHud(p_afterHud);
}


bool Bindings::isColorGradingAfterHud()
{
	if (AppGlobal::hasGame() == false)
	{
		return false;
	}
	
	return AppGlobal::getGame()->isColorGradingAfterHud();
}


tt::math::Vector2 Bindings::getLeftStick()
{
	return AppGlobal::getController(tt::input::ControllerIndex_One).cur.direction;
}


tt::math::Vector2 Bindings::getRightStick()
{
	return AppGlobal::getController(tt::input::ControllerIndex_One).cur.scroll;
}


bool Bindings::isMousePositionValid()
{
	if (AppGlobal::hasGame() == false)
	{
		return false;
	}
	return AppGlobal::getController(tt::input::ControllerIndex_One).cur.pointer.valid;
}


tt::math::Vector2 Bindings::getMouseWorldPosition()
{
	if (AppGlobal::hasGame() == false)
	{
		return tt::math::Vector2(-1.0f, -1.0f);
	}
	
	Camera& inputCamera(AppGlobal::getGame()->getInputCamera());
	const tt::math::Vector2 pointerWorldPos(inputCamera.screenToWorld(AppGlobal::getController(tt::input::ControllerIndex_One).cur.pointer));
	return pointerWorldPos;
}


bool Bindings::isMouseLeftDown()
{
	// FIXME: Game should have its own state; don't misuse the editor state for this
	const input::Controller::State::EditorState& editorState(AppGlobal::getController(tt::input::ControllerIndex_One).cur.editor);
	const bool noModifiersDown =
		editorState.keys[tt::input::Key_Control].down == false &&
		editorState.keys[tt::input::Key_Alt    ].down == false &&
		editorState.keys[tt::input::Key_Shift  ].down == false &&
		editorState.keys[tt::input::Key_Space  ].down == false;
	
	return editorState.pointerLeft.down && noModifiersDown;
}


bool Bindings::isMouseRightDown()
{
	// FIXME: Game should have its own state; don't misuse the editor state for this
	const input::Controller::State::EditorState& editorState(AppGlobal::getController(tt::input::ControllerIndex_One).cur.editor);
	const bool noModifiersDown =
		editorState.keys[tt::input::Key_Control].down == false &&
		editorState.keys[tt::input::Key_Alt    ].down == false &&
		editorState.keys[tt::input::Key_Shift  ].down == false &&
		editorState.keys[tt::input::Key_Space  ].down == false;
	
	return editorState.pointerRight.down && noModifiersDown;
}


real Bindings::getAspectRatio()
{
	tt::engine::renderer::Renderer* r = tt::engine::renderer::Renderer::getInstance();
	return r->getScreenWidth() / static_cast<real>(r->getScreenHeight());
}


s32 Bindings::getScreenWidth()
{
	tt::engine::renderer::Renderer* r = tt::engine::renderer::Renderer::getInstance();
	return r->getScreenWidth();
}


s32 Bindings::getScreenHeight()
{
	tt::engine::renderer::Renderer* r = tt::engine::renderer::Renderer::getInstance();
	return r->getScreenHeight();
}


tt::str::Strings Bindings::getPresentationNames()
{
	return AppGlobal::getPresentationNames();
}


tt::str::Strings Bindings::getLevelNames()
{
	return AppGlobal::getLevelNames();
}


tt::str::Strings Bindings::getParticleEffectNames()
{
	return AppGlobal::getParticleEffectNames();
}


tt::str::Strings Bindings::getEntityNames()
{
	return AppGlobal::getEntityNames();
}


tt::str::Strings Bindings::getColorGradingNames()
{
	return AppGlobal::getColorGradingNames();
}


tt::str::Strings Bindings::getLightNames()
{
	return AppGlobal::getLightNames();
}


tt::str::Strings Bindings::getShoeboxIncludeNames()
{
	return AppGlobal::getShoeboxIncludeNames();
}


tt::str::Strings Bindings::getSoundCueNames()
{
	tt::str::StringSet names = audio::AudioPlayer::getInstance()->getAllCueNames();
	return tt::str::Strings(names.begin(), names.end());
}


tt::str::Strings Bindings::getSoundCueNamesInBanks(const tt::str::Strings& p_soundBankNames)
{
	audio::AudioPlayer* player = audio::AudioPlayer::getInstance();
	tt::str::Strings    retval;
	
	for (tt::str::Strings::const_iterator it = p_soundBankNames.begin();
	     it != p_soundBankNames.end(); ++it)
	{
		const tt::str::StringSet& namesInBank(player->getCueNames(*it));
		retval.insert(retval.end(), namesInBank.begin(), namesInBank.end());
	}
	
	return retval;
}


tt::str::Strings Bindings::getMusicTrackNames()
{
	const tt::str::StringSet& names(audio::AudioPlayer::getInstance()->getMusicTrackMgr().getAvailableMusicNames());
	return tt::str::Strings(names.begin(), names.end());
}


tt::str::Strings Bindings::getAudioCategoryNames()
{
	tt::str::StringSet names;
	for (s32 i = 0; i < audio::Category_Count; ++i)
	{
		names.insert(audio::getCategoryName(static_cast<audio::Category>(i)));
	}
	return tt::str::Strings(names.begin(), names.end());
}


tt::str::Strings Bindings::getReverbEffectNames()
{
	tt::str::StringSet names;
	for (s32 i = 0; i < audio::ReverbEffect_Count; ++i)
	{
		names.insert(audio::getReverbEffectName(static_cast<audio::ReverbEffect>(i)));
	}
	return tt::str::Strings(names.begin(), names.end());
}


bool Bindings::isInLevelEditorMode()
{
	return AppGlobal::isInLevelEditorMode();
}


bool Bindings::isInDemoMode()
{
	return AppGlobal::isInDemoMode();
}


bool Bindings::isInLevelMode()
{
#if !defined(TT_BUILD_FINAL)
	const tt::args::CmdLine& cmdLine(tt::app::getApplication()->getCmdLine());
	return cmdLine.exists("level");
#else
	return false;
#endif
}


bool Bindings::isInMissionMode()
{
#if !defined(TT_BUILD_FINAL)
	const tt::args::CmdLine& cmdLine(tt::app::getApplication()->getCmdLine());
	return cmdLine.exists("mission");
#else
	return false;
#endif
}


bool Bindings::isSteamBuild()
{
#if defined(TT_STEAM_BUILD)
	return true;
#else
	return false;
#endif
}


EntityBaseCollection Bindings::getEntitiesByTag(const std::string& p_tag)
{
	EntityScriptMgr& mgr = AppGlobal::getEntityScriptMgr();
	return mgr.getEntitiesByTag(p_tag);
}


EntityBase* Bindings::getEntityByID(s32 p_id)
{
	entity::EntityMgr& entityMgr(AppGlobal::getGame()->getEntityMgr());
	entity::EntityHandle handle = entityMgr.getEntityHandleByID(p_id);
	entity::Entity* entity = handle.getPtr();
	if (entity == 0)
	{
		return 0;
	}
	return entity->getEntityScript().get();
}


SQInteger Bindings::getEntityTypeByID(HSQUIRRELVM v)
{
	SQInteger argc = sq_gettop(v);
	
	if (argc != 2 )
	{
		TT_PANIC("getEntityTypeByID(integer) has %d argument(s), expected 1",
			argc - 1);
		return 0;
	}
	
	// Arg1: Entity ID
	SQInteger id = -1;
	if (SQ_FAILED(sq_getinteger(v, 2, &id)))
	{
		TT_PANIC("getEntityTypeByID() first argument should be of type integer");
		return 0;
	}
	const level::LevelDataPtr& levelData(AppGlobal::getGame()->getLevelData());
	const level::entity::EntityInstancePtr& entity(levelData->getEntityByID(static_cast<s32>(id)));
	if (entity != 0)
	{
		const std::string& type(entity->getType());
		sq_pushstring(v, type.c_str(), type.size());
		return 1;
	}
	
	return 0;
}


SQInteger Bindings::getEntityClassByID(HSQUIRRELVM v)
{
	SQInteger argc = sq_gettop(v);
	
	if (argc != 2 )
	{
		TT_PANIC("getEntityClassByID(integer) has %d argument(s), expected 1",
			argc - 1);
		return 0;
	}
	
	// Arg1: Entity ID
	SQInteger id = -1;
	if (SQ_FAILED(sq_getinteger(v, 2, &id)))
	{
		TT_PANIC("getEntityClassByID() first argument should be of type integer");
		return 0;
	}
	const level::LevelDataPtr& levelData(AppGlobal::getGame()->getLevelData());
	const level::entity::EntityInstancePtr& entity(levelData->getEntityByID(static_cast<s32>(id)));
	
	if (entity != 0)
	{
		EntityScriptMgr& mgr = AppGlobal::getEntityScriptMgr();
		const std::string& type(entity->getType());
		const EntityScriptClassPtr& scriptClass(mgr.getClass(type));
		TT_NULL_ASSERT(scriptClass);
		if (scriptClass != 0)
		{
			sq_pushobject(v, scriptClass->getBaseClass());
			return 1;
		}
	}
	
	return 0;
}


EntityBase* Bindings::getEntityByHandleValue(s32 p_handleValue)
{
	entity::EntityMgr& entityMgr(AppGlobal::getGame()->getEntityMgr());
	entity::Entity* entity = entityMgr.getEntity(entity::EntityHandle::createFromRawValue(p_handleValue));
	if (entity == 0)
	{
		return 0;
	}
	return entity->getEntityScript().get();
}


SQInteger Bindings::spawnEntity(HSQUIRRELVM v)
{
	SQInteger argc = sq_gettop(v);
	
	if (argc != 3 && argc != 4)
	{
		TT_PANIC("spawnEntity(string, Vector2, [property table]) has %d argument(s), expected 2 or 3",
			argc - 1);
		return 0;
	}
	
	// Arg1: Entity type
	const SQChar* strPtr = 0;
	if (SQ_FAILED(sq_getstring(v, 2, &strPtr)))
	{
		TT_PANIC("spawnEntity() first argument should be of type string");
		return 0;
	}
	std::string type(strPtr);
	
	// Arg2: Position
	tt::math::Vector2 position = SqBind<tt::math::Vector2>::get(v, 3);
	
	// Arg3: Optional properties
	HSQOBJECT properties;
	sq_resetobject(&properties);
	if (argc == 4)
	{
		// make sure a table exists on -1, this is required for initEntityFromSquirrel to work properly
		if (sq_gettype(v, -1) != OT_TABLE)
		{
			TT_PANIC("spawnEntity() third argument should be a table of properties");
		}
		else
		{
			sq_getstackobj(v, -1, &properties);
		}
	}
	
	entity::EntityMgr& entityMgr(AppGlobal::getGame()->getEntityMgr());
	entity::EntityHandle handle = entityMgr.createEntity(type, -1);
	if (handle.isEmpty())
	{
		// Creating new entity failed (or was prevented by script): do not try to init
		return 0;
	}
	
	const bool isInitialized = entityMgr.initEntity(handle, position, properties);
	if (isInitialized == false)
	{
		return 0;
	}
	
	entity::Entity* entity = entityMgr.getEntity(handle);
	if (entity == 0)
	{
		return 0; // something failed, nothing returned
	}
	
	EntityBase* base = entity->getEntityScript().get();
	SqBind<EntityBase>::push(v, base);
	
	// The weak ref code below is commented out because script needs to be updated
	// in order for this to work.
	// Replace instance with weak ref.
	//sq_weakref(v, -1); // Make weak ref
	//sq_remove(v, -2);  // Remove instance
	
	return 1; // return 1 value; the entity
}


EntityBase* Bindings::respawnEntity(s32 p_id)
{
	// FIXME: Code duplication with respawnEntityAtPosition
	entity::EntityMgr& entityMgr(AppGlobal::getGame()->getEntityMgr());
	const level::LevelDataPtr& levelData(AppGlobal::getGame()->getLevelData());
	
	// Find the entity in the leveldata
	// FIXME: Should we filter out non-mission specific entities?
	level::entity::EntityInstancePtr requestedEntity = levelData->getEntityByID(p_id);
	if (requestedEntity == 0)
	{
		// Couldn't find entity
		TT_WARN("Couldn't find entity with id '%d' in current level.", p_id);
		return 0;
	}
	
	entity::EntityHandle handle = entityMgr.createEntity(requestedEntity->getType(), p_id);
	if (handle.isEmpty())
	{
		// Couldn't create entity
		TT_WARN("Couldn't create entity with id '%d' and type '%s' in current level.",
		        p_id, requestedEntity->getType().c_str());
		return 0;
	}
	
	// We should make a copy of the properties here as initEntity might alter them
	entity::EntityProperties props(requestedEntity->getProperties());
	if (entityMgr.initEntity(handle, requestedEntity->getPosition(), props) == false)
	{
		// Entity died after initialization
		return 0;
	}
	
	entity::Entity* entity = handle.getPtr();
	if (entity != 0 && entity->getEntityScript() != 0)
	{
		return entity->getEntityScript().get();
	}
	
	// Something went wrong
	TT_PANIC("Created and initialized entity with id '%d' and type '%s' is still invalid.",
	         p_id, requestedEntity->getType().c_str());
	return 0;
}


EntityBase* Bindings::respawnEntityAtPosition(s32 p_id, const tt::math::Vector2& p_position)
{
	// FIXME: Code duplication with respawnEntity
	entity::EntityMgr& entityMgr(AppGlobal::getGame()->getEntityMgr());
	const level::LevelDataPtr& levelData(AppGlobal::getGame()->getLevelData());
	
	// Find the entity in the leveldata
	// FIXME: Should we filter out non-mission specific entities?
	level::entity::EntityInstancePtr requestedEntity = levelData->getEntityByID(p_id);
	if (requestedEntity == 0)
	{
		// Couldn't find entity
		TT_WARN("Couldn't find entity with id '%d' in current level.", p_id);
		return 0;
	}
	
	entity::EntityHandle handle = entityMgr.createEntity(requestedEntity->getType(), p_id);
	if (handle.isEmpty())
	{
		// Couldn't create entity
		TT_WARN("Couldn't create entity with id '%d' and type '%s' in current level.",
		        p_id, requestedEntity->getType().c_str());
		return 0;
	}
	
	// We should make a copy of the properties here as initEntity might alter them
	entity::EntityProperties props(requestedEntity->getProperties());
	if (entityMgr.initEntity(handle, p_position, props) == false)
	{
		// Entity died after initialization
		return 0;
	}
	
	entity::Entity* entity = handle.getPtr();
	if (entity != 0 && entity->getEntityScript() != 0)
	{
		return entity->getEntityScript().get();
	}
	
	// Something went wrong
	TT_PANIC("Created and initialized entity with id '%d' and type '%s' is still invalid.",
	         p_id, requestedEntity->getType().c_str());
	return 0;
}


void Bindings::killEntity(EntityBase* p_entity)
{
	if (p_entity != 0)
	{
		entity::Entity* entity = p_entity->getHandle().getPtr();
		if (entity != 0 && entity->isInitialized())
		{
			entity->kill();
		}
	}
}


void Bindings::addButtonInputListeningEntity(EntityBase* p_entity, s32 p_priority)
{
	TT_NULL_ASSERT(p_entity);
	if (p_entity != 0)
	{
		return AppGlobal::getGame()->addButtonInputListeningEntity(p_entity->getHandle(), p_priority, false);
	}
}


void Bindings::addButtonInputBlockingListeningEntity(EntityBase* p_entity, s32 p_priority)
{
	TT_NULL_ASSERT(p_entity);
	if (p_entity != 0)
	{
		return AppGlobal::getGame()->addButtonInputListeningEntity(p_entity->getHandle(), p_priority, true);
	}
}


void Bindings::removeButtonInputListeningEntity(EntityBase* p_entity)
{
	TT_NULL_ASSERT(p_entity);
	if (p_entity != 0)
	{
		return AppGlobal::getGame()->removeButtonInputListeningEntity(p_entity->getHandle());
	}
}


void Bindings::removeAllButtonInputListeningEntities()
{
	return AppGlobal::getGame()->removeAllButtonInputListeningEntities();
}


void Bindings::addMouseInputListeningEntity(EntityBase* p_entity, s32 p_priority)
{
	TT_NULL_ASSERT(p_entity);
	if (p_entity != 0)
	{
		return AppGlobal::getGame()->addMouseInputListeningEntity(p_entity->getHandle(), p_priority, false);
	}
}


void Bindings::addMouseInputBlockingListeningEntity(EntityBase* p_entity, s32 p_priority)
{
	TT_NULL_ASSERT(p_entity);
	if (p_entity != 0)
	{
		return AppGlobal::getGame()->addMouseInputListeningEntity(p_entity->getHandle(), p_priority, true);
	}
}


void Bindings::removeMouseInputListeningEntity(EntityBase* p_entity)
{
	TT_NULL_ASSERT(p_entity);
	if (p_entity != 0)
	{
		return AppGlobal::getGame()->removeMouseInputListeningEntity(p_entity->getHandle());
	}
}


void Bindings::removeAllMouseInputListeningEntities()
{
	return AppGlobal::getGame()->removeAllMouseInputListeningEntities();
}


void Bindings::addKeyboardListeningEntity(EntityBase* p_entity, s32 p_priority)
{
	TT_NULL_ASSERT(p_entity);
	if (p_entity != 0)
	{
		return AppGlobal::getGame()->addKeyboardListeningEntity(p_entity->getHandle(), p_priority, false);
	}
}


void Bindings::addKeyboardBlockingListeningEntity(EntityBase* p_entity, s32 p_priority)
{
	TT_NULL_ASSERT(p_entity);
	if (p_entity != 0)
	{
		return AppGlobal::getGame()->addKeyboardListeningEntity(p_entity->getHandle(), p_priority, true);
	}
}


void Bindings::removeKeyboardListeningEntity(EntityBase* p_entity)
{
	TT_NULL_ASSERT(p_entity);
	if (p_entity != 0)
	{
		return AppGlobal::getGame()->removeKeyboardListeningEntity(p_entity->getHandle());
	}
}


void Bindings::removeAllKeyboardListeningEntities()
{
	return AppGlobal::getGame()->removeAllKeyboardListeningEntities();
}


real Bindings::getAppTimeInSeconds()
{
	// Drop the double precision because squirrel doesn't support that.
	return static_cast<real>(AppGlobal::getAppTime());
}


real Bindings::getGameTimeInSeconds()
{
	// Drop the double precision because squirrel doesn't support that.
	return (AppGlobal::hasGame()) ? static_cast<real>(AppGlobal::getGame()->getGameTimeInSeconds()) : 0.0f;
}


s32 Bindings::getDateDay()
{
	return tt::system::Calendar::getCurrentDate().getDay();
}


s32 Bindings::getDateMonth()
{
	return tt::system::Calendar::getCurrentDate().getMonth();
}


s32 Bindings::getDateYear()
{
	return tt::system::Calendar::getCurrentDate().getYear();
}


std::string Bindings::getDateString()
{
	tt::system::Calendar date(tt::system::Calendar::getCurrentDate());
	char buf[256];
	sprintf(buf, "%04d%02d%02d", date.getYear(), date.getMonth(), date.getDay());
	return buf;
}


std::string Bindings::getTimeString()
{
	tt::system::Calendar date(tt::system::Calendar::getCurrentDate());
	char buf[256];
	sprintf(buf, "%02d%02d", date.getHour(), date.getMinute());
	return buf;
}


std::string Bindings::getVersion()
{
	const std::string clientVersion(tt::str::toStr(tt::version::getClientRevisionNumber()));
	const std::string libVersion(tt::str::toStr(tt::version::getLibRevisionNumber()));
	
	return clientVersion + "." + libVersion;
}


std::string Bindings::getRegion()
{
	return tt::settings::getRegionName();
}


level::CollisionType Bindings::getCollisionType(const tt::math::Vector2& p_worldPos)
{
	return AppGlobal::getGame()->getTileRegistrationMgr().getCollisionType(level::worldToTile(p_worldPos));
}


level::CollisionType Bindings::getCollisionTypeLevelOnly(const tt::math::Vector2& p_worldPos)
{
	const tt::math::Point2 tilePos(level::worldToTile(p_worldPos));
	level::AttributeLayerPtr attribLayer(AppGlobal::getGame()->getLevelData()->getAttributeLayer());
	return attribLayer->contains(tilePos) ? attribLayer->getCollisionType(tilePos) : level::CollisionType_Solid;
}


level::ThemeType Bindings::getThemeType(const tt::math::Vector2& p_worldPos)
{
	return AppGlobal::getGame()->getThemeAtTilePosition(level::worldToTile(p_worldPos));
}


void Bindings::setThemeType(const tt::math::Vector2& p_worldPos, level::ThemeType p_type)
{
	return AppGlobal::getGame()->setThemeAtTilePosition(level::worldToTile(p_worldPos), p_type);
}


void Bindings::addShoeboxPlaneToShoebox(const wrappers::ShoeboxPlaneDataWrapper& p_shoeboxPlane)
{
	AppGlobal::getGame()->addShoeboxPlaneToShoebox(p_shoeboxPlane.getData());
}


void Bindings::addShoeboxInclude(const std::string& p_filename, const tt::math::Vector2& p_offset, real p_offsetZ, s32 p_priority, real p_scale)
{
	AppGlobal::getGame()->addShoeboxInclude(p_filename, tt::math::Vector3(p_offset.x, p_offset.y, p_offsetZ), p_priority, p_scale);
}


tt::math::Vector2 Bindings::getShoeboxSpaceFromWorldPos(const tt::math::Vector2& p_worldPosition)
{
	return AppGlobal::getGame()->getShoeboxSpaceFromWorldPos(p_worldPosition).xy();
}


void Bindings::setTileAttributesVisible(bool p_visible)
{
	AppGlobal::getGame()->setGameLayerVisible(GameLayer_Attributes, p_visible);
}


void Bindings::sendShoeboxTagEvent(const std::string& p_tag, const std::string& p_event,
                                   const std::string& p_param)
{
	AppGlobal::getGame()->queueShoeboxTagEvent(p_tag, p_event, p_param);
}


std::string Bindings::getUsername()
{
#if defined(TT_STEAM_BUILD)
	return tt::steam::getSanitizedUsername();
#elif defined(TT_PLATFORM_WIN)
	static std::string name;
	if (name.empty())
	{
		TCHAR username[UNLEN+1];
		DWORD usernameLen = UNLEN+1;
		GetUserName(username, &usernameLen);
		
		// Sanitize the username; only allow a-z0-9 with a max of 32 chars
		std::string chopped = tt::str::toLower(tt::str::narrow(username).substr(0, 32));
		for (std::string::const_iterator it = chopped.begin(); it != chopped.end(); ++it)
		{
			u8 c = static_cast<u8>(*it);
			if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9'))
			{
				name.push_back(c);
			}
			else
			{
				name.push_back('-');
			}
		}
	}
	return name;
#else
	return "<invalid>";
#endif
}


s32 Bindings::sendGetRequest(const std::string& p_server, const std::string& p_url, const tt::str::Strings& p_params)
{
	if (tt::http::HttpConnectMgr::hasInstance() == false)
	{
		return -1;
	}
	
	tt::http::HttpRequest request;
	
	request.url = p_url;
	request.server = p_server;
#if !defined(TT_BUILD_FINAL)
	request.responseHandler = &g_httpResponseHandler;
#endif
	
	// Build FunctionArguments
	TT_ASSERT((p_params.size() % 2) == 0);
	for (s32 i = 0; i < static_cast<s32>(p_params.size())-1; i += 2)
	{
		request.getParameters[p_params[i]] = p_params[i+1];
	}
	request.requestMethod = tt::http::HttpRequest::RequestMethod_Get;
	
	return tt::http::HttpConnectMgr::getInstance()->queueRequest(request);
}


void Bindings::openURL(const std::string& p_url)
{
#if defined(TT_STEAM_BUILD)
	tt::steam::openURL(p_url);
#else
	if (tt::http::HttpConnectMgr::hasInstance())
	{
		tt::http::HttpConnectMgr::getInstance()->openUrlExternally(p_url);
	}
#endif
}


void Bindings::openStorePage()
{
#if defined(TT_STEAM_BUILD)
	if (tt::steam::openStorePage(201420) == false)
#else
	TT_PANIC("openStorePage() only works for steam builds. Will open URL to steam store page now.");
#endif
	{
		// Open of store page failed, try url.
		openURL("http://store.steampowered.com/app/201420/");
	}
}


bool Bindings::allowOnlineSessions()
{
#if defined(TT_STEAM_BUILD)
	return true;
#elif !defined(TT_BUILD_FINAL)
	return tt::app::getCmdLine().exists("emulate_online");
#else
	return false;
#endif
}


SQInteger Bindings::isSteamGroupMember(HSQUIRRELVM v)
{
#if defined(TT_STEAM_BUILD)
	ISteamUser* steamUser = SteamUser();
	if (steamUser != 0 && steamUser->BLoggedOn() == false || SteamFriends() == 0)
	{
		// Not logged on (offline mode). Don't check; pass null to script
		sq_pushnull(v);
		return 1;
	}
	
	// RIVE OGG Steam ID
	const CSteamID SteamGroupID = CSteamID(103582791437500089ULL);
	
	// Iterate all groups for this user
	for (int i = 0; i < SteamFriends()->GetClanCount(); ++i)
	{
		if(SteamFriends()->GetClanByIndex(i) == SteamGroupID)
		{
			sq_pushbool(v, true);
			return 1;
		}
	}
	
	sq_pushbool(v, false);
	return 1;
#else
	sq_pushbool(v, false);
	return 1;
#endif
}


bool Bindings::handleUserLevelCompleted()
{
	if (AppGlobal::hasGame())
	{
		return AppGlobal::getGame()->handleUserLevelCompleted();
	}
	return false;
}


void Bindings::addFluidWarp(const tt::math::VectorRect& p_rect, const std::string& p_name)
{
	AppGlobal::getGame()->getFluidMgr().addFluidWarp(p_rect, p_name);
}


void Bindings::deleteFluidWarpPair(const std::string& p_name)
{
	AppGlobal::getGame()->getFluidMgr().deleteFluidWarpPair(p_name);
}


void Bindings::startWave(const tt::math::VectorRect& p_rect, real p_strength, real p_duration)
{
	if (AppGlobal::hasGame() == false)
	{
		return;
	}
	
	// Center bottom pos.
	tt::math::Vector2 centerPos = p_rect.getCenterPosition();
	centerPos.y                 = p_rect.getMin().y + 0.5f;
	
	AppGlobal::getGame()->getFluidMgr().startWave(level::worldToTile(centerPos),
	                                              0,
	                                              p_rect.getWidth(),
	                                              p_strength,
	                                              p_duration);
}


ProgressType Bindings::getCurrentProgressType()
{
	return AppGlobal::getCurrentProgressType();
}


bool Bindings::isPositionInLight(const tt::math::Vector2& p_position)
{
	if (AppGlobal::hasGame())
	{
		return AppGlobal::getGame()->getLightMgr().isPositionInLight(p_position);
	}
	
	return false;
}


void Bindings::setBackgroundBlurLayers(const std::vector<real>& p_layers)
{
	if (AppGlobal::hasGame() == false)
	{
		return;
	}
	
	tt::engine::scene2d::BlurLayers newLayers;
	
	for(std::vector<real>::const_iterator it = p_layers.begin(); it != p_layers.end(); ++it)
	{
		TT_ASSERTMSG(*it < 0.0f,
			"Background blur layer should be a negative Z value, %g is invalid.", *it);
		
		newLayers.insert(*it);
	}
	
	const tt::engine::scene2d::shoebox::ShoeboxPtr& shoebox = AppGlobal::getGame()->getShoebox();
	TT_NULL_ASSERT(shoebox);
	if (shoebox != 0)
	{
		shoebox->setBackBlurLayers(newLayers);
	}
}


void Bindings::setForegroundBlurLayers(const std::vector<real>& p_layers)
{
	if (AppGlobal::hasGame() == false)
	{
		return;
	}
	
	tt::engine::scene2d::BlurLayers newLayers;
	
	for(std::vector<real>::const_iterator it = p_layers.begin(); it != p_layers.end(); ++it)
	{
		TT_ASSERTMSG(*it > 0.0f,
			"Foreground blur layer should be a positive Z value, %g is invalid.", *it);
		
		newLayers.insert(*it);
	}
	
	const tt::engine::scene2d::shoebox::ShoeboxPtr& shoebox = AppGlobal::getGame()->getShoebox();
	TT_NULL_ASSERT(shoebox);
	if (shoebox != 0)
	{
		shoebox->setForeBlurLayers(newLayers);
	}
}


std::vector<real> Bindings::getBackgroundBlurLayers()
{
	if (AppGlobal::hasGame() == false)
	{
		return std::vector<real>();
	}
	
	using namespace tt::engine::scene2d;
	const shoebox::ShoeboxConstPtr& shoebox = AppGlobal::getGame()->getShoebox();
	TT_NULL_ASSERT(shoebox);
	if (shoebox != 0)
	{
		const BlurLayers& layers = shoebox->getBackBlurLayers();
		std::vector<real> blurLayers(layers.begin(), layers.end());
		return blurLayers;
	}
	return std::vector<real>();
}


std::vector<real> Bindings::getForegroundBlurLayers()
{
	if (AppGlobal::hasGame() == false)
	{
		return std::vector<real>();
	}
	
	using namespace tt::engine::scene2d;
	const shoebox::ShoeboxConstPtr& shoebox = AppGlobal::getGame()->getShoebox();
	TT_NULL_ASSERT(shoebox);
	if (shoebox != 0)
	{
		const BlurLayers& layers = shoebox->getForeBlurLayers();
		std::vector<real> blurLayers(layers.begin(), layers.end());
		return blurLayers;
	}
	return std::vector<real>();
}


const tt::engine::renderer::ColorRGB& Bindings::getDefaultFogColor()
{
	if (AppGlobal::hasGame() == false)
	{
		return tt::engine::renderer::ColorRGB::white;
	}
	return AppGlobal::getGame()->getFogEffectMgr().getDefaultColorEnd();
}


void Bindings::setDefaultFogColor(const tt::engine::renderer::ColorRGB& p_color, real p_duration,
                                  tt::math::interpolation::EasingType p_easingType)
{
	if (AppGlobal::hasGame() == false)
	{
		return;
	}
	AppGlobal::getGame()->getFogEffectMgr().setDefaultColor(p_color, p_duration, p_easingType);
}


real Bindings::getDefaultFogNear()
{
	if (AppGlobal::hasGame() == false)
	{
		return -1.0f;
	}
	return AppGlobal::getGame()->getFogEffectMgr().getDefaultNearEnd();
}


real Bindings::getDefaultFogFar()
{
	if (AppGlobal::hasGame() == false)
	{
		return -1.0f;
	}
	return AppGlobal::getGame()->getFogEffectMgr().getDefaultFarEnd();
}


void Bindings::setDefaultFogNearFar(real p_near, real p_far, real p_duration,
                                    tt::math::interpolation::EasingType p_easingType)
{
	if (AppGlobal::hasGame() == false)
	{
		return;
	}
	
	TT_ASSERTMSG(tt::math::realEqual(p_near, p_far) == false,
		"setDefaultFogNearFar near '%.2f and far %.2f should not be equal. Undefined behavior.",
		p_near, p_far);
	
	AppGlobal::getGame()->getFogEffectMgr().setDefaultNearFar(p_near, p_far, p_duration, p_easingType);
}


void Bindings::resetFogSettings()
{
	if (AppGlobal::hasGame() == false)
	{
		return;
	}
	AppGlobal::getGame()->getFogEffectMgr().resetSettings();
}


wrappers::TextureWrapper Bindings::getColorGradingTexture(const std::string& p_effectName)
{
	using namespace tt::engine::renderer;
	TexturePtr texture = TextureCache::get(p_effectName, "color_grading", false);
	
	TT_ASSERTMSG(texture != 0, "Failed to load color grading effect: '%s'", p_effectName.c_str());
	
	if (entity::effect::ColorGradingEffectMgr::validateTextureForColorGrading(texture) == false)
	{
		texture.reset();
	}
	return wrappers::TextureWrapper(texture);
}


wrappers::TextureWrapper Bindings::getDefaultColorGrading()
{
	if (AppGlobal::hasGame() == false)
	{
		return wrappers::TextureWrapper();
	}
	return wrappers::TextureWrapper(AppGlobal::getGame()->getColorGradingEffectMgr().getDefaultColorGrading());
}


void Bindings::setDefaultColorGrading(const wrappers::TextureWrapper* p_effectTexture, real p_duration)
{
	if (AppGlobal::hasGame() == false)
	{
		return;
	}
	tt::engine::renderer::TexturePtr texture;
	if (p_effectTexture != 0)
	{
		texture = p_effectTexture->getTexture();
	}
	
	AppGlobal::getGame()->getColorGradingEffectMgr().setDefaultColorGrading(texture, p_duration);
}


bool Bindings::areColorGradingEffectsEnabled()
{
	return AppGlobal::getGame()->getColorGradingEffectMgr().areEffectsEnabled();
}


void Bindings::setColorGradingEffectsEnabled(bool p_enabled)
{
	AppGlobal::getGame()->getColorGradingEffectMgr().setEffectsEnabled(p_enabled);
}


bool Bindings::storeGameState(bool p_waitForThreadToFinish)
{
	return serialization::savePersistentDataAndShutdownState(p_waitForThreadToFinish);
}


void Bindings::setTVDisplayGamma(real p_gamma)
{
	(void)p_gamma;
}


void Bindings::setDRCDisplayGamma(real p_gamma)
{
	(void)p_gamma;
}


bool Bindings::isRumbleEnabled()
{
	return AppGlobal::getController(tt::input::ControllerIndex_One).isRumbleEnabled();
}


void Bindings::setRumbleEnabled(bool p_enabled)
{
	AppGlobal::getController(tt::input::ControllerIndex_One).setRumbleEnabled(p_enabled);
}


void Bindings::startRumble(input::RumbleStrength p_strength, real p_duration, real p_panning)
{
	AppGlobal::getController(tt::input::ControllerIndex_One).rumble(p_strength, p_duration, p_panning);
}


void Bindings::stopRumble()
{
	AppGlobal::getController(tt::input::ControllerIndex_One).stopRumble(true);
}


bool Bindings::isPointerAllowed()
{
	return AppGlobal::getController(tt::input::ControllerIndex_One).getPointerAutoVisibility();
}


void Bindings::setPointerAllowed(bool p_allow)
{
	AppGlobal::getController(tt::input::ControllerIndex_One).setPointerAutoVisibility(p_allow);
	if (p_allow == false)
	{
		AppGlobal::getController(tt::input::ControllerIndex_One).setPointerVisible(false);
	}
}


bool Bindings::isPointerVisible()
{
	return AppGlobal::getController(tt::input::ControllerIndex_One).isPointerVisible();
}


void Bindings::setPlayerCount(s32 p_newCount)
{
	tt::app::getApplication()->setPlayerCount(p_newCount);
}


void Bindings::setGamepadControlScheme(input::GamepadControlScheme p_scheme)
{
	AppGlobal::getController(tt::input::ControllerIndex_One).setGamepadControlScheme(p_scheme);
}


input::GamepadControlScheme Bindings::getGamepadControlScheme()
{
	return AppGlobal::getController(tt::input::ControllerIndex_One).getGamepadControlScheme();
}


SQInteger Bindings::getKeyBindingsTable(HSQUIRRELVM p_vm)
{
	tt::script::SqTopRestorerHelper helper(p_vm, true, 1);
	
	using namespace tt::input;
	ActionMap keyboardMapping = KeyBindings::getCustomControllerBindings(keyboardID);
	sq_newtable(p_vm);
	
	for (ActionMap::const_iterator it = keyboardMapping.begin(); it != keyboardMapping.end(); ++it)
	{
		sq_pushstring(p_vm, (*it).first.c_str(), -1);
		sq_pushinteger(p_vm, *((*it).second.begin()));
		if (SQ_FAILED(sq_newslot(p_vm, -3, SQFalse)))
		{
			TT_PANIC("Cannot create table.");
			return 0;
		}
	}
	return 1;
}


SQInteger Bindings::setKeyBindingsTable(HSQUIRRELVM p_vm)
{
	tt::script::SqTopRestorerHelper helper(p_vm);
	
	SQInteger argc = sq_gettop(p_vm);
	
	if (argc != 2)
	{
		TT_PANIC("setKeyBindingsTable([table]) has %d argument(s), expected 1",
			argc - 1);
		sq_pushbool(p_vm, SQFalse);
		return 1;
	}
	
	// make sure a table exists on -1, this is required for initEntityFromSquirrel to work properly
	if (sq_gettype(p_vm, -1) != OT_TABLE)
	{
		TT_PANIC("setKeyBindingsTable() argument should be a table of key bindings");
		sq_pushbool(p_vm, SQFalse);
		return 1;
	}
	
	using namespace tt::input;
	ActionMap keyboardMapping;
	
	sq_pushnull(p_vm); //null iterator
	while(SQ_SUCCEEDED(sq_next(p_vm, -2)))
	{
		const SQChar* action = 0;
		if (SQ_FAILED(sq_getstring(p_vm, -2, &action)))
		{
			TT_PANIC("Table key should be a string");
			sq_pushbool(p_vm, SQFalse);
			return 1;
		}
		
		SQInteger code = 0;
		if (SQ_FAILED(sq_getinteger(p_vm, -1, &code)) || code <= 0 || code > 255)
		{
			TT_PANIC("Table value should be a positive integer (<= 255)");
			sq_pushbool(p_vm, SQFalse);
			return 1;
		}
		keyboardMapping[action].push_back(static_cast<KeyCode>(code));
		sq_pop(p_vm, 2);
	}
	sq_poptop(p_vm);
	
	AppGlobal::getController(tt::input::ControllerIndex_One).updateCustomKeyBindings(keyboardID, keyboardMapping);
	sq_pushbool(p_vm, SQTrue);
	return 1;
}


std::string Bindings::getKeyName(tt::input::Key p_key)
{
	return tt::input::getKeyName(p_key);
}


SQInteger Bindings::getMetaData(HSQUIRRELVM /*p_vm*/)
{
	toki::script::ScriptMgr::pushMetaData();
	return 1;
}


bool Bindings::isMinimapEnabled()
{
	return AppGlobal::getGame()->getMinimap().isEnabled();
}


bool Bindings::isMinimapHidden()
{
	return AppGlobal::getGame()->getMinimap().isHidden();
}


void Bindings::setMinimapEnabled(bool p_enabled)
{
	AppGlobal::getGame()->getMinimap().setEnabled(p_enabled);
}


void Bindings::setMinimapHidden( bool p_hidden)
{
	AppGlobal::getGame()->getMinimap().setHidden(p_hidden);
}


void Bindings::setMinimapYOffset(real p_offset)
{
	AppGlobal::getGame()->getMinimap().setYOffset(p_offset);
}


real Bindings::getMinimapYOffset()
{
	return AppGlobal::getGame()->getMinimap().getYOffset();
}


void Bindings::setMinimapSideBorderSize(real p_size)
{
	AppGlobal::getGame()->getMinimap().setSidesBorder(p_size);
}


real Bindings::getMinimapSideBorderSize()
{
	return AppGlobal::getGame()->getMinimap().getSidesBorder();
}


void Bindings::setMinimapLargestLevelWidth(s32 p_size)
{
	AppGlobal::getGame()->getMinimap().setBiggestLevelWidth(p_size);
}


real Bindings::getMinimapGraphicsSize()
{
	return AppGlobal::getGame()->getMinimap().getGraphicsSize();
}


void Bindings::setMinimapGraphicsSize(real p_size)
{
	AppGlobal::getGame()->getMinimap().setGraphicsSize(p_size);
}


real Bindings::getMinimapExtraWidth()
{
	return AppGlobal::getGame()->getMinimap().getExtraWidth();
}


void Bindings::setMinimapExtraWidth(real p_size)
{
	AppGlobal::getGame()->getMinimap().setExtraWidth(p_size);
}


real Bindings::getMinimapSideFadeWidth()
{
	return AppGlobal::getGame()->getMinimap().getSideFadeWidth();
}


void Bindings::setMinimapSideFadeWidth(real p_size)
{
	AppGlobal::getGame()->getMinimap().setSideFadeWidth(p_size);
}


wrappers::MusicTrackWrapper Bindings::createMusicTrack(const std::string& p_musicName)
{
	return wrappers::MusicTrackWrapper(
			audio::AudioPlayer::getInstance()->getMusicTrackMgr().createTrack(p_musicName));
}


void Bindings::destroyMusicTrack(wrappers::MusicTrackWrapper& p_track)
{
	audio::AudioPlayer::getInstance()->getMusicTrackMgr().destroyTrack(p_track.getHandle());
	p_track.invalidate();
}


void Bindings::setRestoreFailureLevel(const std::string& p_levelName)
{
	TT_ASSERTMSG(AppGlobal::findInLevelNames(p_levelName),
		"setRestoreFailureLevel: level name '%s' doesn't exist.", p_levelName.c_str());
	
	// Set restore failure level name; even if this is an incorrect name, the loading code will handle it
	// correctly by defaulting to restore_failure_level which is set in the config.xml
	AppGlobal::setRestoreFailureLevelName(p_levelName);
}


s32 Bindings::getShoeboxSplitPriority()
{
	const tt::engine::scene2d::shoebox::ShoeboxConstPtr& shoebox = AppGlobal::getGame()->getShoebox();
	TT_NULL_ASSERT(shoebox);
	if (shoebox != 0)
	{
		return shoebox->getSplitPriority();
	}
	return 0;
}


void Bindings::setShoeboxSplitPriority(s32 p_priority)
{
	const tt::engine::scene2d::shoebox::ShoeboxPtr& shoebox = AppGlobal::getGame()->getShoebox();
	TT_NULL_ASSERT(shoebox);
	if (shoebox != 0)
	{
		return shoebox->setSplitPriority(p_priority);;
	}
}


void Bindings::resetDemo()
{
	AppGlobal::resetDemo();
}


SQInteger Bindings::notifyGame(HSQUIRRELVM p_vm)
{
	if (AppGlobal::hasGame() == false)
	{
		return 0;
	}
	
	SQInteger argc = sq_gettop(p_vm);
	
	if (argc <= 1)
	{
		TT_PANIC("notifyGame(string) has %d argument(s), expected >= 1",
			argc - 1);
		return 0;
	}
	
	// Arg1: the callback
	const SQChar* strPtr = 0;
	if (SQ_FAILED(sq_getstring(p_vm, 2, &strPtr)))
	{
		TT_PANIC("notifyGame() first argument should be of type string");
		return 0;
	}
	
	const std::string callback(strPtr);
	
	if (callback == "onGameStarted")
	{
		AppGlobal::getGame()->onGameStarted();
	}
	else if (callback == "onGameEnded")
	{
		AppGlobal::getGame()->onGameEnded();
	}
	else if (callback == "onGamePaused")
	{
		AppGlobal::getGame()->onGamePaused();
	}
	else if (callback == "onGameResumed")
	{
		AppGlobal::getGame()->onGameResumed();
	}
	else if (callback == "onGamePastIntro")
	{
		bool canBeSkipped = AppGlobal::getGame()->onGamePastIntro();
		sq_pushbool(p_vm, canBeSkipped);
		return 1;
	}
	else if (callback == "onGameProgressChanged")
	{
		SQInteger progress = -1;
		if (SQ_FAILED(sq_getinteger(p_vm, 3, &progress)))
		{
			TT_PANIC("notifyGame(): onGameProgressChanged argument should be of type integer");
			return 0;
		}
		
		AppGlobal::getGame()->onGameProgressChanged(static_cast<s32>(progress));
	}
	else
	{
		TT_PANIC("Unhandled callback '%s'", strPtr);
	}
	
	return 0;
}


void Bindings::quitGame()
{
	tt::app::getApplication()->terminate(true);
}


void Bindings::toggleFpsCounter()
{
	AppGlobal::setShowFps(AppGlobal::getShowFps() == false);
}


void Bindings::simulateCrash()
{
	volatile static u32* crash(0);
	*crash = 42;
}


s32 Bindings::downloadLeaderboardScores(const std::string& p_leaderboard,
                                        tt::score::DownloadRequestRangeType p_rangeType, s32 p_rangeMin, s32 p_rangeMax,
                                        tt::score::DownloadRequestSource p_source, EntityBase* p_handlingEntity)
{
#if defined(TT_STEAM_BUILD)
	using namespace tt::steam;
#endif

#if defined(TT_STEAM_BUILD)
	if (Leaderboards::hasInstance() == false)
	{
		return -1;
	}
	
	Leaderboards* leaderboards = Leaderboards::getInstance();
	if (leaderboards != 0 && p_handlingEntity != 0)
	{
		const u32 userdata = p_handlingEntity->getHandle().getValue();
		tt::score::DownloadRequest request(p_leaderboard, p_rangeType, p_rangeMin, p_rangeMax, p_source,
			downloadLeaderboardScoresCallback, userdata);
		return leaderboards->addDownloadRequest(request);
	}
#else
	(void)p_leaderboard;
	(void)p_rangeType;
	(void)p_rangeMin;
	(void)p_rangeMax;
	(void)p_source;
	(void)p_handlingEntity;
#endif
	
	return -1;
}


s32 Bindings::uploadLeaderboardScore(const std::string& p_leaderboard, s32 p_score)
{
#if defined(TT_STEAM_BUILD)
	using namespace tt::steam;
#endif
	
#if defined(TT_STEAM_BUILD)
	Leaderboards* leaderboards = Leaderboards::getInstance();
	if (leaderboards != 0)
	{
		return leaderboards->addUploadRequest(tt::score::UploadRequest(p_leaderboard, p_score));
	}
#else
	(void)p_leaderboard;
	(void)p_score;
#endif
	
	return -1;
}


void Bindings::commitLeaderboardRequests()
{
#if defined(TT_STEAM_BUILD)
	using namespace tt::steam;
#endif
	
#if defined(TT_STEAM_BUILD)
	if (Leaderboards::hasInstance() == false)
	{
		return;
	}
	
	Leaderboards* leaderboards = Leaderboards::getInstance();
	if (leaderboards != 0)
	{
		leaderboards->commitRequests();
	}
#endif
}


tt::str::Strings Bindings::getAllConversationNames()
{
	static tt::str::Strings conversations;
	if (conversations.empty())
	{
		// FIXME: Remove hardcoded dependency on folder
		tt::str::StringSet filenames(tt::fs::utils::getFilesInDir("conversations", "*.json"));
		for (tt::str::StringSet::iterator it = filenames.begin(); it != filenames.end(); ++it)
		{
			conversations.push_back(*it);
		}
	}
	
	return conversations;
}


SQInteger Bindings::getJSONFromString(HSQUIRRELVM v)
{
	SQInteger argc = sq_gettop(v);
	
	if (argc != 2)
	{
		TT_PANIC("getJSONFromString(string) has %d argument(s), expected 1",
			argc - 1);
		return 0;
	}
	
	// Arg1: the json
	const SQChar* strPtr = 0;
	if (SQ_FAILED(sq_getstring(v, 2, &strPtr)))
	{
		TT_PANIC("getJSONFromString() first argument should be of type string");
		return 0;
	}
	toki::script::ScriptMgr::pushJSONFromString(strPtr);
	return 1;
}


SQInteger Bindings::getJSONFromFile(HSQUIRRELVM v)
{
	SQInteger argc = sq_gettop(v);
	
	if (argc != 2)
	{
		TT_PANIC("getJSONFromFile(string) has %d argument(s), expected 1",
			argc - 1);
		return 0;
	}
	
	// Arg1: filename
	const SQChar* strPtr = 0;
	if (SQ_FAILED(sq_getstring(v, 2, &strPtr)))
	{
		TT_PANIC("getJSONFromFile() first argument should be of type string");
		return 0;
	}
	toki::script::ScriptMgr::pushJSONFromFile(strPtr);
	return 1;
}


void Bindings::clearDebugConsole()
{
#if defined(TT_PLATFORM_WIN)
	COORD coordScreen = { 0, 0 };    // home for the cursor 
	DWORD cCharsWritten;
	CONSOLE_SCREEN_BUFFER_INFO csbi; 
	DWORD dwConSize;
	
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	// Get the number of character cells in the current buffer. 
	if (GetConsoleScreenBufferInfo(hConsole, &csbi) == FALSE)
	{
		return;
	}
	
	dwConSize = csbi.dwSize.X * csbi.dwSize.Y;
	
	// Fill the entire screen with blanks.
	if(FillConsoleOutputCharacter(hConsole,        // Handle to console screen buffer 
	                              (TCHAR) ' ',     // Character to write to the buffer
	                              dwConSize,       // Number of cells to write 
	                              coordScreen,     // Coordinates of first cell 
	                              &cCharsWritten ) == FALSE)// Receive number of characters written
	{
		return;
	}
	
	// Get the current text attribute.
	if(GetConsoleScreenBufferInfo(hConsole, &csbi) == FALSE)
	{
		return;
	}
	
	// Set the buffer's attributes accordingly.
	if(FillConsoleOutputAttribute(hConsole,         // Handle to console screen buffer 
	                              csbi.wAttributes, // Character attributes to use
	                              dwConSize,        // Number of cells to set attribute 
	                              coordScreen,      // Coordinates of first cell 
	                              &cCharsWritten ) == FALSE) // Receive number of characters written
	{
		return;
	}
	
	// Put the cursor at its home coordinates.
	SetConsoleCursorPosition(hConsole, coordScreen);
#endif // #if defined(TT_PLATFORM_WINDOWS)
}


const std::string& Bindings::getLanguage()
{
	return AppGlobal::getLoc().getLanguage();
}


void Bindings::setLanguage(const std::string& p_language)
{
	utils::GlyphSetMgr::unloadAll();
	AppGlobal::getLoc().setLanguage(p_language);
	utils::GlyphSetMgr::loadAll();
}


void Bindings::setLanguageToPlatformDefault()
{
	setLanguage(tt::system::Language::getLanguage());
}


std::string Bindings::getLocalizedStringUTF8(const std::string& p_locID)
{
	return tt::str::utf16ToUtf8(AppGlobal::getLoc().getLocStr(loc::SheetID_Game).getString(p_locID));
}


std::string Bindings::getLocalizedAchievementStringUTF8(const std::string& p_locID)
{
	return tt::str::utf16ToUtf8(AppGlobal::getLoc().getLocStr(loc::SheetID_Achievements).getString(p_locID));
}


// Namespace end
}
}
}
