//
//  CrystalAchievements.h
//  Crystal
//
//  Created by willeeh on 18/01/2011.
//  Copyright 2011 Chillingo Ltd. All rights reserved.
//

#import <Foundation/Foundation.h>

@protocol CrystalAchievementDelegate <NSObject>
@required
/**
 * @brief Reports to the client that achievements has been updated
 */
- (void) crystalAchievementsUpdated;

@end

/**
 * @brief The Crystal interface to the Crystal direct achievement access API.
 *
 * <b>IMPORTANT NOTE:</b> This API has a performance overhead for both the client application as well as the Crystal servers and requires the use of the network. 
 * Therefore this API should only be used for applications that wish to display the leaderboard data manually.
 * Please do not rely on the Crystal Acheivement data for application features.
 */
@interface CrystalAchievements : NSObject 
{
@public
	id<CrystalAchievementDelegate> delegate;		/// Set this to get notifications for downloaded achievement data via crystalAchievementUpdated:.
}

@property (nonatomic, assign)	id<CrystalAchievementDelegate> delegate;


/**
 * @defgroup get Getting an array of achievements unlocked
 * @defgroup query Accessing achievement data
 */

/**
 * @brief Returns the shared singleton instance of CrystalLeaderboards
 */
+ (CrystalAchievements*) sharedInstance;

/**
 * @ingroup get
 * @brief Retrieve the achievements unlocked for the current user
 * @return An array of achievement IDs or nil if the achievement data is not available
 */
- (NSArray*) getAchievementsUnlocked;


@end
