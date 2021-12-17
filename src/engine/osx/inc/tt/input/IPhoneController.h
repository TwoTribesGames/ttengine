#if !defined(INC_TT_INPUT_IPHONECONTROLLER_H)
#define INC_TT_INPUT_IPHONECONTROLLER_H

#if defined(TT_PLATFORM_OSX_IPHONE)

#include <vector>
#include <map>

#include <tt/input/Accelerometer.h>
#include <tt/input/Button.h>
#include <tt/input/ControllerIndex.h>
#include <tt/input/Pointer.h>
#include <tt/input/Touch.h>


namespace tt {
namespace input {

/*! \brief TT input controller implementation for the iPhone. */
struct IPhoneController
{
public:
	static bool isConnected(ControllerIndex p_index);
	
	/*! \brief Returns the state of the controller. */
	static const IPhoneController& getState(ControllerIndex p_index);
	
	static void update();
	
	/*! \brief Indicates whether the IPhoneController has been initialized.
	    \return True when initialized, false if not. */
	static bool isInitialized();
	
	/*! \brief Initializes the controller.
	           Must be called before controller can be used.
	    \param p_accelerometerFrequency Sampling rate (in Hertz) for accelerometer polling.
	    \return True if initialization was succesful, false if not. */
	static bool initialize(/*float p_accelerometerFrequency*/);
	
	static void deinitialize();
	
	// NOTE: These functions are for use by TTdevObjCAppView only!
	// (declaring an Objective C class as friend is impossible, which is why this function is public)
	static inline IPhoneController& appView_getTemporaryState() { return ms_temporary; }
	void      appView_startTouch( Touch::ID p_id, const tt::math::Point2& p_location);
	Touch&    appView_modifyTouch(Touch::ID p_id);
	// END of TTdevObjCAppView only functions
	
	// Touches.
	typedef std::vector<Touch> Touches;	
	Touches allTouches;
	
	// Touch helpers
	bool         hasTouch(Touch::ID p_id) const;
	const Touch& getTouch(Touch::ID p_id) const;
	
	Button  touch1Status;   //!< Deprecated by allTouches and Touch class.
	Button  touch2Status;   //!< Deprecated by allTouches and Touch class.
	Button  touch3Status;   //!< Deprecated by allTouches and Touch class.
	Pointer touch1Location; //!< Deprecated by allTouches and Touch class.
	Pointer touch2Location; //!< Deprecated by allTouches and Touch class.
	Pointer touch3Location; //!< Deprecated by allTouches and Touch class.
	
	Accelerometer accelerometer;
	
private:
	void updateOrAddTouches(const IPhoneController& p_otherController);
	void removeOldTouches();
	
	static bool ms_initialized;
	static IPhoneController ms_controller;
	static IPhoneController ms_temporary;
	static Touch            ms_touchDummy; // Used as failsafe for returning references to Touch.
};


// Namespace end
}
}


#endif  // defined(TT_PLATFORM_OSX_IPHONE)
	
#endif  // !defined(INC_TT_INPUT_IPHONECONTROLLER_H)
