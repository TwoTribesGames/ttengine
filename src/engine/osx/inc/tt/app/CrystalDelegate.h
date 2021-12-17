#if !defined(INC_TT_APP_CRYSTALDELEGATE_H)
#define INC_TT_APP_CRYSTALDELEGATE_H

#if defined(TT_PLATFORM_OSX_IPHONE)


//#import <UIKit/UIKit.h>
//#import <Foundation/Foundation.h>

#include <tt/app/PlatformCallbackInterface.h>


#define TT_USE_PUSH_NOTIFICATION 0
#define TT_USE_CRYSTAL 1


namespace tt {
namespace app {

		
class CrystalDelegate : public tt::app::PlatformCallbackInterface
{
public:
	static void createInstance(const std::string& p_appID, real p_version, const std::string& p_theme, 
	                           const std::string& p_secretKey
#if TT_USE_PUSH_NOTIFICATION
							   , void* p_UIApplication
#endif
							   );
	static inline bool hasInstance()             { return ms_instance != 0; }
	static inline CrystalDelegate* getInstance() { return ms_instance;      }
	static void destoryInstance();
	
	// GUI functions
	
	void displaySplashScreen();
	void enableGameCenterSupport();
	
	void activeMenuScreen();
	void deactiveMenuScreen();
	
	// Pull tab functions
	
	enum Edge
	{
		Edge_Right,
		Edge_Left,
		Edge_Top,
		Edge_Bottom,
		
		Edge_Count,
		Edge_Invalid
	};
	static inline bool isValidEdge(Edge p_edge) { return p_edge >= 0 && p_edge < Edge_Count; }
	static const char* getEdgeName(Edge p_edge);
	static Edge getEdge(const std::string& p_edgeName);
	
	/* \brief Activates the Crystal pull tab user interface on the 'News' screen.  
	          This is a basic news feed that features adverts and three modes of display.
	   \param p_fromEdge The edge to display the pull tab interface from. */
	void activatePullTabOnNews(Edge p_fromEdge);
	
	/* \brief Activates the Crystal pull tab user interface on the 'News' screen.  
	          This is a basic news feed that features adverts and three modes of display.
	   \param p_fromEdge The edge to display the pull tab interface from.
	   \param p_closed Whether the news feed is initially closed or not. */
	void activatePullTabOnNews(Edge p_fromEdge, bool p_closed);
	
	/* \brief Activates the Crystal pull tab user interface on the 'Leaderboards' screen.
	   \param p_fromEdge The edge to display the pull tab interface from.
	   \param p_leaderboardId [Optional] The Id of the leaderboard to be displayed. */
	void activatePullTabOnLeaderboards(Edge p_fromEdge, const std::string& p_leaderboardId = "");
	
	/* \brief Activates the Crystal pull tab user interface on the 'Gifting' screen.
	   \param p_fromEdge The edge to display the pull tab interface from. */
	void activatePullTabOnGifts(Edge p_fromEdge);
	
	/* \brief Activates the Crystal pull tab user interface on the 'challenges' screen.
	   \param p_fromEdge The edge to display the pull tab interface from. */
	void activatePullTabOnChallenges(Edge p_fromEdge);
		
	/* \brief Activates the Crystal pull tab user interface on the 'achievements' screen
	   \param p_fromEdge The edge to display the pull tab interface from. */
	void activatePullTabOnAchievements(Edge p_fromEdge);
	
	/* \brief Deactivates the Crystal pull tab user interface.  This will animate the pull tab off screen. */
	void deactivatePullTab();
	
	/* \brief Internal callbacks. Don't call as client code!
	   \note Don't call as client code! */
	void onCrystalUiDeactivated();
	
	// Post functions
	void postAchievement(const std::string& p_achievementId, bool p_wasObtained,
	                     const std::string& p_description, const std::string& p_gameCenterAchievementId = "",
						 bool p_alwaysPopup = false);
	
	void postLeaderboardResult(real p_result, const std::string& p_leaderboardId, bool p_lowestValFirst, 
	                           const std::string& p_gameCenterLeaderboardId = "");
	
	// Application delegate pass-throughs
#if TT_USE_PUSH_NOTIFICATION
#error unsuppored at this time.
	void onDidRegisterForRemoteNotificationsWithDeviceToken(NSData* p_deviceToken);
	void onDidFailToRegisterForRemoteNotificationsWithError(NSError* p_error);
	void onDidReceiveRemoteNotification( NSDictionary* p_userInfo);
	BOOL onDidFinishLaunchingWithOptions(NSDictionary * p_launchOptions);
#endif
	
	virtual void onWillRotateToOrientation(tt::app::AppOrientation p_orientation, real p_duration);
	virtual void onDidRotateFromOrientation(tt::app::AppOrientation p_fromOrientation);
	
private:
	CrystalDelegate(const std::string& p_appID, real p_version, const std::string& p_theme,
	                const std::string& p_secretKey
#if TT_USE_PUSH_NOTIFICATION
					, void* p_UIApplication
#endif
					);
	~CrystalDelegate();
	
	static CrystalDelegate* ms_instance;
	
#if TT_USE_CRYSTAL
#	if TT_USE_PUSH_NOTIFICATION
	void* m_UIApplication;
#	endif
	void* m_crystalDelegateObjc;
	tt::app::AppOrientation m_currentInterfaceOrientation;
	bool m_crystalIsActive;
	bool m_crystalIsInit;
#endif
};


// Namespace end
}
}


#endif // #if defined(TT_PLATFORM_OSX_IPHONE)

#endif // !defined(INC_TT_APP_CRYSTALDELEGATE_H)
