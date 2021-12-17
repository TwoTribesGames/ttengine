#include <cstdio>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <tt/app/StartupStateSDL2.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
#include <tt/str/str.h>

#include <tt/fs/fs.h>
#include <tt/fs/utils/utils.h>

#include <cstring>


namespace tt {
namespace app {

//--------------------------------------------------------------------------------------------------
// Public member functions

StartupStateSDL2::StartupStateSDL2(s32 p_clientRevision, s32 p_libRevision)
:
StartupState(p_clientRevision, p_libRevision),
m_saveFilename()
{
}

void StartupStateSDL2::initialize(const std::string& p_rootPath)
{
	m_saveFilename = p_rootPath;

	// Ensure this directory path exists (create if not)
	TT_Printf("StartupStateSDL2::StartupStateSDL2: Using state path '%s'.\n", m_saveFilename.c_str());
	fs::utils::createDirRecursive(m_saveFilename);
	
	m_saveFilename += ".startupstate";
	
	TT_Printf("StartupStateSDL2: Using the following state file: '%s'\n",
	          m_saveFilename.c_str());
	
	// Trigger base initialization
	init();
}


StartupStateSDL2::~StartupStateSDL2()
{
}


bool StartupStateSDL2::writeStateFileWithData(const u8* p_fileData, s32 p_dataLen)
{
	FILE* file = fopen(m_saveFilename.c_str(), "wb");
	if (file == 0) return false;
	
	const bool writeOk =
		fwrite(p_fileData, 1, static_cast<size_t>(p_dataLen), file) == static_cast<size_t>(p_dataLen);
	
	fclose(file);
	return writeOk;
}


bool StartupStateSDL2::readStateFile(u8* p_fileData, s32 p_expectedLen)
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
