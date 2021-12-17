#include <SDL2/SDL_clipboard.h>

#include <tt/fs/utils/utils.h>
#include <tt/platform/tt_error.h>
#include <tt/system/utils.h>
#include <tt/str/manip.h>


namespace tt {
namespace system {


bool setSystemClipboardText(const str::Strings& p_lines)
{
	return SDL_SetClipboardText(str::implode(p_lines, "\n").c_str()) == 0;
}


bool getSystemClipboardText(str::Strings* p_lines_OUT)
{
	TT_NULL_ASSERT(p_lines_OUT);
	if (p_lines_OUT == 0)
	{
		return false;
	}

	if (SDL_HasClipboardText()) {
		char *text = SDL_GetClipboardText();
		if (text) {
			*p_lines_OUT = str::explode(text, "\n", true);
		}
	}
	return false;
}


// Namespace end
}
}
