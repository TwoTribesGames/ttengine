#include <tt/fs/utils/utils.h>

#include <toki/game/entity/EntityMgr.h>
#include <toki/game/script/wrappers/LevelWrapper.h>
#include <toki/game/script/sqbind_bindings.h>
#include <toki/game/Game.h>
#include <toki/input/Recorder.h>
#include <toki/level/LevelData.h>
#include <toki/steam/Workshop.h>
#include <toki/AppGlobal.h>


namespace toki {
namespace game {
namespace script {
namespace wrappers {


std::string LevelWrapper::getName()
{
	return AppGlobal::getGame()->getStartInfo().getLevelName();
}


bool LevelWrapper::isUserLevel()
{
	return AppGlobal::getGame()->getStartInfo().isUserLevel();
}


bool LevelWrapper::isWorkshopLevel()
{
#if defined(TT_STEAM_BUILD)
	return AppGlobal::getGame()->getStartInfo().getWorkshopFileID() != 0;
#else
	return false;  // not a Steam build: level can never be a Workshop level
#endif
}


void LevelWrapper::load(const std::string& p_levelName)
{
	Game::loadLevel(p_levelName);
}


void LevelWrapper::loadWithProgressType(const std::string& p_levelName, ProgressType p_overrideProgressType)
{
	Game::loadLevel(p_levelName, p_overrideProgressType);
}


void LevelWrapper::setMissionID(const std::string& p_missionID)
{
	AppGlobal::getGame()->setMissionID(p_missionID);
}


std::string LevelWrapper::getMissionID()
{
	return AppGlobal::getGame()->getMissionID();
}


void LevelWrapper::startMissionID(const std::string& p_missionID)
{
	Game::startMission(p_missionID);
}


tt::str::Strings LevelWrapper::getAllMissionIDs()
{
	static tt::str::Strings missions;
	if (missions.empty())
	{
		// FIXME: Remove hardcoded dependency on folder
		tt::str::StringSet filenames(tt::fs::utils::getFilesInDir("missions", "*.json"));
		for (tt::str::StringSet::iterator it = filenames.begin(); it != filenames.end(); ++it)
		{
			missions.push_back(*it);
		}
	}
	
	return missions;
}


void LevelWrapper::restart()
{
	// Make sure the startup info is set to the current level
	AppGlobal::setGameStartInfo(AppGlobal::getGame()->getStartInfo());
	
	// Restart the level by restarting the game state
	AppGlobal::getGame()->forceReload();
}


void LevelWrapper::signalExit()
{
	// Notify recorder so that it can record a new section
	AppGlobal::getInputRecorder()->onLevelExit();
}


s32 LevelWrapper::getWidth()
{
	return AppGlobal::getGame()->getLevelData()->getLevelWidth();
}


s32 LevelWrapper::getHeight()
{
	return AppGlobal::getGame()->getLevelData()->getLevelHeight();
}


void LevelWrapper::setWorkshopVote(bool p_thumbsUp)
{
#if defined(TT_STEAM_BUILD)
	if (AppGlobal::hasGame() == false)
	{
		TT_PANIC("Cannot set Workshop vote for current level: no level is loaded.");
		return;
	}
	
	const u64 workshopId(AppGlobal::getGame()->getStartInfo().getWorkshopFileID());
	if (workshopId == 0)
	{
		TT_PANIC("Cannot set Workshop vote for current level: this level is not a Workshop level.");
		return;
	}
	
	//TT_Printf("LevelWrapper::setWorkshopVote: Setting Workshop vote to '%s'\n",
	//          p_thumbsUp ? "thumbs up" : "thumbs down");
	steam::Workshop::getInstance()->setUserVote(workshopId, p_thumbsUp);
#else
	(void)p_thumbsUp;
#endif
}


SQInteger LevelWrapper::createSpawnSection(HSQUIRRELVM p_vm)
{
	const SQInteger argc = sq_gettop(p_vm);
	
	if (argc < 3 || argc > 5)
	{
		TT_PANIC("createSpawnSection(sectionID, VectorRect, [array], [array]) has %d argument(s), expected 2, 3 or 4.",
			argc - 1);
		return 0;
	}
	
	// Arg1: Section ID
	if (sq_gettype(p_vm, 2) != OT_INTEGER)
	{
		TT_PANIC("createSpawnSection() argument 1 should be an integer");
		return 0;
	}
	SQInteger sectionId = 0;
	sq_getinteger(p_vm, 2, &sectionId);
	
	// Arg2: VectorRect
	level::entity::EntityInstances instances;
	game::Game* game = AppGlobal::getGame();
	const level::LevelDataPtr& levelData(game->getLevelData());
	if (sq_gettype(p_vm, 3) != OT_NULL)
	{
		tt::math::VectorRect rect(SqBind<tt::math::VectorRect>::get(p_vm, 3));
		instances = levelData->getEntitiesInRect(rect);
	}
	
	// Arg3: Entity inclusion ids
	if (argc > 3 && sq_gettype(p_vm, 4) != OT_NULL)
	{
		if (sq_gettype(p_vm, 4) != OT_ARRAY)
		{
			TT_PANIC("createSpawnSection() argument 3 should be an array of entity ids");
			return 0;
		}
		HSQOBJECT obj;
		sq_getstackobj(p_vm, 4, &obj);
		sq_pushobject(p_vm, obj);
		sq_pushnull(p_vm); //null iterator
		while(SQ_SUCCEEDED(sq_next(p_vm, -2)))
		{
			SQInteger id = 0;
			if (SQ_FAILED(sq_getinteger(p_vm, -1, &id)))
			{
				TT_PANIC("createSpawnSection() argument 3 contains non-integer values");
				sq_pop(p_vm, 2);
				continue;
			}
			
			level::entity::EntityInstancePtr entity = levelData->getEntityByID(static_cast<s32>(id));
			if (entity != 0)
			{
				instances.push_back(entity);
			}
			else
			{
				TT_PANIC("Unknown entity id '%d'", id);
			}
			sq_pop(p_vm, 2);
		}
		sq_pop(p_vm, 2);
	}
	
	// Arg4: Entity exclusion ids
	if (argc > 4 && sq_gettype(p_vm, 5) != OT_NULL)
	{
		if (sq_gettype(p_vm, 5) != OT_ARRAY)
		{
			TT_PANIC("createSpawnSection() argument 4 should be an array of entity ids");
			return 0;
		}
		HSQOBJECT obj;
		sq_getstackobj(p_vm, 5, &obj);
		sq_pushobject(p_vm, obj);
		sq_pushnull(p_vm); //null iterator
		while(SQ_SUCCEEDED(sq_next(p_vm, -2)))
		{
			SQInteger id = 0;
			if (SQ_FAILED(sq_getinteger(p_vm, -1, &id)))
			{
				TT_PANIC("createSpawnSectiontities() argument 4 contains non-integer values");
				sq_pop(p_vm, 2);
				continue;
			}
			
			for (level::entity::EntityInstances::iterator it = instances.begin(); it != instances.end(); ++it)
			{
				if ((*it)->getID() == id)
				{
					instances.erase(it);
					break;
				}
			}
			sq_pop(p_vm, 2);
		}
		sq_pop(p_vm, 2);
	}
	
	const entity::EntityLibrary& entityLib(AppGlobal::getEntityLibrary());
	for (level::entity::EntityInstances::iterator it = instances.begin(); it != instances.end(); ++it)
	{
		const level::entity::EntityInfo* entityInfo(entityLib.getEntityInfo((*it)->getType()));
		if (entityInfo != 0 && entityInfo->ignoreSpawnSections() == false)
		{
			(*it)->setSpawnSectionID(static_cast<s32>(sectionId));
		}
	}
	
	return 0; // No return value
}


void LevelWrapper::spawnSpawnSection(s32 p_spawnSectionID)
{
	if (p_spawnSectionID < 0)
	{
		TT_PANIC("p_spawnSectionID should be >= 0");
		return;
	}
	game::Game* game = AppGlobal::getGame();
	entity::EntityMgr& entityMgr(game->getEntityMgr());
	entityMgr.createEntities(game->getLevelData()->getAllEntities(), game->getMissionID(),
		-1, tt::math::Vector2::zero, false, p_spawnSectionID);
	entityMgr.initCullingForAllEntities(game->getCamera());
}


void LevelWrapper::killSpawnSection(s32 p_spawnSectionID)
{
	if (p_spawnSectionID < 0)
	{
		TT_PANIC("p_spawnSectionID should be >= 0");
		return;
	}
	game::Game* game = AppGlobal::getGame();
	entity::EntityMgr& entityMgr(game->getEntityMgr());
	
	// Kill entities
	const level::entity::EntityInstances& entities = game->getLevelData()->getAllEntities();
	for (level::entity::EntityInstances::const_iterator it = entities.begin();
	     it != entities.end(); ++it)
	{
		if ((*it)->getSpawnSectionID() == p_spawnSectionID)
		{
			game::entity::EntityHandle handle(entityMgr.getEntityHandleByID((*it)->getID()));
			entity::Entity* entity = handle.getPtr();
			if (entity != 0 && entity->isInitialized())
			{
				entity->getEntityScript()->onPreSpawnSectionKill();
				entity->kill();
			}
		}
	}
}


void LevelWrapper::bind(const tt::script::VirtualMachinePtr& p_vm)
{
	TT_SQBIND_SETVM(p_vm);
	
	TT_SQBIND_INIT_NO_INSTANCING_NAME(LevelWrapper, "Level");
	TT_SQBIND_STATIC_METHOD(LevelWrapper, getName);
	TT_SQBIND_STATIC_METHOD(LevelWrapper, isUserLevel);
	TT_SQBIND_STATIC_METHOD(LevelWrapper, isWorkshopLevel);
	TT_SQBIND_STATIC_METHOD(LevelWrapper, load);
	TT_SQBIND_STATIC_METHOD(LevelWrapper, loadWithProgressType);
	TT_SQBIND_STATIC_METHOD(LevelWrapper, setMissionID);
	TT_SQBIND_STATIC_METHOD(LevelWrapper, getMissionID);
	TT_SQBIND_STATIC_METHOD(LevelWrapper, startMissionID);
	TT_SQBIND_STATIC_METHOD(LevelWrapper, getAllMissionIDs);
	TT_SQBIND_STATIC_METHOD(LevelWrapper, restart);
	TT_SQBIND_STATIC_METHOD(LevelWrapper, signalExit);
	TT_SQBIND_STATIC_METHOD(LevelWrapper, getWidth);
	TT_SQBIND_STATIC_METHOD(LevelWrapper, getHeight);
	TT_SQBIND_STATIC_METHOD(LevelWrapper, setWorkshopVote);
	TT_SQBIND_STATIC_SQUIRREL_METHOD(LevelWrapper, createSpawnSection);
	TT_SQBIND_STATIC_METHOD(LevelWrapper, spawnSpawnSection);
	TT_SQBIND_STATIC_METHOD(LevelWrapper, killSpawnSection);
}

// Namespace end
}
}
}
}
