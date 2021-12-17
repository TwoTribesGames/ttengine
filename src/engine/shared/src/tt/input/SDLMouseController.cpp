#include <tt/input/SDLMouseController.h>

#include <tt/input/ControllerType.h>
#include <tt/fileformat/cursor/CursorData.h>
#include <tt/fileformat/cursor/CursorDirectory.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/UpScaler.h>
#include <tt/mem/util.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>

#include <SDL2/SDL.h>
#ifdef USE_LODEPNG
#include <lodepng/SDL_LodePNG.h>
#endif

namespace tt {
namespace input {

bool               SDLMouseController::ms_initialized = false;
SDLMouseController SDLMouseController::ms_controller;
SDLMouseController SDLMouseController::ms_temporary;
SDLMouseCursorPtr  SDLMouseController::ms_defaultCursor;
SDLMouseCursorPtr  SDLMouseController::ms_cursor;
bool               SDLMouseController::ms_cursorVisible = true;


SDLMouseCursorPtr SDLMouseCursor::create(const std::string& p_windowsCursorFilename, s32 p_cursorIndex)
{
	// Load the Windows cursor data
	using namespace fileformat::cursor;
	CursorDirectoryPtr dir(CursorDirectory::load(p_windowsCursorFilename));
	if (dir == 0)
	{
		TT_PANIC("Loading Windows cursor file '%s' failed.", p_windowsCursorFilename.c_str());
		return SDLMouseCursorPtr();
	}

	if (p_cursorIndex < 0 || p_cursorIndex >= dir->getCursorCount())
	{
		TT_PANIC("Windows cursor file '%s' does not have cursor index %d. It has %d cursor(s).",
				 p_windowsCursorFilename.c_str(), p_cursorIndex, dir->getCursorCount());
		return SDLMouseCursorPtr();
	}

	const CursorData* cursorData = dir->getCursor(p_cursorIndex);
	TT_NULL_ASSERT(cursorData);
	if (cursorData == 0)
	{
		return SDLMouseCursorPtr();
	}

	const s32 cursorWidth  = cursorData->getWidth();
	const s32 cursorHeight = cursorData->getHeight();

	SDL_Surface *bitmap = SDL_CreateRGBSurface(0, cursorWidth, cursorHeight, 32,
			0x00ff0000,
			0x0000ff00,
			0x000000ff,
			0xff000000);
	if (!bitmap) {
		TT_PANIC("Could not create an SDL Surface for Windows cursor file '%s'.", p_windowsCursorFilename.c_str());
		return SDLMouseCursorPtr();
	}
	if (SDL_LockSurface(bitmap) != 0) {
		TT_PANIC("Failed to lock SDL Surface for Windows cursor file '%s'.", p_windowsCursorFilename.c_str());
		SDL_FreeSurface(bitmap);
		return SDLMouseCursorPtr();
	}
	const mem::size_type dataSize = cursorWidth * cursorHeight * 4;
	mem::copy8(bitmap->pixels, cursorData->getImageData(), dataSize);
	SDL_UnlockSurface(bitmap);

	SDL_Cursor *cursor = SDL_CreateColorCursor(bitmap, cursorData->getHotSpot().x, cursorData->getHotSpot().y);

	SDL_FreeSurface(bitmap);

	return SDLMouseCursorPtr(new SDLMouseCursor(cursor));
}

SDLMouseCursorPtr SDLMouseCursor::create(const std::string& p_imageFilename, const math::Point2& p_hotSpot)
{
#if USE_LODEPNG
	SDL_Surface* image = SDL_LodePNG(p_imageFilename.c_str());
	if (image)
	{
		SDL_Cursor *cursor = SDL_CreateColorCursor(image, p_hotSpot.x, p_hotSpot.y);
		SDL_FreeSurface(image);
		return SDLMouseCursorPtr(new SDLMouseCursor(cursor));
	}
#else
	(void)p_imageFilename;
	(void)p_hotSpot;
#endif
	return SDLMouseCursorPtr();
}

SDLMouseCursorPtr SDLMouseCursor::create(unsigned int _id)
{
	SDL_Cursor *cursor = SDL_CreateSystemCursor(SDL_SystemCursor(_id));
	if (cursor) {
		return SDLMouseCursorPtr(new SDLMouseCursor(cursor));
	}
	return SDLMouseCursorPtr();
}

SDLMouseCursor::SDLMouseCursor(SDL_Cursor* p_cursor)
:
m_cursor(p_cursor)
{
}


SDLMouseCursor::~SDLMouseCursor()
{
	if (m_cursor != 0)
	{
		// We don't free them as SDL free's them for us on video quit.. (incorrectly IMHO)
		if (SDL_WasInit(SDL_INIT_VIDEO) == SDL_INIT_VIDEO)
		{
			TT_Printf("Freeing Cursor %x\n", m_cursor);
			SDL_FreeCursor(m_cursor);
		}
		m_cursor = 0;
	}
}

//--------------------------------------------------------------------------------------------------
// Public member functions

bool SDLMouseController::isConnected(ControllerIndex p_index)
{
	return p_index == ControllerIndex_One && isInitialized();
}


const SDLMouseController& SDLMouseController::getState(ControllerIndex p_index)
{
	TT_ASSERTMSG(isInitialized(), "SDLMouseController has not been initialized yet.");
	TT_ASSERTMSG(p_index == ControllerIndex_One, "Invalid controller index: %d", p_index);
	return ms_controller;
}


void SDLMouseController::update()
{
	if (isInitialized() == false)
	{
		TT_PANIC("SDLMouseController has not been initialized yet.");
		return;
	}
	
	// Detect usage.
	if (distanceSquared(ms_controller.cursor, ms_temporary.cursor) > 16 ||
	    ms_temporary.left.down                      ||
	    ms_temporary.middle.down                    ||
	    ms_temporary.right.down                     ||
	    ms_temporary.wheelNotches != 0)
	{
		onControllerTypeUsed(input::ControllerType_Keyboard);
	}
	
	// Update controller state
	ms_controller.left.update  (ms_temporary.left.down);
	ms_controller.middle.update(ms_temporary.middle.down);
	ms_controller.right.update (ms_temporary.right.down);
	ms_controller.cursor = ms_temporary.cursor;
	
	ms_controller.wheelNotches = ms_temporary.wheelNotches;
	
	ms_temporary.wheelNotches = 0;
}


int SDLMouseController::processEvent(const SDL_Event& event)
{
	switch (event.type)
	{
	case SDL_MOUSEMOTION:
		{
			tt::engine::renderer::Renderer* renderer = tt::engine::renderer::Renderer::getInstance();
			ms_temporary.cursor.valid = true;

			// Handle up-scaling by Renderer (must down-scale cursor position)
			math::Vector2 scaling = renderer->getUpScaler()->getScaleFactor();
			const math::Vector2 offset  = renderer->getUpScaler()->getOffset();
			if (renderer->isIOS2xMode())
			{
				scaling *= 0.5f;
			}

			ms_temporary.cursor.x = static_cast<int>((event.motion.x - offset.x)* scaling.x);
			ms_temporary.cursor.y = static_cast<int>((event.motion.y - offset.y)* scaling.y);
		}
		break;
	case SDL_MOUSEBUTTONUP:
	case SDL_MOUSEBUTTONDOWN:
		{
			bool state = event.button.state == SDL_PRESSED;
			switch (event.button.button) {
				case 1: ms_temporary.left.update(state); break;
				case 2: ms_temporary.middle.update(state); break;
				case 3: ms_temporary.right.update(state); break;
				default:
					TT_Printf("Un-handled mouse button press: %d:%d", event.button.button, event.button.state);
			}
		}
		break;
	case SDL_MOUSEWHEEL:
		ms_temporary.wheelNotches += event.wheel.y;
		break;
	}
	return 0;
}


bool SDLMouseController::isInitialized()
{
	return ms_initialized;
}


bool SDLMouseController::initialize()
{
	if (isInitialized())
	{
		TT_PANIC("SDLMouseController is already initialized.");
		return false;
	}
	
	ms_initialized = true;

	setDefaultCursor(SDLMouseCursor::create(SDL_SYSTEM_CURSOR_ARROW));
	
	return true;
}


void SDLMouseController::deinitialize()
{
	TT_ASSERTMSG(isInitialized(), "SDLMouseController is not initialized.");
	
	ms_initialized = false;
}


void SDLMouseController::setCustomCursor(const SDLMouseCursorPtr& p_cursor)
{
	if (ms_initialized == false)
	{
		TT_PANIC("Cannot set custom cursor when controller is not initialized.");
		return;
	}
	
	ms_cursor = p_cursor;
	
	if (p_cursor != 0)
	{
		SDL_SetCursor(p_cursor->m_cursor);
	}
}


void SDLMouseController::setCursorVisible(bool p_visible)
{
	if (ms_cursorVisible != p_visible)
	{
		ms_cursorVisible = p_visible;
		
		SDL_ShowCursor(p_visible ? SDL_ENABLE : SDL_DISABLE);
	}
}


//--------------------------------------------------------------------------------------------------
// Private member functions


SDLMouseController::SDLMouseController()
:
left(),
middle(),
right(),
cursor(),
wheelNotches(0)
{
}


// Namespace end
}
}
