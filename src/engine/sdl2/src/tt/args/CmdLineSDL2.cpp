#include <tt/args/CmdLine.h>
#include <tt/platform/tt_error.h>
#include <tt/str/common.h>
#include <tt/args/CmdLineSDL2.h>

namespace tt {
namespace args {

// Global variabls that are set via "main"
static int gArgc = 0;
static const char * const *gArgv = NULL;

void setArgcArgv(int argc, const char* const* argv)
{
    gArgc = argc;
    gArgv = argv;
}

//--------------------------------------------------------------------------------------------------
// Public member functions

CmdLine CmdLine::getApplicationCmdLine(const std::string& p_delimiter)
{
	// Get command line parameters
	str::Strings args;

    TT_ASSERTMSG(gArgv != 0, "Unable to fetch command line.  Did you forget to call tt::args::setArgcArgv?");

    if (gArgv) {
        args.reserve(static_cast<str::Strings::size_type>(gArgc));
        for (int i = 0; i < gArgc; ++i) {
            args.push_back(gArgv[i]);
        }
    }
	return CmdLine(args, p_delimiter);
}

// Namespace end
}
}
