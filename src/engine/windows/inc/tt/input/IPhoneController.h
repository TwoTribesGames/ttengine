#if !defined(INC_TT_INPUT_IPHONECONTROLLER_H)
#define INC_TT_INPUT_IPHONECONTROLLER_H


#include <tt/platform/tt_types.h>
#include <tt/input/Button.h>
#include <tt/input/ControllerIndex.h>
#include <tt/input/Pointer.h>


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
	    \return True if initialization was succesful, false if not. */
	static bool initialize();
	
	static void deinitialize();
	
	Button  touch1Status;
	Button  touch2Status;
	Pointer touch1Location;
	Pointer touch2Location;
	
private:
	IPhoneController();

	static IPhoneController ms_controller;
};

// Namespace end
}
}
	
#endif  // !defined(INC_TT_INPUT_IPHONECONTROLLER_H)
