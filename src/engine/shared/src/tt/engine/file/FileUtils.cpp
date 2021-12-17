#if defined(TT_PLATFORM_WIN)
#define NOMINMAX
#include <windows.h>
#endif

#include <tt/engine/file/FileUtils.h>
#include <tt/platform/tt_printf.h>
#include <tt/fs/fs.h>
#include <tt/fs/File.h>
#include <tt/doc/str/StringDocument.h>
#include <tt/str/format.h>
#include <tt/str/common.h>
#include <tt/system/Language.h>

namespace tt {
namespace engine {
namespace file {


FileUtils* FileUtils::ms_instance = 0;


const char* getFileTypeExtension(FileType p_type)
{
	switch (p_type)
	{
	case FileType_Object:     return "eso";
	case FileType_Texture:    return "etx";
	case FileType_Animation:  return "ean";
	case FileType_Material:   return "ema";
	case FileType_Scene:      return "esc";
	case FileType_Layer:      return "elr";
	case FileType_Font:       return "eft";
	case FileType_Palette:    return "epl";
	case FileType_Shader:     return "esh";
	
	default: 
		TT_PANIC("Unknown file type %d", p_type);
		return "";
	}
}


bool FileUtils::createInstance()
{
	if(ms_instance == 0)
	{
		ms_instance = new FileUtils;
	}
	if (ms_instance == 0)
	{
		TT_PANIC("Failed to initialize FileUtils.");
		return false;
	}
	return true;
}


void FileUtils::destroyInstance()
{
	delete ms_instance;
	ms_instance = 0;
}


bool FileUtils::exists(const engine::EngineID& p_id, FileType p_type)
{
	std::string path(m_fileRoot + m_namespaces[p_id.crc2]);
	path += getFilename(p_id, p_type);

	return fs::fileExists(path);
}


fs::FilePtr FileUtils::getDataFile(const engine::EngineID& p_id, FileType p_type)
{
	// Prefix with namespace
	std::string filePath = m_fileRoot + m_namespaces[p_id.crc2] + getFilename(p_id, p_type);
	
#if !defined(TT_BUILD_FINAL)
	if (m_showLoadedFiles)
	{
#if defined(TT_PLATFORM_WIN)
		HANDLE promptHandle = GetStdHandle(STD_OUTPUT_HANDLE);
		CONSOLE_SCREEN_BUFFER_INFO info;
		GetConsoleScreenBufferInfo(promptHandle, &info);
		SetConsoleTextAttribute(promptHandle, FOREGROUND_BLUE | FOREGROUND_INTENSITY);
#endif
		TT_Printf("-> %s.%s (%s)\n", p_id.getNamespace().c_str(), p_id.getName().c_str(), filePath.c_str());
#if defined(TT_PLATFORM_WIN)
		SetConsoleTextAttribute(promptHandle, info.wAttributes);
#endif
	}
#endif
	
	// Check for existence
	if(fs::fileExists(filePath) == false)
	{
		static const std::string langRoot("lang/");
		
		std::string newNamespacePath(langRoot + tt::system::Language::getLanguage() + "/" + m_namespaces[p_id.crc2]);
		std::string newNamespace(newNamespacePath);
		
		str::replace(newNamespace, "/", ".");
		if (str::endsWith(newNamespace, "."))
		{
			newNamespace = newNamespace.substr(0, newNamespace.size() - 1);
		}
		
		engine::EngineID newID(p_id.crc1, file::FileUtils::getInstance()->getCRC(newNamespace, 1));
		
		// Try loading localized file.
		filePath = m_fileRoot + newNamespacePath + getFilename(newID, p_type);
		if (fs::fileExists(filePath) == false)
		{
			// fall back on 'en'. (FIXME: Remove hardcoded fall back languages
			newNamespacePath = langRoot + "en/" + m_namespaces[p_id.crc2];
			newNamespace = newNamespacePath;
			
			str::replace(newNamespace, "/", ".");
			if (str::endsWith(newNamespace, "."))
			{
				newNamespace = newNamespace.substr(0, newNamespace.size() - 1);
			}
			
			newID.crc2 = file::FileUtils::getInstance()->getCRC(newNamespace, 1);
			
			filePath = m_fileRoot + newNamespacePath + getFilename(newID, p_type);
			if (fs::fileExists(filePath) == false)
			{
				// ResourceCache will issue a more clearer warning
				//TT_WARN("File does not exist: %s\n", filename.c_str());
				return fs::FilePtr();
			}
		}
	}

	// Open the file
	fs::FilePtr file(fs::open(filePath, tt::fs::OpenMode_Read));
	TT_ASSERTMSG(file != 0, "Error opening file '%s'", filePath.c_str());
	
	return file;
}


std::string FileUtils::getFilename(const engine::EngineID& p_id, FileType p_type)
{
	return p_id.toString() + "." + getFileTypeExtension(p_type);
}


tt::fs::time_type FileUtils::getLastWriteTime(const EngineID& p_id, FileType p_type)
{
	// Prefix with namespace
	std::string filePath = m_fileRoot + m_namespaces[p_id.crc2] + getFilename(p_id, p_type);
	fs::FilePtr file(fs::open(filePath, tt::fs::OpenMode_Read));
	if (file != 0)
	{
		return file->getWriteTime();
	}
	return -1;
}


void FileUtils::generateNamespaceMapping()
{
	// Because the asset monitor can consume a lot of time on the generation of the namespaces, try to load the 
	// ~ version of the namespace file, if it exists. The ~ file is the most recent copy. It also means the conversion is still in progress,
	// causing the namespace mapping to be incomplete. It's better to load the copy then. When the ~ file doesn't exist load the normal version
#if !defined(TT_BUILD_FINAL)
	std::string filename(m_fileRoot + "namespace.txt~");
	if (fs::fileExists(filename) == false)
	{
		filename = m_fileRoot + "namespace.txt";
	}
	else if (m_namespaces.empty() == false)
	{
		TT_WARN("Skipping loading of namespace file. Conversion in progress.");
		return;
	}
	else
	{
		TT_WARN("Loading backup namespace file");
	}
#else
	const std::string filename(m_fileRoot + "namespace.txt");
#endif
	if (fs::fileExists(filename))
	{
		// Build the namespace mapping from file
		doc::str::StringDocument namespaces(filename);
		
		for (s32 i = 0; i < namespaces.getStringCount(); ++i)
		{
			// Get current namespace
			std::string ns(namespaces.getString(i));
			
			// Get crc-ed namespace
			u32 crc(getCRC(ns, 1));
			
			// Create output path
			str::replace(ns, ".", "/");
			ns += "/";
			
			// Add to map
			m_namespaces[crc] = ns;
		}
	}
	else
	{
		TT_PANIC("Could not find 'namespace.txt' (path '%s') in output folder root, "
		         "unable to build mapping.", filename.c_str());
	}
}


/////////////////////////////////////////////////
// Private

u32 FileUtils::getCRC(const std::string& p_string, s32 p_index)
{
	u32 crc(0);

	// CRC The name
	for (s32 i = static_cast<s32>(p_string.length() - 1); i >= 0; --i)
	{
		crc = m_CRCTable[p_index][(crc ^ p_string.at(static_cast<u32>(i))) & 0xff] ^ (crc >> 8);
	}

	return crc;
}


void FileUtils::createCRCTable(u32 p_polynomial, s32 p_index)
{
	for (u32 n = 0; n < tableEntries; ++n)
	{
		u32 c(n);
		
		for (s32 bit = 0; bit < 8; ++bit)
		{
			c = c & 1 ? (p_polynomial ^ (c >> 1)) : (c >> 1);
		}
		m_CRCTable[p_index][n] = c;
	}
}


FileUtils::FileUtils()
:
m_namespaces(),
m_fileRoot(),
m_showLoadedFiles(false)
{
	createCRCTable(0xedb88320, 0);
	createCRCTable(0x12477cdf, 1);
}

// Namespace end
}
}
}

