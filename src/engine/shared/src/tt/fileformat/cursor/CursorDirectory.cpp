#include <tt/code/bufferutils_get.h>
#include <tt/code/helpers.h>
#include <tt/fileformat/cursor/CursorData.h>
#include <tt/fileformat/cursor/CursorDirectory.h>
#include <tt/fs/File.h>
#include <tt/fs/fs.h>
#include <tt/platform/tt_error.h>


namespace tt {
namespace fileformat {
namespace cursor {

struct CursorDirectoryEntry
{
	CursorData::HeaderInfo header;
	u32                    resourceSize;     // in bytes
	u32                    imageDataOffset;  // in bytes, from start of file
	
	inline CursorDirectoryEntry()
	:
	header(),
	resourceSize(0),
	imageDataOffset(0)
	{ }
};


//--------------------------------------------------------------------------------------------------
// Public member functions

CursorDirectoryPtr CursorDirectory::load(const std::string& p_filename)
{
	if (fs::fileExists(p_filename) == false)
	{
		TT_PANIC("Cursor file '%s' does not exist.", p_filename.c_str());
		return CursorDirectoryPtr();
	}
	
	code::BufferPtr content = fs::getFileContent(p_filename);
	if (content == 0)
	{
		TT_PANIC("Could not load file content from cursor file '%s'.\n", p_filename.c_str());
		return CursorDirectoryPtr();
	}
	
	code::BufferReadContext context(content->getReadContext());
	namespace bu = code::bufferutils;
	
	// Load the directory header information
	bu::get<u16>(&context);  // first 2 bytes: reserved
	const u16 cursorType  = bu::get<u16>(&context);
	const u16 cursorCount = bu::get<u16>(&context);
	
	enum IconType
	{
		IconType_Icon   = 1,
		IconType_Cursor = 2
	};
	
	if (cursorType != IconType_Cursor)
	{
		TT_PANIC("File '%s' is not a valid cursor file.\nThe icon type is %d, but it should be 2.",
		         p_filename.c_str(), cursorType);
		return CursorDirectoryPtr();
	}
	
	if (cursorCount == 0)
	{
		TT_PANIC("File '%s' is not a valid cursor file.\nThe file does not contain any cursors.",
		         p_filename.c_str());
		return CursorDirectoryPtr();
	}
	
	/*
	TT_Printf("CursorDirectory::load: Loaded cursor file '%s': type %u with %u image(s) (file is %d bytes).\n",
	          p_filename.c_str(), cursorType, cursorCount, content->getSize());
	// */
	
	// First load the header information for each cursor
	// (describing the basic properties of the cursor and where its image data resides)
	typedef std::vector<CursorDirectoryEntry> Entries;
	Entries entries;
	entries.reserve(static_cast<Entries::size_type>(cursorCount));
	for (u16 i = 0; i < cursorCount; ++i)
	{
		CursorDirectoryEntry entry;
		entry.header.width      = static_cast<s32>(bu::get<s8>(&context));
		entry.header.height     = static_cast<s32>(bu::get<s8>(&context));
		entry.header.colorCount = static_cast<s32>(bu::get<s8>(&context));
		bu::get<s8>(&context);  // reserved; ignore
		entry.header.hotSpot.x  = static_cast<s32>(bu::get<u16>(&context));
		entry.header.hotSpot.y  = static_cast<s32>(bu::get<u16>(&context));
		entry.resourceSize      = bu::get<u32>(&context);  // bytes in resource: size of image data
		entry.imageDataOffset   = bu::get<u32>(&context);  // offset to image data, from start of file
		
		entries.push_back(entry);
	}
	
	// Header loading must not have exhausted the buffer: image data must come next
	if (context.statusCode != 0)
	{
		TT_PANIC("Invalid cursor file '%s': end of file reached while loading header information.",
		         p_filename.c_str());
		return CursorDirectoryPtr();
	}
	
	// Now load the image data for each cursor
	CursorDirectoryPtr cursorDirectory(new CursorDirectory);
	cursorDirectory->m_cursors.reserve(static_cast<Cursors::size_type>(cursorCount));
	
	const u8* const fileStart = reinterpret_cast<const u8*>(content->getData());
	const u32       fileSize  = static_cast<u32>(content->getSize());
	s32 cursorIndex = 0;
	for (Entries::iterator it = entries.begin(); it != entries.end(); ++it, ++cursorIndex)
	{
		const CursorDirectoryEntry& entry(*it);
		
		// Image data must reside within the file
		if ((entry.imageDataOffset + entry.resourceSize) > fileSize)
		{
			TT_PANIC("Cursor %d specifies an invalid image data location (outside of file bounds): "
			         "offset %u and size %u, while file size is %u bytes.\nCursor file '%s'.",
			         cursorIndex, entry.imageDataOffset, entry.resourceSize, fileSize, p_filename.c_str());
			continue;
		}
		
		code::BufferReadContext imageContext(code::BufferReadContext::createForRawBuffer(
				fileStart + entry.imageDataOffset, static_cast<size_t>(entry.resourceSize)));
		
		CursorData* cursorData = CursorData::createFromData(entry.header, &imageContext);
		// FIXME: Should failure to load provide an error message here, or should CursorData do so?
		if (cursorData != 0)
		{
			cursorDirectory->m_cursors.push_back(cursorData);
		}
	}
	
	if (cursorDirectory->m_cursors.empty())
	{
		// No cursors were loaded: this cannot happen if the cursor file was valid
		TT_PANIC("No cursors could be loaded from cursor file '%s'.\nThe file is not a valid cursor file.",
		         p_filename.c_str());
		return CursorDirectoryPtr();
	}
	
	return cursorDirectory;
}


CursorDirectory::~CursorDirectory()
{
	code::helpers::freePointerContainer(m_cursors);
}


const CursorData* CursorDirectory::getCursor(s32 p_index) const
{
	if (p_index < 0 || p_index >= getCursorCount())
	{
		TT_PANIC("Cursor index %d out of bounds 0 - %d.", p_index, getCursorCount());
		return 0;
	}
	
	return m_cursors[p_index];
}


//--------------------------------------------------------------------------------------------------
// Private member functions

CursorDirectory::CursorDirectory()
:
m_cursors()
{
}

// Namespace end
}
}
}
