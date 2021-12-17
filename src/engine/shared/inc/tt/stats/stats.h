#if !defined(INC_TT_STATS_STATS_H)
#define INC_TT_STATS_STATS_H

#include <string>

#include <tt/platform/tt_types.h>
#include <tt/script/fwd.h>

namespace tt {
namespace stats {

enum StatType
{
	StatType_Int,        //!< Statistic Type for Integers
	StatType_Float,      //!< Statistic Type for Floats
	// supported StatType_AverageRate //!< Statistic Type for Average Rate (Float)
};

struct StatValue
{
	union
	{
		s32  intValue;
		real floatValue;
	};
	
	StatValue(s32 p_intValue)
	:
	intValue(p_intValue)
	{}
	
	StatValue(real p_floatValue)
	:
	floatValue(p_floatValue)
	{}
};


void createInstance(const tt::script::VirtualMachinePtr& p_vm);
void destroyInstance();
void update();

void addStatDefinition(const std::string& p_name, StatType p_type);
void setIntStat(  const std::string& p_name, s32  p_value);
void setFloatStat(const std::string& p_name, real p_value);
s32  getIntStat(  const std::string& p_name);
real getFloatStat(const std::string& p_name);
void storeStats();
void resetStats();

void addAchievement(const std::string& p_name, s32 p_id);
s32  getAchievementID(const std::string& p_name);
void setAchievementUnlockedStatus(const std::string& p_name, bool p_isUnlocked);
void addProgressAchievementRequirement(const std::string& p_statName, const std::string& p_achievementName, const StatValue& p_requirement);
bool isProgressAchievement(const std::string& p_achievementName);
real getProgressAchievementCompletionPercentage(const std::string& p_achievementName);
void unlockAchievement(const std::string& p_name);
void unlockProgressAchievement(const std::string& p_name);
void storeAchievements();
void resetAchievements();

// Namespace end
}
}


#endif  // !defined(INC_TT_STATS_STATS_H)
