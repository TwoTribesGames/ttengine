#if defined(TT_PLATFORM_OSX_IPHONE)  // Crystal is only available for iOS

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

// Crystal needs Foundation and UIKit import first.
#import  <crystal/CrystalSession+PullTab.h>

#include <tt/app/Application.h>
#include <tt/app/CrystalDelegate.h>
#include <tt/app/objc_helpers/UIApplication.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
#include <tt/str/str.h>


//--------------------------------------------------------------------------------------------------
// Interface Obj C


@interface CrystalDelegateObjC : NSObject <CrystalSessionDelegate>

/**
 * @brief Reports to the client that a challenge has been started from within the Crystal UI.
 * On receiving this notification the client should instruct the user on the details of the challenge task and start the task.
 * After completing the challenge task the client should inform the CrystalSession by calling postChallengeResultForLastChallenge:
 * @param gameConfig The game configuration ID as shown in the Developer Dashboard
 */
- (void) challengeStartedWithGameConfig:(NSString*)gameConfig;

/**
 * @brief Reports to the client that the Crystal splash screen has been dismissed
 * After the Crystal splash dialog (displaySplashScreen) has been dismissed the client will be notified with this method.
 * The method can be used to resume the normal game startup and UI, and also to immediately activate the Crystal UI and encourage sign-up.
 * @param activateCrystal Will be set to YES if the user decided to activate Crystal. The game should activate the crystal UI at the next available opportunity in this case.
 */
- (void) splashScreenFinishedWithActivateCrystal:(BOOL)activateCrystal;

/**
 * @brief Reports to the client that the Crystal UI has been closed.
 * This is a good place to reactivate the game from a paused state and reactivate sound.
 */
- (void) crystalUiDeactivated;

/**
 * @brief Reports to the client that the iPad popovers have been activated or deactivated
 * Some titles may wish to disable parts of their menu sytem when the Crystal popovers are visible.
 * During the period that the popovers are visible the game's screen is greyed and the user cannot interact with your menu system.
 * Tapping any area of the screen outside of the Crystal UI removes the grey overlay and normal interaction with your menu system can resume.
 * @param activated YES if the popovers have been activated or NO if not
 */
- (void) crystaliPadPopoversActivated:(BOOL)activated;

@end


//--------------------------------------------------------------------------------------------------
// Implementation Obj C


@implementation CrystalDelegateObjC


- (void) challengeStartedWithGameConfig:(NSString*)gameConfig;
{
	(void)gameConfig;
	TT_Printf("CrystalDelegateObjC::challengeStartedWithGameConfig\n");
}


- (void) splashScreenFinishedWithActivateCrystal:(BOOL)activateCrystal
{
	TT_Printf("CrystalDelegateObjC::splashScreenFinishedWithActivateCrystal %d\n", activateCrystal);
	
	if (activateCrystal)
	{
		tt::app::CrystalDelegate::getInstance()->activeMenuScreen();
	}
}


- (void) crystalUiDeactivated
{
	TT_Printf("CrystalDelegateObjC::crystalUiDeactivated\n");
	tt::app::CrystalDelegate::getInstance()->onCrystalUiDeactivated();
}


- (void) crystaliPadPopoversActivated:(BOOL)activated
{
	(void)activated;
	TT_Printf("CrystalDelegateObjC::crystaliPadPopoversActivated %d\n", activated);
}


@end




//--------------------------------------------------------------------------------------------------
// C++ class code:


namespace tt {
namespace app {
	
	
CrystalDelegate* CrystalDelegate::ms_instance = 0;


void CrystalDelegate::createInstance(const std::string& p_appID, real p_version, const std::string& p_theme, 
                                     const std::string& p_secretKey
#if TT_USE_PUSH_NOTIFICATION
									 , void* p_UIApplication
#endif
									 )
{
	if (ms_instance != 0)
	{
		TT_PANIC("Trying to create CrystalDelegate instance twice!");
		return;
	}
	
	ms_instance = new CrystalDelegate(p_appID, p_version, p_theme, p_secretKey
#if TT_USE_PUSH_NOTIFICATION
									  , p_UIApplication
#endif
									  );
}


void CrystalDelegate::destoryInstance()
{
	if (ms_instance == 0)
	{
		TT_PANIC("Trying to destory CrystalDeleagte instance twice!");
		return;
	}
	delete ms_instance;
	ms_instance = 0;
}

	
void CrystalDelegate::displaySplashScreen()
{
#if TT_USE_CRYSTAL
	TT_ASSERT(m_crystalIsActive == false);
	
	[CrystalSession displaySplashScreen];
#endif
}


void CrystalDelegate::enableGameCenterSupport()
{
#if TT_USE_CRYSTAL
	[CrystalSession activateCrystalSetting:CrystalSettingEnableGameCenterSupport value:@"YES"];
	
	// Authenticate the local player
	[CrystalSession authenticateLocalPlayerWithCompletionHandler:^ (NSError* error)
	{
		if(error)
		{
			TT_PANIC("An error occured authenticating the local game center player. Error: '%s'",
					 [[error localizedDescription] UTF8String]);
		}
		else
		{
			// Handle success here if needed
		}
	}
	];
#endif
}


void CrystalDelegate::activeMenuScreen()
{
#if TT_USE_CRYSTAL
	TT_WARNING(m_crystalIsActive == false, "Crystal Already active.");
	
	[CrystalSession activateCrystalUIAtProfile]; // Start at the profile tab (Use this on iPad)
	// Use this on iPhone: [CrystalSession activateCrystalUI];
	m_crystalIsActive = true;
#endif
}


void CrystalDelegate::deactiveMenuScreen()
{
#if TT_USE_CRYSTAL
	[CrystalSession deactivateCrystalUI];
#endif
}


const char* CrystalDelegate::getEdgeName(Edge p_edge)
{
	switch (p_edge)
	{
	case Edge_Right:  return "right";
	case Edge_Left:   return "left";
	case Edge_Top:    return "top";
	case Edge_Bottom: return "bottom";
	default:
		TT_PANIC("Unknown edge: %d", p_edge);
		return "";
	}
}


CrystalDelegate::Edge CrystalDelegate::getEdge(const std::string& p_edgeName)
{
	for (s32 i = 0; i < Edge_Count; ++i)
	{
		Edge edge = static_cast<Edge>(i);
		if (getEdgeName(edge) == p_edgeName)
		{
			return edge;
		}
	}
	// Not found.
	return Edge_Invalid;
}


void CrystalDelegate::activatePullTabOnNews(Edge p_fromEdge)
{
#if TT_USE_CRYSTAL
	if (isValidEdge(p_fromEdge) == false)
	{
		TT_PANIC("Invalid edge: %d", p_fromEdge);
		return;
	}
	NSString* edge = [NSString stringWithUTF8String: getEdgeName(p_fromEdge)];
	[CrystalSession activateCrystalPullTabOnNewsFromScreenEdge:edge];
#else
	(void) p_fromEdge;
#endif
}


void CrystalDelegate::activatePullTabOnNews(Edge p_fromEdge, bool p_closed)
{
#if TT_USE_CRYSTAL
	if (isValidEdge(p_fromEdge) == false)
	{
		TT_PANIC("Invalid edge: %d", p_fromEdge);
		return;
	}
	NSString* edge = [NSString stringWithUTF8String: getEdgeName(p_fromEdge)];
	[CrystalSession activateCrystalPullTabOnNewsFromScreenEdge:edge closed:(p_closed)?TRUE:FALSE];
#else
	(void)p_fromEdge;
	(void)p_closed;
#endif
}


void CrystalDelegate::activatePullTabOnLeaderboards(Edge p_fromEdge, const std::string& p_leaderboardId)
{
#if TT_USE_CRYSTAL
	if (isValidEdge(p_fromEdge) == false)
	{
		TT_PANIC("Invalid edge: %d", p_fromEdge);
		return;
	}
	NSString* edge = [NSString stringWithUTF8String: getEdgeName(p_fromEdge)];
	
	if (p_leaderboardId.empty())
	{
		[CrystalSession activateCrystalPullTabOnLeaderboardsFromScreenEdge:edge];
	}
	else
	{
		NSString* leaderboardId = [NSString stringWithUTF8String: p_leaderboardId.c_str()];
		[CrystalSession activateCrystalPullTabOnLeaderboardWithId: leaderboardId fromScreenEdge: edge];
	}
#else
	(void)p_fromEdge;
	(void)p_leaderboardId;
#endif
}


void CrystalDelegate::activatePullTabOnGifts(Edge p_fromEdge)
{
#if TT_USE_CRYSTAL
	if (isValidEdge(p_fromEdge) == false)
	{
		TT_PANIC("Invalid edge: %d", p_fromEdge);
		return;
	}
	NSString* edge = [NSString stringWithUTF8String: getEdgeName(p_fromEdge)];
	[CrystalSession activateCrystalPullTabOnGiftsFromScreenEdge:edge];
#else
	(void) p_fromEdge;
#endif
}


void CrystalDelegate::activatePullTabOnChallenges(Edge p_fromEdge)
{
#if TT_USE_CRYSTAL
	if (isValidEdge(p_fromEdge) == false)
	{
		TT_PANIC("Invalid edge: %d", p_fromEdge);
		return;
	}
	NSString* edge = [NSString stringWithUTF8String: getEdgeName(p_fromEdge)];
	[CrystalSession activateCrystalPullTabOnChallengesFromScreenEdge:edge];
#else
	(void) p_fromEdge;
#endif
}


void CrystalDelegate::activatePullTabOnAchievements(Edge p_fromEdge)
{
#if TT_USE_CRYSTAL
	if (isValidEdge(p_fromEdge) == false)
	{
		TT_PANIC("Invalid edge: %d", p_fromEdge);
		return;
	}
	NSString* edge = [NSString stringWithUTF8String: getEdgeName(p_fromEdge)];
	[CrystalSession activateCrystalPullTabOnAchievementsFromScreenEdge:edge];
#else
	(void) p_fromEdge;
#endif
}


void CrystalDelegate::deactivatePullTab()
{
#if TT_USE_CRYSTAL
	[CrystalSession deactivateCrystalPullTab];
#endif
}


void CrystalDelegate::onCrystalUiDeactivated()
{
#if TT_USE_CRYSTAL
	TT_WARNING(m_crystalIsActive, "got Crystal UI Deactivated callback but was already inactive.");
	m_crystalIsActive = false;
#else
	TT_PANIC("Should never be called");
#endif
}


void CrystalDelegate::postAchievement(const std::string& p_achievementId, bool p_wasObtained,
                                      const std::string& p_description, const std::string& p_gameCenterAchievementId,
                                      bool p_alwaysPopup)
{
	if (p_achievementId.empty() || p_description.empty())
	{
		TT_PANIC("Can't post achievement when achievementId or descriptio is empty!");
		return;
	}
	
#if TT_USE_CRYSTAL
	NSString* achievementIdNS = [NSString stringWithUTF8String: p_achievementId.c_str()];
	NSString* descriptionNS   = [NSString stringWithUTF8String: p_description.c_str()];
	
	if (p_gameCenterAchievementId.empty())
	{
		[CrystalSession postAchievement: achievementIdNS wasObtained: p_wasObtained withDescription: descriptionNS alwaysPopup: p_alwaysPopup];
	}
	else
	{
		NSString* gameCenterIDNS = [NSString stringWithUTF8String: p_gameCenterAchievementId.c_str()];
		[CrystalSession postAchievement: achievementIdNS wasObtained: p_wasObtained withDescription: descriptionNS alwaysPopup: p_alwaysPopup forGameCenterAchievementId: gameCenterIDNS];
	}
#else
	(void)p_wasObtained;
	(void)p_gameCenterAchievementId;
	(void)p_alwaysPopup;
#endif
}


void CrystalDelegate::postLeaderboardResult(real p_result, const std::string& p_leaderboardId, bool p_lowestValFirst, 
                                            const std::string& p_gameCenterLeaderboardId)
{
	if (p_leaderboardId.empty())
	{
		TT_PANIC("Can't post leaderboard result when leaderboardId is empty!");
		return;
	}
	
#if TT_USE_CRYSTAL
	NSString* leaderboardIdNS = [NSString stringWithUTF8String: p_leaderboardId.c_str()];
	
	if (p_gameCenterLeaderboardId.empty())
	{
		[CrystalSession postLeaderboardResult: p_result forLeaderboardId: leaderboardIdNS lowestValFirst: p_lowestValFirst];
	}
	else
	{
		NSString* gameCenterIDNS = [NSString stringWithUTF8String: p_gameCenterLeaderboardId.c_str()];
		[CrystalSession postLeaderboardResult: (double)p_result forLeaderboardId: leaderboardIdNS lowestValFirst: p_lowestValFirst forGameCenterLeaderboardId: gameCenterIDNS];
	}
#else
	(void)p_result;
	(void)p_lowestValFirst;
	(void)p_gameCenterLeaderboardId;
#endif
}


#if TT_USE_PUSH_NOTIFICATION
void CrystalDelegate::onDidRegisterForRemoteNotificationsWithDeviceToken(NSData* p_deviceToken)
{
	[CrystalSession application:static_cast<UIApplication*>(m_UIApplication) didRegisterForRemoteNotificationsWithDeviceToken:p_deviceToken];
}


void CrystalDelegate::onDidFailToRegisterForRemoteNotificationsWithError(NSError* p_error)
{
	[CrystalSession application:static_cast<UIApplication*>(m_UIApplication) didFailToRegisterForRemoteNotificationsWithError:p_error];
}


void CrystalDelegate::onDidReceiveRemoteNotification(NSDictionary* p_userInfo)
{
	[CrystalSession application:static_cast<UIApplication*>(m_UIApplication) didReceiveRemoteNotification:p_userInfo];
}


BOOL CrystalDelegate::onDidFinishLaunchingWithOptions(NSDictionary * p_launchOptions)
{
	return [CrystalSession application:static_cast<UIApplication*>(m_UIApplication) didFinishLaunchingWithOptions:p_launchOptions];
}
#endif
	

void CrystalDelegate::onWillRotateToOrientation(tt::app::AppOrientation p_orientation, real p_duration)
{
#if TT_USE_CRYSTAL
	TT_Printf("CrystalDelegate::onWillRotateToOrientation\n");
	m_currentInterfaceOrientation = p_orientation;
	[CrystalSession willRotateToInterfaceOrientation:getUIInterfaceOrientation(p_orientation) duration:p_duration];
#else
	(void)p_orientation;
	(void)p_duration;
#endif
}


void CrystalDelegate::onDidRotateFromOrientation(tt::app::AppOrientation p_fromOrientation)
{
#if TT_USE_CRYSTAL
	TT_Printf("CrystalDelegate::onDidRotateFromInterfaceOrientation\n");
	//m_currentInterfaceOrientation = p_fromInterfaceOrientation;
	[CrystalSession didRotateFromInterfaceOrientation:getUIInterfaceOrientation(p_fromOrientation)];
#else
	(void)p_fromOrientation;
#endif
}


//--------------------------------------------------------------------------------------------------
// Private Functions


CrystalDelegate::CrystalDelegate(const std::string& p_appID, real p_version, const std::string& p_theme,
                                 const std::string& p_secretKey
#if TT_USE_PUSH_NOTIFICATION
								 , void* p_UIApplication
#endif
								 )
#if TT_USE_CRYSTAL
:
#if TT_USE_PUSH_NOTIFICATION
m_UIApplication(p_UIApplication),
#endif
m_crystalDelegateObjc(0),
m_currentInterfaceOrientation(tt::app::AppOrientation_LandscapeRight),
m_crystalIsActive(false),
m_crystalIsInit(false)
#endif
{
#if TT_USE_CRYSTAL
	CrystalDelegateObjC* crystalDelegateObjcPtr = [[CrystalDelegateObjC alloc] init];
	m_crystalDelegateObjc = crystalDelegateObjcPtr;
	
	NSString* appID     = [NSString stringWithUTF8String: p_appID.c_str() ];
	NSString* theme     = [NSString stringWithUTF8String: p_theme.c_str() ];
	NSString* secretKey = [NSString stringWithUTF8String: p_secretKey.c_str() ];
	
	[CrystalSession initWithAppID:appID delegate:crystalDelegateObjcPtr version:p_version theme:theme secretKey:secretKey];
	
	[CrystalSession lockToOrientationList:
			[NSArray arrayWithObjects:
					[NSNumber numberWithInt:UIDeviceOrientationLandscapeLeft],
					[NSNumber numberWithInt:UIDeviceOrientationLandscapeRight],
					nil] ];
	
	tt::app::getApplication()->registerPlatformCallbackInterface(this);
#else
	(void)p_appID;
	(void)p_version;
	(void)p_theme;
	(void)p_secretKey;
#endif
}


CrystalDelegate::~CrystalDelegate()
{
#if TT_USE_CRYSTAL
	tt::app::getApplication()->unregisterPlatformCallbackInterface(ms_instance);
	[static_cast<CrystalDelegateObjC*>(m_crystalDelegateObjc) release];
#endif
}


// End namespace
}
}

#endif  // defined(TT_PLATFORM_OSX_IPHONE)
