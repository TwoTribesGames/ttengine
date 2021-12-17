#if !defined(INC_TT_IAP_PURCHASEMGR_H)
#define INC_TT_IAP_PURCHASEMGR_H

#if defined(TT_PLATFORM_OSX_IPHONE)  // currently, only iOS has in-app purchase support


#include <map>
#include <set>
#include <string>

#include <tt/iap/ProductInfo.h>
#include <tt/iap/PurchaseEventHandler.h>
#include <tt/platform/tt_error.h>


namespace tt {
namespace iap {

typedef std::set<std::string> ProductIDs;


class PurchaseMgr
{
public:
	/*! \param p_eventHandler REQUIRED! Receives callbacks about in-app purchase events.
	    \param p_useSandboxStore Whether to use a sandbox (development) store or the actual live store. */
	static void createInstance(const PurchaseEventHandlerPtr& p_eventHandler, bool p_useSandboxStore = false);
	static void destroyInstance();
	inline static bool         hasInstance() { return ms_instance != 0; }
	inline static PurchaseMgr* getInstance() { TT_NULL_ASSERT(ms_instance); return ms_instance; }
	
	/*! \brief Indicates whether in-app purchases are enabled by the user (whether anything can be purchased at all). */
	bool arePurchasesEnabled() const;
	
	/*! \brief Retrieves information about the specified product. Asynchronous operation;
	           client code will be notified via event handler when product information has been retrieved. */
	void requestProductInfo(const std::string& p_productID);
	
	/*! \brief Retrieves information about multiple products in one go. Asynchronous operation;
	           client code will be notified via event handler when product information has been retrieved (once per product). */
	void requestProductInfo(const ProductIDs& p_productIDs);
	
	/*! \brief Indicates whether information for the specified product ID is available. */
	bool hasProductInfo(const std::string& p_productID) const;
	
	/*! \brief Returns product information that was previously retrieved via requestProductInfo.
	           Returns null if information is not available for this product. */
	const ProductInfo* getProductInfo(const std::string& p_productID) const;
	
	/*! \brief Indicates whether the specified product has been purchased. */
	bool isProductPurchased(const std::string& p_productID);
	
	/*! \brief Purchases the specified product. Asynchronous operation; result is delivered via event handler callbacks. */
	void purchaseProduct(const std::string& p_productID, s32 p_quantity = 1);
	
	
	// Helper class to bridge between Objective C and C++ code (not for client code use)
	class DelegateBridge;
	
private:
	typedef std::map<std::string, ProductInfo> ProductInfoCache;
	
	
	PurchaseMgr(const PurchaseEventHandlerPtr& p_eventHandler, bool p_useSandboxStore);
	~PurchaseMgr();
	
	static std::string getPurchaseRegistrationDir();
	// NOTE: Just the filename, no path (use the function above for that)
	static std::string createProductFilename(const std::string& p_productID);
	
	/*
	void loadPurchaseRegistration();
	void savePurchaseRegistration() const;
	*/
	
	void handleProductInfoRetrieved(const std::string&  p_productID,
	                                const std::wstring& p_title,
	                                const std::wstring& p_description,
	                                const std::wstring& p_formattedPrice);
	void handlePurchaseCompleted(const std::string& p_productID);
	void handlePurchaseCancelled(const std::string& p_productID);
	void handlePurchaseFailed(const std::string&  p_productID,
	                          const std::wstring& p_message);
	
	// No copying
	PurchaseMgr(const PurchaseMgr&);
	PurchaseMgr& operator=(const PurchaseMgr&);
	
	
	static PurchaseMgr* ms_instance;
	
	PurchaseEventHandlerPtr m_eventHandler;
	DelegateBridge*         m_delegateBridge;
	
	ProductIDs       m_productsPurchased; // cache of product IDs which are already known to be purchased
	ProductInfoCache m_productInfoCache;
	
	
	friend class DelegateBridge;
};

// Namespace end
}
}


#endif  // defined(TT_PLATFORM_OSX_IPHONE)

#endif  // !defined(INC_TT_IAP_PURCHASEMGR_H)
