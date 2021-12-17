//
//  CrystalLeaderboards.h
//  Crystal
//
//  Created by Gareth Reese on 16/04/2010.
//  Copyright 2010 Chillingo Ltd. All rights reserved.
//

#import <Foundation/Foundation.h>

/**
 * @brief The data categories available to clients of CrystalLeaderboards
 * This is used in the categoriesToGet property of CrystalLeaderboards and specifies which data will be requested from the server for each leaderboard.
 * Metadata for the leaderboard will always be retrieved.
 */
typedef enum
{
	CLCTop20			= 0x1,		///< Get via top20EntriesForLeaderboardID:
	CLCTop20Friends		= 0x2,		///< Get via top20FriendsForLeaderboardID:
	CLCRandom20			= 0x4,		///< Get via random20ForLeaderboardID:
	CLCCurrentUser		= 0x8,		///< Get via currentUserDataForLeaderboardID:
} CrystalLeaderboardCategories;


/**
 * @brief The leaderboard types available for request in CrystalLeaderboards
 * By default the CLTGlobal leaderboard type is assumed, and this is the leaderboard type requested before this API was introduced.
 * The Crystal back-end will only generate leaderboards that are configured through the developer dashboard, so attempting to request a leaderboard type
 * for a leaderboard ID that does not have the type configured will return no data.
 */
typedef enum
{
	CLTGlobal,			///< The standard 'all time' global leaderboard
	CLTNational,		///< The national 'all time' leaderboard for the current user's configured location
	CLTLocal,			///< The local 'all time' leaderboard for the current user's configured location
	CLTThisMonth,		///< This month's leaderboard
	CLTLastMonth,		///< The previous month's leaderboard
	CLTThisWeek,		///< This week's leaderboard
	CLTLastWeek,		///< The previous week's leaderboard
	CLTToday,			///< Today's leaderboard
	CLTYesterday,		///< The previous day's leaderboard
	CLTThisHour_TEST,	///< Internal, do not use
	CLTLastHour_TEST,	///< Internal, do not use
} CrystalLeaderboardType;


@protocol CrystalLeaderboardDelegate <NSObject>

@optional
/**
 * @brief Reports to the client that a leaderboard has been updated
 * @deprecated This method has now been deprecated in favour of the more accurate notification below. This method will only get called for leaderboards of the type CLTGlobal, the default.
 * @param leaderboardId The leaderboard ID that has been updated. This will match the leaderboard ID on the Developer Dashboard
 */
- (void) crystalLeaderboardUpdated:(NSString*)leaderboardId DEPRECATED_ATTRIBUTE;

@required
/**
 * @brief Reports to the client that a leaderboard has been updated
 * This method is mor accurate version of the above notification and capable of notifying the observer for all leaderboard types.
 * @param leaderboardId The leaderboard ID that has been updated. This will match the leaderboard ID on the Developer Dashboard
 */
- (void) crystalLeaderboardUpdated:(NSString*)leaderboardId leaderboardType:(CrystalLeaderboardType)leaderboardType;

@end


/**
 * @brief The Crystal interface to the Crystal direct leaderboard access API.
 * The categoriesToGet property must be consistent throughout the lifetime of this class, i.e. the same data must be retrieved for each of the leaderboards.
 * This allows the Crystal SDK to cache the data more consistently and eases the use of the APIs.
 * The categoriesToGet property must also be set before calling downloadLeaderboardDataForID:
 *
 * <b>IMPORTANT NOTE:</b> This API has a performance overhead for both the client application as well as the Crystal servers and requires the use of the network.
 * Therefore this API should only be used for applications that wish to display the leaderboard data manually.
 * Please do not rely on the Crystal Leaderboard data for application features.
 */
@interface CrystalLeaderboards : NSObject 
{
@public
	CrystalLeaderboardCategories categoriesToGet;	///< @see CrystalLeaderboardCategories for details. Must be set before calling downloadLeaderboardDataForID:.
	id<CrystalLeaderboardDelegate> delegate;		///< @see CrystalLeaderboardDelegate. Set this to get notifications for downloaded leaderboard data via crystalLeaderboardUpdated:.
}

@property (nonatomic)								CrystalLeaderboardCategories categoriesToGet;
@property (nonatomic, assign, setter=setDelegate:)	id<CrystalLeaderboardDelegate> delegate;


/**
 * @defgroup get Getting an array of leaderboard entries
 * @defgroup query Accessing leaderboard data
 */

/**
 * @brief Returns the shared singleton instance of CrystalLeaderboards
 */
+ (CrystalLeaderboards*) sharedInstance;

/**
 * @ingroup get
 * @brief Initiate the download of leaderboard data for the supplied leaderboard ID
 * If the leaderboard data for the specified leaderboardID is already downloaded for this game session the data will not be updated from the server to avoid excessive server load.
 * When the leaderboard data has been downloaded the crystalLeaderboardUpdated: method will be called on CrystalLeaderboardDelegate.
 * There is no indication of an error while downloading the leaderboard data and there is no need to manually retry the process.
 * Assumes the standard CLTGlobal leaderboard type.
 * @param leaderboardID The ID of the leaderboard as shown in the developer dashboard
 */
- (void) downloadLeaderboardDataForID:(NSString*)leaderboardID;

/**
 * @ingroup get
 * @brief Initiate the download of leaderboard data for the supplied leaderboard ID and leaderboard type
 * If the leaderboard data for the specified leaderboardID is already downloaded for this game session the data will not be updated from the server to avoid excessive server load.
 * When the leaderboard data has been downloaded the crystalLeaderboardUpdated: method will be called on CrystalLeaderboardDelegate.
 * There is no indication of an error while downloading the leaderboard data and there is no need to manually retry the process.
 * @param leaderboardID The ID of the leaderboard as shown in the developer dashboard
 * @param leaderboardType The CrystalLeaderboardType of the leaderboard to retrieve. If this leaderboard type is not configured in the developer dashboard no data will be returned.
 */
- (void) downloadLeaderboardDataForID:(NSString*)leaderboardID leaderboardType:(CrystalLeaderboardType)leaderboardType;


/**
 * @ingroup get
 * @brief Initiate the download of leaderboard data for the supplied leaderboard ID and leaderboard type
 * If the leaderboard data for the specified leaderboardID is already downloaded for this game session the data will only be 
 * updated from the server once every 30 minutes to avoid excessive server load.
 * When the leaderboard data has been downloaded the crystalLeaderboardUpdated: method will be called on CrystalLeaderboardDelegate.
 * There is no indication of an error while downloading the leaderboard data and there is no need to manually retry the process.
 * @param leaderboardID The ID of the leaderboard as shown in the developer dashboard
 * @param leaderboardType The CrystalLeaderboardType of the leaderboard to retrieve. If this leaderboard type is not configured in the developer dashboard no data will be returned.
 * @param force Set to YES to attempt to force a server-side refresh of downloaded leaderboard data.
 */
- (void) downloadLeaderboardDataForID:(NSString*)leaderboardID leaderboardType:(CrystalLeaderboardType)leaderboardType force:(BOOL)force;

//////////////////////////////////////////////////////////////////////////////////////
// Get an array of leaderboard entries

/**
 * @ingroup get
 * @brief Retrieve the top 20 entries for the specified leaderboard
 * Assumes the standard CLTGlobal leaderboard type.
 * @param leaderboardID The ID of the leaderboard as shown in the developer dashboard
 * @return An array of leaderboard entries or nil if the leaderboard data is not available
 */
- (NSArray*) top20EntriesForLeaderboardID:(NSString*)leaderboardID;

/**
 * @ingroup get
 * @brief Retrieve the top 20 entries for the specified leaderboard
 * @param leaderboardID The ID of the leaderboard as shown in the developer dashboard
 * @param leaderboardType The CrystalLeaderboardType of the leaderboard to retrieve
 * @return An array of leaderboard entries or nil if the leaderboard data is not available
 */
- (NSArray*) top20EntriesForLeaderboardID:(NSString*)leaderboardID leaderboardType:(CrystalLeaderboardType)leaderboardType;

/**
 * @ingroup get
 * @brief Retrieve the top 20 friends for the specified leaderboard, including the current user
 * Assumes the standard CLTGlobal leaderboard type.
 * @param leaderboardID The ID of the leaderboard as shown in the developer dashboard
 * @return An array of leaderboard entries or nil if the leaderboard data is not available
 */
- (NSArray*) top20FriendsForLeaderboardID:(NSString*)leaderboardID;

/**
 * @ingroup get
 * @brief Retrieve the top 20 friends for the specified leaderboard, including the current user
 * @param leaderboardID The ID of the leaderboard as shown in the developer dashboard
 * @param leaderboardType The CrystalLeaderboardType of the leaderboard to retrieve
 * @return An array of leaderboard entries or nil if the leaderboard data is not available
 */
- (NSArray*) top20FriendsForLeaderboardID:(NSString*)leaderboardID leaderboardType:(CrystalLeaderboardType)leaderboardType;

/**
 * @ingroup get
 * @brief Retrieve a random selection of 20 entries for the specified leaderboard
 * Assumes the standard CLTGlobal leaderboard type.
 * @param leaderboardID The ID of the leaderboard as shown in the developer dashboard
 * @return An array of leaderboard entries or nil if the leaderboard data is not available
 */
- (NSArray*) random20ForLeaderboardID:(NSString*)leaderboardID;

/**
 * @ingroup get
 * @brief Retrieve a random selection of 20 entries for the specified leaderboard
 * @param leaderboardID The ID of the leaderboard as shown in the developer dashboard
 * @param leaderboardType The CrystalLeaderboardType of the leaderboard to retrieve
 * @return An array of leaderboard entries or nil if the leaderboard data is not available
 */
- (NSArray*) random20ForLeaderboardID:(NSString*)leaderboardID leaderboardType:(CrystalLeaderboardType)leaderboardType;

/**
 * @ingroup get
 * @brief Retrieve the data for the current user for the specified leaderboard
 * Assumes the standard CLTGlobal leaderboard type.
 * @param leaderboardID The ID of the leaderboard as shown in the developer dashboard
 * @return An array of leaderboard entries or nil if the leaderboard data is not available
 */
- (NSDictionary*) currentUserEntryForLeaderboardID:(NSString*)leaderboardID;

/**
 * @ingroup get
 * @brief Retrieve the data for the current user for the specified leaderboard
 * @param leaderboardID The ID of the leaderboard as shown in the developer dashboard
 * @param leaderboardType The CrystalLeaderboardType of the leaderboard to retrieve
 * @return An array of leaderboard entries or nil if the leaderboard data is not available
 */
- (NSDictionary*) currentUserEntryForLeaderboardID:(NSString*)leaderboardID leaderboardType:(CrystalLeaderboardType)leaderboardType;

//////////////////////////////////////////////////////////////////////////////////////
// Accessing leaderboard entry data


/**
 * @ingroup query
 * @brief Get the user Id from a leaderboard entry.
 * @return The user Id or nil if no user Id available
 */
- (NSString*) userIdForLeaderboardEntry:(NSDictionary*)leaderboardEntry;


/**
 * @ingroup query
 * @brief Get the username from a leaderboard entry.
 * @return The username or nil if no username available
 */
- (NSString*) usernameForLeaderboardEntry:(NSDictionary*)leaderboardEntry;

/**
 * @ingroup query
 * @brief Get the position of a leaderboard entry.
 * This is a convenience method to extract data from the array items returned from top20EntriesForLeaderboardID: et al
 * @return -1 if the position is not available (for instance for random entries)
 * @deprecated This method has now been deprecated because of performance issues related to calculating the exact position of a leaderboard entry.
 * Please get in touch via Crystal Developer Support <devsupport@crystalsdk.com> if you rely on this method.
 * In general you should be fine to use the relative position in your UI - i.e. the position of the entry in the array
 */
- (int) positionForLeaderboardEntry:(NSDictionary*)leaderboardEntry DEPRECATED_ATTRIBUTE;


/**
 * @ingroup query
 * @brief Get the percentile rank of a leaderboard entry.
 * This is a convenience method to extract data from the array items returned from top20EntriesForLeaderboardID: et al
 * @return nil if the percentile rank is not available (for instance for random entries) otherwise returns a string such as "top 10%"
 */
- (NSString*) percentileRankForLeaderboardEntry:(NSDictionary*)leaderboardEntry;

/**
 * @ingroup query
 * @brief Returns the score for the supplied entry as a numeric value
 * This is a convenience method to extract data from the array items returned from top20EntriesForLeaderboardID: et al
 * Note that you should use timeForLeaderboardEntry: for time-based scores
 * @return The leaderboard score
 */
- (double) scoreForLeaderboardEntry:(NSDictionary*)leaderboardEntry;

/**
 * @ingroup query
 * @brief Returns the score of the supplied entry as a time-based value
 * This is a convenience method to extract data from the array items returned from top20EntriesForLeaderboardID: et al
 * Note that you should use scoreForLeaderboardEntry: for numeric scores
 * @return the leaderboard time value
 */
- (NSTimeInterval) timeForLeaderboardEntry:(NSDictionary*)leaderboardEntry;

@end
