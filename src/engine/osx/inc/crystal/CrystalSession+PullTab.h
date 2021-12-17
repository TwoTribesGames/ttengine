//
//  CrystalSession+PullTab.h
//  Crystal
//
//  Created by Duane Bradbury on 13/01/2011.
//  Copyright 2011 Chillingo Ltd. All rights reserved.
//

#import "CrystalSession.h"
 

/**
 * @brief Enum for the different pull tab edges.  Used internally.
 */
typedef enum
{
	Bottom = 0,
	Right,
	Top,
	Left
} PullTabEdge;



/**
 * @brief The interface to all of the Crystal pull tab functionality.
 * The pull tab can be activated on any screen edge ("right", "left", "top", "bottom") but please ensure the theme has been updated to support 
 * the appropriate screen edge(s) to be used.
 * The pull tab UI is generally functional but not yet supported on the iPad. If you wish to use the pull tabs on the ipad consider the standard UI with popovers
 * or contact devsupport@crystalsdk.com for more information and advice.
 */
@interface CrystalSession (PullTab)


/**
 * @brief Deactivates the Crystal pull tab user interface.  This will animate the pull tab off screen.
 * @ingroup ui
 */
+ (void) deactivateCrystalPullTab;


/**
 * @brief This will animate the Crystal pull tab to the default closed postion. 
 * @ingroup ui
 */
+ (void) resetCrystalPullTabState;


/**
 * @brief Activates the Crystal pull tab user interface with the main Crystal elements of "profile", "achievements", "leaderboards" and "challenges".  
 * @ingroup ui
 * @param edgeString The edge to display the pull tab interface from, one of "right", "left", "top" or "bottom"
 */
+ (void) activateCrystalPullTabFromScreenEdge:(NSString*)edgeString;

/**
 * @brief Activates the Crystal pull tab user interface with the main Crystal elements of "profile", "achievements", "leaderboards" and "challenges".  
 * @ingroup ui
 * @param edgeString The edge to display the pull tab interface from, one of "right", "left", "top" or "bottom"
 * @param closedState The initial closed state of the tab
 */
+ (void) activateCrystalPullTabFromScreenEdge:(NSString*)edgeString closed:(BOOL)closedState;


/**
 * @brief Activates the Crystal pull tab user interface on the 'News' screen.  
 * This is a basic news feed that features adverts and three modes of display.
 * @ingroup ui
 * @param edgeString The edge to display the pull tab interface from, one of "right", "left", "top" or "bottom"
 */
+ (void) activateCrystalPullTabOnNewsFromScreenEdge:(NSString*)edgeString;


/**
 * @brief Activates the Crystal pull tab user interface on the 'News' screen in the specified closed state.  
 * This is a basic news feed that features adverts and three modes of display.
 * @ingroup ui
 * @param edgeString The edge to display the pull tab interface from, one of "right", "left", "top" or "bottom"
 * @param closed Whether the news feed is initially closed or not
 * @param closedState The initial closed state of the tab
 */
+ (void) activateCrystalPullTabOnNewsFromScreenEdge:(NSString*)edgeString closed:(BOOL)closedState;



/**
 * @brief Activates the Crystal pull tab user interface on the 'Leaderboards' screen
 * @ingroup ui
 * @param edgeString The edge to display the pull tab interface from, one of "right", "left", "top" or "bottom"
 */
+ (void) activateCrystalPullTabOnLeaderboardsFromScreenEdge:(NSString*)edgeString;


/**
 * @brief Activates the Crystal pull tab user interface on the 'Leaderboards' screen
 * @ingroup ui
 * @param edgeString The edge to display the pull tab interface from, one of "right", "left", "top" or "bottom"
 * @param closedState The initial closed state of the tab
 */
+ (void) activateCrystalPullTabOnLeaderboardsFromScreenEdge:(NSString*)edgeString closed:(BOOL)closedState;



/**
 * @brief Activates the Crystal pull tab user interface with a specific leaderboard to be displayed
 * @ingroup ui
 * @param leaderboardId The Id of the leaderboard to be displayed
 * @param edgeString The edge to display the pull tab interface from, one of "right", "left", "top" or "bottom"
 */
+ (void) activateCrystalPullTabOnLeaderboardWithId:(NSString*)leaderboardId fromScreenEdge:(NSString*)edgeString;


/**
 * @brief Activates the Crystal pull tab user interface with a specific leaderboard to be displayed
 * @ingroup ui
 * @param leaderboardId The Id of the leaderboard to be displayed
 * @param edgeString The edge to display the pull tab interface from, one of "right", "left", "top" or "bottom"
 * @param closedState The initial closed state of the tab
 */
+ (void) activateCrystalPullTabOnLeaderboardWithId:(NSString*)leaderboardId fromScreenEdge:(NSString*)edgeString closed:(BOOL)closedState;



/**
 * @brief Activates the Crystal pull tab user interface on the 'achievements' screen
 * @ingroup ui
 * @param edgeString The edge to display the pull tab interface from, one of "right", "left", "top" or "bottom"
 */
+ (void) activateCrystalPullTabOnAchievementsFromScreenEdge:(NSString*)edgeString;


/**
 * @brief Activates the Crystal pull tab user interface on the 'achievements' screen
 * @ingroup ui
 * @param edgeString The edge to display the pull tab interface from, one of "right", "left", "top" or "bottom"
 * @param closedState The initial closed state of the tab
 */
+ (void) activateCrystalPullTabOnAchievementsFromScreenEdge:(NSString*)edgeString closed:(BOOL)closedState;


/**
 * @brief Activates the Crystal pull tab user interface on the 'challenges' screen
 * @ingroup ui
 * @param edgeString The edge to display the pull tab interface from, one of "right", "left", "top" or "bottom"
 */
+ (void) activateCrystalPullTabOnChallengesFromScreenEdge:(NSString*)edgeString;

/**
 * @brief Activates the Crystal pull tab user interface on the 'challenges' screen
 * @ingroup ui
 * @param edgeString The edge to display the pull tab interface from, one of "right", "left", "top" or "bottom"
 * @param closedState The initial closed state of the tab
 */
+ (void) activateCrystalPullTabOnChallengesFromScreenEdge:(NSString*)edgeString closed:(BOOL)closedState;


/**
 * @brief Activates the Crystal pull tab user interface on the 'Promotional Gifts' screen
 * @ingroup ui
 * @param edgeString The edge to display the pull tab interface from, one of "right", "left", "top" or "bottom"
 */
+ (void) activateCrystalPullTabOnGiftsFromScreenEdge:(NSString*)edgeString;


/**
 * @brief Activates the Crystal pull tab user interface on the 'Promotional Gifts' screen
 * @ingroup ui
 * @param edgeString The edge to display the pull tab interface from, one of "right", "left", "top" or "bottom"
 * @param closedState The initial closed state of the tab
 */
+ (void) activateCrystalPullTabOnGiftsFromScreenEdge:(NSString*)edgeString closed:(BOOL)closedState;

/**
 * @brief Activates the Crystal pull tab user interface on the 'Gifts and Market' screen
 * @ingroup ui
 * @param edgeString The edge to display the pull tab interface from, one of "right", "left", "top" or "bottom"
 */
+ (void) activateCrystalPullTabOnGiftsAndMarketFromScreenEdge:(NSString*)edgeString;


/**
 * @brief Activates the Crystal pull tab user interface on the 'Gifts and Market' screen
 * @ingroup ui
 * @param edgeString The edge to display the pull tab interface from, one of "right", "left", "top" or "bottom"
 * @param closedState The initial closed state of the tab
 */
+ (void) activateCrystalPullTabOnGiftsAndMarketFromScreenEdge:(NSString*)edgeString closed:(BOOL)closedState;



/**
 * @brief Activates the Crystal pull tab user interface on the 'Virtual Goods' screen
 * @ingroup ui
 * @param edgeString The edge to display the pull tab interface from, one of "right", "left", "top" or "bottom"
 */
+ (void) activateCrystalPullTabOnVirtualGoodsFromScreenEdge:(NSString*)edgeString;


/**
 * @brief Activates the Crystal pull tab user interface on the 'Virtual Goods' screen
 * @ingroup ui
 * @param edgeString The edge to display the pull tab interface from, one of "right", "left", "top" or "bottom"
 * @param closedState The initial closed state of the tab
 */
+ (void) activateCrystalPullTabOnVirtualGoodsFromScreenEdge:(NSString*)edgeString closed:(BOOL)closedState;


/**
 * @brief Activates the Crystal pull tab user interface on the 'Virtual Currencies' screen
 * @ingroup ui
 * @param edgeString The edge to display the pull tab interface from, one of "right", "left", "top" or "bottom"
 */
+ (void) activateCrystalPullTabOnVirtualCurrenciesFromScreenEdge:(NSString*)edgeString;


/**
 * @brief Activates the Crystal pull tab user interface on the 'Virtual Currencies' screen
 * @ingroup ui
 * @param edgeString The edge to display the pull tab interface from, one of "right", "left", "top" or "bottom"
 * @param closedState The initial closed state of the tab
 */
+ (void) activateCrystalPullTabOnVirtualCurrenciesFromScreenEdge:(NSString*)edgeString closed:(BOOL)closedState;


/**
 * @brief Activates the Crystal pull tab user interface on the 'Profile' screen
 * @ingroup ui
 * @param edgeString The edge to display the pull tab interface from, one of "right", "left", "top" or "bottom"
 */
+ (void) activateCrystalPullTabOnProfileFromScreenEdge:(NSString*)edgeString;


/**
 * @brief Activates the Crystal pull tab user interface on the 'Profile' screen
 * @ingroup ui
 * @param edgeString The edge to display the pull tab interface from, one of "right", "left", "top" or "bottom"
 * @param closedState The initial closed state of the tab
 */
+ (void) activateCrystalPullTabOnProfileFromScreenEdge:(NSString*)edgeString closed:(BOOL)closedState;


/**
 * @brief Activates the Crystal pull tab user interface on the 'Settings' screen
 * @ingroup ui
 * @param edgeString The edge to display the pull tab interface from, one of "right", "left", "top" or "bottom"
 */
+ (void) activateCrystalPullTabOnSettingsFromScreenEdge:(NSString*)edgeString;


/**
 * @brief Activates the Crystal pull tab user interface on the 'Settings' screen
 * @ingroup ui
 * @param edgeString The edge to display the pull tab interface from, one of "right", "left", "top" or "bottom"
 * @param closedState The initial closed state of the tab
 */
+ (void) activateCrystalPullTabOnSettingsFromScreenEdge:(NSString*)edgeString closed:(BOOL)closedState;



/**
 * @brief Activates the Crystal pull tab user interface on the 'Friends' screen
 * @ingroup ui
 * @param edgeString The edge to display the pull tab interface from, one of "right", "left", "top" or "bottom"
 */
+ (void) activateCrystalPullTabOnFriendsFromScreenEdge:(NSString*)edgeString;


/**
 * @brief Activates the Crystal pull tab user interface on the 'Friends' screen
 * @ingroup ui
 * @param edgeString The edge to display the pull tab interface from, one of "right", "left", "top" or "bottom"
 * @param closedState The initial closed state of the tab
 */
+ (void) activateCrystalPullTabOnFriendsFromScreenEdge:(NSString*)edgeString closed:(BOOL)closedState;


/**
 * @brief Activates the Crystal pull tab user interface on the 'Find Friends' screen
 * @ingroup ui
 * @param edgeString The edge to display the pull tab interface from, one of "right", "left", "top" or "bottom"
 */
+ (void) activateCrystalPullTabOnFindFriendsFromScreenEdge:(NSString*)edgeString;


/**
 * @brief Activates the Crystal pull tab user interface on the 'Find Friends' screen
 * @ingroup ui
 * @param edgeString The edge to display the pull tab interface from, one of "right", "left", "top" or "bottom"
 * @param closedState The initial closed state of the tab
 */
+ (void) activateCrystalPullTabOnFindFriendsFromScreenEdge:(NSString*)edgeString closed:(BOOL)closedState;



/**
 * @brief Activates the Crystal pull tab user interface on a custom set of multiple tabs
 * @ingroup ui
 * @param tabs Array of tabs as strings in lowercase. 
 *			   Valid tab strings are: 
 *					"news"
 *					"profile"
 *					"settings"
 *					"leaderboards", 
 *					"achievements", 
 *					"challenges", 
 *					"gifting", 
 *					"virtualgoods",
 *					"virtualcurrencies"  
 *					"giftsandmarket", 
 *					"friends"  
 *					"findfriends"  
 *			   NOTE: A maximum of four tabs will be parsed from the array and any invalid tabs will be ignored.  
 * @param edgeString The edge to display the pull tab interface from, one of "right", "left", "top" or "bottom"
 */
+ (void) activateCrystalPullTabOn:(NSArray*)tabs fromScreenEdge:(NSString*)edgeString;

/**
 * @brief Activates the Crystal pull tab user interface on a custom set of multiple tabs
 * @ingroup ui
 * @param tabs Array of tabs as strings in lowercase. 
 *			   Valid tab strings are: 
 *					"news"
 *					"profile"
 *					"settings"
 *					"leaderboards", 
 *					"achievements", 
 *					"challenges", 
 *					"gifting", 
 *					"virtualgoods",
 *					"virtualcurrencies"  
 *					"giftsandmarket", 
 *					"friends"  
 *					"findfriends"  
 *			   NOTE: A maximum of four tabs will be parsed from the array and any invalid tabs will be ignored.  
 * @param edgeString The edge to display the pull tab interface from, one of "right", "left", "top" or "bottom"
 * @param closedState The initial closed state of the tab
 */
+ (void) activateCrystalPullTabOn:(NSArray*)tabs fromScreenEdge:(NSString*)edgeString closed:(BOOL)closedState;


@end

