#if !defined(INC_TT_IAP_PURCHASEMGR_H)
#define INC_TT_IAP_PURCHASEMGR_H

#include <map>
#include <set>
#include <string>

#include <tt/iap/ProductInfo.h>
#include <tt/iap/PurchaseEventHandler.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace iap {

typedef std::set<std::string> ProductIDs;


/*! \brief The Windows implementation of this class is a dummy implementation
           that simulates the behavior of in-app purchases on iOS. */
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
	bool isProductPurchased(const std::string& p_productID) const;
	
	/*! \brief Purchases the specified product. Asynchronous operation; result is delivered via event handler callbacks. */
	void purchaseProduct(const std::string& p_productID, s32 p_quantity = 1);
	
private:
	typedef std::map<std::string, ProductInfo> ProductInfoCache;
	
	
	PurchaseMgr(const PurchaseEventHandlerPtr& p_eventHandler, bool p_useSandboxStore);
	~PurchaseMgr();
	
	const ProductInfo& loadProductInfoFromFile(const std::string& p_productID);
	
	void loadPurchaseRegistration();
	void savePurchaseRegistration() const;
	
	// No copying
	PurchaseMgr(const PurchaseMgr&);
	PurchaseMgr& operator=(const PurchaseMgr&);
	
	
	static PurchaseMgr* ms_instance;
	
	PurchaseEventHandlerPtr m_eventHandler;
	
	ProductIDs       m_productsPurchased;
	ProductInfoCache m_productInfoCache;
};

// Namespace end
}
}

#endif  // !defined(INC_TT_IAP_PURCHASEMGR_H)
