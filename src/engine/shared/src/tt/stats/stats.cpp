#include <tt/code/helpers.h>
#include <tt/math/math.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
#include <tt/platform/tt_types.h>
#include <tt/script/VirtualMachine.h>
#include <tt/stats/stats.h>

namespace tt {
namespace stats {

struct StatEntry
{
	std::string name;
	StatType    type;
	StatValue   value;
	
	StatEntry()
	:
	name(),
	type(),
	value(static_cast<s32>(0))
	{}
	
	StatEntry(const std::string& p_name, StatType p_type)
	:
	name(p_name),
	type(p_type),
	value(static_cast<s32>(0))
	{}
};

struct ProgressAchievementEntry
{
	std::string achievementName;
	StatValue   unlockRequirement;
	
	ProgressAchievementEntry()
	:
	achievementName(),
	unlockRequirement(static_cast<s32>(0))
	{
	}
	
	ProgressAchievementEntry(const std::string& p_name, const StatValue& p_requirement)
	:
	achievementName(p_name),
	unlockRequirement(p_requirement)
	{}
};


using AchievementUnlockedStatus = std::map<std::string, bool>;
using AchievementNameToID = std::map<std::string, s32>;
using StatEntries = std::map<std::string, StatEntry>;
using ProgressAchievementEntries = std::map<std::string, ProgressAchievementEntry>;

static StatEntries                g_statDefinitionEntries;
static ProgressAchievementEntries g_progressAchievementEntries;
static bool                       g_statDefinitionPeriod = true;
tt::script::VirtualMachinePtr     g_scriptVM = nullptr;
AchievementUnlockedStatus         g_achievementUnlockedStatus;
AchievementNameToID               g_achievementNameToID;

#if defined(TT_STEAM_BUILD)
// We don't want to call script functions from another thread than main.
// To make sure this happens we don't call script in the onUserLoggedOn callback, but instead do that in the update.
enum UserLoggedOnStatus
{
	UserLoggedOnStatus_Initial,
	UserLoggedOnStatus_LoggedOn
};
static UserLoggedOnStatus g_status = UserLoggedOnStatus_Initial;


/*! \brief Callback - Check for achievements that might have been unlocked while offline. */
static void onUserLoggedOn()
{
	// Check stats based achievements
	TT_Printf("stats::onUserLoggedOn: [ACHIEVEMENTS]\n");
	
	TT_ASSERT(g_status == UserLoggedOnStatus_Initial);
	g_status = UserLoggedOnStatus_LoggedOn;
}
#endif // #if defined(TT_STEAM_BUILD)


//-------------------------------------------------------------------------------------------------
// Create/Destroy Functions

void createInstance(const tt::script::VirtualMachinePtr& p_vm)
{
	TT_ASSERT(g_scriptVM == nullptr);
	g_scriptVM = p_vm;
	g_statDefinitionPeriod = false;
	
#if defined(TT_STEAM_BUILD)
	// Initialize stats and achievements
	if (tt::steam::Achievements::createInstance(defineStatistics, 0, onUserLoggedOn) == false)
	{
		TT_PANIC("Failed to create Achievements Instance! (Is the steam client running and are you connected to the internet?)");
	}
#endif
}


void destroyInstance()
{
	g_statDefinitionPeriod = true;
	g_scriptVM.reset();
	g_achievementNameToID.clear();
	g_statDefinitionEntries.clear();
	g_achievementUnlockedStatus.clear();
	g_progressAchievementEntries.clear();
	
#if defined(TT_STEAM_BUILD)
	tt::steam::Achievements::destroyInstance();
	g_status = UserLoggedOnStatus_Initial;
#endif
}


void update()
{
}


//-------------------------------------------------------------------------------------------------
// Stats Functions

void addStatDefinition(const std::string& p_name, StatType p_type)
{
	if (g_statDefinitionPeriod == false)
	{
		TT_PANIC("Too late to add stat defintion '%s'", p_name.c_str());
		return;
	}
	
	StatEntries::const_iterator it = g_statDefinitionEntries.find(p_name);
	if (it != g_statDefinitionEntries.end())
	{
		TT_PANIC("Stat definition with the name '%s' is defined more than once", p_name.c_str());
		return;
	}
	g_statDefinitionEntries[p_name] = StatEntry(p_name, p_type);
}


void setIntStat(const std::string& p_name, s32  p_value)
{
	TT_ASSERT(g_statDefinitionPeriod == false);
	
	StatEntries::iterator it = g_statDefinitionEntries.find(p_name);
	if (it != g_statDefinitionEntries.end() )
	{
		it->second.value.intValue = p_value;
	}
	else
	{
		TT_PANIC("Cannot find intstat '%s'", p_name.c_str());
	}
	
#if defined(TT_STEAM_BUILD)
	tt::steam::Achievements* achievements = tt::steam::Achievements::getInstance();
	if (achievements != 0)
	{
		const s32 id = achievements->getStatisticID(p_name);
		achievements->updateStatistic(id, p_value);
	}
#endif
}


void setFloatStat(const std::string& p_name, real p_value)
{
	TT_ASSERT(g_statDefinitionPeriod == false);
	
	StatEntries::iterator it = g_statDefinitionEntries.find(p_name);
	if (it != g_statDefinitionEntries.end())
	{
		it->second.value.floatValue = p_value;
	}
	else
	{
		TT_PANIC("Cannot find floatstat '%s'", p_name.c_str());
	}
	
#if defined(TT_STEAM_BUILD)
	tt::steam::Achievements* achievements = tt::steam::Achievements::getInstance();
	if (achievements != 0)
	{
		const s32 id = achievements->getStatisticID(p_name);
		achievements->updateStatistic(id, p_value);
	}
#endif
}


s32 getIntStat(const std::string& p_name)
{
	TT_ASSERT(g_statDefinitionPeriod == false);
	
	StatEntries::iterator it = g_statDefinitionEntries.find(p_name);
	if (it != g_statDefinitionEntries.end() )
	{
		return it->second.value.intValue;
	}
	TT_PANIC("Cannot find intstat '%s'", p_name.c_str());
	return 0;
}


real getFloatStat(const std::string& p_name)
{
	TT_ASSERT(g_statDefinitionPeriod == false);
	
	StatEntries::iterator it = g_statDefinitionEntries.find(p_name);
	if (it != g_statDefinitionEntries.end())
	{
		return it->second.value.floatValue;
	}
	TT_PANIC("Cannot find floatstat '%s'", p_name.c_str());
	return 0.0f;
}


void storeStats()
{
	TT_ASSERT(g_statDefinitionPeriod == false);
	
	for (StatEntries::const_iterator it = g_statDefinitionEntries.begin(); it != g_statDefinitionEntries.end(); ++it)
	{
		const StatEntry& statEntry(it->second);
		
		ProgressAchievementEntries::const_iterator achievementIt = g_progressAchievementEntries.find(statEntry.name);
		if (achievementIt != g_progressAchievementEntries.end())
		{
			const ProgressAchievementEntry& entry(achievementIt->second);
			
			if ((statEntry.type == StatType_Int && statEntry.value.intValue >= entry.unlockRequirement.intValue) ||
				(statEntry.type == StatType_Float && statEntry.value.floatValue >= entry.unlockRequirement.floatValue))
			{
				unlockProgressAchievement(entry.achievementName);
			}
		}
	}
	
#if defined(TT_STEAM_BUILD)
	tt::steam::Achievements* achievements = tt::steam::Achievements::getInstance();
	if (achievements != 0)
	{
		achievements->storeStatistics();
	}
#endif
}


void resetStats()
{
#if defined(TT_BUILD_FINAL)
	return;
#else
	TT_ASSERT(g_statDefinitionPeriod == false);
	for (StatEntries::iterator it = g_statDefinitionEntries.begin(); it != g_statDefinitionEntries.end(); ++it)
	{
		StatEntry& entry = it->second;
		entry.value.intValue = 0;
	}
	// TODO: remove save file
#endif
}


#if defined(TT_STEAM_BUILD)
tt::steam::StatType getSteamStatType(StatType p_type)
{
	switch (p_type)
	{
	case StatType_Int:         return tt::steam::StatType_Int;
	case StatType_Float:       return tt::steam::StatType_Float;
	//case StatType_AverageRate: return tt::steam::StatType_AverageRate;
	default:
		TT_PANIC("Unknown StatType %d", p_type);
		return tt::steam::StatType_Int;
	}
}


void defineStatistics(tt::steam::Achievements* p_mgr)
{
	s32 index = 0;
	for (auto& it : g_statDefinitionEntries)
	{
		++index;
		const StatEntry& entry(it.second);
		p_mgr->addStatistic(index, entry.name, getSteamStatType(entry.type));
	}
}
#endif // #if defined(TT_STEAM_BUILD)


//-------------------------------------------------------------------------------------------------
// Achievement Functions

void addAchievement(const std::string& p_name, s32 p_id)
{
	TT_ASSERTMSG(p_name.empty() == false, "Internal name for trophy ID shouldn't be empty.");
	TT_ASSERTMSG(p_id >= 0, "Achievment id should be >= 0");
	
	auto it = g_achievementNameToID.find(p_name);
	TT_ASSERTMSG(it == g_achievementNameToID.end(),
	             "Name '%s' was already mapped to achievement ID %d.",
	             p_name.c_str(), g_achievementNameToID[p_name]);
	
	if (it == g_achievementNameToID.end() && p_id >= 0)
	{
		g_achievementNameToID[p_name] = p_id;
	}
}


s32 getAchievementID(const std::string& p_name)
{
	auto it = g_achievementNameToID.find(p_name);
	if (it != g_achievementNameToID.end())
	{
		return it->second;
	}
	TT_PANIC("Cannot find achievement '%s' in id mapping", p_name.c_str());
	return -1;
}


void setAchievementUnlockedStatus(const std::string& p_name, bool p_isUnlocked)
{
	g_achievementUnlockedStatus[p_name] = p_isUnlocked;
}


void addProgressAchievementRequirement(const std::string& p_statName, const std::string& p_achievementName, const StatValue& p_requirement)
{
	// Ensure the stat and achievement are known
	if (g_statDefinitionEntries.find(p_statName) == g_statDefinitionEntries.end())
	{
		TT_PANIC("addProgressAchievementRequirement requires missing stat '%s'", p_statName.c_str());
		return;
	}
	
	if (getAchievementID(p_achievementName) < 0)
	{
		TT_PANIC("addProgressAchievementRequirement '%s' requires missing achievement '%s'", p_statName.c_str(), p_achievementName.c_str());
		return;
	}
	
	for (auto& it : g_progressAchievementEntries)
	{
		if (it.second.achievementName == p_achievementName)
		{
			TT_PANIC("Already added a progress achievement for '%s'", p_achievementName.c_str());
			return;
		}
	}
	
	g_progressAchievementEntries[p_statName] = ProgressAchievementEntry(p_achievementName, p_requirement);
}


bool isProgressAchievement(const std::string& p_achievementName)
{
	for (auto& it : g_progressAchievementEntries)
	{
		if (it.second.achievementName == p_achievementName)
		{
			return true;
		}
	}
	return false;
}


real getProgressAchievementCompletionPercentage(const std::string& p_achievementName)
{
	for (auto& it : g_progressAchievementEntries)
	{
		if (it.second.achievementName == p_achievementName)
		{
			auto statIt = g_statDefinitionEntries.find(it.first);
			if (statIt == g_statDefinitionEntries.end())
			{
				TT_PANIC("Stat '%s' which is required for achievement '%s' is missing", it.first.c_str(), p_achievementName.c_str());
				return 0.0f;
			}
			
			real percentage = 0.0f;
			switch (statIt->second.type)
			{
			case StatType_Int:   percentage = statIt->second.value.intValue / static_cast<real>(it.second.unlockRequirement.intValue); break;
			case StatType_Float: percentage = statIt->second.value.floatValue / it.second.unlockRequirement.floatValue; break;
			default:
				TT_PANIC("Unhandled StatType '%d'\n", statIt->second.type);
				
			}
			math::clamp(percentage, 0.0f, 1.0f);
			return percentage * 100.0f;
		}
	}
	TT_PANIC("Achievement '%s' is not a progress achievement. Use addProgressAchievementRequirement.", p_achievementName.c_str());
	return 0.0f;
}


void unlockAchievement(const std::string& p_name)
{
	if (p_name.empty())
	{
		return;
	}
	
	const s32 id(getAchievementID(p_name));
	if (id < 0)
	{
		TT_PANIC("unlockAchievement tries to unlock an unknown achievement '%s'. Use addAchievement first.", p_name.c_str());
		return;
	}
	
	auto it(g_achievementUnlockedStatus.find(p_name));
	if (it != g_achievementUnlockedStatus.end() && it->second)
	{
		// Achievement already unlocked
		return;
	}
	
	TT_Printf("[UNLOCKING ACHIEVEMENT '%s']\n", p_name.c_str());
	setAchievementUnlockedStatus(p_name, true);
	if (g_scriptVM != nullptr)
	{
		g_scriptVM->callSqFun("onAchievementUnlocked", p_name, id);
	}
#if defined(TT_STEAM_BUILD)
	if (tt::steam::Achievements::hasInstance())
	{
		tt::steam::Achievements::getInstance()->unlockAchievement(p_name, false);
	}
#endif
}


void unlockProgressAchievement(const std::string& p_name)
{
	if (p_name.empty())
	{
		return;
	}
	
	auto it(g_achievementUnlockedStatus.find(p_name));
	if (it != g_achievementUnlockedStatus.end() && it->second)
	{
		// Achievement already unlocked
		return;
	}
	
	const s32 id(getAchievementID(p_name));
	if (id < 0)
	{
		TT_PANIC("unlockAchievement tries to unlock an unknown achievement '%s'. Use addAchievement first.", p_name.c_str());
		return;
	}
	
	TT_Printf("[UNLOCKING PROGRESS ACHIEVEMENT '%s']\n", p_name.c_str());
	setAchievementUnlockedStatus(p_name, true);
	if (g_scriptVM != nullptr)
	{
		g_scriptVM->callSqFun("onAchievementUnlocked", p_name, id);
	}
	// Don't unlock on Steam or other platforms, as they will handle unlocking internally (through Steam stats or through script)
}


void storeAchievements()
{
#if defined(TT_STEAM_BUILD)
	if (tt::steam::Achievements::hasInstance())
	{
		tt::steam::Achievements::getInstance()->storeStatistics();
	}
#endif
}


void resetAchievements()
{
#if defined(TT_BUILD_FINAL)
	return;
#else
	TT_Printf("[RESET ACHIEVEMENTS]\n");
	g_achievementUnlockedStatus.clear();
	
	if (g_scriptVM != nullptr)
	{
		g_scriptVM->callSqFun("onAchievementsReset");
	}
	
#if defined(TT_STEAM_BUILD)
	tt::steam::Achievements* achievements = tt::steam::Achievements::getInstance();
	if (achievements != 0)
	{
		achievements->resetAll();
	}
#endif
#endif // #if defined(TT_BUILD_FINAL)
}


// Namespace end
}
}
