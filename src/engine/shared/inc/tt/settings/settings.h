#if !defined(INC_TT_SETTINGS_SETTINGS_H)
#define INC_TT_SETTINGS_SETTINGS_H

#include <string>


namespace tt {
namespace settings {

enum Region
{
	Region_WW, //!< Worldwide
	Region_JP, //!< Japan
	Region_US, //!< North America
	Region_EU, //!< Europe
	
	Region_Invalid //!< Invalid region value
};


/*! \brief Gets the region this application is compiled for.
    \return The region this application is compiled for.*/
Region getRegion();


/*! \brief Gets the region name this application is compiled for. E.g., "jp", "us"
    \return The region name this application is compiled for. Returns 0 if the region is worldwide. */
std::string getRegionName();

/*! \brief Sets the region this application is compiled for.
    \param p_region The region this application is compiled for.*/
void setRegion(Region p_region);

/*! \brief Gets the name of the application.
    \return The name of the application.*/
std::wstring getApplicationName();

/*! \brief Sets the name of the application.
    \param p_name The name of the application.*/
void setApplicationName(const std::wstring& p_name);

/*! \brief Gets the region based langauge (us/ue/uf for Region_US).
    \return The region based language.*/
std::string getRegionLanguage();

// namespace end
}
}


#endif // !defined(INC_TT_SETTINGS_SETTINGS_H)
