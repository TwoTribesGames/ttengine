#import <Foundation/Foundation.h>
#include <cstdio>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <tt/app/StartupStateOsx.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
#include <tt/str/str.h>


namespace tt {
namespace app {

//--------------------------------------------------------------------------------------------------
// Public member functions

StartupStateOsx::StartupStateOsx(s32 p_clientRevision, s32 p_libRevision,
                                 const std::string& /*p_appName*/)
:
StartupState(p_clientRevision, p_libRevision),
m_saveFilename()
{
#if defined(TT_PLATFORM_OSX_IPHONE)
	m_saveFilename = [NSHomeDirectory() UTF8String];
	m_saveFilename += "/Documents/.startupstate";
#else
	
	// NOTE: This code generates a path that is compatible with Mac App Store guidelines
	// Example path that this code generates: /Users/name/Library/com.twotribes.appname/.startupstate
	
	// Get the user's Library path
	NSArray* paths = NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES);
	m_saveFilename = [[paths objectAtIndex:0] UTF8String];
	if (*m_saveFilename.rbegin() != '/') m_saveFilename += "/";
	
	// Append this application's bundle identifier (requires that this app is actually a bundle)
	m_saveFilename += [[[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleIdentifier"] UTF8String];
	m_saveFilename += "/";
	
	// Ensure this directory path exists (create if not)
	TT_Printf("StartupStateOsx::StartupStateOsx: Using state path '%s'.\n", m_saveFilename.c_str());
	{
		const std::string dirSep("/");
		
		const bool pathStartsWithDirSep = str::startsWith(m_saveFilename, dirSep);
		
		const str::Strings allDirectories(str::explode(m_saveFilename, dirSep));
		
		std::string normalizedDir;
		if (pathStartsWithDirSep) normalizedDir = dirSep;
		for (str::Strings::const_iterator it = allDirectories.begin(); it != allDirectories.end(); ++it)
		{
			normalizedDir += *it + dirSep;
			
			struct stat entryInfo = { 0 };
			if (stat(normalizedDir.c_str(), &entryInfo) != 0)
			{
				if (errno != ENOENT && errno != ENOTDIR)
				{
					TT_PANIC("Retrieving information about path '%s' failed with error %d: %s",
							 normalizedDir.c_str(), errno, strerror(errno));
					break;
				}
			}
			else
			{
				continue;
			}
			
			if ((entryInfo.st_mode & S_IFDIR) != S_IFDIR)
			{
				// Directory path does not exist; attempt to create it
				if (mkdir(normalizedDir.c_str(), S_IRWXU | S_IRWXG) != 0)
				{
					TT_PANIC("Creating directory '%s' failed with error %d: %s",
							 normalizedDir.c_str(), errno, strerror(errno));
					break;
				}
			}
		}
	}
	
	m_saveFilename += ".startupstate";
	
#endif
	
	TT_Printf("StartupStateOsx: Using the following state file: '%s'\n",
	          m_saveFilename.c_str());
	
	// Trigger base initialization
	init();
}


StartupStateOsx::~StartupStateOsx()
{
}


bool StartupStateOsx::writeStateFileWithData(const u8* p_fileData, s32 p_dataLen)
{
	FILE* file = fopen(m_saveFilename.c_str(), "wb");
	if (file == 0) return false;
	
	const bool writeOk =
		fwrite(p_fileData, 1, static_cast<size_t>(p_dataLen), file) == static_cast<size_t>(p_dataLen);
	
	fclose(file);
	return writeOk;
}


bool StartupStateOsx::readStateFile(u8* p_fileData, s32 p_expectedLen)
{
	FILE* file = fopen(m_saveFilename.c_str(), "rb");
	if (file == 0) return false;
	
	const bool readOk =
		fread(p_fileData, 1, static_cast<size_t>(p_expectedLen), file) == static_cast<size_t>(p_expectedLen);
	
	// FIXME: Also verify that no extra data remains in the file.
	
	fclose(file);
	return readOk;
}

// Namespace end
}
}
