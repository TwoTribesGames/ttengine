#define NOMINMAX
#include <windows.h>
#include <shellapi.h>

#include <tt/args/CmdLine.h>
#include <tt/platform/tt_error.h>
#include <tt/str/common.h>


namespace tt {
namespace args {

//--------------------------------------------------------------------------------------------------
// Public member functions

CmdLine CmdLine::getApplicationCmdLine(const std::string& p_delimiter)
{
	str::Strings args;
	{
		int     argc = 0;
		LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
		TT_ASSERTMSG(argv != 0, "Retrieving Windows command line arguments failed.");
		if (argv != 0)
		{
			args.reserve(static_cast<str::Strings::size_type>(argc));
			for (int i = 0; i < argc; ++i)
			{
				args.push_back(tt::str::utf16ToUtf8(argv[i]));
			}
			LocalFree(argv);
		}
	}
	
	return CmdLine(args, p_delimiter);
}

// Namespace end
}
}
