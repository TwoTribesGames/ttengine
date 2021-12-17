#if !defined(INC_TT_IAP_PURCHASEEVENTHANDLER_H)
#define INC_TT_IAP_PURCHASEEVENTHANDLER_H


#include <string>

#include <tt/platform/tt_types.h>


namespace tt {
namespace iap {

/*! \brief Interface for in-app purchase event handlers. Receives callbacks about in-app purchase events. */
class PurchaseEventHandler
{
public:
	PurchaseEventHandler() { }
	virtual ~PurchaseEventHandler() { }
	
	// FIXME: A callback might also be needed for failing to retrieve product info
	//        (however, the iOS back-end only reports success, there is no way of detecting failure)
	
	/*! \brief Called when information for the specified product has been retrieved (and is now available). */
	virtual void handleProductInfoRetrieved(const std::string& p_productID) = 0;
	
	/*! \brief Called when the purchase of the specified product has been completed successfully. */
	virtual void handlePurchaseCompleted(const std::string& p_productID) = 0;
	
	/*! \brief Called when the purchase of the specified product was cancelled by the user. */
	virtual void handlePurchaseCancelled(const std::string& p_productID) = 0;
	
	/*! \brief Called when the purchase of the specified product failed for any reason (other than cancellation). */
	virtual void handlePurchaseFailed(const std::string& p_productID, const std::wstring& p_message) = 0;
};

typedef tt_ptr<PurchaseEventHandler>::shared PurchaseEventHandlerPtr;
typedef tt_ptr<PurchaseEventHandler>::weak   PurchaseEventHandlerWeakPtr;

// Namespace end
}
}


#endif  // !defined(INC_TT_IAP_PURCHASEEVENTHANDLER_H)
