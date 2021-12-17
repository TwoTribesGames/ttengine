#if !defined(INC_TT_INPUT_SDLMOUSECONTROLLER_H)
#define INC_TT_INPUT_SDLMOUSECONTROLLER_H

#include <tt/input/Button.h>
#include <tt/input/ControllerIndex.h>
#include <tt/input/KeyList.h>
#include <tt/input/Pointer.h>
#include <tt/math/Point2.h>
#include <tt/platform/tt_types.h>

struct SDL_Cursor;
union SDL_Event;

namespace tt {
namespace input {

class SDLMouseCursor;
typedef tt_ptr<SDLMouseCursor>::shared SDLMouseCursorPtr;

struct SDLMouseController;
class SDLMouseCursor
{
public:
	static SDLMouseCursorPtr create(const std::string& p_windowsCursorFilename, s32 p_cursorIndex = 0);
	static SDLMouseCursorPtr create(const std::string& p_imageFilename, const math::Point2& p_hotSpot);
	static SDLMouseCursorPtr create(unsigned int _id);
	~SDLMouseCursor();
	
private:
	explicit SDLMouseCursor(SDL_Cursor* p_cusror);

	// No copying
	SDLMouseCursor(const SDLMouseCursor&);
	SDLMouseCursor& operator=(const SDLMouseCursor&);
	
	SDL_Cursor* m_cursor;
	
	friend struct SDLMouseController;
};

/*! \brief TT input controller implementation for SDL mouse events. */
struct SDLMouseController
{
public:
	static bool isConnected(ControllerIndex p_index);
	
	/*! \brief Returns the state of the controller. */
	static const SDLMouseController& getState(ControllerIndex p_index);
	
	static void update();
	
	/*! \brief Indicates whether the controller has been initialized.
	    \return True when initialized, false if not. */
	static bool isInitialized();
	
	/*! \brief Initializes the controller.
	           Must be called before controller can be used.
	    \return True if initialization was successful, false if not. */
	static bool initialize();
	
	static void deinitialize();

	static void setDefaultCursor(const SDLMouseCursorPtr& p_cursor)
	{
		ms_defaultCursor = p_cursor;
		resetToDefaultCursor();
	}
	static void resetToDefaultCursor()
	{
		setCustomCursor(ms_defaultCursor);
	}
	static void setCustomCursor(const SDLMouseCursorPtr& p_cursor);
	static void setCursorVisible(bool p_visible);
	static inline bool isCursorVisible() { return ms_cursorVisible; }

	static inline const SDLMouseCursorPtr& getCustomCursor() { return ms_cursor; }

	static int processEvent(const SDL_Event& event);
	
	Button  left;
	Button  middle;
	Button  right;
	Pointer cursor;
	
	s32     wheelNotches;
	
private:
	// Should not instantiate directly
	SDLMouseController();
	
	
	static bool ms_initialized;
	static SDLMouseController ms_controller;
	static SDLMouseController ms_temporary;  // stores updated (from event) state until update()

	static SDLMouseCursorPtr ms_defaultCursor;
	static SDLMouseCursorPtr ms_cursor;
	static bool              ms_cursorVisible; // whether the cursor should be visible
};

// Namespace end
}
}

#endif  // !defined(INC_TT_INPUT_SDLMOUSECONTROLLER_H)
