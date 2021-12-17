#include <tt/app/StartupState.h>
#include <tt/code/bufferutils.h>
#include <tt/math/hash/CRC32.h>
#include <tt/mem/util.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>


namespace tt {
namespace app {

enum
{
	SignatureSize = 4,
	
	// Size of the actual data in the state file (in bytes)
	FileBodySize = 4 + 4 + 4, // Client revision, lib revision, startup step
	
	// Size of the entire state file (with signature, CRC, etc, in bytes)
	//               Signature     + version + body CRC + body
	FileBufferSize = SignatureSize + 4       + 4        + FileBodySize
};
static const u8  g_signatureBuf[SignatureSize] = { 'S', 'S', 'T', 0 };
static const u32 g_fileVersion                 = 1;


//--------------------------------------------------------------------------------------------------
// Startup enum helper functions

const char* getStartupStepName(StartupStep p_step)
{
	switch (p_step)
	{
	case StartupStep_PreInit:      return "pre_init";
	case StartupStep_SystemInit:   return "system_init";
	case StartupStep_ClientInit:   return "client_init";
	case StartupStep_Running:      return "running";
	case StartupStep_InBackground: return "in_background";
	case StartupStep_Shutdown:     return "shutdown";
	default: /*TT_PANIC("Invalid startup step: %d", p_step);*/ return "";
	}
}


StartupStep getStartupStepFromName(const std::string& p_name)
{
	for (s32 i = 0; i < StartupStep_Count; ++i)
	{
		StartupStep step = static_cast<StartupStep>(i);
		if (p_name == getStartupStepName(step))
		{
			return step;
		}
	}
	
	TT_PANIC("Invalid startup step name: '%s'", p_name.c_str());
	return StartupStep_Invalid;
}


const char* getStartupTypeName(StartupType p_type)
{
	switch (p_type)
	{
	case StartupType_NewInstall: return "new_install";
	case StartupType_Upgrade:    return "upgrade";
	case StartupType_Normal:     return "normal";
	default: /*TT_PANIC("Invalid startup type: %d", p_type);*/ return "";
	}
}


StartupType getStartupTypeFromName(const std::string& p_name)
{
	for (s32 i = 0; i < StartupType_Count; ++i)
	{
		StartupType type = static_cast<StartupType>(i);
		if (p_name == getStartupTypeName(type))
		{
			return type;
		}
	}
	
	TT_PANIC("Invalid startup type name: '%s'", p_name.c_str());
	return StartupType_Invalid;
}


//--------------------------------------------------------------------------------------------------
// Public member functions

void StartupState::setStartupStep(StartupStep p_step)
{
	namespace buff = code::bufferutils;
	
	// Compose the payload/body of the state file
	u8     bodyBuf[FileBodySize] = { 0 };
	u8*    buffer         = bodyBuf;
	size_t remainingBytes = FileBodySize;
	
	buff::put(m_clientRevision,         buffer, remainingBytes);
	buff::put(m_libRevision,            buffer, remainingBytes);
	buff::put(static_cast<s32>(p_step), buffer, remainingBytes);
	
	TT_ASSERTMSG(remainingBytes == 0, "State body buffer was not filled completely.");
	
	// Calculate the CRC of this body data
	const u32 bodyCRC = math::hash::CRC32().calcCRC(bodyBuf, FileBodySize);
	
	// Prepare the entire save file in memory (data is small enough for this)
	u8 saveBuf[FileBufferSize] = { 0 };
	buffer         = saveBuf;
	remainingBytes = FileBufferSize;
	
	buff::put(g_signatureBuf, SignatureSize, buffer, remainingBytes); // File signature
	buff::put(g_fileVersion,                 buffer, remainingBytes); // File version
	buff::put(bodyCRC,                       buffer, remainingBytes); // CRC of the actual file (body) data
	buff::put(bodyBuf, FileBodySize,         buffer, remainingBytes); // Body data
	
	TT_ASSERTMSG(remainingBytes == 0, "State file buffer was not filled completely.");
	
	// And now dump everything to a file
	const bool saveOk = writeStateFileWithData(saveBuf, FileBufferSize);
	TT_ASSERTMSG(saveOk, "Writing startup state data to file failed.");
}


//--------------------------------------------------------------------------------------------------
// Protected member functions

StartupState::StartupState(s32 p_clientRevision, s32 p_libRevision)
:
m_clientRevision(p_clientRevision),
m_libRevision(p_libRevision),
m_startupStepFromLastStart(StartupStep_None),
m_currentStartupType(StartupType_Invalid)
{
}


StartupState::~StartupState()
{
}


void StartupState::init()
{
	loadExistingState();
	
	if (isValidStartupStep(m_startupStepFromLastStart))
	{
		TT_WARNING(didLastRunExitCleanly(),
		           "Application was not shut down cleanly the last time it started. "
		           "The application exited in step '%s' (%d).",
		           getStartupStepName(m_startupStepFromLastStart), m_startupStepFromLastStart);
	}
	else
	{
		TT_WARN("Application was not shut down cleanly the last time it started. "
		        "The application startup state data was corrupted.");
	}
	
	// Save the initial (pre-init) startup step
	setStartupStep(StartupStep_PreInit);
}


//--------------------------------------------------------------------------------------------------
// Private member functions

void StartupState::loadExistingState()
{
	// Load the entire state file
	u8 fileBuf[FileBufferSize] = { 0 };
	if (readStateFile(fileBuf, FileBufferSize) == false)
	{
		// Loading state data failed: assume this is a fresh install
		m_currentStartupType       = StartupType_NewInstall;
		m_startupStepFromLastStart = StartupStep_None;
		return;
	}
	
	namespace buff = code::bufferutils;
	
	const u8* buffer         = fileBuf;
	size_t    remainingBytes = FileBufferSize;
	
	// Verify the file signature
	u8 signature[SignatureSize] = { 0 };
	buff::get(signature, SignatureSize, buffer, remainingBytes);
	for (s32 i = 0; i < SignatureSize; ++i)
	{
		if (signature[i] != g_signatureBuf[i])
		{
			// Invalid data (but data was available: assume app was upgraded)
			m_currentStartupType       = StartupType_Upgrade;
			m_startupStepFromLastStart = StartupStep_Invalid;
			return;
		}
	}
	
	// Verify the file version
	const u32 fileVersion = buff::get<u32>(buffer, remainingBytes);
	if (fileVersion != g_fileVersion)
	{
		// Version was upgraded; application itself was upgraded as well
		m_currentStartupType       = StartupType_Upgrade;
		m_startupStepFromLastStart = StartupStep_Invalid;
		return;
	}
	
	// Load the body CRC and body data
	const u32 bodyCRCFromData = buff::get<u32>(buffer, remainingBytes);
	u8 bodyBuf[FileBodySize] = { 0 };
	buff::get(bodyBuf, FileBodySize, buffer, remainingBytes);
	
	TT_ASSERTMSG(remainingBytes == 0, "State file buffer was not read completely.");
	
	// Verify the body CRC
	const u32 bodyCRC = math::hash::CRC32().calcCRC(bodyBuf, FileBodySize);
	if (bodyCRCFromData != bodyCRC)
	{
		// Body CRC does not match actual data (data was corrupted/tampered with?)
		m_currentStartupType       = StartupType_Upgrade;
		m_startupStepFromLastStart = StartupStep_Invalid;
		return;
	}
	
	// Load the payload/body of the state file
	buffer         = bodyBuf;
	remainingBytes = FileBodySize;
	
	const s32 clientRevision = buff::get<s32>(buffer, remainingBytes);
	const s32 libRevision    = buff::get<s32>(buffer, remainingBytes);
	const s32 startupStep    = buff::get<s32>(buffer, remainingBytes);
	
	TT_ASSERTMSG(remainingBytes == 0, "State body buffer was not read completely.");
	
	if (clientRevision != m_clientRevision ||
	    libRevision != m_libRevision)
	{
		// Application version does not match: application was upgraded
		m_currentStartupType = StartupType_Upgrade;
	}
	else
	{
		// Application version remained the same: this is a normal startup
		m_currentStartupType = StartupType_Normal;
	}
	
	m_startupStepFromLastStart = static_cast<StartupStep>(startupStep);
	TT_ASSERTMSG(isValidStartupStep(m_startupStepFromLastStart),
	             "Startup step retrieved from existing startup state (%d) is invalid.",
	             startupStep);
	
	
	TT_Printf("StartupState::loadExistingState: Startup type: '%s' (%d) -- "
	          "previous version: %d.%d, current version: %d.%d\n",
	          getStartupTypeName(m_currentStartupType), m_currentStartupType,
	          clientRevision, libRevision, m_clientRevision, m_libRevision);
	TT_Printf("StartupState::loadExistingState: Previous startup reached step '%s' (%d)\n",
	          getStartupStepName(m_startupStepFromLastStart), m_startupStepFromLastStart);
}

// Namespace end
}
}
