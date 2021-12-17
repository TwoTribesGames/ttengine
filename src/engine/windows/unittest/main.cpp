#define NOMINMAX
#include <windows.h>

#include <unittest/unittest.h>


// Note: Maybe this needs to be replace with a app initialization.
//       Might be needed to test some part of the codebase.

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	return runUnitTests();
}
