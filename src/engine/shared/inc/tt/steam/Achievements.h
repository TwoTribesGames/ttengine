#if !defined(INC_STEAM_ACHIEVEMENTS_H)
#define INC_STEAM_ACHIEVEMENTS_H

#if !defined(TT_PLATFORM_OSX_IPHONE)  // Steam support works on Mac OS X, but not on iOS


#include <string>
#include <vector>
#include <map>

#if defined(TT_PLATFORM_WIN)
#pragma warning (push)
#pragma warning (disable:4127)
#endif

#include <steam/steam_api.h>

#if defined(TT_PLATFORM_WIN)
#pragma warning (pop)
#endif

#include <tt/platform/tt_error.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace steam {

enum StatType
{
	StatType_Int         = 0,
	StatType_Float       = 1,
	StatType_AverageRate = 2
};


// Define function for aquiring stats & achievements definition from client
class Achievements;
typedef void (*AcquireDataFunction)(Achievements*);
typedef void (*InitializedCallback)(void);


class Achievements
{
public:
	// Instance management
	static bool createInstance(AcquireDataFunction p_statsFunc,
	                           AcquireDataFunction p_achievementsFunc,
	                           InitializedCallback p_callback = 0);
	inline static Achievements* getInstance()
	{
		TT_NULL_ASSERT(ms_instance);
		return ms_instance;
	}
	inline static bool hasInstance() { return ms_instance != 0; }
	static void destroyInstance();
	
	/*! \brief Add a statistic (must be called by client in AcquireDataFunction).
	    \param p_id Client-specified number to identify the statistic by in later statistic calls.
	    \param p_name Identifying name as registered on Valve's "Game Admin" page.
	    \param p_type The type the statistic should be. */
	void addStatistic(s32 p_id, const std::string& p_name, StatType p_type);
	
	/*! \brief Add an achievement (must be called by client in AcquireDataFunction).
	    \param p_apiName Identifying name as registered on Valve's "Game Admin" page. */
	void addAchievement(const std::string& p_apiName);
	
	/*! \brief Get statistics and achievements from Steam server (asynchronous). */
	bool requestData();
	
	/*! \brief Store statistics on Steam server (asynchronous). */
	bool storeStatistics();
	
	/*! \brief Unlock achievement on Steam server (asynchronous).
	    \param p_apiName Identifying name as registered on Valve's "Game Admin" page.
	    \param p_storeInstantly Whether to save the unlock immediately or during the next storeStatistics call. */
	bool unlockAchievement(const std::string& p_apiName, bool p_storeInstantly = true);
	
	/*! \brief Update integer statistic with new value.
	    \param p_id The statistic ID (as specified with addStatistic). */
	void updateStatistic(s32 p_id, s32   p_newValue);
	
	/*! \brief Update floating point statistic with new value.
	    \param p_id The statistic ID (as specified with addStatistic). */
	void updateStatistic(s32 p_id, float p_newValue);
	
	/*! \brief Update average rate statistic with new value.
	    \param p_id The statistic ID (as specified with addStatistic). */
	void updateStatistic(s32 p_id, float p_avgNumerator, float p_avgDenominator);
	
	/*! \brief Increment integer statistic.
	    \param p_id The statistic ID (as specified with addStatistic). */
	void incrementStatistic(s32 p_id, s32 p_amount = 1);
	
	/*! \brief Increment floating point statistic.
	    \param p_id The statistic ID (as specified with addStatistic). */
	void incrementStatistic(s32 p_id, float p_amount);
	
	/*! \brief Get integer statistic.
	    \param p_id The statistic ID (as specified with addStatistic). */
	s32   getStatisticInt(  s32 p_id);
	
	/*! \brief Get floating point statistic.
	    \param p_id The statistic ID (as specified with addStatistic). */
	float getStatisticFloat(s32 p_id);
	
	/*! \brief Get the statistic ID from name. (as specified with addStatistic).
	    \param p_name Identifying name as registered on Valve's "Game Admin" page as specified with addStatistic.
	    \note It's better if client code doesn't need to use this and keeps track of the IDs. */
	s32 getStatisticID(const std::string& p_name);
	
	/*! \brief Reset all statistics and achievements.
	    \note FOR DEBUGGING ONLY! Does nothing in final builds. */
	void resetAll();
	
	/*! \brief Open Steam overlay on the stats page. */
	void openStatsOverlay();
	
	/*! \brief Open Steam overlay on the achievements. */
	void openAchievementsOverlay();
	
private:
	struct Statistic
	{
		StatType    type;
		std::string apiName;      //!< Identifying name as registered on Valve's "Game Admin" page.
		int32       intValue;
		float       floatValue;
		float       avgNumerator;
		float       avgDenominator;
		bool        valid;
		
		inline Statistic(const std::string& p_apiName, StatType p_type)
		:
		type(p_type),
		apiName(p_apiName),
		intValue(0),
		floatValue(0),
		avgNumerator(0),
		avgDenominator(0),
		valid(false)
		{
		}
	};
	/*! \brief Maps client-specified statistic ID to statistic details. */
	typedef std::map<s32, Statistic> StatisticCollection;
	
	struct Achievement
	{
		std::string apiName;      //!< Identifying name as registered on Valve's "Game Admin" page.
		std::string name;         //!< Display name as retrieved from Steam.
		std::string description;  //!< Description as retrieved from Steam.
		bool        achieved;     //!< Whether the achievement has been achieved.
		int         icon;         //!< Not used.
		
		explicit inline Achievement(const std::string& p_apiName)
		:
		apiName(p_apiName),
		name(),
		description(),
		achieved(false),
		icon(0)
		{
		}
	};
	typedef std::vector<Achievement> AchievementCollection;
	
	
	Achievements(AcquireDataFunction p_statsFunc,
	             AcquireDataFunction p_achievementsFunc,
	             InitializedCallback p_callback);
	~Achievements();
	
	// Declare callbacks for steam server interaction
	STEAM_CALLBACK_MANUAL(Achievements, onUserStatsReceived, UserStatsReceived_t,     m_userStatsReceived);
	STEAM_CALLBACK_MANUAL(Achievements, onUserStatsStored,   UserStatsStored_t,       m_userStatsStored  );
	STEAM_CALLBACK_MANUAL(Achievements, onAchievementStored, UserAchievementStored_t, m_achievementStored);
	
	
	// Single instance
	static Achievements* ms_instance;
	
	StatisticCollection   m_statistics;
	AchievementCollection m_achievements;
	InitializedCallback   m_initCallback;
	
	u32  m_appID;
	bool m_initialized;
	bool m_needToUpload;
};

// Namespace end
}
}


#endif  // !defined(TT_PLATFORM_OSX_IPHONE)

#endif // !defined(INC_STEAM_ACHIEVEMENTS_H)
