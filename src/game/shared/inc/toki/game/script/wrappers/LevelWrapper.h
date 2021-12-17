#if !defined(INC_TOKITORI_GAME_SCRIPT_WRAPPERS_LEVELWRAPPER_H)
#define INC_TOKITORI_GAME_SCRIPT_WRAPPERS_LEVELWRAPPER_H


#include <tt/script/helpers.h>
#include <toki/constants.h>


namespace toki {
namespace game {
namespace script {
namespace wrappers {

/*! \brief 'Level' in Squirrel. */
class LevelWrapper
{
public:
	/*! \brief Returns the name of the current level */
	static std::string getName();
	
	/*! \brief Indicates whether this is a user level or a built-in one. */
	static bool isUserLevel();
	
	/*! \brief Indicates whether this is a level from Steam Workshop. */
	static bool isWorkshopLevel();
	
	/*! \brief Loads another level. */
	static void load(const std::string& p_levelName);
	
	/*! brief Load another level which will use a specific progresstype. */
	static void loadWithProgressType(const std::string& p_levelName, ProgressType p_overrideProgressType);
	
	/*! \brief Sets the current mission id. Careful with this one; only use this before loading a level. */
	static void setMissionID(const std::string& p_missionID);
	
	/*! \brief Gets the current mission id. */
	static std::string getMissionID();
	
	/*! \brief Starts a mission with a certain id */
	static void startMissionID(const std::string& p_missionID);
	
	/*! \brief Returns the IDs of all missions */
	static tt::str::Strings getAllMissionIDs();
	
	/*! \brief Restarts the current level. */
	static void restart();
	
	/*! \brief Use this function to signal the game that a levelexit is reached. */
	static void signalExit();
	
	/*! \brief Returns the width of the currently loaded level, in tiles. */
	static s32 getWidth();
	
	/*! \brief Returns the height of the currently loaded level, in tiles. */
	static s32 getHeight();
	
	/*! \brief Votes up or down on the current level, but only if the current level is a Steam Workshop level. */
	static void setWorkshopVote(bool p_thumbsUp);
	
	/*! \brief Creates an entity section in the level data
		\param [integer] Spawnsection ID
		\param [VectorRect] The area occupied by the spawnsection
		\param Optional [array] Additional including entities
		\param Optional [array] Additional excluding entities
		\return Nothing */
	static SQInteger createSpawnSection(HSQUIRRELVM p_vm);
	
	/*! \brief Spawns a spawn section */
	static void spawnSpawnSection(s32 p_spawnSectionID);
	
	/*! \brief Deletes all entities spawned by the spawn section */
	static void killSpawnSection(s32 p_spawnSectionID);
	
	static void bind(const tt::script::VirtualMachinePtr& p_vm);
};


// Namespace end
}
}
}
}

#endif // !defined(INC_TOKITORI_GAME_SCRIPT_WRAPPERS_LEVELWRAPPER_H)
