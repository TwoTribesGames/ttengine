#include <tt/code/bufferutils.h>
#include <tt/code/helpers.h>
#include <tt/compression/fastlz.h>
#include <tt/compression/lzma.h>
#include <tt/compression/lz4/lz4.h>
#include <tt/compression/lz4/lz4hc.h>
#include <tt/fs/utils/utils.h>
#include <tt/fs/File.h>
#include <tt/fs/MemoryArchive.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
#include <tt/str/str.h>


namespace tt {
namespace fs {

// NOTE: For the reason for these specific signature bytes, see
// http://en.wikipedia.org/wiki/Portable_Network_Graphics#File_header
// (these bytes are inspired by the PNG file header bytes,
//  for the same reasons as listed there: mainly to detect file transmission problems)
const size_t g_signatureLength = 9;
const u8     g_signature[g_signatureLength] =
{
	0x89,        // Has the high bit set to detect transmission systems that do not support 8 bit data
	'M', 'E', 'M', 'A', // The actual signature bytes
	0x0D, 0x0A,  // A DOS-style line ending (CRLF) to detect DOS-Unix line ending conversion of the data.
	0x1A,        // A byte that stops display of the file under DOS when the command type has been used—the end-of-file character
	0x0A         // A Unix-style line ending (LF) to detect Unix-DOS line ending conversion.
};
const u32    g_currentVersion = 1;

struct InternalFileHeader
{
	InternalFileHeader()
	:
	path(),
	start(0),
	size(0),
	writeTime(0)
	{ }
	
	std::string       path;
	u32               start;
	fs::size_type size;
	fs::time_type writeTime;
	
	inline u32 getStructSize() const
	{
		return static_cast<u32>(sizeof(u16) +  // nameLength
			path.size() + // name
			sizeof(start) +
			sizeof(size) +
			sizeof(writeTime));
	}
};
typedef std::vector<InternalFileHeader> InternalFileHeaders;

static u32 align(u32 p_value, u32 p_alignment)
{
	if ((p_value % p_alignment) != 0)
	{
		p_value += p_alignment - (p_value % p_alignment);
	}
	
	return p_value;
}

static void addPaddingToContext(code::BufferWriteContext* p_context, u32 p_alignment)
{
	u32 bytesWritten = static_cast<u32>(p_context->cursor - p_context->start);
	const u32 alignedBytesWritten = align(bytesWritten, p_alignment);
	while (bytesWritten < alignedBytesWritten)
	{
		code::bufferutils::put<u8>(0, p_context);
		++bytesWritten;
	}
}

//--------------------------------------------------------------------------------------------------
// Public member functions

MemoryArchive::MemoryArchive()
:
m_files(),
m_archive()
{
}


MemoryArchivePtr MemoryArchive::load(const std::string& p_path)
{
	if (fs::fileExists(p_path) == false)
	{
		TT_WARN("Couldn't load memory archive '%s'. File doesn't exist.", p_path.c_str());
		return MemoryArchivePtr();
	}
	
	// Read archive
	code::BufferPtr content = fs::getFileContent(p_path);
	if (content == 0)
	{
		TT_PANIC("Failed to open memory archive '%s'", p_path.c_str());
		return MemoryArchivePtr();
	}
	
	namespace bu = code::bufferutils;
	code::BufferReadContext context = content->getReadContext();
	
	// Verify signature
	u8 signature[g_signatureLength] = { 0 };
	bu::get(signature, g_signatureLength, &context);
	if (std::memcmp(signature, g_signature, g_signatureLength) != 0)
	{
		TT_PANIC("Memory archive file '%s' doesn't have valid signature.", p_path.c_str());
		return MemoryArchivePtr();
	}
	
	// Verify version
	u32 dataVersion = bu::get<u32>(&context);
	if (dataVersion != g_currentVersion)
	{
		TT_PANIC("Memory archive file '%s' file format is not the expected version. "
		         "Loaded version %u, expected version %u.", p_path.c_str(),
		         dataVersion, g_currentVersion);
		return MemoryArchivePtr();
	}
	
	// Write total (uncompressed) size
	const u32 totalSize = bu::get<u32>(&context);
	
	// Write total (uncompressed) size
	const u32 totalHeaderSize = bu::get<u32>(&context);
	
	// Get compression type
	const CompressionType compressionType = bu::getEnum<u8, CompressionType>(&context);
	
	switch (compressionType)
	{
	case CompressionType_None:
		// remaining buffer contains all files; do nothing
		break;
		
	case CompressionType_FastLZ:
	case CompressionType_LZMA:
	case CompressionType_LZ4:
	case CompressionType_LZ4HC:
		{
			// Decompress to a buffer
			code::BufferPtrForCreator decompressionBuffer(new code::Buffer(totalSize));
			if (decompressionBuffer == 0)
			{
				TT_PANIC("Failed to create decompression buffer");
				return MemoryArchivePtr();
			}
			
			const u32 compressedSize = bu::get<u32>(&context);
			u32 decompressedSize = 0;
			if (compressionType == CompressionType_FastLZ)
			{
				decompressedSize = fastlz_decompress(context.cursor, compressedSize,
					decompressionBuffer->getData(), totalSize);
			}
			else if (compressionType == CompressionType_LZMA)
			{
				decompressedSize = lzma_decompress(context.cursor, compressedSize,
					decompressionBuffer->getData(), totalSize);
			}
			else if (compressionType == CompressionType_LZ4 || compressionType == CompressionType_LZ4HC)
			{
				decompressedSize = LZ4_decompress_fast(reinterpret_cast<const char*>(context.cursor),
					reinterpret_cast<char*>(decompressionBuffer->getData()), totalSize);
				if (compressedSize == decompressedSize)
				{
					// Succes condition
					decompressedSize = totalSize;
				}
			}
			else
			{
				TT_PANIC("Unsupported compression type '%d'", compressionType);
			}
			
			
			if (decompressedSize != totalSize)
			{
				TT_PANIC("Failed to decompress memory archive. Decompression size '%d' mismatches the stored size '%d'", decompressedSize, totalSize);
				return MemoryArchivePtr();
			}
			
			// Content and context are now useless as they contain compressed data, replace them with
			// decompressed data
			content = decompressionBuffer;
			context = content->getReadContext();
		}
		break;
		
	default:
		TT_PANIC("Memory archive file '%s' contains unknown compression type '%d'",
		         p_path.c_str(), compressionType);
		return MemoryArchivePtr();
	}
	
	// Finally create archive and read all files
	MemoryArchivePtr result(new MemoryArchive(content));
	
	const u32 fileCount = bu::get<u32>(&context);
	Files files;
	for (u32 i = 0; i < fileCount; ++i)
	{
		File file;
		file.path      = bu::get<std::string      >(&context);
		u32 start      = bu::get<u32              >(&context);
		u32 size       = bu::get<u32              >(&context);
		file.writeTime = bu::get<tt::fs::time_type>(&context);
		
		start += totalHeaderSize;
		
		file.content.reset(new code::Buffer(reinterpret_cast<u8*>(const_cast<void*>(content->getData())) + start,
		                                    static_cast<code::Buffer::size_type>(size)));
		
		result->addFile(file);
	}
	
	return result;
}


bool MemoryArchive::save(const std::string& p_path,
                         CompressionType    p_compressionType,
                         u32                p_fileAlignment)
{
	// Open output file before actual processing
	fs::FilePtr file = fs::open(p_path, fs::OpenMode_Write);
	if (file == 0)
	{
		TT_PANIC("Unable to open '%s' for writing.\n", p_path .c_str());
		return false;
	}
	
	// Construct header
	InternalFileHeaders fileHeaders;
	u32 start           = 0;
	u32 totalHeaderSize = 0;
	for (Files::const_iterator it = m_files.begin(); it != m_files.end(); ++it)
	{
		TT_NULL_ASSERT((*it).second.content);
		
		InternalFileHeader header;
		header.path      = (*it).second.path;
		header.size      = (*it).second.content->getSize();
		header.writeTime = (*it).second.writeTime;
		header.start     = start;
		
		start += header.size;
		
		// Align the start offset
		start = align(start, p_fileAlignment);
		
		totalHeaderSize += header.getStructSize();
		fileHeaders.push_back(header);
	}
	
	totalHeaderSize += sizeof(u32); // m_files.size()
	
	// Align the totalHeaderSize
	totalHeaderSize = align(totalHeaderSize, p_fileAlignment);
	
	const u32 totalSize = totalHeaderSize + start;
	
	TT_ASSERT((totalSize % p_fileAlignment) == 0);
	
	// Create buffer
	code::BufferPtrForCreator writeBuffer(new code::Buffer(totalSize));
	if (writeBuffer == 0)
	{
		TT_Printf("Failed to create write buffer");
		return false;
	}
	code::BufferWriteContext context(writeBuffer->getWriteContext());
	
	namespace bu = code::bufferutils;
	
	// First write header to buffer
	u32 fileCount(static_cast<u32>(m_files.size()));
	bu::put(fileCount, &context);
	
	for (InternalFileHeaders::iterator it = fileHeaders.begin(); it != fileHeaders.end(); ++it)
	{
		bu::put((*it).path,      &context);
		bu::put((*it).start,     &context);
		bu::put((*it).size,      &context);
		bu::put((*it).writeTime, &context);
	}
	addPaddingToContext(&context, p_fileAlignment);
	
	// Do some sanity checking
	{
		const u32 bytesWritten = static_cast<u32>(context.cursor-context.start);
		TT_ASSERT(bytesWritten == totalHeaderSize);
	}
	
	// Next write the actual files to buffer
	for (Files::iterator it = m_files.begin(); it != m_files.end(); ++it)
	{
		bu::put(static_cast<const u8*>((*it).second.content->getData()),
		        (*it).second.content->getSize(), &context);
		addPaddingToContext(&context, p_fileAlignment);
	}
	
	// Do some sanity checking
	{
		const u32 bytesWritten = static_cast<u32>(context.cursor-context.start);
		TT_ASSERT(bytesWritten == totalSize);
	}
	
	// Header and files are now written to write buffer; write to archive file
	bool saveOk = true;
	const fs::size_type fslen = static_cast<fs::size_type>(g_signatureLength);
	saveOk = saveOk && (file->write(g_signature, fslen) == fslen);
	saveOk = saveOk && fs::writeInteger(file, g_currentVersion);
	
	// Write total size
	if (fs::writeInteger(file, totalSize) == false)
	{
		TT_Printf("Failed to write to output file '%s'\n", p_path.c_str());
		return false;
	}
	
	// Write header size
	if (fs::writeInteger(file, totalHeaderSize) == false)
	{
		TT_Printf("Failed to write to output file '%s'\n", p_path.c_str());
		return false;
	}
	
	// Write compression type
	saveOk = saveOk && fs::writeEnum<u8, CompressionType>(file, p_compressionType);
	
	if (saveOk == false)
	{
		TT_PANIC("Failed to write to output file '%s'", p_path.c_str());
		return false;
	}
	
	switch (p_compressionType)
	{
	case CompressionType_None:
		// Simply write the contents of the buffer to the output file
		if (fs::write(file, writeBuffer->getData(), totalSize) != static_cast<fs::size_type>(totalSize))
		{
			TT_PANIC("Failed to write to output file '%s'", p_path.c_str());
			return false;
		}
		break;
		
	case CompressionType_FastLZ:
	case CompressionType_LZMA:
	case CompressionType_LZ4:
	case CompressionType_LZ4HC:
		{
			// Compress to a buffer and write that to the output file
			code::BufferPtrForCreator compressionBuffer(new code::Buffer(totalSize));
			if (compressionBuffer == 0)
			{
				TT_PANIC("Failed to create compression buffer");
				return false;
			}
			
			u32 compressedSize = 0;
			if (p_compressionType == CompressionType_FastLZ)
			{
				compressedSize = fastlz_compress_level(2, writeBuffer->getData(),
					totalSize, compressionBuffer->getData());
			}
			else if (p_compressionType == CompressionType_LZMA)
			{
				compressedSize = lzma_compress(writeBuffer->getData(),
					totalSize, compressionBuffer->getData(), 9, 1024 * 1024);
			}
			else if (p_compressionType == CompressionType_LZ4)
			{
				compressedSize = LZ4_compress_default(reinterpret_cast<const char*>(writeBuffer->getData()), reinterpret_cast<char*>(compressionBuffer->getData()), totalSize, totalSize);
			}
			else if (p_compressionType == CompressionType_LZ4HC)
			{
				compressedSize = LZ4_compress_HC(reinterpret_cast<const char*>(writeBuffer->getData()), reinterpret_cast<char*>(compressionBuffer->getData()), totalSize, totalSize, LZ4HC_CLEVEL_DEFAULT);
			}
			else
			{
				TT_PANIC("Unsupported compression type '%d'", p_compressionType);
				return false;
			}
			
			if (compressedSize == 0)
			{
				TT_PANIC("Compression failed, as size returns 0.");
				return false;
			}
			
			if (compressedSize > totalSize)
			{
				TT_PANIC("Cannot use the current compressor for compression as the resulting output size"
				          " is larger than its uncompressed size. Please re-run archiver with different compressor.");
				return false;
			}
			
			// Write compressed size
			if (fs::writeInteger(file, compressedSize) == false)
			{
				TT_PANIC("Failed to write to output file '%s'", p_path.c_str());
				return false;
			}
			
			// Write compressed buffer
			if (fs::write(file, compressionBuffer->getData(), compressedSize) != 
				static_cast<fs::size_type>(compressedSize))
			{
				TT_PANIC("Failed to write to output file '%s'\n", p_path.c_str());
				return false;
			}
		}
		break;
		
	default:
		TT_PANIC("Invalid compression type specified '%d'", p_compressionType);
		return false;
	}
	
	return true;
}


bool MemoryArchive::hasFile(const std::string& p_relativeFilePath) const
{
	return m_files.find(NameHash(p_relativeFilePath)) != m_files.end();
}


bool MemoryArchive::addFile(const std::string& p_relativeFilePath, const std::string& p_relativeRootPath)
{
	const std::string fullPath(fs::utils::makeCorrectFSPath(fs::getWorkingDir() + p_relativeRootPath + 
	                           fs::getDirSeparator() + p_relativeFilePath));
	
	fs::FilePtr file = fs::open(fullPath, fs::OpenMode_Read);
	if (file == 0)
	{
		TT_PANIC("Unable to open file '%s'\n", fullPath.c_str());
		return false;
	}
	
	code::BufferPtr content = fs::getFileContent(file);
	if (content == 0)
	{
		TT_PANIC("Unable to read file '%s'\n", fullPath.c_str());
		return false;
	}
	
	// Tidy path string
	std::string path(p_relativeFilePath);
	str::replace(path, "//", "/");
	str::replace(path, "\\", "/");
	path = str::trim(path, "/");
	
	File archiveFile;
	archiveFile.content   = content;
	archiveFile.writeTime = file->getWriteTime();
	archiveFile.path      = path;
	
	return addFile(archiveFile);
}


MemoryArchive::File MemoryArchive::getFile(const std::string& p_relativeFilePath) const
{
	Files::const_iterator it = m_files.find(NameHash(p_relativeFilePath));
	if (it == m_files.end())
	{
		return File();
	}
	
	return (*it).second;
}


code::BufferPtr MemoryArchive::getFileContent(const std::string& p_relativeFilePath) const
{
	Files::const_iterator it = m_files.find(NameHash(p_relativeFilePath));
	if (it == m_files.end())
	{
		return code::BufferPtr();
	}
	
	return (*it).second.content;
}


//--------------------------------------------------------------------------------------------------
// Private member functions

MemoryArchive::MemoryArchive(const code::BufferPtr& p_archiveBuffer)
:
m_files(),
m_archive(p_archiveBuffer)
{
}


bool MemoryArchive::addFile(const File& p_file)
{
	const NameHash hash(p_file.path);
	
#ifndef TT_BUILD_FINAL
	if (m_files.find(hash) != m_files.end())
	{
		TT_PANIC("File '%s' is already added in this archive (or hashclash?), overwriting data.",
		         p_file.path.c_str());
	}
#endif
	
	m_files[hash] = p_file;
	
	return true;
}


// Namespace end
}
}
