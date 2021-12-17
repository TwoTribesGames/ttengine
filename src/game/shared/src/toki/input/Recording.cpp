#include <tt/app/Application.h>
#include <tt/args/CmdLine.h>
#include <tt/code/AutoGrowBuffer.h>
#include <tt/code/FourCC.h>
#include <tt/code/bufferutils.h>
#include <tt/compression/lzma.h>
#include <tt/fs/utils/utils.h>
#include <tt/fs/File.h>
#include <tt/platform/tt_error.h>
#include <tt/steam/helpers.h>
#include <tt/system/Time.h>
#include <tt/system/Calendar.h>
#include <tt/version/Version.h>

#include <toki/AppGlobal.h>
#include <toki/game/CheckPointMgr.h>
#include <toki/game/Game.h>
#include <toki/input/Recording.h>
#include <toki/level/LevelData.h>
#include <toki/savedata/utils.h>
#include <toki/serialization/SerializationMgr.h>


namespace toki {
namespace input {


void InputState::serialize  (tt::code::BufferWriteContext* p_context) const
{
	namespace bu = tt::code::bufferutils;
	TT_NULL_ASSERT(p_context);
	
	bu::putEnum<u8>(controllerType, p_context);
	pointer.             serialize(p_context);
	accept.              serialize(p_context);
	cancel.              serialize(p_context);
	faceUp.              serialize(p_context);
	faceLeft.            serialize(p_context);
	left.                serialize(p_context);
	right.               serialize(p_context);
	up.                  serialize(p_context);
	down.                serialize(p_context);
	virusUpload.         serialize(p_context);
	jump.                serialize(p_context);
	primaryFire.         serialize(p_context);
	secondaryFire.       serialize(p_context);
	selectWeapon1.       serialize(p_context);
	selectWeapon2.       serialize(p_context);
	selectWeapon3.       serialize(p_context);
	selectWeapon4.       serialize(p_context);
	toggleWeapons.       serialize(p_context);
	demoReset.           serialize(p_context);
	respawn.             serialize(p_context);
	menu.                serialize(p_context);
	screenSwitch.        serialize(p_context);
	startupFailSafeLevel.serialize(p_context);
	
	bu::put(direction.x, p_context);
	bu::put(direction.y, p_context);
	bu::put(gyroStick.x, p_context);
	bu::put(gyroStick.y, p_context);
	bu::put(scroll.x,    p_context);
	bu::put(scroll.y,    p_context);
	
	debugCheat.  serialize(p_context);
	debugRestart.serialize(p_context);
}

void InputState::unserialize(tt::code::BufferReadContext*  p_context)
{
	namespace bu = tt::code::bufferutils;
	TT_NULL_ASSERT(p_context);
	
	controllerType = bu::getEnum<u8, tt::input::ControllerType>(p_context);
	pointer.             unserialize(p_context);
	accept.              unserialize(p_context);
	cancel.              unserialize(p_context);
	faceUp.              unserialize(p_context);
	faceLeft.            unserialize(p_context);
	left.                unserialize(p_context);
	right.               unserialize(p_context);
	up.                  unserialize(p_context);
	down.                unserialize(p_context);
	virusUpload.         unserialize(p_context);
	jump.                unserialize(p_context);
	primaryFire.         unserialize(p_context);
	secondaryFire.       unserialize(p_context);
	selectWeapon1.       unserialize(p_context);
	selectWeapon2.       unserialize(p_context);
	selectWeapon3.       unserialize(p_context);
	selectWeapon4.       unserialize(p_context);
	toggleWeapons.       unserialize(p_context);
	demoReset.           unserialize(p_context);
	respawn.             unserialize(p_context);
	menu.                unserialize(p_context);
	screenSwitch.        unserialize(p_context);
	startupFailSafeLevel.unserialize(p_context);
	
	direction.x = bu::get<real>(p_context);
	direction.y = bu::get<real>(p_context);
	gyroStick.x = bu::get<real>(p_context);
	gyroStick.y = bu::get<real>(p_context);
	scroll.x    = bu::get<real>(p_context);
	scroll.y    = bu::get<real>(p_context);
	
	debugCheat.  unserialize(p_context);
	debugRestart.unserialize(p_context);
}


#if !defined(TT_BUILD_FINAL)
static const char* getBuildName(BuildConfiguration p_config)
{
	switch(p_config)
	{
	case Build_Dev  : return "Dev";
	case Build_Test : return "Test";
	case Build_Final: return "Final";
	default:
		TT_PANIC("Unknown build configuration.");
		return "Invalid";
	}
}
#endif


// NOTE: For the reason for these specific signature bytes, see
// http://en.wikipedia.org/wiki/Portable_Network_Graphics#File_header
// (these bytes are inspired by the PNG file header bytes,
//  for the same reasons as listed there: mainly to detect file transmission problems)
const size_t g_recordingFormatSignatureLength = 9;
const u8     g_recordingFormatSignature[g_recordingFormatSignatureLength] =
{
	0x89,        // Has the high bit set to detect transmission systems that do not support 8 bit data
	'I', 'R', 'E', 'C', // The actual signature bytes for a Toki Tori 2 input recording
	0x0D, 0x0A,  // A DOS-style line ending (CRLF) to detect DOS-Unix line ending conversion of the data.
	0x1A,        // A byte that stops display of the file under DOS when the command type has been used—the end-of-file character
	0x0A         // A Unix-style line ending (LF) to detect Unix-DOS line ending conversion.
};
const u32    g_recordingFormatCurrentVersion = 10;

static const u32 g_sectionMarker   = tt::code::FourCC<'I', 'S', 'E', 'C'>::value;
static const u32 g_frameMarker     = tt::code::FourCC<'I', 'F', 'R', 'M'>::value;
static const u32 g_gameStateMarker = tt::code::FourCC<'I', 'G', 'S', 'T'>::value;


static u32 g_bufferIncrementSize = 32;


//--------------------------------------------------------------------------------------------------
// FileHeader member functions

bool FileHeader::save(tt::fs::FilePtr& p_file) const
{
	const char* filename = "";
	(void)filename; // CAT hack
#if !defined(TT_BUILD_FINAL)
	filename = p_file->getPath();
#endif
	
	// Write the file signature and version
	if (p_file->write(g_recordingFormatSignature, static_cast<tt::fs::size_type>(g_recordingFormatSignatureLength)) !=
	    static_cast<tt::fs::size_type>(g_recordingFormatSignatureLength))
	{
		TT_PANIC("Writing file format signature to file '%s' failed.", filename);
		return false;
	}
	if (tt::fs::writeInteger(p_file, g_recordingFormatCurrentVersion) == false)
	{
		TT_PANIC("Writing file format version to file '%s' failed.", filename);
		return false;
	}
	
	// Write compression info
	if (tt::fs::writeBool(p_file, compressed) == false)
	{
		return false;
	}
	if (tt::fs::writeInteger(p_file, compressedSize) == false)
	{
		return false;
	}
	if (tt::fs::writeInteger(p_file, uncompressedSize) == false)
	{
		return false;
	}
	return true;
}


bool FileHeader::load(const tt::fs::FilePtr& p_file)
{
	const char* filename = "";
	(void)filename; // CAT hack
#if !defined(TT_BUILD_FINAL)
	filename = p_file->getPath();
#endif
	
	// Signature
	u8 signature[g_recordingFormatSignatureLength] = { 0 };
	const tt::fs::size_type fslen = static_cast<tt::fs::size_type>(g_recordingFormatSignatureLength);
	if (p_file->read(signature, fslen) != fslen ||
	    std::memcmp(signature, g_recordingFormatSignature, g_recordingFormatSignatureLength) != 0)
	{
		TT_PANIC("Input recording data does not appear to be valid. File has invalid signature.\nFile: '%s'",
		         filename);
		return false;
	}
	
	// Version
	u32 dataVersion = 0;
	if (tt::fs::readInteger(p_file, &dataVersion) == false || dataVersion != g_recordingFormatCurrentVersion)
	{
		TT_PANIC("Input recording data format is not the expected version. "
		         "Loaded version %u, expected version %u.\nFile: '%s'",
		         dataVersion, g_recordingFormatCurrentVersion, filename);
		return false;
	}
	
	// Read compression info
	if (tt::fs::readBool(p_file, &compressed) == false)
	{
		return false;
	}
	if (tt::fs::readInteger(p_file, &compressedSize) == false)
	{
		return false;
	}
	if (tt::fs::readInteger(p_file, &uncompressedSize) == false)
	{
		return false;
	}
	return true;
}


//--------------------------------------------------------------------------------------------------
// RecordingHeader member functions

RecordingHeader::RecordingHeader()
:
host(tt::app::getPlatform()),
#if defined(TT_BUILD_DEV)
config(Build_Dev),
#elif defined(TT_BUILD_TEST)
config(Build_Test),
#elif defined(TT_BUILD_FINAL)
config(Build_Final),
#else
#error Unsupported build configuration
#endif
revision(tt::version::getClientRevisionNumber()),
gameStateFrequency(60),
startTime(0),
stopTime(0),
targetFPS(tt::app::getApplication()->getTargetFPS()),
startState(serialization::SerializationMgr::createEmpty())
{
}


void RecordingHeader::validate() const
{
	const s32 currentRevision(tt::version::getClientRevisionNumber());
	
	TT_ASSERTMSG(revision == currentRevision,
		"Input Recording was recorded with build revision %d, the current build has revision %d."
		" Recorded input may not play back correctly. ", revision, currentRevision);
	
#if defined(TT_BUILD_DEV)
	TT_ASSERTMSG(config == Build_Dev,
		"The current input recording was recorded with a %s build. Results may not be correct.",
		getBuildName(config));
#endif
	
#if defined(TT_BUILD_TEST)
	TT_ASSERTMSG(config == Build_Test || config == Build_Final,
		"The current input recording was recorded with a %s build. Results may not be correct.",
		getBuildName(config));
#endif
	
	TT_ASSERTMSG(host == tt::app::getPlatform(),
		"Input was recorded on a different host platform. Results may not be correct.");
}


bool RecordingHeader::save(tt::fs::FilePtr& p_file) const
{
	// Write information about the current build
	if (tt::fs::writeEnum<u8, tt::app::Platform>(p_file, host) == false)
	{
		return false;
	}
		
	if (tt::fs::writeEnum<u8, BuildConfiguration>(p_file, config) == false)
	{
		return false;
	}
	
	if(tt::fs::writeInteger(p_file, revision) == false)
	{
		return false;
	}
	
	// Write information about the frame data
	u32 frameSize(sizeof(Frame));
	if(tt::fs::writeInteger(p_file, frameSize) == false)
	{
		return false;
	}
	u32 gameStateSize(sizeof(GameState));
	if(tt::fs::writeInteger(p_file, gameStateSize) == false)
	{
		return false;
	}
	if(tt::fs::writeInteger(p_file, gameStateFrequency) == false)
	{
		return false;
	}
	if(tt::fs::writeInteger(p_file, startTime) == false)
	{
		return false;
	}
	if(tt::fs::writeInteger(p_file, targetFPS) == false)
	{
		return false;
	}
	
	// Starting point of recording
	
	u32 checkPointSize(0);
	TT_NULL_ASSERT(startState);
	if(startState != 0)
	{
		checkPointSize = startState->getTotalSize();
	}
	
	if(tt::fs::writeInteger(p_file, checkPointSize) == false)
	{
		return false;
	}
	
	if(checkPointSize > 0)
	{
		if(startState->saveToFile(p_file) == false)
		{
			return false;
		}
	}
	
	return true;
}


bool RecordingHeader::load(const tt::fs::FilePtr& p_file)
{
	// Read information about the current build
	if (tt::fs::readEnum<u8, tt::app::Platform>(p_file, &host) == false)
	{
		return false;
	}
	if (tt::fs::readEnum<u8, BuildConfiguration>(p_file, &config) == false)
	{
		return false;
	}
	if(tt::fs::readInteger(p_file, &revision) == false)
	{
		return false;
	}
	
	// Make sure configuration is the same
	validate();
	
	// Read information about the frame data
	u32 frameSize(0);
	if(tt::fs::readInteger(p_file, &frameSize) == false)
	{
		return false;
	}
	TT_ASSERT(frameSize == sizeof(Frame));
	
	u32 gameStateSize(0);
	if(tt::fs::readInteger(p_file, &gameStateSize) == false)
	{
		return false;
	}
	TT_ASSERT(gameStateSize == sizeof(GameState));
	
	if(tt::fs::readInteger(p_file, &gameStateFrequency) == false)
	{
		return false;
	}
	if(tt::fs::readInteger(p_file, &startTime) == false)
	{
		return false;
	}
	if(tt::fs::readInteger(p_file, &targetFPS) == false)
	{
		return false;
	}
	
	// Starting point of recording
	u32 checkPointSize(0);
	if(tt::fs::readInteger(p_file, &checkPointSize) == false)
	{
		return false;
	}
	
	if(checkPointSize > 0)
	{
		if(startState->loadFromFile(p_file) == false)
		{
			return false;
		}
	}
	
	return true;
}


//--------------------------------------------------------------------------------------------------
// Recording public member functions

Recording::~Recording()
{
	if (m_file != 0)
	{
		m_file->flush();
		m_file.reset();
	}
}


RecordingPtr Recording::create()
{
	// Create header
	RecordingHeader recordingHeader;
	
	// Create a checkpoint
	AppGlobal::getGame()->serializeAll(*recordingHeader.startState);
	
	// Restore immediately to get exact same situation as when playing
	AppGlobal::getGame()->unserializeAll(*recordingHeader.startState, "recorder");
	
	// Store the start time of the recording
	recordingHeader.startTime = tt::system::Time::getInstance()->getMilliSeconds();
	
	// Create output file
	tt::system::Calendar now = tt::system::Calendar::getCurrentDate();
	std::stringstream s;
#if defined(TT_STEAM_BUILD)
	s << tt::steam::getSanitizedUsername() << '_';
#endif
	s << std::setw(2) << std::setfill('0') << now.getYear();
	s << std::setw(2) << std::setfill('0') << now.getMonth();
	s << std::setw(2) << std::setfill('0') << now.getDay();
	s << '_';
	s << std::setw(2) << std::setfill('0') << now.getHour();
	s << std::setw(2) << std::setfill('0') << now.getMinute();
	s << std::setw(2) << std::setfill('0') << now.getSecond();
	s << '_';
	s << AppGlobal::getGame()->getStartInfo().getLevelName();
	
	const std::string relativeFilepath = "input/" + s.str() + ".ttrec";
	tt::fs::FilePtr file = savedata::createSaveFile(relativeFilepath, false);
	if (file == 0)
	{
		return RecordingPtr();
	}
	
	const std::string fullFilePath = tt::fs::utils::makeCorrectFSPath(
		savedata::makeSaveFilePath(relativeFilepath, false));
	
	// Add compression info; not compressed
	FileHeader fileHeader;
	if (fileHeader.save(file) == false)
	{
		TT_PANIC("Failed to save file header information to file '%s'", fullFilePath.c_str());
		return RecordingPtr();
	}
	
	if(recordingHeader.save(file) == false)
	{
		TT_PANIC("Failed to save recording header information to file '%s'", fullFilePath.c_str());
		return RecordingPtr();
	}
	
	// All good; create recording
	return RecordingPtr(new Recording(recordingHeader, file, fullFilePath));
}


RecordingPtr Recording::load(const std::string& p_filename)
{
	tt::fs::FilePtr file(decompress(p_filename));
	if (file == 0)
	{
		return RecordingPtr();
	}
	
	// Load recording header
	RecordingHeader recordingHeader;
	
	if(recordingHeader.load(file) == false)
	{
		return RecordingPtr();
	}
	
	// Load sections
	RecordingSections sections;
	
	bool readOk(true);
	while(readOk)
	{
		// Read frame or section
		readOk = read(file, sections);
	}
	// Close file
	file.reset();
	
	// FIXME: loading (and saving) should work with buffers to prevent this magic
	const std::string tmpFilename(p_filename + ".~tmp");
	if (tt::fs::fileExists(tmpFilename))
	{
		// Get rid of temporary file containing decompressed data
		tt::fs::destroyFile(tmpFilename);
	}
	if (sections.empty())
	{
		return RecordingPtr();
	}
	
	// Find the first section with frames.
	recordingHeader.startTime = 0;
	for (RecordingSections::const_iterator it = sections.begin(); it != sections.end(); ++it)
	{
		const RecordingSection& section = (*it);
		const Frames& frames = section.frames;
		if (frames.empty() == false)
		{
			recordingHeader.startTime = frames.front().time;
			break;
		}
	}
	
	// Find the last seconds with frames.
	recordingHeader.stopTime = 0;
	const RecordingSections& constSections = sections; // Cat compile doesn't return const it from rend().
	for (RecordingSections::const_reverse_iterator it = constSections.rbegin(); it != constSections.rend(); ++it)
	{
		const RecordingSection& section = (*it);
		const Frames& frames = section.frames;
		if (frames.empty() == false)
		{
			recordingHeader.stopTime = frames.back().time;
			break;
		}
	}
	
	// Strip path
	return RecordingPtr(new Recording(recordingHeader, sections,
		tt::fs::utils::makeCorrectFSPath(p_filename)));
}


void Recording::stop()
{
	TT_ASSERT(m_mode == Mode_Created);
	
	m_header.stopTime = m_sections.back().frames.back().time;
	m_file->flush();
	m_file.reset();
	
	TT_Printf("Recording file saved...\n");
	
	compress();
}


bool Recording::addSection(const RecordingSection& p_newSection)
{
	TT_ASSERT(m_mode == Mode_Created);
	
	m_sections.push_back(p_newSection);
	
	// Write to disk
	if(tt::fs::writeInteger(m_file, g_sectionMarker) == false)
	{
		return false;
	}
	
	if(tt::fs::writeNarrowString(m_file, p_newSection.levelName) == false)
	{
		return false;
	}
	
	if(tt::fs::writeInteger(m_file, p_newSection.randomSeed) == false)
	{
		return false;
	}
	
	for (s32 progType = 0; progType < ProgressType_Count; ++progType)
	{
		const ProgressType type = static_cast<ProgressType>(progType);
		const ProgressSection& progressSection = p_newSection.progress[type];
		
		if(progressSection.registry.saveFull(m_file) == false)
		{
			return false;
		}
		
		TT_NULL_ASSERT(progressSection.checkPoints);
		if(progressSection.checkPoints->save(m_file) == false)
		{
			return false;
		}
	}
	
	// Always flush at new section
	m_file->flush();
	
	return true;
}


bool Recording::addFrame(const Frame& p_newFrame)
{
	TT_ASSERT(m_mode == Mode_Created);
	
	// There should be already a section set at this point
	TT_ASSERT(m_sections.empty() == false);
	
	RecordingSection& currentSection = m_sections.back();
	currentSection.frames.push_back(p_newFrame);
	
	// Write to disk
	if(tt::fs::writeInteger(m_file, g_frameMarker) == false)
	{
		return false;
	}
	
	tt::code::AutoGrowBufferPtr writeBuffer =
		tt::code::AutoGrowBuffer::create(g_bufferIncrementSize, g_bufferIncrementSize);
	
	tt::code::BufferWriteContext context(writeBuffer->getAppendContext());
	
	namespace bu = tt::code::bufferutils;
	
	// Write time information
	bu::put(p_newFrame.time,      &context);
	bu::put(p_newFrame.deltaTime, &context);
	
	// Write button states
	p_newFrame.state.serialize(&context);
	
	context.flush();
	
	// Write frame to file
	
	bool saveOk = true;
	
	// Save the data's total (used/written) size
	saveOk = saveOk && tt::fs::writeInteger(m_file, static_cast<u32>(writeBuffer->getUsedSize()));
	
	// Save all data blocks
	const s32 blockCount = writeBuffer->getBlockCount();
	for (s32 i = 0; i < blockCount; ++i)
	{
		const tt::fs::size_type blockSize = static_cast<tt::fs::size_type>(writeBuffer->getBlockSize(i));
		saveOk = saveOk && (m_file->write(writeBuffer->getBlock(i), blockSize) == blockSize);
	}
	
	if (m_alwaysFlush)
	{
		m_file->flush();
	}
	
	return saveOk;
}


bool Recording::addGameState(const GameState& p_newGameState)
{
	TT_ASSERT(m_mode == Mode_Created);

	// There should be already a section set at this point
	TT_ASSERT(m_sections.empty() == false);
	
	RecordingSection& currentSection = m_sections.back();
	currentSection.gameStates.push_back(p_newGameState);
	
	// Write to disk
	if(tt::fs::writeInteger(m_file, g_gameStateMarker) == false)
	{
		return false;
	}
	
	if(tt::fs::writeInteger(m_file, p_newGameState.activeEntities) == false)
	{
		return false;
	}
	
	if(tt::fs::writeReal(m_file, p_newGameState.currentPosition.x) == false)
	{
		return false;
	}
	
	if(tt::fs::writeReal(m_file, p_newGameState.currentPosition.y) == false)
	{
		return false;
	}
	
	if(tt::fs::writeInteger(m_file, p_newGameState.randomSeedValue) == false)
	{
		return false;
	}
	
	if (m_alwaysFlush)
	{
		m_file->flush();
	}
	
	return true;
}


bool Recording::isGameStateFrame(u32 p_frameIndex) const
{
	return m_header.gameStateFrequency > 0 && ((p_frameIndex - 1) % m_header.gameStateFrequency) == 0;
}


const RecordingSection& Recording::getSection(u32 p_index) const
{
	if (m_sections.empty() || p_index >= m_sections.size())
	{
		TT_PANIC("Recording::getSection: cannot get section %d. Index out of bounds. Total sections %d.",
			p_index, m_sections.size());
		static RecordingSection emptySection;
		return emptySection;
	}
	
	return m_sections[p_index];
}


//--------------------------------------------------------------------------------------------------
// Private member functions

Recording::Recording(const RecordingHeader& p_header, const tt::fs::FilePtr& p_file,
                     const std::string& p_filePath)
:
m_header(p_header),
m_sections(),
m_file(p_file),
m_filePath(p_filePath),
m_mode(Mode_Created),
m_alwaysFlush(tt::app::getCmdLine().exists("flush_recording"))
{ 
	TT_NULL_ASSERT(m_file);
}


Recording::Recording(const RecordingHeader& p_header, const RecordingSections& p_sections,
                     const std::string& p_filePath)
:
m_header(p_header),
m_sections(p_sections),
m_file(),
m_filePath(p_filePath),
m_mode(Mode_Created),
m_alwaysFlush(tt::app::getCmdLine().exists("flush_recording"))
{
}


bool Recording::read(const tt::fs::FilePtr& p_file, RecordingSections& p_sections_OUT)
{
	u32 marker(0);
	if(tt::fs::readInteger(p_file, &marker) == false)
	{
		return false;
	}
	
	if(marker == g_sectionMarker)
	{
		return readSection(p_file, p_sections_OUT);
	}
	else if(marker == g_frameMarker)
	{
		return readFrame(p_file, p_sections_OUT);
	}
	else if(marker == g_gameStateMarker)
	{
		return readGameState(p_file, p_sections_OUT);
	}
	
	return false;
}


bool Recording::readSection(const tt::fs::FilePtr& p_file, RecordingSections& p_sections_OUT)
{
	RecordingSection section;
	if(tt::fs::readNarrowString(p_file, &section.levelName) == false)
	{
		return false;
	}
	
	if(tt::fs::readInteger(p_file, &section.randomSeed) == false)
	{
		return false;
	}

	for (s32 progType = 0; progType < ProgressType_Count; ++progType)
	{
		const ProgressType type = static_cast<ProgressType>(progType);
		ProgressSection& progressSection = section.progress[type];
		
		if(progressSection.registry.loadFull(p_file) == false)
		{
			return false;
		}
		
		progressSection.checkPoints = game::CheckPointMgr::create(type);
		if(progressSection.checkPoints->load(p_file) == false)
		{
			return false;
		}
	}
	
	p_sections_OUT.push_back(section);
	
	return true;
}


bool Recording::readFrame(const tt::fs::FilePtr& p_file, RecordingSections& p_sections_OUT)
{
	TT_ASSERT(p_sections_OUT.empty() == false);

	u32 dataSize = 0;
	if (tt::fs::readInteger(p_file, &dataSize) == false)
	{
		return false;
	}
	
	if (dataSize == 0)
	{
		return false;
	}
	
	u8* data = new u8[dataSize];
	
	const tt::fs::size_type readSize = static_cast<tt::fs::size_type>(dataSize);
	if (p_file->read(data, readSize) != readSize)
	{
		TT_PANIC("Available data is not the expected amount (%d bytes)", readSize);
		delete[] data;
		return false;
	}
	
	tt::code::AutoGrowBufferPtr buffer = tt::code::AutoGrowBuffer::createPrePopulated(
			data,
			static_cast<tt::code::Buffer::size_type>(dataSize),
			g_bufferIncrementSize);
	delete[] data;
	data = 0;
	
	if (buffer == 0)
	{
		TT_PANIC("Could not create buffer (of %u bytes) from input data.", dataSize);
		return false;
	}
	
	// Read data from buffer
	tt::code::BufferReadContext context = buffer->getReadContext();
	
	namespace bu = tt::code::bufferutils;
	
	Frame frame;
	
	frame.time      = bu::get<u64>( &context);
	frame.deltaTime = bu::get<real>(&context);
	frame.state.unserialize(        &context);
	
	RecordingSection& currentSection = p_sections_OUT.back();
	
	currentSection.frames.push_back(frame);
	
	return true;
}


bool Recording::readGameState(const tt::fs::FilePtr& p_file, RecordingSections& p_sections_OUT)
{
	GameState state;
	
	if(tt::fs::readInteger(p_file, &state.activeEntities) == false)
	{
		return false;
	}
	
	if(tt::fs::readReal(p_file, &state.currentPosition.x) == false)
	{
		return false;
	}
	
	if(tt::fs::readReal(p_file, &state.currentPosition.y) == false)
	{
		return false;
	}
	
	if(tt::fs::readInteger(p_file, &state.randomSeedValue) == false)
	{
		return false;
	}
	
	p_sections_OUT.back().gameStates.push_back(state);
	
	return true;
}


void Recording::compress() const
{
	TT_ASSERT(m_file == 0);
	TT_ASSERT(m_mode == Mode_Created);
	
	const std::string tempFilePath(m_filePath + ".~tmp");
	
	// Read file again
	tt::fs::FilePtr file = tt::fs::open(m_filePath, tt::fs::OpenMode_Read);
	if (file == 0)
	{
		TT_PANIC("Cannot open recording file '%s' for compression.", m_filePath.c_str());
		return;
	}
	tt::code::BufferPtr fileContent = file->getContent();
	file.reset();
	
	tt::code::BufferPtrForCreator compressionBuffer(new tt::code::Buffer(fileContent->getSize()));
	
	u32 compressedSize = 
		lzma_compress(fileContent->getData(), fileContent->getSize(), compressionBuffer->getData(), 9, 1024 * 1024);
	
	TT_ASSERT(compressedSize > 0);
	
	// Compress data
	file = tt::fs::open(tempFilePath, tt::fs::OpenMode_Write);
	if (file == 0)
	{
		TT_PANIC("Cannot open compressed output file '%s'", tempFilePath.c_str());
		return;
	}
	
	FileHeader fileHeader;
	fileHeader.compressed = true;
	fileHeader.compressedSize = compressedSize;
	fileHeader.uncompressedSize = fileContent->getSize();
	if (fileHeader.save(file) == false)
	{
		TT_PANIC("Failed to save file header information to compressed file '%s'", tempFilePath.c_str());
		return;
	}
	
	if (tt::fs::write(file, compressionBuffer->getData(), compressedSize) != (s32)compressedSize)
	{
		TT_PANIC("Failed to save compressed file '%s'", tempFilePath.c_str());
		return;
	}
	// Close the file
	file.reset();
	
	// All good now swap the original file with the compressed one
	tt::fs::moveFile(tempFilePath, m_filePath, false);
}


tt::fs::FilePtr Recording::decompress(const std::string& p_filename)
{
	tt::fs::FilePtr file = tt::fs::open(p_filename, tt::fs::OpenMode_Read);
	if (file == 0)
	{
		return tt::fs::FilePtr();
	}
	
	// Load file header containing CRCs and compression info
	FileHeader fileHeader;
	if (fileHeader.load(file) == false)
	{
		return tt::fs::FilePtr();
	}
	
	// If file is not compressed, early out!
	if (fileHeader.compressed == false)
	{
		return file;
	}
	
	tt::code::BufferPtrForCreator compressionBuffer(new tt::code::Buffer(fileHeader.compressedSize));
	tt::code::BufferPtrForCreator decompressionBuffer(new tt::code::Buffer(fileHeader.uncompressedSize));
	
	// Read compressed data
	if (tt::fs::read(file, compressionBuffer->getData(), fileHeader.compressedSize) !=
		static_cast<tt::fs::size_type>(fileHeader.compressedSize))
	{
		TT_PANIC("Failed to read compressed recording file");
		return tt::fs::FilePtr();
	}
	
	// Close file
	file.reset();
	
	const u32 size = lzma_decompress(compressionBuffer->getData(), fileHeader.compressedSize,
	                                 decompressionBuffer->getData(), fileHeader.uncompressedSize);
	
	TT_ASSERT(size == fileHeader.uncompressedSize);
	
	// Write to temp file
	const std::string& tempFilename(p_filename + ".~tmp");
	file = tt::fs::open(tempFilename, tt::fs::OpenMode_Write);
	if (tt::fs::write(file, decompressionBuffer->getData(), size) != static_cast<s32>(size))
	{
		TT_PANIC("Failed to create temporary file '%s'", tempFilename.c_str());
		return tt::fs::FilePtr();
	}
	file.reset();
	
	// Finally read the temp file again
	file = tt::fs::open(tempFilename, tt::fs::OpenMode_Read);
	if (file == 0)
	{
		return tt::fs::FilePtr();
	}
	
	// Load file header containing CRCs and compression info
	if (fileHeader.load(file) == false)
	{
		return tt::fs::FilePtr();
	}
	
	// At this point the file should not be compressed
	TT_ASSERT(fileHeader.compressed == false);
	return file;
}

// Namespace end
}
}
