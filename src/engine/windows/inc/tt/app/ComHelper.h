#if !defined(INC_TT_APP_COMHELPER_H)
#define INC_TT_APP_COMHELPER_H


#include <tt/platform/tt_types.h>


namespace tt {
namespace app {

/*! \brief Central location for initializing and uninitializing COM (Windows Component Object Model).
    \note This class does not work in a multi-threading setup! Each thread needs to initialize COM for itself. */
class ComHelper
{
public:
	/*! \brief Initialize COM (can safely be called multiple times). */
	static void initCom();
	
	/*! \brief Uninitialize COM (can safely be called multiple times). */
	static void uninitCom();
	
	/*! \brief Set whether COM should be used in multi-threaded mode (the default)
	           or single-threaded (needed if also using OLE). */
	static void setMultiThreaded(bool p_multithreaded);
	
	/*! \brief Indicates whether COM is (set to be) initialized as multi-threaded or single-threaded. */
	inline static bool isMultiThreaded() { return ms_multithreaded; }
	
private:
	// No instantiation (static class)
	ComHelper();
	~ComHelper();
	ComHelper(const ComHelper&);
	ComHelper& operator=(const ComHelper&);
	
	
	//! Number of COM initializations that have been performed ('reference counting')
	static s32  ms_initializedCount;
	static bool ms_multithreaded; //!< Whether COM should be initialized as multi-threaded
};

// Namespace end
}
}


#endif  // !defined(INC_TT_APP_COMHELPER_H)
