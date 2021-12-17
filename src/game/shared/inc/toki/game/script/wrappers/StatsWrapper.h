#if !defined(INC_TOKITORI_GAME_SCRIPT_WRAPPERS_STATWRAPPER_H)
#define INC_TOKITORI_GAME_SCRIPT_WRAPPERS_STATWRAPPER_H

#include <tt/script/helpers.h>
#include <tt/stats/stats.h>

#include <toki/game/script/wrappers/fwd.h>
#include <toki/game/script/sqbind_bindings.h>


namespace toki {
namespace game {
namespace script {
namespace wrappers {

/*! \brief 'Stats' in Squirrel. */
class StatsWrapper
{
public:
	/*! \brief Register a Stat (Steam statistic) definition.
	    \note Call this at compile time! (Will assert if this is not the case.)
	          The name needs to be the same as what's added to the steam partner website! */
	static void addAchievement(const std::string& p_name, s32 p_id);
	
	/*! \brief Register a Stat (Steam statistic) definition.
	    \note Call this at compile time! (Will assert if this is not the case.)
	          The name needs to be the same as what's added to the steam partner website! */
	static void addStatDefinition(const std::string& p_name, tt::stats::StatType p_type);
	
	/*! \brief Adds an integer progress achievement */
	static void addIntProgressAchievementRequirement(const std::string& p_statName, const std::string& p_unlockAchievement, s32 p_unlockValue);
	
	/*! \brief Adds a float progress achievement */
	static void addFloatProgressAchievementRequirement(const std::string& p_statName, const std::string& p_unlockAchievement, real p_unlockValue);
	
	/*! \brief Sets unlocked status of an achievement */
	static void setAchievementUnlockedStatus(const std::string& p_name, bool p_isUnlocked);
	
	/*! \brief Is achievement a progress achievement or not */
	static bool isProgressAchievement(const std::string& p_name);
	
	/*! \brief Is achievement a progress achievement or not */
	static real getProgressAchievementCompletionPercentage(const std::string& p_name);
	
	/*! \brief Set a integer statistic.
	    \param p_name The name as defined in addStatDefinition.
	    \param p_value The new value for the statistic.
	    \note Call storeStats after this! (type defined with addStatDefinition needs to be int!)*/
	static void setIntStat(  const std::string& p_name, s32  p_value);
	
	/*! \brief Set a float statistic.
	    \param p_name The name as defined in addStatDefinition.
	    \param p_value The new value for the statistic.
	    \note Call storeStats after this! (type defined with addStatDefinition needs to be float!)*/
	static void setFloatStat(const std::string& p_name, real p_value);
	
	/*! \brief Upload the statistics to (steam) server. */
	static void storeStats();
	
	/*! \brief Unlock an achievement
	    \param p_name achievement name
	    \note Call storeAchievements after this! */
	static void unlockAchievement(const std::string& p_name);
	
	/*! \brief Upload the achievements to (steam) server. */
	static void storeAchievements();
	
	/*! \brief DEBUG reset all statistics and achievements.
	    \note Non-final builds only, does nothing in final builds. */
	static void resetAll();
	
	static void bind(const tt::script::VirtualMachinePtr& p_vm);
};


// Namespace end
}
}
}
}

#endif // !defined(INC_TOKITORI_GAME_SCRIPT_WRAPPERS_STATWRAPPER_H)
