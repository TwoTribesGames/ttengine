#if defined(TT_PLATFORM_OSX_IPHONE)  // currently, only iOS has in-app purchase support

#import <StoreKit/StoreKit.h>
/*
#define NOMINMAX
#include <windows.h>
#include <fstream>
#include <sstream>
*/

#include <tt/app/Application.h>
#include <tt/fs/fs.h>
#include <tt/fs/utils/utils.h>
#include <tt/iap/PurchaseMgr.h>
#include <tt/str/str.h>
/*
#include <tt/xml/XmlDocument.h>
#include <tt/xml/XmlNode.h>
*/


//--------------------------------------------------------------------------------------------------
// Objective C delegate class for all in-app purchase callbacks (declaration)

@interface TTInAppPurchaseDelegate : NSObject<SKPaymentTransactionObserver, SKProductsRequestDelegate>
{
	tt::iap::PurchaseMgr::DelegateBridge* m_bridge;
}

- (id)init:(tt::iap::PurchaseMgr::DelegateBridge*)p_bridge;

// SKPaymentTransactionObserver delegate
- (void)paymentQueue:(SKPaymentQueue*)p_queue updatedTransactions:(NSArray*)p_transactions;
- (void)paymentQueue:(SKPaymentQueue*)p_queue removedTransactions:(NSArray*)p_transactions;
- (void)paymentQueueRestoreCompletedTransactionsFinished:(SKPaymentQueue*)p_queue;
- (void)paymentQueue:(SKPaymentQueue*)p_queue restoreCompletedTransactionsFailedWithError:(NSError*)p_error;

// SKProductsRequestDelegate delegate
- (void)productsRequest:(SKProductsRequest*)p_request didReceiveResponse:(SKProductsResponse*)p_response;

@end


//--------------------------------------------------------------------------------------------------
// DelegateBridge helper inner class (bridge between Objective C and C++ code)

namespace tt {
namespace iap {

class PurchaseMgr::DelegateBridge
{
public:
	explicit inline DelegateBridge(PurchaseMgr* p_manager)
	:
	m_manager(p_manager),
	m_delegate(nil)
	{
		m_delegate = [[TTInAppPurchaseDelegate alloc] init:this];
	}
	
	inline ~DelegateBridge()
	{
		// Destroy Objective C delegate
		if (m_delegate != nil)
		{
			[m_delegate release];
			m_delegate = nil;
		}
	}
	
	inline TTInAppPurchaseDelegate* getDelegate() const { return m_delegate; }
	
	inline void handleProductInfoRetrieved(const std::string&  p_productID,
	                                       const std::wstring& p_title,
	                                       const std::wstring& p_description,
	                                       const std::wstring& p_formattedPrice)
	{
		m_manager->handleProductInfoRetrieved(p_productID, p_title, p_description, p_formattedPrice);
	}
	
	// FIXME: Provide much more info to the C++ code (we need to securely store proof of the purchase)
	inline void handlePurchaseCompleted(const std::string& p_productID)
	{
		m_manager->handlePurchaseCompleted(p_productID);
	}
	
	inline void handlePurchaseCancelled(const std::string& p_productID)
	{
		m_manager->handlePurchaseCancelled(p_productID);
	}
									 
	inline void handlePurchaseFailed(const std::string& p_productID,
									 const std::wstring& p_message)
	{
		m_manager->handlePurchaseFailed(p_productID, p_message);
	}
	
	// create C++ functions for all applicable delegate callbacks here...
	/*
	inline void somethingOrOther() { m_manager->somethingOrOther(); }
	*/
	
private:
	PurchaseMgr*             m_manager;
	TTInAppPurchaseDelegate* m_delegate;
};

// Namespace end
}
}



//--------------------------------------------------------------------------------------------------
// Objective C delegate class for all in-app purchase callbacks (implementation)

@implementation TTInAppPurchaseDelegate

- (id)init:(tt::iap::PurchaseMgr::DelegateBridge*)p_bridge
{
	self = [super init];
	if (self != nil)
	{
		m_bridge = p_bridge;
	}
	return self;
}

// SKPaymentTransactionObserver delegate
- (void)paymentQueue:(SKPaymentQueue*)p_queue updatedTransactions:(NSArray*)p_transactions
{
	for (SKPaymentTransaction* transaction in p_transactions)
	{
		bool removeFromQueue = true;
		
		switch (transaction.transactionState)
		{
		case SKPaymentTransactionStatePurchased:
			NSLog(@"TTInAppPurchaseDelegate::paymentQueue:updatedTransactions: [PURCHASED] Product '%@'. Transaction ID '%@'",
				  transaction.payment.productIdentifier, transaction.transactionIdentifier);
			//[self completeTransaction:transaction];
			// FIXME: Provide much more info to the C++ code (we need to securely store proof of the purchase)
			m_bridge->handlePurchaseCompleted([transaction.payment.productIdentifier UTF8String]);
			break;
			
		case SKPaymentTransactionStateFailed:
			if (transaction.error.code == SKErrorPaymentCancelled)
			{
				NSLog(@"TTInAppPurchaseDelegate::paymentQueue:updatedTransactions: [CANCELLED] Transaction for product '%@' was cancelled.",
					  transaction.payment.productIdentifier);
				m_bridge->handlePurchaseCancelled([transaction.payment.productIdentifier UTF8String]);
			}
			else
			{
				NSLog(@"TTInAppPurchaseDelegate::paymentQueue:updatedTransactions: [FAILED] Product '%@'. Error '%@'",
				      transaction.payment.productIdentifier, transaction.error.localizedDescription);
				//[self failedTransaction:transaction];
				m_bridge->handlePurchaseFailed([transaction.payment.productIdentifier UTF8String],
											   tt::str::utf8ToUtf16([transaction.error.localizedDescription UTF8String]));
			}
			break;
			
		case SKPaymentTransactionStateRestored:
			NSLog(@"TTInAppPurchaseDelegate::paymentQueue:updatedTransactions: [RESTORED] Product '%@'. Transaction ID '%@'",
				  transaction.payment.productIdentifier, transaction.transactionIdentifier);
			//[self restoreTransaction:transaction];
			// FIXME: What to do with restored transactions? Treat as normal purchase completion?
			break;
			
		case SKPaymentTransactionStatePurchasing:
			NSLog(@"TTInAppPurchaseDelegate::paymentQueue:updatedTransactions: [BUSY] Product '%@' is busy purchasing.",
				  transaction.payment.productIdentifier);
			removeFromQueue = false;
			break;
			
		default:
			NSLog(@"TTInAppPurchaseDelegate::paymentQueue:updatedTransactions: Unsupported transaction state: %d",
				  transaction.transactionState);
			break;
		}
		
		if (removeFromQueue)
		{
			[p_queue finishTransaction: transaction];
		}
	}
}

- (void)paymentQueue:(SKPaymentQueue*)p_queue removedTransactions:(NSArray*)p_transactions
{
	(void)p_queue;
	(void)p_transactions;
	NSLog(@"TTInAppPurchaseDelegate::paymentQueue:removedTransactions:");
}

- (void)paymentQueueRestoreCompletedTransactionsFinished:(SKPaymentQueue*)p_queue
{
	(void)p_queue;
	NSLog(@"TTInAppPurchaseDelegate::paymentQueueRestoreCompletedTransactionsFinished:");
}

- (void)paymentQueue:(SKPaymentQueue*)p_queue restoreCompletedTransactionsFailedWithError:(NSError*)p_error
{
	(void)p_queue;
	(void)p_error;
	NSLog(@"TTInAppPurchaseDelegate::paymentQueue:restoreCompletedTransactionsFailedWithError:");
}

// SKProductsRequestDelegate delegate
- (void)productsRequest:(SKProductsRequest*)p_request didReceiveResponse:(SKProductsResponse*)p_response
{
	NSLog(@"TTInAppPurchaseDelegate::productsRequest:didReceiveResponse:");
	
	// Handle all valid products
	for (SKProduct* product in p_response.products)
	{
		NSNumberFormatter* numberFormatter = [[NSNumberFormatter alloc] init];
		[numberFormatter setFormatterBehavior:NSNumberFormatterBehavior10_4];
		[numberFormatter setNumberStyle:NSNumberFormatterCurrencyStyle];
		[numberFormatter setLocale:product.priceLocale];
		
		m_bridge->handleProductInfoRetrieved(
			[product.productIdentifier UTF8String],
			tt::str::utf8ToUtf16([product.localizedTitle UTF8String]),
			tt::str::utf8ToUtf16([product.localizedDescription UTF8String]),
			tt::str::utf8ToUtf16([[numberFormatter stringFromNumber:product.price] UTF8String]));
	}
	
	// Notify client about any invalid products
	for (NSString* productID in p_response.invalidProductIdentifiers)
	{
		NSLog(@"- [INVALID] Product '%@'", productID);
	}
	
	[p_request autorelease];
}

@end



namespace tt {
namespace iap {

//const char* const g_purchasedProductsRegistrationFilename = "iap_purchased_products.txt";
//const char* const g_productDetailsFilename                = "iap_product_details.xml";


PurchaseMgr* PurchaseMgr::ms_instance = 0;


//--------------------------------------------------------------------------------------------------
// Public member functions

void PurchaseMgr::createInstance(const PurchaseEventHandlerPtr& p_eventHandler, bool p_useSandboxStore)
{
	TT_ASSERTMSG(ms_instance == 0, "PurchaseMgr instance was already created.");
	if (ms_instance == 0)
	{
		ms_instance = new PurchaseMgr(p_eventHandler, p_useSandboxStore);
	}
}


void PurchaseMgr::destroyInstance()
{
	TT_ASSERTMSG(ms_instance != 0, "PurchaseMgr instance was already destroyed.");
	delete ms_instance;
	ms_instance = 0;
}


bool PurchaseMgr::arePurchasesEnabled() const
{
	return [SKPaymentQueue canMakePayments] == YES;
}


void PurchaseMgr::requestProductInfo(const std::string& p_productID)
{
	ProductIDs ids;
	ids.insert(p_productID);
	requestProductInfo(ids);
}


void PurchaseMgr::requestProductInfo(const ProductIDs& p_productIDs)
{
	// Create an Objectice C set for the product IDs
	NSMutableSet* products = [[NSMutableSet alloc] initWithCapacity:static_cast<NSUInteger>(p_productIDs.size())];
	for (ProductIDs::const_iterator it = p_productIDs.begin(); it != p_productIDs.end(); ++it)
	{
		// Do not re-request info if it was already available
		if (m_productInfoCache.find(*it) != m_productInfoCache.end())
		{
			m_eventHandler->handleProductInfoRetrieved(*it);
		}
		else
		{
			[products addObject:[NSString stringWithUTF8String:(*it).c_str()]];
		}
	}
	
	// Abort if no products remain for an actual retrieval request
	if ([products count] == 0)
	{
		return;
	}
	
	// Send a request to the app store for info about the specified products
	SKProductsRequest* request = [[SKProductsRequest alloc] initWithProductIdentifiers:products];
	
	request.delegate = m_delegateBridge->getDelegate();
	
#if !defined(TT_BUILD_FINAL)
	std::string allIds;
	for (ProductIDs::const_iterator it = p_productIDs.begin(); it != p_productIDs.end(); ++it)
	{
		allIds += "- " + *it + "\n";
	}
	NSLog(@"PurchaseMgr::requestProductInfo: Sending request for info about the following product IDs:\n%s", allIds.c_str());
#endif
	
	[request start];
}


bool PurchaseMgr::hasProductInfo(const std::string& p_productID) const
{
	return m_productInfoCache.find(p_productID) != m_productInfoCache.end();
}


const ProductInfo* PurchaseMgr::getProductInfo(const std::string& p_productID) const
{
	ProductInfoCache::const_iterator it = m_productInfoCache.find(p_productID);
	if (it == m_productInfoCache.end())
	{
		return 0;
	}
	
	return &(*it).second;
}


bool PurchaseMgr::isProductPurchased(const std::string& p_productID)
{
	// Check if product is already known to have been purchased (speedy check)
	if (m_productsPurchased.find(p_productID) != m_productsPurchased.end())
	{
		return true;
	}
	
	// Product ID was not cached as being purchased; check saved data to see if it was purchased after all
	const std::string filename(getPurchaseRegistrationDir() + createProductFilename(p_productID));
	if (fs::fileExists(filename, app::getApplication()->getSaveFsID()) == false)
	{
		// No saved proof of purchase: not purchased
		return false;
	}
	
	// Open file, validate data somehow...
	
	// FIXME: What he said (above)...
	
	// DEBUG: File exists, assume product is purchased (for now)
	return true;
}


void PurchaseMgr::purchaseProduct(const std::string& p_productID, s32 p_quantity)
{
	(void)p_productID;
	(void)p_quantity;
	TT_PANIC("purchaseProduct has been disabled for iOS because of iOS 5 deprecation issues.");
#if 0  // Disabled for now, because of deprecation issues in iOS 5
	if (arePurchasesEnabled() == false)
	{
		TT_PANIC("Cannot purchase product '%s': purchases are not enabled on this system.", p_productID.c_str());
		m_eventHandler->handlePurchaseCancelled(p_productID);
		return;
	}
	
	if (isProductPurchased(p_productID))
	{
		// Product was already purchased
		TT_PANIC("Product '%s' was already purchased.", p_productID.c_str());
		m_eventHandler->handlePurchaseCompleted(p_productID);
		return;
	}
	
	NSLog(@"PurchaseMgr::purchaseProduct: Sending purchase/payment request for %d x product '%s'.",
	      (int)p_quantity, p_productID.c_str());
	// FIXME: paymentWithProductIdentifier is deprecated in iOS 5. Apple expects developers to
	// always pass an SKProduct instance to SKPayment. The only way to get one of these instances
	// is by sending an SKProductsRequest. It is unclear whether we're allowed to store these objects
	// after they've been retrieved. Depending on this, we either need to request (all?) product
	// information once, store it and use it when trying to purchase products, or request product
	// information for the product we're trying to purchase each time a purchase must be made.
	SKMutablePayment* payment = [SKMutablePayment paymentWithProductIdentifier:[NSString stringWithUTF8String:p_productID.c_str()]];
	payment.quantity = static_cast<NSInteger>(p_quantity);
	
	[[SKPaymentQueue defaultQueue] addPayment:payment];
#endif
}


//--------------------------------------------------------------------------------------------------
// Private member functions

PurchaseMgr::PurchaseMgr(const PurchaseEventHandlerPtr& p_eventHandler, bool /*p_useSandboxStore*/)
:
m_eventHandler(p_eventHandler),
m_delegateBridge(0)
{
	TT_ASSERTMSG(m_eventHandler != 0, "A valid PurchaseEventHandler pointer must be passed to PurchaseMgr!");
	
	// Load purchase registration
	//loadPurchaseRegistration();
	
	m_delegateBridge = new DelegateBridge(this);
	[[SKPaymentQueue defaultQueue] addTransactionObserver:m_delegateBridge->getDelegate()];
}


PurchaseMgr::~PurchaseMgr()
{
	[[SKPaymentQueue defaultQueue] removeTransactionObserver:m_delegateBridge->getDelegate()];
	delete m_delegateBridge;
}


std::string PurchaseMgr::getPurchaseRegistrationDir()
{
	return fs::getSaveRootDir(app::getApplication()->getSaveFsID()) + ".iap/";
}


std::string PurchaseMgr::createProductFilename(const std::string& p_productID)
{
	// FIXME: Somehow obfuscate this filename so that it isn't immediately clear which product ID it maps to
	return "." + p_productID;
}


/*
void PurchaseMgr::loadPurchaseRegistration()
{
	// Clear any previous products from the registration (just in case)
	m_productsPurchased.clear();
	
	std::ifstream in(g_purchasedProductsRegistrationFilename);
	if (!in)
	{
		// Failed to open file; no purchased products to load
		return;
	}
	
	while (in.eof() == false)
	{
		std::string productID;
		std::getline(in, productID);
		if (productID.empty() == false)
		{
			m_productsPurchased.insert(productID);
		}
	}
}


void PurchaseMgr::savePurchaseRegistration() const
{
	std::ofstream out(g_purchasedProductsRegistrationFilename);
	if (!out)
	{
		// Failed to open file: cannot store purchased products!
		TT_PANIC("Could not open file '%s' for writing; cannot store purchased products.",
		         g_purchasedProductsRegistrationFilename);
		return;
	}
	
	for (ProductIDs::const_iterator it = m_productsPurchased.begin(); it != m_productsPurchased.end(); ++it)
	{
		out << *it << "\n";
	}
}
//*/


void PurchaseMgr::handleProductInfoRetrieved(const std::string&  p_productID,
                                             const std::wstring& p_title,
                                             const std::wstring& p_description,
                                             const std::wstring& p_formattedPrice)
{
	TT_ASSERTMSG(m_productInfoCache.find(p_productID) == m_productInfoCache.end(),
	             "Product information for ID '%s' was already loaded.", p_productID.c_str());
	ProductInfo info(p_title, p_description, p_formattedPrice);
	m_productInfoCache[p_productID] = info;
	
	// Notify client code that product info is available
	m_eventHandler->handleProductInfoRetrieved(p_productID);
}


void PurchaseMgr::handlePurchaseCompleted(const std::string& p_productID)
{
	TT_ASSERTMSG(m_productsPurchased.find(p_productID) == m_productsPurchased.end(),
				 "Product '%s' was already purchased.", p_productID.c_str());
	
	m_productsPurchased.insert(p_productID);
	
	// Save proof of purchase
	// FIXME: This needs to be much more extensive (also save information allowing better validation)
	const fs::identifier saveFsID = app::getApplication()->getSaveFsID();
	const std::string saveDir(getPurchaseRegistrationDir());
	
	if (fs::dirExists(saveDir, saveFsID) == false)
	{
		if (fs::utils::createDirRecursive(saveDir, saveFsID) == false)
		{
			TT_PANIC("Could not create in-app purchase registration dir!");
		}
	}
	
	{
		const std::string filename(saveDir + createProductFilename(p_productID));
		fs::FilePtr file = fs::open(filename, fs::OpenMode_Write, saveFsID);
		TT_ASSERTMSG(file != 0, "Could not save proof of purchase!");
		(void)file;
	}
	
	m_eventHandler->handlePurchaseCompleted(p_productID);
}


void PurchaseMgr::handlePurchaseCancelled(const std::string& p_productID)
{
	m_eventHandler->handlePurchaseCancelled(p_productID);
}


void PurchaseMgr::handlePurchaseFailed(const std::string& p_productID, const std::wstring& p_message)
{
	m_eventHandler->handlePurchaseFailed(p_productID, p_message);
}
	
// Namespace end
}
}


#endif  // defined(TT_PLATFORM_OSX_IPHONE)
