//
//  CrystalPlayer.h
//  Crystal
//
//  Created by Gareth Reese on 06/10/2010.
//  Copyright 2010 Chillingo Ltd. All rights reserved.
//

#import <Foundation/Foundation.h>


@class CCPlayerData;


/**
 * @brief Delegate protocol for the CrystalPlayer class
 */
@protocol CrystalPlayerDelegate <NSObject>
@optional

/**
 * @brief Reports to the client that the CrystalPlayer info has been updated
 * The updates can come as a result of a new gift being received or a change in the user logged into the system.
 * This update message can still be received while the Crystal UI is still open, so it's important not to play any sounds or change the UI in response to the message.
 * @param success YES if the update was successful
 */
- (void) crystalPlayerInfoUpdatedWithSuccess:(BOOL)success;

@end


/**
 * @brief The interface to the Crystal gifting functionality and player info
 * This class updates automatically from the server after the user calls startUpdating.
 * The class provides access to various pieces of information about the user and can be used to improve the UI of the game and integrate more completely with the Crystal SDK.
 * For more details on the information provided see the properties available.
 */
@interface CrystalPlayer : NSObject
{
@public
	id<CrystalPlayerDelegate> _delegate;
	
@private
	int					_updateState;
	CCPlayerData*		_playerData;
}

// Doxygen won't pick up readonly properties unless they're specified as @public

/// @public
/// Returns YES if the user is currently signed into Crystal
@property (nonatomic, readonly)		BOOL isSignedIn;

/// @public
/// Returns YES if the user has signed into Crystal
@property (nonatomic, readonly)		BOOL isCrystalUser;

/// @public
/// Returns YES if the user has signed into Crystal and entered their Facebook details
@property (nonatomic, readonly)		BOOL isFacebookUser;

/// @public
/// Returns YES if the user has signed into Crystal and entered their Twitter details
@property (nonatomic, readonly)		BOOL isTwitterUser;

/// @public
/// The number of friends this user has
@property (nonatomic, readonly)		unsigned int numberOfCrystalFriends;

/// @public
/// The number of friends this user has that also own the game
@property (nonatomic, readonly)		unsigned int numberOfCrystalFriendsWithGame;

/// @public
/// An array of NSDecimalNumber IDs for each of the gifts owned by the user, or nil if the user doesn't have any gifts
@property (nonatomic, readonly)		NSArray* gifts;

/// @public
/// The player's current alias in Crystal
@property (nonatomic, readonly)		NSString* alias;

/// @public
/// The badge number that Crystal will set for the application icon and that can be shown over the Crystal button in-game.
/// If you don't want to go to the effort of implementing the badges a >0 value can be used to highlight the Crystal button somehow.
/// The badge number will indicate that the user has outstanding gifts, friend requests or challenges.
@property (nonatomic, readonly)		unsigned int badgeNumber;

/// @public
///	Unique identifier for player
@property (nonatomic, readonly)		NSString* crystalPlayerId;

/// @public
/// An array of NSString IDs for each friend of the user, or nil if the user doesn't have any friends
@property (nonatomic, readonly)		NSArray* crystalFriendIds;

/// @see CrystalPlayerDelegate
@property (nonatomic, assign)		id<CrystalPlayerDelegate> delegate;

@property (nonatomic)				int _updateState;
@property (nonatomic, retain)		CCPlayerData* _playerData;


/**
 * @brief Returns the shared singleton instance of CrystalPlayer
 */
+ (CrystalPlayer*) sharedInstance;


/**
 * @brief Instructs the SDK to start keeping the CrystalPlayer class up-to-date with the current player's info
 * Because this method initiates network traffic and server load it is important that apps refrain from calling the method unless the CrystalPlayer data is used
 */
- (void) startUpdating;


/**
 * @brief Returns YES if the info for the curent player is up-to-date
 * Crystal will make every effort to keep the data up-to-date, so as long as the device has an Internet connection it should be rare that this method returns NO
 */
- (BOOL) playerInfoIsUpToDate;

@end
