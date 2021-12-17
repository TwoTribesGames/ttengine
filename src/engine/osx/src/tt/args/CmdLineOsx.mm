#import <Foundation/Foundation.h>

#include <tt/args/CmdLine.h>


namespace tt {
namespace args {

//--------------------------------------------------------------------------------------------------
// Public member functions

CmdLine CmdLine::getApplicationCmdLine(const std::string& p_delimiter)
{
	// Get command line parameters
	str::Strings argList;
	NSArray* args = [[NSProcessInfo processInfo] arguments];
	for (NSUInteger i = 0; i < [args count]; ++i)
	{
		argList.push_back(std::string([[args objectAtIndex:i] UTF8String]));
	}
	return CmdLine(argList, p_delimiter);
}

// Namespace end
}
}
