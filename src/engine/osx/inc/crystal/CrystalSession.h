/*
 *  CrystalSession.h
 *  Crystal
 *
 *  Created by Gareth Reese on 06/07/2009.
 *  Copyright 2010 Chillingo Ltd. All rights reserved.
 *
 */


/**
 * @defgroup ui Crystal UI handling
 * @defgroup post Posting data to Crystal
 * @defgroup gc Game Center related functionality
 * @defgroup pass Pass-through methods that help Crystal to integrate with iOS
 * @defgroup challenge Functionality related to Crystal challenges
 */


/**
 * @brief Crystal settings that enable workaround functionality within the system.
 * You should only enable these settings if you're seeing a specific problem that you think may be resolved by one of these options.
 * In general you will be instructed to use one of these settings by an FAQ article or directly by the Crystal SDK support team.
 * Settings are set by calling activateCrystalSetting: on CrystalSession.
 */
typedef enum 
{
	/** 
	 *  @brief Activate the CrystalSettingCocosAchievementWorkaround setting if you're seeing a vertical bar instead of the achievement popup.
	 *  value:@"YES" activates the setting
	 */
	CrystalSettingCocosAchievementWorkaround = 1,
	
	/** 
	 *  @brief Activate the CrystalSettingAvoidBackgroundActivity setting if you're seeing occasional slowdowns during high-CPU activity.
	 *  This setting is intended to be used sparingly during 3D cut scenes and similar and will affect achievement posting etc.
	 *  The setting should be returned to the NO state as soon as possible and the NO state should be the game's default.
	 *  No user data should be lost while the setting is set to NO however.
	 *  value:@"YES" - Crystal will avoid any background processing or network activity.
	 *  value:@"NO"  - Return Crystal to its default state.
	 */
	CrystalSettingAvoidBackgroundActivity = 2,
	
	/** 
	 *  @brief Activate this setting for framework crashes in [UIWindow _shouldAutorotateToInterfaceOrientation:].
	 *  You'll most commonly see this problem while rotating the device in projects with no UIViewController instance
	 *  such as purely OpenGL-based games.
	 *  value:@"YES" activates this setting
	 */
	CrystalSettingShouldAutorotateWorkaround = 3,
	
	/** 
	 *  @brief Activate this setting to restrict Crystal on iPad to one popover, rather than the hierarchical popover method.
	 *  The hierarchical popoevers are more visually appealing but ma contradict some recent Apple Human Interface Guideline changes.
	 *  value:@"YES" activates this setting
	 *	@deprecated This option will no longer have any effect from version 1.3 build 0149
	 */
	CrystalSettingSingleiPadPopover = 4,
	
	/** 
	 *  @brief Activate this setting to enable the Game Center support within Crystal.
	 *  This setting can be called as a result of an enable/disable Game Center switch in the game UI if desired.
	 *  value:@"YES" activates this setting
	 */
	CrystalSettingEnableGameCenterSupport = 5,
	
	/**
	 * @brief Activate this setting to enable virtual goods support within Crystal.
	 * value should be set as an NSSet of strings which represent the IAP IDs of the products used by IAP.
	 */
	CrystalSettingEnableVirtualGoods = 6,


	/**
	 * @brief Activate this setting to disable application badge support within Crystal.
	 *  value:@"YES" activates this setting
	 */
	CrystalSettingDisableApplicationIconBadgeNumber = 7,
	
} CrystalSetting;


/**
 * @brief Session delegate protocol to be implemented by one of your game classes.
 * For a standard Objective C application this would most likely be the class that implements UIApplicationDelegate.
 * Though the methods are optional it is recommended to implement all of them.
 */
@protocol CrystalSessionDelegate <NSObject>
@optional

/**
 * @brief Reports to the client that a challenge has been started from within the Crystal UI.
 * @ingroup ui challenge
 * On receiving this notification the client should instruct the user on the details of the challenge task and start the task.
 * After completing the challenge task the client should inform the CrystalSession by calling postChallengeResultForLastChallenge:
 * @param gameConfig The game configuration ID as shown in the Developer Dashboard
 */
- (void) challengeStartedWithGameConfig:(NSString*)gameConfig;

/**
 * @brief Reports to the client that the Crystal splash screen has been dismissed
 * @ingroup ui
 * After the Crystal splash dialog (displaySplashScreen) has been dismissed the client will be notified with this method.
 * The method can be used to resume the normal game startup and UI, and also to immediately activate the Crystal UI and encourage sign-up.
 * @param activateCrystal Will be set to YES if the user decided to activate Crystal. The game should activate the crystal UI at the next available opportunity in this case.
 */
- (void) splashScreenFinishedWithActivateCrystal:(BOOL)activateCrystal;

/**
 * @brief Reports to the client that the Crystal UI has been closed.
 * @ingroup ui
 * This is a good place to reactivate the game from a paused state and reactivate sound.
 */
- (void) crystalUiDeactivated;

/**
 * @brief Reports to the client that the iPad popovers have been activated or deactivated
 * @ingroup ui
 * Some titles may wish to disable parts of their menu sytem when the Crystal popovers are visible.
 * During the period that the popovers are visible the game's screen is greyed and the user cannot interact with your menu system.
 * Tapping any area of the screen outside of the Crystal UI removes the grey overlay and normal interaction with your menu system can resume.
 * @param activated YES if the popovers have been activated or NO if not
 */
- (void) crystaliPadPopoversActivated:(BOOL)activated;

@end


/**
 * @brief The interface to all of the Crystal SDK functionality.
 * Though all of the methods are implemented as class methods for ease of use the client must call initWithAppID: to initiate the session as soon as is convenient.
 */
@interface CrystalSession : NSObject
{
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Initialisation and UI handling
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Initializes the crystal session.
 * This method should be called as soon as your game has activated fully. The session must run in the background so that the game can submit high scores, analytics etc.
 * @param appId The Application ID for this application, as obtained from the Developer Dashboard
 * @param delegate The class implementing the CrystalSessionDelegate to receive notifications from the session
 * @param version The version of your application, which must match one of the versions shown in the Developer Dashboard
 * @param theme The theme to use for the Crystal UI. This must match the .crystaltheme file included in the application bundle and would be 'violet_002' for the theme file 'violet_002.crystaltheme'.
 * @param secretKey The secret key string shown in the Developer Dashboard for this application
 */
+ (void) initWithAppID:(NSString*)appId delegate:(id<CrystalSessionDelegate>)delegate version:(double)version theme:(NSString*)theme secretKey:(NSString*)secretKey;

/**
 * @brief Locks the crystal UI to the specified orientation.
 * @ingroup ui
 * Just call this once after initializing the session. All orientations are supported.
 * If your application supports more than one screen orientation use lockToOrientationList: instead.
 * @param orientation The orientation to lock the Crystal UI to
 */
+ (void) lockToOrientation:(UIDeviceOrientation)orientation;

/**
 * @brief Locks the crystal UI to the specified list of orientations.
 * @ingroup ui
 * Just call this once after initializing the session. All orientations are supported.
 * A typical usage of this call is to limit Crystal to only the landscape orientations like so: 
 * <code>[CrystalSession lockToOrientationList:[NSArray arrayWithObjects:[NSNumber numberWithInt:UIDeviceOrientationLandscapeLeft], [NSNumber numberWithInt:UIDeviceOrientationLandscapeRight], nil] ];</code>
 * @param orientationList The list of orientations to lock the Crystal UI to
 */
+ (void) lockToOrientationList:(NSArray*)orientationList;

/**
 * @brief Closes the Crystal session.
 * This should only be called when absolutely necessary for memory usage reasons. The Crystal session without the UI is very small, so this should cause no problems.
 * Without a running sesssion high scores, achievements and analytics will not be posted to the server.
 */
+ (void) closeCrystalSession;

/**
 * @brief Determine if the Crystal UI is currently active and visible.
 * @ingroup ui
 * Returns YES if active and visible, otherwise returns NO.
 */
+ (BOOL) isCrystalUIActive;


/**
 * @brief Activates the Crystal user interface with the default view options.
 * @ingroup ui
 * <b>On iPhone</b> before calling this method your application should stop rendering and stop sound if not appropriate to the Crystal UI.
 * <br><b>On iPad</b> this method is used to activate the Crystal navigation tabs at the left of the screen.
 * Please see http://devsupport.crystalsdk.com/default.asp?W8 for more details.
 */
+ (void) activateCrystalUI;

/**
 * @brief Activates the Crystal user interface with the 'profile' tab open.
 * @ingroup ui
 * <b>On iPhone</b> this is equivelant to calling activateCrystalUI.
 * <b>On iPad</b> this method can be used to activate the Crystal UI from a dedicated Crystal button, though the permanent overlay via activateCrystalUI is preferred. 
 * Please see http://devsupport.crystalsdk.com/default.asp?W8 for more details.
 * @see activateCrystalUI
 */
+ (void) activateCrystalUIAtProfile;

/**
 * @brief Activates the Crystal user interface with the 'challenges' tab open.
 * @ingroup ui challenge
 * @see activateCrystalUI
 */
+ (void) activateCrystalUIAtChallenges;

/**
 * @brief Activates the Crystal user interface with the 'leaderboards' tab open.
 * @ingroup ui
 * @see activateCrystalUI
 */
+ (void) activateCrystalUIAtLeaderboards;

/**
 * @brief Activates the Crystal user interface with the 'leaderboards' tab open at the specified leaderboard.
 * @ingroup ui
 * @see activateCrystalUI
 * @param leaderboardId The leaderboard to be displayed
 */
+ (void) activateCrystalUIAtLeaderboardWithId:(NSString*)leaderboardId;

/**
 * @brief Activates the Crystal user interface with the 'achievements' tab open.
 * @ingroup ui
 * @see activateCrystalUI
 */
+ (void) activateCrystalUIAtAchievements;

/**
 * @brief Activates the Crystal user interface at the 'Add more friends' screen
 * @ingroup ui
 * @see activateCrystalUI
 */
+ (void) activateCrystalUIAtAddFriends;

/**
 * @brief Activates the Crystal user interface at the 'Settings' screen
 * @ingroup ui
 * @see activateCrystalUI
 */
+ (void) activateCrystalUIAtSettings;

/**
 * @brief Activates the Crystal user interface at the gifting and promotions functionality.
 * @ingroup ui
 * If the theme for your game has a gifting/promotions tab defined this will open the Crystal UI at this tab.
 * If the theme doesn't have this tab then the UI will be opened at the Profile tab but with the gifting/promotions functionality visible.
 */
+ (void) activateCrystalUIAtGifting;

/**
 * @brief Activates the Crystal user interface at the gifting and market functionality.
 * @ingroup ui
 * If the theme for your game has a gifting/market tab defined this will open the Crystal UI at this tab.
 * If the theme doesn't have this tab then the UI will be opened at the Profile tab but with the gifting/market functionality visible.
 */
+ (void) activateCrystalUIAtGiftsAndMarket;

/**
 * @brief Activates the Crystal user interface at the virtual goods functionality.
 * @ingroup ui
 * If the theme for your game has a gifting/promotions tab defined this will open the Crystal UI at this tab.
 * If the theme doesn't have this tab then the UI will be opened at the Profile tab but with the virtual goods functionality visible.
 */
+ (void) activateCrystalUIAtVirtualGoods;

/**
 * @brief Activates the Crystal user interface at the virtual currency functionality.
 * @ingroup ui
 * If the theme for your game has a gifting/promotions tab defined this will open the Crystal UI at this tab.
 * If the theme doesn't have this tab then the UI will be opened at the Profile tab but with the virtual currencies functionality visible.
 */
+ (void) activateCrystalUIAtVirtualCurrencies;

/**
 * @brief Activates the Crystal user interface at the 'Find Friends' screen.
 * @ingroup ui
 */
+ (void) activateCrystalUIAtFindFriends;

/**
 * @brief Activates the Crystal user interface at the 'Invite Friends' screen.
 * @ingroup ui
 */
+ (void) activateCrystalUIAtInviteFriends;

/**
 * @brief Forces the Crystal UI to be removed from view.
 * @ingroup ui
 * <b>On iPhone</b> in normal circumstances it shouldn't be necessary to call this method, but is added for completeness in case the client receives its own notifications.
 * <br><b>On iPad</b> this method is used to dismiss the Crystal navigation tabs at the left of the screen.
 */
+ (void) deactivateCrystalUI;


/**
 * @brief Sets values for various internal Crystal settings.
 * This method is provided for developers to activate workarounds for bugs on the iPhone and the graphics libraries used by developers
 * @param setting the CrystalSetting to set
 * @param value the value to set for this setting, which will generally be @"YES" to activate a setting
 */
+ (void) activateCrystalSetting:(CrystalSetting)setting value:(id)value;

/**
 * @brief Displays the Crystal splash screen
 * @ingroup ui
 * This method displays the Crystal splash screen to the user above the current game UI.
 * It is the responsibility of the developer to ensure that this dialog is displayed at an appropriate time, generally just after the game is started.
 * When the splash screen has been dismissed splashScreenFinishedWithActivateCrystal: will be called in the delegate.
 * Normally the game would only display this splash screen once to the user.
 */
+ (void) displaySplashScreen;

/**
 * @brief Determines whether the user has signed into Crystal
 * This method can be used to determine whether the user has signed into Crystal. The result of this method can be used to display
 * promotional UI elements in the game UI to encourage the use of Crystal-enabled features.
 * @return YES if the user is signed into Crystal
 * @deprecated This method is deprecated.  Please use the isSignedIn method on the CrystalPlayer API instead.
 */
+ (BOOL) userIsSignedIntoCrystal DEPRECATED_ATTRIBUTE;

/**
 * @brief - TESTING ONLY - 
 * Deletes any user data for Crystal for testing purposes.
 * Any data stored on the server is not deleted however.
 */
+ (void) TEST_deleteCrystalUserData;


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Notifications for the results of challenges, achievements, scores etc.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Post the result of the last challenge that the game was notified for.
 * @ingroup post challenge
 * The game will be notified of the challenge via the challengeStartedWithGameConfig: method of CrystalSessionDelegate.
 * @param result The result of the challenge, either as a numerical value or as a number of seconds (NSTimeInterval) for time-based scores
 * @param doDialog If YES Crystal will display a dialog over the game user interface
 */
+ (void) postChallengeResultForLastChallenge:(double)result withCrystalDialog:(BOOL)doDialog;

/**
 * @brief Notify the Crystal servers and the user (via a popup notification) that an achievement has been completed.
 * @ingroup post
 * The game designer should not attempt to cache, buffer or otherwise restrict the number of times this method is called.
 * All logic with regards to reducing server load, handling multiple users and avoiding excessive popups is handled within the Crystal SDK.
 * Please call this method whenever an achievement is achieved. Failure to do so will almost certainly cause problems for multiple Crystal users on the same device.
 * @param achievementId The ID of the achievement as shown in the Crystal control panel
 * @param obtained YES if the achievement has been obtained or NO to 'unobtain' it. Ordinarily you will only obtain achievements.
 * @param description A description of the achievement to be displayed to the user. Supplying the description here allows the developer to localize the description. Crystal may not necessarily have a network connection so the only way to get the achievement description is here. If no desctiption is supplied then no notification will be displayed to the user.
 * @param alwaysPopup if YES the achievement popup will always be displayed to the user. If no the popup will only be displayed the first time that the achievement is achieved.
 * @return YES if the achievement popup was or would be displayed and NO if the popup was not displayed
 */
+ (BOOL) postAchievement:(NSString*)achievementId wasObtained:(BOOL)obtained withDescription:(NSString*)description alwaysPopup:(BOOL)alwaysPopup;

/**
 * @brief Notify the Crystal servers and the user (via a popup notification) that an achievement has been completed.
 * @ingroup post
 * The game designer should not attempt to cache, buffer or otherwise restrict the number of times this method is called.
 * All logic with regards to reducing server load, handling multiple users and avoiding excessive popups is handled within the Crystal SDK.
 * Please call this method whenever an achievement is achieved. Failure to do so will almost certainly cause problems for multiple Crystal users on the same device.
 * This asynchronous method should be invoked if game designers experience performance issues calling postAchievement.
 * @param achievementId The ID of the achievement as shown in the Crystal control panel
 * @param obtained YES if the achievement has been obtained or NO to 'unobtain' it. Ordinarily you will only obtain achievements.
 * @param description A description of the achievement to be displayed to the user. Supplying the description here allows the developer to localize the description. Crystal may not necessarily have a network connection so the only way to get the achievement description is here. If no desctiption is supplied then no notification will be displayed to the user.
 * @param alwaysPopup if YES the achievement popup will always be displayed to the user. If no the popup will only be displayed the first time that the achievement is achieved.
 */
+ (void) postAchievementAsync:(NSString*)achievementId wasObtained:(BOOL)obtained withDescription:(NSString*)description alwaysPopup:(BOOL)alwaysPopup;

/**
 * @brief Notify the Crystal servers and the user (via a popup notification) that an achievement has been completed.
 * @ingroup post gc
 * The game designer should not attempt to cache, buffer or otherwise restrict the number of times this method is called.
 * All logic with regards to reducing server load, handling multiple users and avoiding excessive popups is handled within the Crystal SDK.
 * Please call this method whenever an achievement is achieved. Failure to do so will almost certainly cause problems for multiple Crystal users on the same device.
 * @param achievementId The ID of the achievement as shown in the Crystal control panel
 * @param obtained YES if the achievement has been obtained or NO to 'unobtain' it. Ordinarily you will only obtain achievements.
 * @param description A description of the achievement to be displayed to the user. Supplying the description here allows the developer to localize the description. Crystal may not necessarily have a network connection so the only way to get the achievement description is here. If no desctiption is supplied then no notification will be displayed to the user.
 * @param alwaysPopup if YES the achievement popup will always be displayed to the user. If no the popup will only be displayed the first time that the achievement is achieved.
 * @param gameCenterAchievementId The ID of the Game Center achievement to use.  This will be defined in iTunes Connect for the application.
 * @return YES if the achievement popup was or would be displayed and NO if the popup was not displayed
 */
+ (BOOL) postAchievement:(NSString*)achievementId wasObtained:(BOOL)obtained withDescription:(NSString*)description alwaysPopup:(BOOL)alwaysPopup forGameCenterAchievementId:(NSString*)gameCenterAchievementId;

/**
 * @brief Notify the Crystal servers and the user (via a popup notification) that an achievement has been completed.
 * @ingroup post gc
 * The game designer should not attempt to cache, buffer or otherwise restrict the number of times this method is called.
 * All logic with regards to reducing server load, handling multiple users and avoiding excessive popups is handled within the Crystal SDK.
 * Please call this method whenever an achievement is achieved. Failure to do so will almost certainly cause problems for multiple Crystal users on the same device.
 * This asynchronous method should be invoked if game designers experience performance issues calling postAchievement.
 * @param achievementId The ID of the achievement as shown in the Crystal control panel
 * @param obtained YES if the achievement has been obtained or NO to 'unobtain' it. Ordinarily you will only obtain achievements.
 * @param description A description of the achievement to be displayed to the user. Supplying the description here allows the developer to localize the description. Crystal may not necessarily have a network connection so the only way to get the achievement description is here. If no desctiption is supplied then no notification will be displayed to the user.
 * @param alwaysPopup if YES the achievement popup will always be displayed to the user. If no the popup will only be displayed the first time that the achievement is achieved.
 * @param gameCenterAchievementId The ID of the Game Center achievement to use.  This will be defined in iTunes Connect for the application.
 */
+ (void) postAchievementAsync:(NSString*)achievementId wasObtained:(BOOL)obtained withDescription:(NSString*)description alwaysPopup:(BOOL)alwaysPopup forGameCenterAchievementId:(NSString*)gameCenterAchievementId;

/**
 * @brief Report progress made towards an achievement both to Crystal and Game Center
 * @ingroup post gc
 * This method updates the Crystal UI to show the progress that the player has made towards an achievement in plain text.
 * This method also updates Game Center to report the percentage complete for the achievement.
 * Achievement popups are not shown if this method is called.
 * @param crystalId The ID of the achievement as shown in the Crystal control panel
 * @param gameCenterId The ID of the Game Center achievement to use.  This will be defined in iTunes Connect for the application.
 * @param percentageComplete The percentage complete for the achievement, i.e. 40.0 (passed to Game Center)
 * @param achievementDescription A textual description of the achievement progress, i.e. "4 out of 10 coins collected" (shown in Crystal UI)
 */
+ (void) postAchievementProgressWithCrystalId:(NSString*)crystalId gameCenterId:(NSString*)gameCenterId percentageComplete:(double)percentageComplete achievementDescription:(NSString*)achievementDescription;
 
/**
 * @brief Notify the Crystal servers of a leaderboard result for the specified leaderboard ID.
 * @ingroup post
 * The game designer should not attempt to cache, buffer or otherwise restrict the number of times this method is called.
 * All logic with regards to reducing server load, handling multiple users and avoiding unneeded posts is handled within the Crystal SDK.
 * Please call this method whenever a score is scored. Failure to do so will almost certainly cause problems for multiple Crystal users on the same device.
 * @param result The score to publish, either as a numerical value or as a number of seconds (NSTimeInterval) for time-based scores
 * @param leaderboardId The ID of the leaderboard to post to, which should be taken from the Developer Dashboard
 * @param lowestFirst Set this if you have YES set for lowest value first in the developer dashboard. This parameter MUST match the flag in the developer dashboard!
 */
+ (void) postLeaderboardResult:(double)result forLeaderboardId:(NSString*)leaderboardId lowestValFirst:(BOOL)lowestFirst;

/**
 * @brief Notify the Crystal servers of a leaderboard result for the specified leaderboard ID.
 * @ingroup post
 * The game designer should not attempt to cache, buffer or otherwise restrict the number of times this method is called.
 * All logic with regards to reducing server load, handling multiple users and avoiding unneeded posts is handled within the Crystal SDK.
 * Please call this method whenever a score is scored. Failure to do so will almost certainly cause problems for multiple Crystal users on the same device.
 * @param result The score to publish, either as a numerical value or as a number of seconds (NSTimeInterval) for time-based scores
 * @param leaderboardId The ID of the leaderboard to post to, which should be taken from the Developer Dashboard
 * @param lowestFirst Set this if you have YES set for lowest value first in the developer dashboard. This parameter MUST match the flag in the developer dashboard!
 * @param isTimeBased Set this to YES if the leaderboard ID is set to have 'time based' leaderboards in the developer dashboard. This setting creates server load and must not be set for leaderboards that are not time-based.
 */
+ (void) postLeaderboardResult:(double)result forLeaderboardId:(NSString*)leaderboardId lowestValFirst:(BOOL)lowestFirst isTimeBased:(BOOL)isTimeBased;

/**
 * @brief Notify the Crystal servers of a leaderboard result for the specified leaderboard ID.
 * @ingroup post gc
 * The game designer should not attempt to cache, buffer or otherwise restrict the number of times this method is called.
 * All logic with regards to reducing server load, handling multiple users and avoiding unneeded posts is handled within the Crystal SDK.
 * Please call this method whenever a score is scored. Failure to do so will almost certainly cause problems for multiple Crystal users on the same device.
 * @param result The score to publish, either as a numerical value or as a number of seconds (NSTimeInterval) for time-based scores
 * @param leaderboardId The ID of the leaderboard to post to, which should be taken from the Developer Dashboard
 * @param lowestFirst Set this if you have YES set for lowest value first in the developer dashboard. This parameter MUST match the flag in the developer dashboard!
 * @param gameCenterLeaderboardId The ID of the Game Center leaderboard to use.  This will be defined in iTunes Connect for the application.
 */
+ (void) postLeaderboardResult:(double)result forLeaderboardId:(NSString*)leaderboardId lowestValFirst:(BOOL)lowestFirst forGameCenterLeaderboardId:(NSString*)gameCenterLeaderboardId;

/**
 * @brief Notify the Crystal servers of a leaderboard result for the specified leaderboard ID.
 * @ingroup post gc
 * The game designer should not attempt to cache, buffer or otherwise restrict the number of times this method is called.
 * All logic with regards to reducing server load, handling multiple users and avoiding unneeded posts is handled within the Crystal SDK.
 * Please call this method whenever a score is scored. Failure to do so will almost certainly cause problems for multiple Crystal users on the same device.
 * @param result The score to publish, either as a numerical value or as a number of seconds (NSTimeInterval) for time-based scores
 * @param leaderboardId The ID of the leaderboard to post to, which should be taken from the Developer Dashboard
 * @param lowestFirst Set this if you have YES set for lowest value first in the developer dashboard. This parameter MUST match the flag in the developer dashboard!
 * @param gameCenterLeaderboardId The ID of the Game Center leaderboard to use.  This will be defined in iTunes Connect for the application.
 * @param isTimeBased Set this to YES if the leaderboard ID is set to have 'time based' leaderboards in the developer dashboard. This setting creates server load and must not be set for leaderboards that are not time-based.
 */
+ (void) postLeaderboardResult:(double)result forLeaderboardId:(NSString*)leaderboardId lowestValFirst:(BOOL)lowestFirst forGameCenterLeaderboardId:(NSString*)gameCenterLeaderboardId isTimeBased:(BOOL)isTimeBased;


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Query the Crystal session about its status
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Ask the session whether the application was started from an incoming challenge (push) notification.
 * @ingroup challenge
 * @return YES if the application was started from an incoming challenge
 */
+ (BOOL) appWasStartedFromPendingChallenge;


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Game Center support
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/**
 * @brief Tests to see if Game Center is available for use on the device
 * @ingroup gc
 * Use this method to determine if Game Center is avaialble on the device at runtime. 
 * Game Center will only be available for iOS 4.1 and above.
 */
+ (BOOL) isGameCenterAvailableOnDevice; 

#ifdef __IPHONE_4_1
/**
 * @brief Call this after your game has loaded to initiate login to Game Center
 * @ingroup gc
 * Calling this method can initiate the Game Center login dialog, so ensure that you call this at a suitable moment.
 * After the user has signed up to Game Center this method will display a 'welcome back' overlay.
 * At the time of writing (iOS 4.1 beta 3) the iOS APIs used by this method must be called before scores and achievements can be posted.
 * As a result of this you must be careful of the timing of this call.
 * Game Center functionality must also be enabled in the Crystal developer dashboard.
 */
+ (void) authenticateLocalPlayerWithCompletionHandler:(void (^)(NSError *error))completionHandler;
#endif


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// UIApplicationDelegate pass-through for Crystal
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** 
 * Needed for push notifications. Pass-through from UIApplicationDelegate.
 * @ingroup pass
 */
+ (void) application:(UIApplication*)application didRegisterForRemoteNotificationsWithDeviceToken:(NSData*)deviceToken;

/**
 * Needed for push notifications. Pass-through from UIApplicationDelegate.
 * @ingroup pass
 */
+ (void) application:(UIApplication*)application didFailToRegisterForRemoteNotificationsWithError:(NSError*)error;

/**
 * Needed for push notifications. Pass-through from UIApplicationDelegate.
 * @ingroup pass
 */
+ (void) application:(UIApplication*)application didReceiveRemoteNotification:(NSDictionary*)userInfo;

/**
 * Needed for push notifications. Pass-through from UIApplicationDelegate.
 * @ingroup pass
 */
+ (BOOL) application:(UIApplication*)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions;

/**
 * Needed for push notifications via email. Pass-through from UIApplicationDelegate.
 * @ingroup pass
 */
+ (BOOL) application:(UIApplication*)application handleOpenUrl:(NSURL *)url;


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// UIViewController pass-through for Crystal
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Call this from your top-level view controller if your game uses Apple's UIViewController or subclasses.
 * @ingroup pass
 * All view rotations are supported by Crystal, though you can limit this in your own app.
 * If the view rotation is to be limited to one rotation (recommended) call the lockToOrientation: method on CrystalSession.
 * If you would like to support more than one rotation call the lockToOrientationList: method.
 */
+ (void) willRotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation duration:(NSTimeInterval)duration;

/**
 * @brief Call this from your top-level view controller if your game uses Apple's UIViewController or subclasses.
 * @ingroup pass
 * All view rotations are supported by Crystal, though you can limit this in your own app.
 * If the view rotation is to be limited to one rotation (recommended) call the lockToOrientation: method on CrystalSession.
 * If you would like to support more than one rotation call the lockToOrientationList: method.
 */
+ (void) didRotateFromInterfaceOrientation:(UIInterfaceOrientation)fromInterfaceOrientation;

@end



