#include <cstring>

#include <tt/app/Application.h>
#include <tt/code/bufferutils.h>
#include <tt/code/FourCC.h>
#include <tt/compression/fastlz.h>
#include <tt/mem/util.h>
#include <tt/fs/File.h>
#include <tt/fs/fs.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>

#include <toki/savedata/utils.h>
#include <toki/serialization/SerializationMgr.h>


namespace toki {
namespace serialization {

#define UNCOMPRESSED_CHECKPOINTS 1

// Section identifier FourCCs

u32 getSectionSaveID(Section p_section)
{
	// NOTE: Do not modify existing values! These values are used in data saved to disk,
	//       to identify each serialization section. Modifying the values will break existing data!
	switch (p_section)
	{
	case Section_Game:                       return tt::code::FourCC<'g', 'a', 'm', 'e'>::value;
	case Section_Squirrel:                   return tt::code::FourCC<'s', 'q', 'r', 'l'>::value;
	case Section_EntityMgr:                  return tt::code::FourCC<'e', 'n', 't', 't'>::value;
	case Section_TimerMgr:                   return tt::code::FourCC<'t', 'i', 'm', 'r'>::value;
	case Section_Registry:                   return tt::code::FourCC<'r', 'e', 'g', 's'>::value;
	case Section_FluidMgr:                   return tt::code::FourCC<'f', 'l', 'u', 'd'>::value;
	case Section_DarknessMgr:                return tt::code::FourCC<'d', 'a', 'r', 'k'>::value;
	case Section_LightMgr:                   return tt::code::FourCC<'l', 'i', 't', 'e'>::value;
	case Section_SensorMgr:                  return tt::code::FourCC<'s', 'e', 'n', 's'>::value;
	case Section_TileSensorMgr:              return tt::code::FourCC<'t', 'l', 's', 'n'>::value;
	case Section_MovementControllerMgr:      return tt::code::FourCC<'m', 'o', 'v', 'e'>::value;
	case Section_PowerBeamGraphicMgr:        return tt::code::FourCC<'b', 'e', 'a', 'm'>::value;
	case Section_PresentationObjectMgr:      return tt::code::FourCC<'p', 'r', 'e', 's'>::value;
	case Section_PresentationObjectMgrState: return tt::code::FourCC<'p', 's', 's', 't'>::value;
	case Section_GunMgr:                     return tt::code::FourCC<'g', 'u', 'n', 's'>::value;
	case Section_EffectRectMgr:              return tt::code::FourCC<'e', 'r', 'e', 'c'>::value;
	case Section_TextLabelMgr:               return tt::code::FourCC<'t', 'e', 'x', 't'>::value;
	case Section_DemoMgr:                    return tt::code::FourCC<'d', 'e', 'm', 'o'>::value;
		
	default:
		TT_PANIC("Invalid/unsupported serializer section: %d\n"
		         "If this is a valid Section value, no save ID was added for it.", p_section);
		return 0;
	}
}


Section getSectionFromSaveID(u32 p_saveID)
{
	for (s32 i = 0; i < Section_Count; ++i)
	{
		const Section section = static_cast<Section>(i);
		if (getSectionSaveID(section) == p_saveID)
		{
			return section;
		}
	}
	
	return Section_Invalid;
}

//--------------------------------------------------------------------------------------------------
// Static variables

u8 SerializationMgr::ms_compressionInputBuffer [ms_compressionBufferSize];
u8 SerializationMgr::ms_compressionOutputBuffer[ms_compressionBufferSize];

//--------------------------------------------------------------------------------------------------
// Public member functions

SerializationMgrPtr SerializationMgr::createEmpty()
{
	return SerializationMgrPtr(new SerializationMgr);
}


SerializationMgrPtr SerializationMgr::createFromFile(const tt::fs::FilePtr& p_file)
{
	SerializationMgrPtr mgr(new SerializationMgr);
	if (mgr->loadFromFile(p_file) == false)
	{
		return SerializationMgrPtr();
	}
	
	return mgr;
}


SerializationMgrPtr SerializationMgr::createFromCompressedBuffer(const tt::code::BufferPtr& p_buffer)
{
	SerializationMgrPtr mgr(new SerializationMgr);
	if (mgr->loadFromCompressedBuffer(p_buffer) == false)
	{
		return SerializationMgrPtr();
	}
	
	return mgr;
}


SerializationMgr::SerializationMgr()
{
	for (s32 i = 0; i < Section_Count; ++i)
	{
		m_sections[i].reset(new Serializer);
	}
}


void SerializationMgr::clearAll()
{
	for (s32 i = 0; i < Section_Count; ++i)
	{
		m_sections[i]->clear();
	}
}


const SerializerPtr& SerializationMgr::getSection(Section p_section) const
{
	if (isValid(p_section) == false)
	{
		TT_PANIC("Invalid serialization section: %d", p_section);
		static SerializerPtr empty;
		return empty;
	}
	
	return m_sections[p_section];
}


u32 SerializationMgr::getTotalSize() const
{
	u32 totalSize = 0;
	
	for (s32 i = 0; i < Section_Count; ++i)
	{
		if (m_sections[i] != 0)
		{
			totalSize += m_sections[i]->getSize();
		}
	}
	
	return totalSize;
}


bool SerializationMgr::saveToFile(tt::fs::FilePtr& p_file) const
{
	bool saveOk = true;
	
	tt::code::BufferPtr buffer(compress());
	TT_NULL_ASSERT(buffer);
	if (buffer == 0)
	{
		return false;
	}
	saveOk = saveOk && tt::fs::writeInteger(p_file, static_cast<u32>(buffer->getSize()));
	saveOk = saveOk && tt::fs::write(p_file, buffer->getData(), buffer->getSize()) == buffer->getSize();
	
	return saveOk;
}


bool SerializationMgr::loadFromFile(const tt::fs::FilePtr& p_file)
{	
	u32 size = 0;
	if (tt::fs::readInteger<u32>(p_file, &size) == false)
	{
		TT_PANIC("Cannot read compressed buffer size from savedata.");
		return false;
	}
	
	tt::code::BufferPtrForCreator buffer(new tt::code::Buffer(size));
	if (tt::fs::read(p_file, buffer->getData(), size) != static_cast<tt::fs::size_type>(size))
	{
		TT_PANIC("Cannot read compressed buffer from savedata. Mismatch in read bytes.");
		return false;
	}
	
	return loadFromCompressedBuffer(buffer);
}


SerializationMgrPtr SerializationMgr::clone() const
{
	return SerializationMgrPtr(new SerializationMgr(*this));
}


tt::code::BufferPtr SerializationMgr::compress() const
{
	tt::code::BufferWriteContext input(
		tt::code::BufferWriteContext::createForRawBuffer(ms_compressionInputBuffer, ms_compressionBufferSize));
	
	namespace bu = tt::code::bufferutils;
	
	// Store the number of sections
	u32 sectionCount = 0;
	for (s32 i = 0; i < Section_Count; ++i)
	{
		if (m_sections[i] != 0 && m_sections[i]->getSize() != 0) ++sectionCount;
	}
	bu::put(sectionCount, &input);
	
	// Load all sections into compression input buffer
	for (s32 i = 0; i < Section_Count; ++i)
	{
		const Section section = static_cast<Section>(i);
		if (m_sections[section] == 0 || m_sections[section]->getSize() == 0)
		{
			// No section data available: do not store
			continue;
		}
		// Write sectionID in buffer
		bu::put(getSectionSaveID(section), &input);
		
		// Write section to buffer
		if (m_sections[section]->saveToBuffer(&input) == false)
		{
			return tt::code::BufferPtr();
		}
	}
	
	const u32 totalSize = static_cast<u32>(input.cursor - input.start);
#if UNCOMPRESSED_CHECKPOINTS
	tt::code::BufferPtrForCreator buffer(new tt::code::Buffer(totalSize));
	tt::mem::copy8(buffer->getData(), &ms_compressionInputBuffer, totalSize);
	return buffer;
#else
	// Compress the buffer
	// FIXME: Use LZ4 fast with acceleration 50 here; performs better than FastLZ. Only you need to store the
	// size of the uncompressed buffer as well!
	const s32 compressedSize = 
		fastlz_compress(&ms_compressionInputBuffer, totalSize, &ms_compressionOutputBuffer);
	
	tt::code::BufferPtrForCreator buffer(new tt::code::Buffer(compressedSize));
	
	// Copy compressed data to return buffer
	tt::mem::copy8(buffer->getData(), &ms_compressionOutputBuffer, compressedSize);
	
	return buffer;
#endif
}


//--------------------------------------------------------------------------------------------------
// Private member functions

SerializationMgr::SerializationMgr(const SerializationMgr& p_rhs)
{
	for (s32 i = 0; i < Section_Count; ++i)
	{
		TT_NULL_ASSERT(p_rhs.m_sections[i]);
		if (p_rhs.m_sections[i] != 0)
		{
			m_sections[i] = p_rhs.m_sections[i]->clone();
		}
	}
}


bool SerializationMgr::loadFromCompressedBuffer(const tt::code::BufferPtr& p_buffer)
{
#if UNCOMPRESSED_CHECKPOINTS
	tt::code::BufferReadContext input(
		tt::code::BufferReadContext::createForRawBuffer(static_cast<const u8*>(p_buffer->getData()), p_buffer->getSize()));
#else
	const u32 decompressedSize = fastlz_decompress(p_buffer->getData(), p_buffer->getSize(),
	                                               &ms_compressionInputBuffer, ms_compressionBufferSize);
	
	if (decompressedSize == 0)
	{
		TT_PANIC("loadFromCompressedBuffer failed to decompress the buffer");
		return false;
	}
	tt::code::BufferReadContext input(
		tt::code::BufferReadContext::createForRawBuffer(ms_compressionInputBuffer, decompressedSize));
#endif
	
	namespace bu = tt::code::bufferutils;
	
	// Store the number of sections
	u32 sectionCount = bu::get<u32>(&input);
	
	// Load all sections
	for (u32 i = 0; i < sectionCount; ++i)
	{
		// Load (and parse/validate) section ID
		u32 sectionID = bu::get<u32>(&input);
		
		Section section = getSectionFromSaveID(sectionID);
		if (isValid(section) == false)
		{
			TT_PANIC("Loaded unsupported section (ID 0x%08X) from serialization data", sectionID);
			return false;
		}
		
		// Load section data
		m_sections[section] = Serializer::createFromBuffer(&input);
		if (m_sections[section] == 0 && section != Section_DemoMgr)
		{
			TT_PANIC("Empty section '%d'", section);
			return false;
		}
	}
	
	return true;
}

// Namespace end
}
}
