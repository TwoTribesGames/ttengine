#if !defined(INC_TT_IAP_PRODUCTINFO_H)
#define INC_TT_IAP_PRODUCTINFO_H


#include <string>


namespace tt {
namespace iap {

class PurchaseMgr;


/*! \brief Simple data class that provides details about an in-app purchase product. All strings are localized. */
class ProductInfo
{
public:
	inline ProductInfo() { }
	
	inline const std::wstring& getTitle()       const    { return m_title;          }
	inline const std::wstring& getDescription() const    { return m_description;    }
	inline const std::wstring& getFormattedPrice() const { return m_formattedPrice; }
	
private:
	inline ProductInfo(const std::wstring& p_title,
	                   const std::wstring& p_description,
	                   const std::wstring& p_formattedPrice)
	:
	m_title(p_title),
	m_description(p_description),
	m_formattedPrice(p_formattedPrice)
	{ }
	
	
	std::wstring m_title;
	std::wstring m_description;
	std::wstring m_formattedPrice;
	
	
	friend class PurchaseMgr;
};

// Namespace end
}
}


#endif  // !defined(INC_TT_IAP_PRODUCTINFO_H)
