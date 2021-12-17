#define NOMINMAX
#include <windows.h>
#include <fstream>
#include <sstream>

#include <tt/engine/renderer/DXUT/DXUT.h>
#include <tt/iap/PurchaseMgr.h>
#include <tt/str/str.h>
#include <tt/xml/XmlDocument.h>
#include <tt/xml/XmlNode.h>


namespace tt {
namespace iap {

const char* const g_purchasedProductsRegistrationFilename = "iap_purchased_products.txt";
const char* const g_productDetailsFilename                = "iap_product_details.xml";


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
	// FIXME: Also provide a way of disabling this for testing?
	return true;
}


void PurchaseMgr::requestProductInfo(const std::string& p_productID)
{
	ProductIDs ids;
	ids.insert(p_productID);
	requestProductInfo(ids);
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


void PurchaseMgr::requestProductInfo(const ProductIDs& p_productIDs)
{
	for (ProductIDs::const_iterator it = p_productIDs.begin(); it != p_productIDs.end(); ++it)
	{
		loadProductInfoFromFile(*it);
		
		m_eventHandler->handleProductInfoRetrieved(*it);
	}
}


bool PurchaseMgr::isProductPurchased(const std::string& p_productID) const
{
	return m_productsPurchased.find(p_productID) != m_productsPurchased.end();
}


void PurchaseMgr::purchaseProduct(const std::string& p_productID, s32 p_quantity)
{
	// FIXME: What to do if product was already purchased? Mimic iOS behavior!
	
	// Retrieve product details to provide more information in the message
	const ProductInfo& info(loadProductInfoFromFile(p_productID));
	
	// Compose a message with product details in it, confirming the purchase
	std::wostringstream message;
	message << L"Are you sure you wish to purchase " << p_quantity << L" of the following product?\n\n"
	           L"Title: "       << info.getTitle()          << L"\n"
	           L"Description: " << info.getDescription()    << L"\n"
	           L"Price: "       << info.getFormattedPrice() << L"\n"
	           L"Store ID: "    << str::utf8ToUtf16(p_productID);
	
	const int result = MessageBoxW(DXUTGetHWND(), message.str().c_str(), L"Confirm Purchase", MB_YESNO | MB_ICONQUESTION);
	if (result == IDNO)
	{
		// Purchase cancelled
		m_eventHandler->handlePurchaseCancelled(p_productID);
	}
	else if (result == IDYES)
	{
		m_productsPurchased.insert(p_productID);
		
		savePurchaseRegistration();
		
		m_eventHandler->handlePurchaseCompleted(p_productID);
	}
}


//--------------------------------------------------------------------------------------------------
// Private member functions

PurchaseMgr::PurchaseMgr(const PurchaseEventHandlerPtr& p_eventHandler, bool /*p_useSandboxStore*/)
:
m_eventHandler(p_eventHandler)
{
	TT_ASSERTMSG(m_eventHandler != 0, "A valid PurchaseEventHandler pointer must be passed to PurchaseMgr!");
	
	// Load purchase registration
	loadPurchaseRegistration();
}


PurchaseMgr::~PurchaseMgr()
{
}


const ProductInfo& PurchaseMgr::loadProductInfoFromFile(const std::string& p_productID)
{
	// Only load product info if it wasn't loaded yet
	ProductInfoCache::iterator it = m_productInfoCache.find(p_productID);
	if (it != m_productInfoCache.end())
	{
		return (*it).second;
	}
	
	ProductInfo info(L"Unknown Product", L"Unknown Product", L"$0");
	
	{
		// Load the product details XML file and get the product details from there
		xml::XmlDocument doc(g_productDetailsFilename);
		const xml::XmlNode* root = doc.getRootNode();
		
		if (root != 0)
		{
			for (const xml::XmlNode* node = root->getChild(); node != 0; node = node->getSibling())
			{
				if (node->getName() != "product" ||
				    node->getAttribute("id") != p_productID)
				{
					continue;
				}
				
				const xml::XmlNode* titleNode       = node->getFirstChild("title");
				const xml::XmlNode* descriptionNode = node->getFirstChild("description");
				const xml::XmlNode* priceNode       = node->getFirstChild("price");
				
				if (titleNode != 0 && descriptionNode != 0 && priceNode != 0)
				{
					info.m_title          = str::utf8ToUtf16(titleNode->getData());
					info.m_description    = str::utf8ToUtf16(descriptionNode->getData());
					info.m_formattedPrice = L"$" + str::utf8ToUtf16(priceNode->getData());
				}
			}
		}
	}
	
	m_productInfoCache.insert(std::make_pair(p_productID, info));
	return m_productInfoCache[p_productID];
}


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

// Namespace end
}
}
