/*
 *  CrystalVirtualGoods.h
 *  Crystal
 *
 *  Created by Gareth Reese on 03/11/2010.
 *  Copyright 2010 Chillingo Ltd. All rights reserved.
 *
 */

#import <Foundation/Foundation.h>


/**
 * @brief Delegate protocol for the CrystalVirtualGoods class
 */
@protocol CrystalVirtualGoodsDelegate <NSObject>
@optional

/**
 * @brief Reports to the client that the CrystalVirtualGoods info has been updated
 * The updates can come as a result of changes in virtual currency balances, purchased items, etc.
 * When updating balances and goods via the postGoods: and postBalances: APIs this method will be called to inform the client that the server-side data has been updated.
 * This update message can still be received while the Crystal UI is still open, so it's important not to play any sounds or change the UI in response to the message.
 * @param success YES if the update was successful
 */
- (void) crystalVirtualGoodsInfoUpdatedWithSuccess:(BOOL)success;

@end


/**
 * This class is intended to be used in conjunction with the CrystalPlayer class, as some of it's properties (i.e. isCrystalUser) preclude the use of this class.
 * While the Crystal SDK makes every effort to keep the CrystalVirtualGoods class up-to-date on the server side it is ultimately the responsibility of the developer
 * to ensure that the state of goods and currencies is maintained on the user-facing side. The CrystalVirtualGoods class is intended to provide a representation of the data on the server
 * and no attempt is made to ensure that changes are recorded within the client SDK.
 */
@interface CrystalVirtualGoods : NSObject
{
@public
	id<CrystalVirtualGoodsDelegate>		_delegate;
}


/**
 * @brief Returns the shared singleton instance of CrystalVirtualGoods
 */
+ (CrystalVirtualGoods*) sharedInstance;


/**
 * @brief Instructs the SDK to start keeping the CrystalPlayer class up-to-date with the current player's info
 * Because this method initiates network traffic and server load it is important that apps refrain from calling the method unless virtual goods and currencies are implemented
 */
- (void) startUpdating;


/**
 * @brief Instructs the SDK to update the CrystalVirtualGoods info as soon as possible
 * Ordinarily any activity relating to virtual goods or currencies inside the Crystal UI will automatically instruct CrystalVirtualGoods to update itself.
 * The same is true when using APIs like postGoods:. If a post fails the client should always retry via the post calls and not call updateNow. 
 * updateNow will retrieve updated information from the server and any information previously posted will be lost.
 * It should be rare that the client will need to call this method but it is added for completeness.
 */
- (void) updateNow;


/**
 * @brief Returns YES if the info for the curent player's virtual goods info is up-to-date
 * Crystal will make every effort to keep the data up-to-date, so as long as the device has an Internet connection it should be rare that this method returns NO.
 */
- (BOOL) virtualGoodsInfoIsUpToDate;


// Doxygen won't pick up readonly properties unless they're specified as @public

/**
 * @public
 * Returns the virtual goods owned by the user.
 * Will return nil if no goods are owned.
 * Will return a dictionary of goods otherwise, where the key is the virtual good ID (NSString) and the value is the number of goods owned (NSNumber).
 */
@property (nonatomic, readonly)			NSDictionary* goods;


/**
 * @public
 * Returns the balances of any virtual currencies owned by the user.
 * Will return nil if no currencies are owned.
 * Will return a dictionary of balances otherwise, where the key is the currency ID (NSString) and the value is the balance (NSNumber).
 */
@property (nonatomic, readonly)			NSDictionary* balances;


/** 
 * @see CrystalPlayerDelegate for a description of this property.
 * This property must be set to get notified when the info in CrystalVirtualGoods is updated.
 * It is the responsibility of the client to ensure that the object set as the delegate remains valid for the duration of the Crystal session.
 */
@property (nonatomic, assign)			id<CrystalVirtualGoodsDelegate> delegate;


/**
 * @brief Post changes to the user's virtual goods record back to the server
 * Use this method to post one or more changes to the user's virtual goods to the server.
 * Changes will only be made to the virtual good that are supplied, and more than one virtual good can be updated at once. There is no need to replicate the information stored in the goods property in full.
 * To avoid server load it is recommended to combine changes into one call by adding the info to the same dictionary.
 * To remove the good from the user supply a 0 for the number of goods owned.
 * When the virtual goods have been updated the client will be informed via crystalVirtualGoodsInfoUpdatedWithSuccess: and the results reflected in the goods property.
 * @param goods The format of the information supplied is a dictionary, where the key is the virtual good ID (NSString) and the value is the number of goods owned (NSNumber).
 */
- (void) postGoods:(NSDictionary*)goods;


/**
 * @brief Post changes to the user's balances back to the server
 * Use this method to post one or more changes to the user's wallet balances to the server.
 * Changes will only be made to the balances that are supplied, and more than one balance can be updated at once. There is no need to replicate the information stored in the balances property in full.
 * To avoid server load it is recommended to combine changes into one call by adding the info to the same dictionary.
 * When the virtual currencies have been updated the client will be informed via crystalVirtualGoodsInfoUpdatedWithSuccess: and the results reflected in the goods property.
 * @param balances The format of the information supplied is a dictionary, where the key is the currency ID (NSString) and the value is the balance (NSNumber).
 */
- (void) postBalances:(NSDictionary*)balances;


/**
 * @brief Post both goods and balances changes to the server
 * Where the client needs to post changes to both currencies and goods it is recommended that it uses this method to avoid excessive server load.
 */
- (void) postGoods:(NSDictionary*)goods andBalances:(NSDictionary*)balances;


/**
 * @brief Set a list of goods that are not selectable and are shown with a 'locked' icon in the Crystal UI
 * This method works differently from postGoods: and postBalances: in that it's not possible to 'update' items peacemeal.
 * The client must send all locked goods in one call, and can remove the locks by passing nil as the parameter.
 * @param lockedGoods An NSSet of NSString good IDs to lock or nil to remove any locks
 */
- (void) setLockedGoods:(NSSet*)lockedGoods;


@end
