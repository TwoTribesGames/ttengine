#include <tt/stats/stats.h>

#include <toki/game/script/wrappers/StatsWrapper.h>

namespace toki {
namespace game {
namespace script {
namespace wrappers {

//--------------------------------------------------------------------------------------------------
// Public member functions

void StatsWrapper::addAchievement(const std::string& p_name, s32 p_id)
{
	tt::stats::addAchievement(p_name, p_id);
}


void StatsWrapper::addStatDefinition(const std::string& p_name, tt::stats::StatType p_type)
{
	tt::stats::addStatDefinition(p_name, p_type);
}


void StatsWrapper::addIntProgressAchievementRequirement(const std::string& p_statName, const std::string& p_unlockAchievement, s32 p_unlockValue)
{
	tt::stats::addProgressAchievementRequirement(p_statName, p_unlockAchievement, tt::stats::StatValue(p_unlockValue));
}


void StatsWrapper::addFloatProgressAchievementRequirement(const std::string& p_statName, const std::string& p_unlockAchievement, real p_unlockValue)
{
	tt::stats::addProgressAchievementRequirement(p_statName, p_unlockAchievement, tt::stats::StatValue(p_unlockValue));
}


void StatsWrapper:: setAchievementUnlockedStatus(const std::string& p_name, bool p_isUnlocked)
{
	tt::stats::setAchievementUnlockedStatus(p_name, p_isUnlocked);
}


bool StatsWrapper::isProgressAchievement(const std::string& p_name)
{
	return tt::stats::isProgressAchievement(p_name);
}


real StatsWrapper::getProgressAchievementCompletionPercentage(const std::string& p_name)
{
	return tt::stats::getProgressAchievementCompletionPercentage(p_name);
}


void StatsWrapper::setIntStat(const std::string& p_name, s32  p_value)
{
	tt::stats::setIntStat(p_name, p_value);
}


void StatsWrapper::setFloatStat(const std::string& p_name, real p_value)
{
	tt::stats::setFloatStat(p_name, p_value);
}


void StatsWrapper::storeStats()
{
	tt::stats::storeStats();
}


void StatsWrapper::unlockAchievement(const std::string& p_APIName)
{
	tt::stats::unlockAchievement(p_APIName);
}


void StatsWrapper::storeAchievements()
{
	tt::stats::storeAchievements();
}


void StatsWrapper::resetAll()
{
	tt::stats::resetStats();
	tt::stats::resetAchievements();
}


void StatsWrapper::bind(const tt::script::VirtualMachinePtr& p_vm)
{
	TT_SQBIND_SETVM(p_vm);
	
	TT_SQBIND_INIT_NO_INSTANCING_NAME(StatsWrapper, "Stats");
	TT_SQBIND_STATIC_METHOD(StatsWrapper, addAchievement);
	TT_SQBIND_STATIC_METHOD(StatsWrapper, addStatDefinition);
	TT_SQBIND_STATIC_METHOD(StatsWrapper, addIntProgressAchievementRequirement);
	TT_SQBIND_STATIC_METHOD(StatsWrapper, addFloatProgressAchievementRequirement);
	TT_SQBIND_STATIC_METHOD(StatsWrapper, setAchievementUnlockedStatus);
	TT_SQBIND_STATIC_METHOD(StatsWrapper, isProgressAchievement);
	TT_SQBIND_STATIC_METHOD(StatsWrapper, getProgressAchievementCompletionPercentage);
	TT_SQBIND_STATIC_METHOD(StatsWrapper, setIntStat);
	TT_SQBIND_STATIC_METHOD(StatsWrapper, setFloatStat);
	TT_SQBIND_STATIC_METHOD(StatsWrapper, storeStats);
	TT_SQBIND_STATIC_METHOD(StatsWrapper, unlockAchievement);
	TT_SQBIND_STATIC_METHOD(StatsWrapper, storeAchievements);
	TT_SQBIND_STATIC_METHOD(StatsWrapper, resetAll);
}


// Namespace end
}
}
}
}
