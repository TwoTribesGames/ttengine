#if !defined(TT_PLATFORM_OSX_IPHONE)  // Steam support works on Mac OS X, but not on iOS

#include <tt/math/hash/CRC32.h>
#include <tt/platform/tt_printf.h>
#include <tt/str/toStr.h>

#include <tt/steam/Achievements.h>
#include <tt/steam/errorhelpers.h>


namespace tt {
namespace steam {

Achievements* Achievements::ms_instance = 0;

//--------------------------------------------------------------------------------------------------
// Public member functions

bool Achievements::createInstance(AcquireDataFunction p_statsFunc,
                                  AcquireDataFunction p_achievementsFunc,
                                  InitializedCallback p_callback)
{
	if (ms_instance == 0)
	{
		// Verify that steam is initialized
		if (SteamUser() == 0)
		{
			TT_WARN("Steam has not been initialized, unable to initialize Achievements");
			return false;
		}
		if (SteamUserStats() == 0)
		{
			TT_PANIC("Steam user statistics interface has not been initialized.");
			return false;
		}
		if (SteamUser()->BLoggedOn() == false)
		{
			// -> User is playing in offline mode, achievements cannot be unlocked
			TT_WARN("Steam user is not logged on, cannot initialize statistics.");
			return false;
		}
		ms_instance = new Achievements(p_statsFunc, p_achievementsFunc, p_callback);
	}
	else
	{
		TT_PANIC("Instance already exists");
	}
	return hasInstance();
}


void Achievements::destroyInstance()
{
	if (ms_instance == 0)
	{
		TT_PANIC("Instance already deleted");
		return;
	}
	
	delete ms_instance;
	ms_instance = 0;
}


void Achievements::addStatistic(s32 p_id, const std::string& p_name, StatType p_type)
{
	// Make sure an statistic is only added once.
	for (StatisticCollection::iterator it = m_statistics.begin(); it != m_statistics.end(); ++it)
	{
		if (it->first == p_id)
		{
			TT_WARN("Statistic %d '%s' was already added.", p_id, p_name.c_str());
			it->second = Statistic(p_name, p_type);
			return;
		}
	}
	m_statistics.insert(std::make_pair(p_id, Statistic(p_name, p_type)));
}


void Achievements::addAchievement(const std::string& p_apiName)
{
	// Make sure an achievement is only added once.
	for (AchievementCollection::iterator it = m_achievements.begin(); it != m_achievements.end(); ++it)
	{
		if (it->apiName == p_apiName)
		{
			TT_WARN("Achievement '%s' was already added.", p_apiName.c_str());
			return;
		}
	}
	m_achievements.push_back(Achievement(p_apiName));
}


bool Achievements::requestData()
{
	// Try to get current statistics
	if (SteamUserStats() != 0)
	{
		TT_Printf("[STEAM] sending request for stats and achievements.\n");
		return SteamUserStats()->RequestCurrentStats();
	}
	return false;
}


bool Achievements::storeStatistics()
{
	if (SteamUserStats() == 0) return false;
	
	if (m_needToUpload == false)
	{
		// Nothing to do here
		TT_Printf("[STEAM] Achievements::storeStatistics() - Nothing to upload\n");
		return true;
	}
	
	if (m_initialized)
	{
		// Update all statistics
		for (StatisticCollection::iterator it = m_statistics.begin(); it != m_statistics.end(); ++it)
		{
			Statistic& stat(it->second);
			
			if (stat.valid == false)
			{
				continue;
			}
			
			switch (stat.type)
			{
			case StatType_Int:
				SteamUserStats()->SetStat(stat.apiName.c_str(), stat.intValue);
				break;
				
			case StatType_Float:
				SteamUserStats()->SetStat(stat.apiName.c_str(), stat.floatValue);
				break;
				
			case StatType_AverageRate:
				SteamUserStats()->UpdateAvgRateStat(
					stat.apiName.c_str(), stat.avgNumerator, stat.avgDenominator); 
				
				// The averaged result is calculated for us
				SteamUserStats()->GetStat(stat.apiName.c_str(), &stat.floatValue);
				break;
				
			default:
				TT_PANIC("Unsupported statistics type: %d", stat.type);
				break;
			}
		}
	}
	
	// Store statistics on server
	bool result = SteamUserStats()->StoreStats();
	
	// If succeeded all is in sync again
	m_needToUpload = (result == false);
	
	return result;
}


bool Achievements::unlockAchievement(const std::string& p_apiName, bool p_storeInstantly)
{
	if (SteamUserStats() == 0) return false;
	
	// Check if it is still locked
	for (AchievementCollection::iterator it = m_achievements.begin(); it != m_achievements.end(); ++it)
	{
		if (it->apiName == p_apiName)
		{
			if (it->achieved)
			{
				// Already achieved, do not upload
				return true;
			}
		}
	}
	
	// Unlock this achievement
	bool result = SteamUserStats()->SetAchievement(p_apiName.c_str());
	TT_Printf("[ACHIEVEMENTS] Unlocking '%s' %s.\n", p_apiName.c_str(), result ? "succeeded" : "failed");
	
	// If succeeded, we need to upload changes at next call to storeStatistics()
	if (result)
	{
		m_needToUpload = true;
		
		if (p_storeInstantly)
		{
			return storeStatistics();
		}
	}
	
	return result;
}


void Achievements::updateStatistic(s32 p_id, s32 p_newValue)
{
	StatisticCollection::iterator it = m_statistics.find(p_id);
	
	TT_ASSERTMSG(it != m_statistics.end(), "No statistic with ID %d registered.", p_id);
	if (it != m_statistics.end())
	{
		TT_ASSERTMSG(it->second.type == StatType_Int,
		             "Can only use this function for INT stats.");
		it->second.intValue = p_newValue;
		it->second.valid    = true;
		m_needToUpload = true;
	}
}


void Achievements::updateStatistic(s32 p_id, float p_newValue)
{
	StatisticCollection::iterator it = m_statistics.find(p_id);
	
	TT_ASSERTMSG(it != m_statistics.end(), "No statistic with ID %d registered.", p_id);
	if (it != m_statistics.end())
	{
		TT_ASSERTMSG(it->second.type == StatType_Float,
		             "Can only use this function for FLOAT stats.");
		it->second.floatValue = p_newValue;
		it->second.valid      = true;
		m_needToUpload = true;
	}
}


void Achievements::updateStatistic(s32 p_id, float p_avgNumerator, float p_avgDenominator)
{
	StatisticCollection::iterator it = m_statistics.find(p_id);
	
	TT_ASSERTMSG(it != m_statistics.end(), "No statistic with ID %d registered.", p_id);
	if (it != m_statistics.end())
	{
		TT_ASSERTMSG(it->second.type == StatType_AverageRate,
		             "Can only use this function for AVGRATE stats.");
		it->second.avgNumerator   = p_avgNumerator;
		it->second.avgDenominator = p_avgDenominator;
		it->second.valid          = true;
		m_needToUpload = true;
	}
}


void Achievements::incrementStatistic(s32 p_id, s32 p_amount)
{
	StatisticCollection::iterator it = m_statistics.find(p_id);
	
	TT_ASSERTMSG(it != m_statistics.end(), "No statistic with ID %d registered.", p_id);
	if (it != m_statistics.end())
	{
		TT_ASSERTMSG(it->second.type == StatType_Int,
		             "Can only use this function for INT stats.");
		TT_ASSERTMSG(it->second.valid,
		             "Can't increment an invalid statistic, first need to get the latest data from server.");
		it->second.intValue += p_amount;
		m_needToUpload = true;
	}
}


void Achievements::incrementStatistic(s32 p_id, float p_amount)
{
	StatisticCollection::iterator it = m_statistics.find(p_id);
	
	TT_ASSERTMSG(it != m_statistics.end(), "No statistic with ID %d registered.", p_id);
	if (it != m_statistics.end())
	{
		TT_ASSERTMSG(it->second.type == StatType_Float,
		             "Can only use this function for FLOAT stats.");
		TT_ASSERTMSG(it->second.valid,
		             "Can't increment an invalid statistic, first need to get the latest data from server.");
		it->second.floatValue += p_amount;
		m_needToUpload = true;
	}
}


s32 Achievements::getStatisticInt(s32 p_id)
{
	StatisticCollection::iterator it = m_statistics.find(p_id);
	
	TT_ASSERTMSG(it != m_statistics.end(), "No statistic with ID %d registered.", p_id);
	if (it != m_statistics.end())
	{
		TT_ASSERTMSG(it->second.type == StatType_Int,
		             "Can only use this function for INT stats.");
		TT_ASSERTMSG(it->second.valid,
		             "Can't get an invalid statistic, first need to get the latest data from server.");
		return it->second.intValue;
	}
	return 0;
}


float Achievements::getStatisticFloat(s32 p_id)
{
	StatisticCollection::iterator it = m_statistics.find(p_id);
	
	TT_ASSERTMSG(it != m_statistics.end(), "No statistic with ID %d registered.", p_id);
	if (it != m_statistics.end())
	{
		TT_ASSERTMSG(it->second.type == StatType_Float,
		             "Can only use this function for FLOAT stats.");
		TT_ASSERTMSG(it->second.valid,
		             "Can't get an invalid statistic, first need to get the latest data from server.");
		return it->second.floatValue;
	}
	return 0.0f;
}


s32 Achievements::getStatisticID(const std::string& p_name)
{
	for (StatisticCollection::iterator it = m_statistics.begin(); it != m_statistics.end(); ++it)
	{
		if (it->second.apiName == p_name)
		{
			return it->first;
		}
	}
	TT_PANIC("(Steam) Statistic ID '%s' not found!.", p_name.c_str());
	return -1;
}


void Achievements::resetAll()
{
#ifndef TT_BUILD_FINAL
	if (SteamUserStats() != 0)
	{
		bool result = SteamUserStats()->ResetAllStats(true);
		
		// All statistic are no longer valid. (First need to get new default values from server.)
		for (StatisticCollection::iterator it = m_statistics.begin(); it != m_statistics.end(); ++it)
		{
			Statistic& stat(it->second);
			stat.valid = false;
		}
		
		if (result)
		{
			// Synchronize internal structures
			requestData();
		}
	}
#endif
}


void Achievements::openStatsOverlay()
{
	if (SteamFriends() != 0)
	{
		SteamFriends()->ActivateGameOverlay("Stats");
	}
}


void Achievements::openAchievementsOverlay()
{
	if (SteamFriends() != 0)
	{
		SteamFriends()->ActivateGameOverlay("Achievements");
	}
}


//--------------------------------------------------------------------------------------------------
// Private member functions

Achievements::Achievements(AcquireDataFunction p_statsFunc,
                           AcquireDataFunction p_achievementsFunc,
                           InitializedCallback p_callback)
:
m_userStatsReceived(),
m_userStatsStored  (),
m_achievementStored(),
m_statistics(),
m_achievements(),
m_initCallback(p_callback),
m_appID(SteamUtils() == 0 ? 0 : SteamUtils()->GetAppID()),
m_initialized(false),
m_needToUpload(false)
{
	m_userStatsReceived.Register(this, &Achievements::onUserStatsReceived);
	m_userStatsStored  .Register(this, &Achievements::onUserStatsStored  );
	m_achievementStored.Register(this, &Achievements::onAchievementStored);
	
	// Get statistics from client
	if (p_statsFunc != 0)
	{
		p_statsFunc(this);
	}
	
	// Get achievements from client
	if (p_achievementsFunc != 0)
	{
		p_achievementsFunc(this);
	}
	
	const bool requestDataResult = requestData();
	TT_ASSERT(requestDataResult);
}


Achievements::~Achievements()
{
}


void Achievements::onUserStatsReceived(UserStatsReceived_t* p_callback)
{
	if (p_callback == 0 || SteamUserStats() == 0) return;
	
	// We may get callbacks for other games, ignore them
	if (m_appID != p_callback->m_nGameID) return;
	
	// Check for errors
	if (p_callback->m_eResult != k_EResultOK)
	{
		TT_PANIC("Failed to retrieve statistics from Steam.\nResult (code %d): %s",
		         p_callback->m_eResult, getHumanReadableResult(p_callback->m_eResult));
		return;
	}
	
	// Succeeded retrieving statistics
	TT_Printf("[STEAM] Received statistics and achievements.\n");
	
	ISteamUserStats* userStats = SteamUserStats();
	
	// Update all statistics
	for (StatisticCollection::iterator it = m_statistics.begin(); it != m_statistics.end(); ++it)
	{
		Statistic& stat(it->second);
		
		stat.valid = true;
		
		switch (stat.type)
		{
		case StatType_Int:
			if (userStats->GetStat(stat.apiName.c_str(), &stat.intValue))
			{
				TT_Printf("[STEAM] Statistic (%s) = %d (INT)\n", stat.apiName.c_str(), stat.intValue);
			}
			else
			{
				TT_PANIC("Get Statistic for '%s' (INT) failed.\n", stat.apiName.c_str());
			}
			break;
			
		case StatType_Float:
		case StatType_AverageRate:
			if (userStats->GetStat(stat.apiName.c_str(), &stat.floatValue))
			{
				TT_Printf("[STEAM] Statistic (%s) = %f (Float/AvgRate)\n", stat.apiName.c_str(), stat.floatValue);
			}
			else
			{
				TT_PANIC("Get Statistic for '%s' (Float/AvgRate) failed.\n", stat.apiName.c_str());
			}
			break;
			
		default:
			TT_PANIC("Unsupported statistics type: %d\n", stat.type);
			break;
		}
	}
	
	const uint32 numAchievements = userStats->GetNumAchievements();
	for (uint32 i = 0; i < numAchievements; ++i)
	{
		const char * name = userStats->GetAchievementName(i);
		addAchievement(name);
	}
	
	// Update all achievements
	for (AchievementCollection::iterator it = m_achievements.begin(); it != m_achievements.end(); ++it)
	{
		const char* achievementID = it->apiName.c_str();
		
		userStats->GetAchievement(achievementID, &it->achieved);
		
		it->name        = userStats->GetAchievementDisplayAttribute(achievementID, "name");
		it->description = userStats->GetAchievementDisplayAttribute(achievementID, "desc");
		
		TT_Printf("[ACH] %40s [%25s] [%s] '%s'\n",
		          it->name.c_str(),
		          achievementID,
		          it->achieved ? "UNLOCKED" : "locked  ",
		          it->description.c_str());
	}
	
	m_initialized = true;
	
	// Notify the application that achievements/stats have been initialized, if a callback was specified
	if (m_initCallback != 0)
	{
		m_initCallback();
	}
}


void Achievements::onUserStatsStored(UserStatsStored_t* p_callback)
{
	if (p_callback == 0) return;
	
	// We may get callbacks for other games, ignore them
	if (m_appID != p_callback->m_nGameID) return;
	
	// Check for errors
	if (p_callback->m_eResult != k_EResultOK)
	{
		if (p_callback->m_eResult == k_EResultInvalidParam)
		{
			// One or more stats we set broke a constraint. They've been reverted,
			// and we should re-iterate the values now to keep in sync.
			TT_WARN("Some statistics failed validation and are being reverted.");
			
			// Trigger a fake callback to reload statistics
			UserStatsReceived_t callback;
			callback.m_eResult = k_EResultOK;
			callback.m_nGameID = m_appID;
			
			return onUserStatsReceived(&callback);
		}
		
		TT_PANIC("Failed to store statistics in Steam.\nResult (code %d): %s",
		         p_callback->m_eResult, getHumanReadableResult(p_callback->m_eResult));
		return;
	}
	
	TT_Printf("[STEAM] Statistics were stored succesfully.\n");
}


void Achievements::onAchievementStored(UserAchievementStored_t* p_callback)
{
	if (p_callback == 0) return;
	
	// We may get callbacks for other games, ignore them
	if (m_appID != p_callback->m_nGameID) return;
	
	TT_Printf("[STEAM] Achievement '%s' was stored succesfully.\n", p_callback->m_rgchAchievementName);
}

// Namespace end
}
}


#endif  // !defined(TT_PLATFORM_OSX_IPHONE)
