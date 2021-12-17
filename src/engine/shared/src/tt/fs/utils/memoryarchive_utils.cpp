#include <tt/code/Buffer.h>
#include <tt/code/BufferReadContext.h>
#include <tt/code/bufferutils.h>
#include <tt/code/BufferWriteContext.h>
#include <tt/compression/fastlz.h>
#include <tt/compression/lzma.h>
#include <tt/fs/utils/memoryarchive_utils.h>
#include <tt/fs/utils/utils.h>
#include <tt/fs/Dir.h>
#include <tt/fs/DirEntry.h>
#include <tt/fs/File.h>
#include <tt/fs/fs.h>
#include <tt/platform/tt_printf.h>
#include <tt/str/str.h>
#include <tt/xml/XmlDocument.h>
#include <tt/xml/XmlNode.h>


namespace tt {
namespace fs {
namespace utils {


static std::string makeAbsolutePath(const std::string p_relativePath)
{
	return fs::utils::makeCorrectFSPath(fs::getWorkingDir() + p_relativePath);
}

static void makeDirectoryTreeForOutput(const fs::MemoryArchive::File& p_file,
                                       const std::string& p_outputPath)
{
	str::Strings directories(str::explode(p_file.path, "/"));
	directories.pop_back();	// last entry is no directory but the file itself
	
	std::string fullPath(p_outputPath);
	for (str::Strings::const_iterator it = directories.begin(); it != directories.end(); ++it)
	{
		fullPath += fs::getDirSeparator() + (*it);
		if (fs::dirExists(fullPath) == false)
		{
			fs::createDir(fullPath);
		}
	}
}


static void scanFolder(const std::string& p_relativePath, const std::string& p_rootPath,
                       const std::string& p_fileExtension, bool p_recursive, s32 p_sizeLimit, const tt::str::Strings& p_exclude,
                       str::StringSet& p_filenames_OUT)
{
	const std::string relativePath = str::trim(p_relativePath, "\\/");
	const std::string fullPath(fs::utils::makeCorrectFSPath(fs::getWorkingDir() + p_rootPath + 
		                       fs::getDirSeparator() + relativePath));
	
	if (fs::dirExists(fullPath) == false)
	{
		return;
	}
	
	fs::DirPtr dir(fs::openDir(fullPath));
	if (dir == 0)
	{
		return;
	}
	
	fs::DirEntry entry;
	while (dir->read(entry))
	{
		const std::string& name(entry.getName());
		
		if (std::find(p_exclude.begin(), p_exclude.end(), name) != p_exclude.end())
		{
			continue;
		}
		
		const std::string curEntryPath(relativePath + fs::getDirSeparator() + name);
		
		if (entry.isDirectory() && p_recursive)
		{
			if (name != "." && name != "..")
			{
				scanFolder(curEntryPath, p_rootPath, p_fileExtension, p_recursive, p_sizeLimit, p_exclude,
				           p_filenames_OUT);
			}
		}
		else if (p_fileExtension.empty() || fs::utils::getExtension(name) == p_fileExtension)
		{
			// Check if already added
			if (p_filenames_OUT.find(curEntryPath) != p_filenames_OUT.end())
			{
				continue;
			}
			
			if (p_sizeLimit > 0)
			{
				fs::FilePtr file = fs::open(fullPath + fs::getDirSeparator() + name,
				                            fs::OpenMode_Read);
				TT_NULL_ASSERT(file);
				if (file == 0 || file->getLength() > p_sizeLimit)
				{
					continue;
				}
			}
			
			p_filenames_OUT.insert(curEntryPath);
		}
	}
}


bool MemoryArchivePack(const std::string& p_configFile,
                       const std::string& p_inputPath,
                       const std::string& p_outputPath,
                       const MemoryArchivePackSettings& p_settings)
{
	// Convert all relative paths to absolute paths and check for their existence
	const std::string configFile = makeAbsolutePath(p_configFile);
	if (fs::fileExists(configFile) == false)
	{
		TT_Printf("Non-existing config file '%s' specified\n", configFile.c_str());
		return false;
	}
	
	const std::string inputPath = makeAbsolutePath(p_inputPath) + fs::getDirSeparator();
	if (fs::dirExists(inputPath) == false)
	{
		TT_Printf("Input directory '%s' doesn't exist", inputPath.c_str());
		return false;
	}
	
	const std::string outputPath = makeAbsolutePath(p_outputPath) + fs::getDirSeparator();
	
	// Make sure output directory exists
	if (fs::dirExists(outputPath) == false)
	{
		fs::createDir(outputPath);
		if (fs::dirExists(outputPath) == false)
		{
			TT_Printf("Cannot create output directory '%s'", outputPath.c_str());
			return false;
		}
	}
	
	if (p_settings.emptySize < 0)
	{
		TT_Printf("Size for empty files cannot be less than 0 bytes (%d was specified).\n", p_settings.emptySize);
		return false;
	}
	
	if (p_settings.verbose)
	{
		TT_Printf("Processing '%s'\n", configFile.c_str());
		TT_Printf("\n");
	}
	
	xml::XmlDocument doc(configFile);
	
	using xml::XmlNode;
	XmlNode* root = doc.getRootNode();
	
	if (root == 0)
	{
		TT_Printf("Error parsing %s.\n", configFile.c_str());
		return false;
	}
	
	if (root->getName() != "archive")
	{
		TT_Printf("%s is not a valid archive configuration file.\n",
		          configFile.c_str());
		return false;
	}
	
	// Fetch all archived filenames
	str::StringSet allFiles;
	for (XmlNode* node = root->getChild(); node != 0; node = node->getSibling())
	{
		if (node->getName() == "file")
		{
			const std::string path = node->getAttribute("path");
			if (path.empty())
			{
				TT_Printf("Missing path in file node in '%s'\n", configFile.c_str());
				return false;
			}
			
			const std::string fileExt   = node->getAttribute("file_extension");
			const bool        recursive = str::parseBool(node->getAttribute("recursive"), 0);
			const s32         sizeLimit = node->hasAttribute("size_limit") ? 
				str::parseS32(node->getAttribute("size_limit"), 0) : 0;
			const tt::str::Strings exclude = tt::str::explode(node->getAttribute("exclude"),";");
			
			scanFolder(path, p_inputPath, fileExt, recursive, sizeLimit, exclude, allFiles);
		}
		else
		{
			TT_Printf("Parsing '%s' failed: unknown node %s.\n",
			          configFile.c_str(),
			          node->getName().c_str());
			return false;
		}
	}
	
	// Add all files to archive
	fs::MemoryArchivePtr archive(new fs::MemoryArchive());
	for (str::StringSet::const_iterator it = allFiles.begin(); it != allFiles.end(); ++it)
	{
		archive->addFile((*it), p_inputPath);
	}
	
	// Construct archive path
	const std::string archiveName = fs::utils::getFileTitle(configFile);
	const std::string archivePath = outputPath + archiveName + ".ma";
	
	// Save archive
	bool result = archive->save(archivePath, p_settings.compressionType, p_settings.alignment);
	
	// Delete originals
	if (p_settings.deleteOriginals)
	{
		for (str::StringSet::const_iterator it = allFiles.begin(); it != allFiles.end(); ++it)
		{
			const std::string fullFilePath(inputPath + (*it));
			fs::destroyFile(fullFilePath);
		}
	}
	// Empty originals
	else if (p_settings.emptyOriginals)
	{
		code::BufferPtrForCreator dataForEmptyFile;
		if (p_settings.emptySize > 0)
		{
			dataForEmptyFile.reset(new code::Buffer(p_settings.emptySize));
			mem::fill8(dataForEmptyFile->getData(), 0, dataForEmptyFile->getSize());
		}
		
		for (str::StringSet::const_iterator it = allFiles.begin(); it != allFiles.end(); ++it)
		{
			const std::string fullFilePath(inputPath + (*it));
			
			// Get write time
			time_type writeTime = 0;
			{
				fs::FilePtr file = fs::open(fullFilePath, fs::OpenMode_Read);
				if (file == 0)
				{
					TT_Printf("Could not empty file '%s'\n", fullFilePath.c_str());
					return false;
				}
				writeTime = file->getWriteTime();
			}
			
			// Set write time
			fs::FilePtr file = fs::open(fullFilePath, fs::OpenMode_Write);
			if (file == 0)
			{
				TT_Printf("Could not empty file '%s'\n", fullFilePath.c_str());
				return false;
			}
			
			if (dataForEmptyFile != 0)
			{
				file->write(dataForEmptyFile->getData(), dataForEmptyFile->getSize());
			}
			
			file->setWriteTime(writeTime);
		}
	}
	return result;
}


bool MemoryArchiveUnpack(const std::string& p_archiveFile,
                         const std::string& p_outputPath,
                         const MemoryArchiveUnpackSettings& p_settings)
{
	// Convert all relative paths to absolute paths and check for their existence
	const std::string archiveFilename = makeAbsolutePath(p_archiveFile);
	if (fs::fileExists(archiveFilename) == false)
	{
		TT_Printf("Non-existing archive file '%s' specified\n", archiveFilename.c_str());
		return false;
	}
	
	const std::string outputPath = makeAbsolutePath(p_outputPath) + fs::getDirSeparator();
	
	// Make sure output directory exists
	if (fs::dirExists(outputPath) == false)
	{
		fs::createDir(outputPath);
		if (fs::dirExists(outputPath) == false)
		{
			TT_Printf("Cannot create output directory '%s'", outputPath.c_str());
			return false;
		}
	}
	
	if (p_settings.verbose)
	{
		TT_Printf("Processing '%s'\n", archiveFilename.c_str());
		TT_Printf("\n");
	}
	
	fs::MemoryArchivePtr archive = fs::MemoryArchive::load(archiveFilename);
	if (archive == 0)
	{
		TT_Printf("Failed to load memory archive '%s'\n", archiveFilename.c_str());
		return false;
	}
	
	// Write files to disk
	const fs::MemoryArchive::Files& files = archive->getFiles();
	for (fs::MemoryArchive::Files::const_iterator it = files.begin(); it != files.end(); ++it)
	{
		fs::MemoryArchive::File archiveFile = (*it).second;
		TT_NULL_ASSERT(archiveFile.content);
		
		// Make sure the directory structure is valid before writing the file
		makeDirectoryTreeForOutput(archiveFile, outputPath);
		const std::string outputFileName(outputPath + archiveFile.path);
		
		// If we should keep newer files, check for existing file first
		if (p_settings.keepNewer && fs::fileExists(outputFileName))
		{
			fs::FilePtr file = fs::open(outputFileName, fs::OpenMode_Read);
			if (file == 0)
			{
				TT_Printf("Failed to open existing file '%s' for reading. Skipping file.\n",
					outputFileName.c_str());
				continue;
			}
			
			const time_type lastWrite = file->getWriteTime();
			if (lastWrite > archiveFile.writeTime)
			{
				// Current file is newer; skip it
				continue;
			}
		}
		
		fs::FilePtr file = fs::open(outputFileName, fs::OpenMode_Write);
		if (file == 0)
		{
			TT_Printf("Failed to open output file '%s'\n", outputFileName.c_str());
			return false;
		}
		
		if (fs::write(file, archiveFile.content->getData(), archiveFile.content->getSize()) !=
			archiveFile.content->getSize())
		{
			TT_Printf("Failed to write output file '%s'\n", outputFileName.c_str());
			return false;
		}
		
		file->setWriteTime(archiveFile.writeTime);
	}
	
	return true;
}



// Namespace end
}
}
}
