#if !defined(INC_ASSETTOOL_CONVERSION_PARTICLE_SOURCEITEMLEVEL_H)
#define INC_ASSETTOOL_CONVERSION_PARTICLE_SOURCEITEMLEVEL_H


#include <tt/platform/tt_types.h>

#include <common/SourceItem.h>


namespace assettool {
namespace conversion {
namespace level {

class SourceItemLevel : public common::SourceItem
{
public:
	explicit SourceItemLevel(common::PlatformType p_type);
	virtual ~SourceItemLevel();
	
	virtual bool doConvert(const std::string&     p_inroot,
	                       const std::string&     p_outroot,
	                       const std::string&     p_intermediateDir,
	                       bool                   p_checkTime,
	                       std::time_t            p_converterBuildTime,
	                       tt::code::ErrorStatus* p_errStatus);
	
	inline static bool supportsParallelProcessing() { return false; }
	
private:
	bool convert(const std::string&     p_source,
	             const std::string&     p_dest,
	             bool                   p_checkTime,
	             tt::code::ErrorStatus* p_errStatus) const;
	
	void writeFileLevelMetaFile(const std::string& p_inputFile,
	                            const std::string& p_outputFile) const;
};

// Namespace end
}
}
}


#endif  // !defined(INC_ASSETTOOL_CONVERSION_PARTICLE_SOURCEITEMLEVEL_H)
