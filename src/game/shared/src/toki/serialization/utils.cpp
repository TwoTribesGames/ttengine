#include <tt/code/bufferutils.h>
#include <tt/engine/scene2d/shoebox/shoebox.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_types.h>
#include <tt/pres/PresentationObject.h>
#include <tt/pres/PresentationMgr.h>
#include <tt/pres/TriggerBase.h>
#include <tt/thread/thread.h>

#include <toki/audio/AudioPlayer.h>
#include <toki/game/CheckPointMgr.h>
#include <toki/game/Game.h>
#include <toki/game/movement/MovementSet.h>
#include <toki/game/script/Registry.h>
#include <toki/savedata/utils.h>
#include <toki/serialization/utils.h>
#include <toki/AppGlobal.h>
#include <toki/cfg.h>
#include <toki/constants.h>


namespace toki {
namespace serialization {

//--------------------------------------------------------------------------------------------------
// ShutdownState save version

const u32    g_shutdownStateFormatCurrentVersion = 145;

// NOTE: For the reason for these specific signature bytes, see
// http://en.wikipedia.org/wiki/Portable_Network_Graphics#File_header
// (these bytes are inspired by the PNG file header bytes,
//  for the same reasons as listed there: mainly to detect file transmission problems)
const size_t g_shutdownStateFormatSignatureLength = 10;
const u8     g_shutdownStateFormatSignature[g_shutdownStateFormatSignatureLength] =
{
	0x89,        // Has the high bit set to detect transmission systems that do not support 8 bit data
	'T', 'T', 'S', 'D', 'S', // The actual signature bytes for serialization data saved to file
	0x0D, 0x0A,  // A DOS-style line ending (CRLF) to detect DOS-Unix line ending conversion of the data.
	0x1A,        // A byte that stops display of the file under DOS when the command type has been used—the end-of-file character
	0x0A         // A Unix-style line ending (LF) to detect Unix-DOS line ending conversion.
};


//--------------------------------------------------------------------------------------------------
// PersistentData save version
// NOTE: Once released, this version number should never change!

const u32    g_persistentDataFormatCurrentVersion = 4; // !If changed make sure it's backwards compatible!

// NOTE: For the reason for these specific signature bytes, see
// http://en.wikipedia.org/wiki/Portable_Network_Graphics#File_header
// (these bytes are inspired by the PNG file header bytes,
//  for the same reasons as listed there: mainly to detect file transmission problems)
const size_t g_persistentDataFormatSignatureLength = 10;
const u8     g_persistentDataFormatSignature[g_persistentDataFormatSignatureLength] =
{
	0x89,        // Has the high bit set to detect transmission systems that do not support 8 bit data
	'T', 'T', 'D', 'A', 'T', // The actual signature bytes for serialization data saved to file
	0x0D, 0x0A,  // A DOS-style line ending (CRLF) to detect DOS-Unix line ending conversion of the data.
	0x1A,        // A byte that stops display of the file under DOS when the command type has been used—the end-of-file character
	0x0A         // A Unix-style line ending (LF) to detect Unix-DOS line ending conversion.
};

//--------------------------------------------------------------------------------------------------
// Bookkeeping variables for LOT compliant saving

const s32 g_maxTimeFrame = 5 * 60;   // 5 minutes
const s32 g_maxSaveSlots = 5;        // 5 saves per 5 minutes allowed as maximum

typedef std::vector<s32> TimeStampList;
static TimeStampList               g_storedTimeStamps;
static s32                         g_currentTimeStamp = 0;

// Thread bookkeeping variables
static tt::thread::handle          g_saveThreadHandle;

//--------------------------------------------------------------------------------------------------


class AutoMounter
{
public:
	inline AutoMounter()
	:
	m_needUnmount(false),
	m_isMounted  (savedata::isSaveVolumeMounted())
	{
		if (m_isMounted == false)
		{
			m_isMounted   = savedata::mountSaveVolumeForWriting();
			m_needUnmount = m_isMounted;
		}
	}
	
	inline ~AutoMounter()
	{
		if (m_needUnmount)
		{
			savedata::unmountSaveVolume();
		}
	}
	
	inline bool isMounted() const { return m_isMounted; }
	
private:
	bool m_needUnmount;
	bool m_isMounted;
};


bool isSavingAllowed();

bool isSavingAllowed()
{
	g_currentTimeStamp = static_cast<s32>(tt::system::Time::getInstance()->getSeconds());
	
	TimeStampList currentTimeStamps;
	
	for (TimeStampList::const_iterator it = g_storedTimeStamps.begin();
	     it != g_storedTimeStamps.end(); ++it)
	{
		if ((*it) > (g_currentTimeStamp - g_maxTimeFrame))
		{
			currentTimeStamps.push_back(*it);
		}
	}
	
	g_storedTimeStamps = currentTimeStamps;
	if (static_cast<s32>(g_storedTimeStamps.size()) >= g_maxSaveSlots)
	{
		return false;
	}
	
	return true;
}


static int savePersistentDataAndShutdownStateThread(void* /*p_arg*/)
{
	TT_Printf("*** Saving persistent data and shutdown state\n");
	
	bool saveOk = true;
	saveOk = saveOk && serialization::savePersistentData();
	saveOk = saveOk && serialization::saveShutdownState();
	if (saveOk)
	{
		g_storedTimeStamps.push_back(g_currentTimeStamp);
		return savedata::commitSaveData() ? 0 : -1;
	}
	return -1;
}


bool savePersistentDataAndShutdownState(bool p_forceAndWait)
{
	if (g_saveThreadHandle != 0)
	{
		tt::thread::wait(g_saveThreadHandle);
		g_saveThreadHandle.reset();
	}
	
	if (savedata::canSave() == false ||
	    AppGlobal::isInLevelEditorMode() ||  // do not overwrite real game progress when level editing
	    AppGlobal::shouldRestoreShutdownData() == false ||
	    (p_forceAndWait == false && isSavingAllowed() == false))
	{
		return false;
	}
	
	g_saveThreadHandle = tt::thread::create(
		savePersistentDataAndShutdownStateThread,
		0,
		false,
		0,
		tt::thread::priority_normal,
		tt::thread::Affinity_None,
		"Save Thread");
	
	if (p_forceAndWait)
	{
		// Wait for save persistent data thread to finish
		tt::thread::wait(g_saveThreadHandle);
		g_saveThreadHandle.reset();
	}
	
	return false;
}


bool savePersistentData()
{
	if (savedata::canSave() == false || AppGlobal::isInLevelEditorMode() ||
	    AppGlobal::shouldRestoreShutdownData() == false)
	{
		return false;
	}
	
	AutoMounter mounter;
	if (mounter.isMounted() == false)
	{
		return false;
	}
	
	// Save the data's total (used/written) size
	tt::fs::FilePtr file = savedata::createSaveFile(AppGlobal::getPersistentDataFilename(AppGlobal::isInDemoMode()));
	if (file == 0)
	{
		TT_PANIC("Couldn't create save file for persistent data");
		return false;
	}
	
	tt::code::AutoGrowBufferPtr buffer(tt::code::AutoGrowBuffer::create(1024, 512));
	tt::code::BufferWriteContext context(buffer->getAppendContext());
	
	namespace bu = tt::code::bufferutils;
	
	// Our PersistentData is the main progress.
	game::script::getRegistry(ProgressType_Main).serializePersistent(&context);
	
	context.flush();
	
	bool saveOk = true;
	
	// Save header data: file signature and version
	const tt::fs::size_type fslen = static_cast<tt::fs::size_type>(g_persistentDataFormatSignatureLength);
	saveOk = saveOk && (file->write(g_persistentDataFormatSignature, fslen) == fslen);
	saveOk = saveOk && tt::fs::writeInteger(file, g_persistentDataFormatCurrentVersion);
	
	// Save data size
	saveOk = saveOk && tt::fs::writeInteger(file, static_cast<u32>(buffer->getUsedSize()));
	
	// Save all data blocks
	const s32 blockCount = buffer->getBlockCount();
	for (s32 i = 0; saveOk && i < blockCount; ++i)
	{
		const tt::fs::size_type blockSize = static_cast<tt::fs::size_type>(buffer->getBlockSize(i));
		saveOk = saveOk && (file->write(buffer->getBlock(i), blockSize) == blockSize);
	}
	
	return saveOk;
}


PersistentDataLoadResult loadPersistentData(PersistentData* p_persistentData)
{
	game::script::getRegistry().clearPersistent();
	bool isImportingDemoFiles = false;
	
	TT_NULL_ASSERT(p_persistentData);
	if (p_persistentData == 0)
	{
		return PersistentDataLoadResult_Fail;
	}
	
	tt::fs::FilePtr file = savedata::openSaveFile(AppGlobal::getPersistentDataFilename(AppGlobal::isInDemoMode()));
	if (file == 0 && AppGlobal::isInDemoMode() == false)
	{
		TT_Printf("Could not find persistent file, trying demo persistent file.");
		file = savedata::openSaveFile(AppGlobal::getPersistentDataFilename(true));
		isImportingDemoFiles = true;
	}
	if (file == 0)
	{
		return PersistentDataLoadResult_Fail;
	}
	
	// Signature
	u8 signature[g_persistentDataFormatSignatureLength] = { 0 };
	const tt::fs::size_type fslen = static_cast<tt::fs::size_type>(g_persistentDataFormatSignatureLength);
	if (file->read(signature, fslen) != fslen ||
	    std::memcmp(signature, g_persistentDataFormatSignature, g_persistentDataFormatSignatureLength) != 0)
	{
		TT_NONFATAL_PANIC("Persistent data save file does not appear to be valid. File has invalid signature.");
		return PersistentDataLoadResult_Fail;
	}
	
	// Version
	u32 dataVersion = 0;
	if (tt::fs::readInteger(file, &dataVersion) == false ||
	    dataVersion != g_persistentDataFormatCurrentVersion)
	{
		TT_NONFATAL_PANIC("Persistent data save file format is not the expected version. "
		                  "Loaded version %u, expected version %u.",
		                  dataVersion, g_persistentDataFormatCurrentVersion);
		return PersistentDataLoadResult_Fail;
	}
	
	u32 dataSize = 0;
	if (tt::fs::readInteger(file, &dataSize) == false)
	{
		return PersistentDataLoadResult_Fail;
	}
	
	tt::code::BufferPtrForCreator buffer(new tt::code::Buffer(dataSize));
	if (tt::fs::read(file, buffer->getData(), dataSize) != static_cast<tt::fs::size_type>(dataSize))
	{
		return PersistentDataLoadResult_Fail;
	}
	
	tt::code::BufferReadContext context(buffer->getReadContext());
	
	namespace bu = tt::code::bufferutils;
	
	// Start with a clear registries.
	clearRegistries();
	
	// Our PersistentData is the main progress.
	if (game::script::getRegistry(ProgressType_Main).unserializePersistent(&context))
	{
		return isImportingDemoFiles ? PersistentDataLoadResult_SuccessWithDemoFile : PersistentDataLoadResult_Success;
	}
	else
	{
		return PersistentDataLoadResult_Fail;
	}
}


void clearRegistries()
{
	for (s32 progType = 0; progType < ProgressType_Count; ++progType)
	{
		ProgressType type = static_cast<ProgressType>(progType);
		game::script::getRegistry(type).clear();
		game::script::getRegistry(type).clearPersistent();
	}
}


bool saveShutdownState()
{
	if (savedata::canSave() == false || AppGlobal::isInLevelEditorMode() ||
	    AppGlobal::shouldRestoreShutdownData() == false)
	{
		return false;
	}
	const game::Game* game = AppGlobal::getGame();
	if (game == 0)
	{
		TT_PANIC("saveShutdownStateToFile should only be called when there is a game.");
		return false;
	}
	
	AutoMounter mounter;
	if (mounter.isMounted() == false)
	{
		return false;
	}
	
	bool saveOk = true;
	
	// Save checkpoints and gamestate
	{
		tt::fs::FilePtr file = savedata::createSaveFile(AppGlobal::getShutdownStateFilename(AppGlobal::isInDemoMode()));
		if (file == 0)
		{
			saveOk = false;
		}
		else
		{
			// Save header data: file signature and version
			const tt::fs::size_type fslen = static_cast<tt::fs::size_type>(g_shutdownStateFormatSignatureLength);
			saveOk = saveOk && (file->write(g_shutdownStateFormatSignature, fslen) == fslen);
			saveOk = saveOk && tt::fs::writeInteger(file, g_shutdownStateFormatCurrentVersion);
			
			// First save all available checkpoints to save file
			for (s32 progType = 0; progType < ProgressType_Count; ++progType)
			{
				ProgressType type = static_cast<ProgressType>(progType);
				saveOk = saveOk && AppGlobal::getCheckPointMgr(type).save(file);
				
				if (type == ProgressType_Main)
				{
					saveOk = saveOk && game::script::getRegistry(type).saveNonPersistent(file);
				}
				else
				{
					saveOk = saveOk && game::script::getRegistry(type).saveFull(file);
				}
			}
		}
	}
	
	// Save volume & keybindings settings
	audio::AudioPlayer::saveVolumeSettings();
	input::Controller::saveCustomKeyBindings();
	
	return saveOk;
}


bool loadShutdownState(u32* p_dataVersion, bool p_forceLoadFromDemo)
{
	if (p_dataVersion != 0)
	{
		*p_dataVersion = 0;
	}
	
	// Load checkpoints and gamestate
	tt::fs::FilePtr file = savedata::openSaveFile(AppGlobal::getShutdownStateFilename(
		p_forceLoadFromDemo ? true : AppGlobal::isInDemoMode()
	));
	if(file != 0)
	{
		// Signature
		u8 signature[g_shutdownStateFormatSignatureLength] = { 0 };
		const tt::fs::size_type fslen = static_cast<tt::fs::size_type>(g_shutdownStateFormatSignatureLength);
		if (file->read(signature, fslen) != fslen ||
		    std::memcmp(signature, g_shutdownStateFormatSignature, g_shutdownStateFormatSignatureLength) != 0)
		{
			TT_NONFATAL_PANIC("ShutdownState save file does not appear to be valid. File has invalid signature.");
			return false;
		}
		
		// Version
		u32 dataVersion = 0;
		if (tt::fs::readInteger(file, &dataVersion) == false ||
		    dataVersion != g_shutdownStateFormatCurrentVersion)
		{
			if (p_dataVersion != 0)
			{
				(*p_dataVersion) = dataVersion;
			}
			// FIXME: Once we actually make use of the version number (have more than one version),
			//        do not abort load if it is different from what we expect (instead just load backwards-compatibly)
			TT_NONFATAL_PANIC("ShutdownState save file format is not the expected version. "
			                  "Loaded version %u, expected version %u.",
			                  dataVersion, g_shutdownStateFormatCurrentVersion);
			return false;
		}
		if (p_dataVersion != 0)
		{
			(*p_dataVersion) = dataVersion;
		}
		
		// Load the checkpoint chain
		for (s32 progType = 0; progType < ProgressType_Count; ++progType)
		{
			ProgressType type = static_cast<ProgressType>(progType);
			AppGlobal::getCheckPointMgr(type).load(file);
			
			if (type == ProgressType_Main)
			{
				game::script::getRegistry(type).loadNonPersistent(file);
			}
			else
			{
				game::script::getRegistry(type).loadFull(file);
			}
		}
	}
	
	file.reset();
	
	return true;
}


bool removeShutdownState()
{
	AutoMounter mounter;
	if (mounter.isMounted() == false)
	{
		return false;
	}
	
	if (savedata::destroySaveFile(AppGlobal::getShutdownStateFilename(AppGlobal::isInDemoMode())) == false)
	{
		return false;
	}

	return savedata::commitSaveData();
}


void serializePresentationRestoreInfo(const PresentationRestoreInfo& p_restoreInfo,
                                      tt::code::BufferWriteContext* p_context)
{
	TT_NULL_ASSERT(p_context);
	
	// Serialize members
	namespace bu = tt::code::bufferutils;
	
	// Store PresentationRestoreInfo.
	bu::put(p_restoreInfo.filename,               p_context);
	serializePresTags(p_restoreInfo.requiredTags, p_context);
	bu::putEnum<u8>(p_restoreInfo.layer,          p_context);
}


PresentationRestoreInfo unserializePresentationRestoreInfo(tt::code::BufferReadContext* p_context)
{
	TT_NULL_ASSERT(p_context);
	
	// Serialize members
	namespace bu = tt::code::bufferutils;
	
	// Get PresentationRestoreInfo
	std::string         filename     = bu::get<std::string>(p_context);
	tt::pres::Tags      requiredTags = unserializePresTags(p_context);
	game::ParticleLayer layer        = bu::getEnum<u8, toki::game::ParticleLayer>(p_context);
	
	return PresentationRestoreInfo(filename, requiredTags, layer);
}


void serializePresentationObjectState(const tt::pres::PresentationObjectPtr& p_object,
                                      tt::code::BufferWriteContext* p_context)
{
	TT_NULL_ASSERT(p_context);
	
	// Serialize members
	namespace bu = tt::code::bufferutils;
	
	const bool hasObject = (p_object != 0);
	bu::put(hasObject , p_context);
	
	if (hasObject == false)
	{
		return;
	}
	
	// Store custom values.
	typedef tt::pres::PresentationObject::CustomPresentationValues CustomValues;
	const CustomValues& customValues = p_object->getCustomValues();
	const u32 customValuesCount = static_cast<u32>(customValues.size());
	bu::put(customValuesCount, p_context);
	for (CustomValues::const_iterator it = customValues.begin(); it != customValues.end(); ++it)
	{
		bu::put((*it).first,  p_context);
		bu::put((*it).second, p_context);
	}
	
	const bool isPlaying = (p_object->isActive() || p_object->isLooping() || p_object->isHidingAtEnd() == false);
	
	bu::put(isPlaying, p_context);
	
	if (isPlaying)
	{
		const tt::pres::Tags& tags = p_object->getCurrentActiveTags();
		serializePresTags(tags, p_context);
		
		const std::string& name    = p_object->getCurrentActiveName();
		bu::put(name, p_context);
		
		const bool hideAtEnd = p_object->isHidingAtEnd();
		bu::put(hideAtEnd, p_context);
	}
	
	bu::put(p_object->isCulled(), p_context);
	bu::put(p_object->isVisible(), p_context);
}


void unserializePresentationObjectState(const tt::pres::PresentationObjectPtr& p_object,
                                        tt::code::BufferReadContext*           p_context,
                                        real p_presTime)
{
	TT_NULL_ASSERT(p_context);
	
	// Serialize members
	namespace bu = tt::code::bufferutils;
	
	const bool hasObject = bu::get<bool>(p_context);
	if (hasObject == false)
	{
		TT_ASSERT(p_object == 0);
		return;
	}
	
	TT_NULL_ASSERT(p_object); // Expected that object was passed.
	
	// Store custom values.
	const u32 customValuesCount = bu::get<u32>(p_context);
	for (u32 i = 0; i < customValuesCount; ++i)
	{
		std::string key   = bu::get<std::string>(p_context);
		real        value = bu::get<real>(       p_context);
		
		if (p_object != 0) // Even if a null ptr was passed we still need to read all the expected data without crashing.
		{
			p_object->addCustomPresentationValue(key, value);
		}
	}
	
	const bool wasPlaying = bu::get<bool>(p_context);
	
	if (wasPlaying)
	{
		const tt::pres::Tags tags = unserializePresTags(p_context);
		const std::string name    = bu::get<std::string>(p_context);
		const bool hideAtEnd      = bu::get<       bool>(p_context);
		
		p_object->start(tags, hideAtEnd, name);
		if (p_object->isLooping() == false)
		{
			// HACK: prevents triggers going off during a update
			tt::pres::TriggerBase::setTriggersDisabled(true);
			p_object->update(p_presTime);
			tt::pres::TriggerBase::setTriggersDisabled(false);
		}
	}
	
	const bool isCulled = bu::get<bool>(p_context);
	p_object->setIsCulled(isCulled);
	
	const bool isVisible = bu::get<bool>(p_context);
	p_object->setVisible(isVisible);
}


void serializePresTags(const tt::pres::Tags& p_tags, tt::code::BufferWriteContext* p_context)
{
	TT_NULL_ASSERT(p_context);
	
	// Serialize members
	namespace bu = tt::code::bufferutils;
	
	const u32 tagCount = static_cast<u32>(p_tags.size());
	bu::put(tagCount, p_context);
	
	for (tt::pres::Tags::const_iterator it = p_tags.begin(); it != p_tags.end(); ++it)
	{
		
		bu::put((*it).getValue(), p_context);
		bu::put((*it).getName(),  p_context); // To keep the extra debug info in non-final builds.
	}
}


tt::pres::Tags unserializePresTags(tt::code::BufferReadContext* p_context)
{
	TT_NULL_ASSERT(p_context);
	
	// Serialize members
	namespace bu = tt::code::bufferutils;
	
	const u32 tagCount = bu::get<u32>(p_context);
	
	tt::pres::Tags result;
	
	for (u32 i = 0; i < tagCount; ++i)
	{
		tt::pres::Tags::value_type::ValueType hashValue =
				bu::get<tt::pres::Tags::value_type::ValueType>(p_context);
		std::string name = bu::get<std::string>(p_context);
		
		if (name.empty())
		{
			result.insert(tt::pres::Tags::value_type(hashValue));
		}
		else
		{
			tt::pres::Tags::value_type tag(name);
			TT_ASSERT(tag.getValue() == hashValue);
			result.insert(tag);
		}
	}
	
	return result;
}



void serializeMove(const game::movement::MoveBasePtr& p_move,
                   const game::movement::MovementSetPtr& p_movementSet,
                   tt::code::BufferWriteContext* p_context)
{
	TT_NULL_ASSERT(p_context);
	const bool saveSomething = (p_move != 0 && p_movementSet != 0);
	
	namespace bu = tt::code::bufferutils;
	
	bu::put(saveSomething, p_context);
	
	if (saveSomething == false)
	{
		return;
	}
	
	game::movement::MovementSet::MoveID moveID(p_movementSet->getMoveID(p_move));
	
	bu::putEnum<u8>(moveID.direction  , p_context); // Direction
	bu::put(moveID.index              , p_context); // s32
	bu::put(moveID.movementSetFilename, p_context); // std::string
	
	p_context->flush();
}


game::movement::MoveBasePtr unserializeMove(const game::movement::MovementSetPtr& p_movementSet,
                                            tt::code::BufferReadContext* p_context)
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	const bool loadSomething = bu::get<bool>(p_context);
	if (loadSomething == false)
	{
		return game::movement::MoveBasePtr();
	}
	
	if (p_movementSet == 0)
	{
		TT_PANIC("Expected valid movementSet to unserializemove");
		return game::movement::MoveBasePtr();
	}
	
	game::movement::MovementSet::MoveID moveID;
	
	moveID.direction           = bu::getEnum<u8, game::movement::Direction>(p_context);
	moveID.index               = bu::get<s32>(                              p_context);
	moveID.movementSetFilename = bu::get<std::string>(                      p_context);
	
	if (moveID.isValid())
	{
		return p_movementSet->getFromMoveID(moveID);
	}
	else
	{
		return game::movement::MoveBasePtr();
	}
}


void serializeBlurLayers(const tt::engine::scene2d::BlurLayers& p_layers,
                         tt::code::BufferWriteContext* p_context)
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	const s32 layerCount = static_cast<s32>(p_layers.size());
	bu::put(layerCount, p_context);
	
	for (tt::engine::scene2d::BlurLayers::const_iterator it = p_layers.begin();
	     it != p_layers.end(); ++it)
	{
		const real value = (*it);
		bu::put(value, p_context);
	}
}


const tt::engine::scene2d::BlurLayers unserializeBlurLayers(tt::code::BufferReadContext* p_context)
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	tt::engine::scene2d::BlurLayers layers;
	
	const s32 layerCount = bu::get<s32>(p_context);
	for (s32 i = 0; i < layerCount; ++i)
	{
		const real value = bu::get<real>(p_context);
		layers.insert(value);
	}
	return layers;
}


void serializeTexturePtr(const tt::engine::renderer::TexturePtr& p_texture,
                         tt::code::BufferWriteContext* p_context)
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	tt::engine::EngineID id(0,0);
	
	if (p_texture != 0)
	{
		id = p_texture->getEngineID();
	}
	
	bu::put(id.crc1, p_context);
	bu::put(id.crc2, p_context);
}


tt::engine::renderer::TexturePtr unserializeTexturePtr(tt::code::BufferReadContext* p_context)
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	u32 crc1 = bu::get<u32>(p_context);
	u32 crc2 = bu::get<u32>(p_context);
	
	tt::engine::EngineID id(crc1, crc2);
	
	if (id.valid() == false)
	{
		return tt::engine::renderer::TexturePtr();
	}
	
	return tt::engine::renderer::TextureCache::get(id, false);
}


// Namespace end
}
}
