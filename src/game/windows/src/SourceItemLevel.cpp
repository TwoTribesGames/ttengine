#define NOMINMAX
#include <windows.h>
#include <fstream>

#include <tt/code/ErrorStatus.h>
#include <tt/compression/compression.h>
#include <tt/fs/utils/utils.h>
#include <tt/fs/File.h>
#include <tt/fs/fs.h>
#include <tt/log/Log.h>
#include <tt/str/str.h>

#include <toki/game/pathfinding/PathMgr.h>
#include <toki/level/LevelData.h>

#include "SourceItemLevel.h"


namespace assettool {
namespace conversion {
namespace level {


//--------------------------------------------------------------------------------------------------
// Public member functions

SourceItemLevel::SourceItemLevel(common::PlatformType p_type)
:
SourceItem(p_type)
{
}


SourceItemLevel::~SourceItemLevel()
{
}


bool SourceItemLevel::doConvert(const std::string&     p_inRoot,
                                const std::string&     p_outRoot,
                                const std::string&     p_interMediateDir,
                                bool                   p_checkTime,
                                tt::fs::time_type      p_converterBuildTime,
                                tt::code::ErrorStatus* p_errStatus)
{
	(void) p_interMediateDir;
	(void) p_errStatus;
	(void) p_converterBuildTime;
	
	TT_ERR_CHAIN(bool, false, "Converting Levels");
	TT_ERR_ASSERTMSG(m_inputfiles.empty() == false, "Asset has no input files.");
	TT_ERR_ASSERTMSG(m_inputfiles.size()   == 1,    "'Level' asset type only supports one input file per asset.");
	
	const std::string assetNS(getSanitizedNamespace());
	
	// Create the output directory based on the asset namespace
	makeOutRootDirectory(p_outRoot + assetNS);
	
	const std::string inputFile(getFirstInputFilePath(p_inRoot));
	const std::string outputFile(getFirstOutputFilePath(p_outRoot));
	
	if (p_checkTime && tt::fs::isUpToDate(outputFile, inputFile) &&
	    converterIsNewer(outputFile) == false)
	{
		return true;
	}
	
	return convert(inputFile, outputFile, p_checkTime, &errStatus);
}


//--------------------------------------------------------------------------------------------------
// Private member functions

bool SourceItemLevel::convert(const std::string&     p_source,
                              const std::string&     p_dest,
                              bool                   p_checkTime,
                              tt::code::ErrorStatus* p_errStatus) const
{
	if (p_checkTime                          &&
	    tt::fs::isUpToDate(p_dest, p_source) &&
	    converterIsNewer(p_dest) == false)
	{
		return true;
	}
	
	TT_ERR_CHAIN(bool, false,
	             "Convert Level '" << p_source << "' to trigger '" << p_dest << "'.");
	
	toki::level::LevelDataPtr level = toki::level::LevelData::loadLevel(p_source);
	if (level == 0)
	{
		TT_ERR_AND_RETURN("Failed to open level '" << p_source << "'");
	}

	// Build and insert tilecaches
	{
		toki::game::pathfinding::PathMgr pathMgr;
		pathMgr.recreateTileCaches(level->getAgentRadii());
		pathMgr.buildTileCaches(level->getAttributeLayer());
		pathMgr.saveTileCachesToLevelData(level);
	}

	level->save(p_dest);
	
	return true;
}


//! \brief Put a metadatafile, designating the sourcefile
void SourceItemLevel::writeFileLevelMetaFile(const std::string& p_inputFilename,
                                             const std::string& p_outputFilename) const
{
	const std::string metaFilename(p_outputFilename + ".meta");
	std::ofstream foutStrm(metaFilename.c_str());
	foutStrm << "src=" << p_inputFilename;
}

// Namespace end
}
}
}
