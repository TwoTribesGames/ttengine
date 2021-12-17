/*
	GWEN
	Copyright (c) 2012 Facepunch Studios
	See license in Gwen.h

	SDL Driver by Edward Rudd
*/

#include "Gwen/Macros.h"
#include "Gwen/Platform.h"

#if defined(TT_PLATFORM_SDL)

#include <tt/input/MouseController.h>
#include <tt/system/utils.h>

std::vector<tt::input::SDLMouseCursorPtr> cursors;

#include <SDL2/SDL.h>

tt::input::SDLMouseCursorPtr getCursor(unsigned char cursor)
{
    if (cursors.empty()) {
        cursors.push_back(tt::input::SDLMouseCursor::create(SDL_SYSTEM_CURSOR_ARROW));
        cursors.push_back(tt::input::SDLMouseCursor::create(SDL_SYSTEM_CURSOR_IBEAM));
        cursors.push_back(tt::input::SDLMouseCursor::create(SDL_SYSTEM_CURSOR_SIZENS));
        cursors.push_back(tt::input::SDLMouseCursor::create(SDL_SYSTEM_CURSOR_SIZEWE));
        cursors.push_back(tt::input::SDLMouseCursor::create(SDL_SYSTEM_CURSOR_SIZENWSE));
        cursors.push_back(tt::input::SDLMouseCursor::create(SDL_SYSTEM_CURSOR_SIZENESW));
        cursors.push_back(tt::input::SDLMouseCursor::create(SDL_SYSTEM_CURSOR_SIZEALL));
        cursors.push_back(tt::input::SDLMouseCursor::create(SDL_SYSTEM_CURSOR_NO));
        cursors.push_back(tt::input::SDLMouseCursor::create(SDL_SYSTEM_CURSOR_WAIT));
        cursors.push_back(tt::input::SDLMouseCursor::create(SDL_SYSTEM_CURSOR_HAND));
    }

    return cursors[cursor];
}

void Gwen::Platform::Sleep( unsigned int iMS )
{
    SDL_Delay(iMS);
}

void Gwen::Platform::SetCursor( unsigned char p_cursor )
{
    if (p_cursor == 255)  // Two Tribes change: interpret 255 as "don't set a cursor"
    {
        return;
    } else {
        tt::input::SDLMouseController::setCustomCursor(getCursor(p_cursor));
        // Do nothing for now?
        return;
    }
}

Gwen::UnicodeString Gwen::Platform::GetClipboardText()
{
    if (SDL_HasClipboardText())
    {
        char *text = SDL_GetClipboardText();
        Gwen::UnicodeString ret = Gwen::Utility::StringToUnicode(text);
        SDL_free(text);
        return ret;
    } else {
        return L"";
    }
}

bool Gwen::Platform::SetClipboardText( const Gwen::UnicodeString & str )
{
    Gwen::String text = Gwen::Utility::UnicodeToString(str);
    SDL_SetClipboardText(text.c_str());
    return true;
}

float Gwen::Platform::GetTimeInSeconds()
{
    float fSeconds = SDL_GetTicks() / 1000.0f;
    return fSeconds;
}

bool Gwen::Platform::FileOpen( const String & /*Name*/, const String & /*StartPath*/, const String & /*Extension*/, Gwen::Event::Handler* /*pHandler*/, Event::Handler::FunctionWithInformation /*fnCallback*/ )
{
    // No platform independent way to do this.
    // Ideally you would open a system dialog here
    return false;
}

bool Gwen::Platform::FileSave( const String & /*Name*/, const String & /*StartPath*/, const String & /*Extension*/, Gwen::Event::Handler* /*pHandler*/, Gwen::Event::Handler::FunctionWithInformation /*fnCallback*/ )
{
    // No platform independent way to do this.
    // Ideally you would open a system dialog here
    return false;
}

bool Gwen::Platform::FolderOpen( const String & /*Name*/, const String & /*StartPath*/, Gwen::Event::Handler* /*pHandler*/, Event::Handler::FunctionWithInformation /*fnCallback*/ )
{
    return false;
}

void* Gwen::Platform::CreatePlatformWindow( int /*x*/, int /*y*/, int /*w*/, int /*h*/, const Gwen::String & /*strWindowTitle*/ )
{
    return NULL;
}

void Gwen::Platform::DestroyPlatformWindow( void* /*pPtr*/ )
{
}

void Gwen::Platform::MessagePump( void* /*pWindow*/, Gwen::Controls::Canvas* /*ptarget*/ )
{
}

void Gwen::Platform::SetBoundsPlatformWindow( void* /*pPtr*/, int /*x*/, int /*y*/, int /*w*/, int /*h*/ )
{
}

void Gwen::Platform::SetWindowMaximized( void* /*pPtr*/, bool /*bMax*/, Gwen::Point & /*pNewPos*/, Gwen::Point & /*pNewSize*/ )
{
}

void Gwen::Platform::SetWindowMinimized( void* /*pPtr*/, bool /*bMinimized*/ )
{
}

bool Gwen::Platform::HasFocusPlatformWindow( void* /*pPtr*/ )
{
    return true;
}

void Gwen::Platform::GetDesktopSize( int & w, int & h )
{
    SDL_DisplayMode mode;
    SDL_GetDesktopDisplayMode(0, &mode);

    w = mode.w;
    h = mode.h;
}

void Gwen::Platform::GetCursorPos( Gwen::Point & /*po*/ )
{
}

#endif