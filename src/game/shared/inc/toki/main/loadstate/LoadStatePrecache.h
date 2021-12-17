#if !defined(INC_TOKI_MAIN_LOADSTATE_LOADSTATEPRECACHE_H)
#define INC_TOKI_MAIN_LOADSTATE_LOADSTATEPRECACHE_H


#include <string>
#include <utility>
#include <vector>

#include <tt/str/str_types.h>
#include <tt/engine/fwd.h>

#include <toki/level/skin/types.h>
#include <toki/main/loadstate/LoadState.h>
#include <toki/utils/types.h>


namespace toki {
namespace main {
namespace loadstate {

class LoadStatePrecache : public LoadState
{
public:
	static inline LoadStatePtr create() { return LoadStatePtr(new LoadStatePrecache); }
	virtual ~LoadStatePrecache() { }
	
	virtual std::string getName()               const;
	virtual s32         getEstimatedStepCount() const;
	
	virtual void doLoadStep();
	virtual bool isDone() const;
	
private:
	LoadStatePrecache();
	
	static void gatherFilenames(const std::string& p_path,
	                            const std::string& p_fileExtension,
	                            bool               p_stripExtension,
	                            tt::str::Strings&  p_filenames_OUT);
	
	static void gatherEngineIDs(const std::string&     p_namespace,
	                            tt::engine::EngineIDs& p_engineIDs_OUT);
	
	
	tt::str::Strings   m_filenamesParticles;
	tt::str::Strings   m_filenamesPresentation;
	tt::str::Strings            m_filenamesMovementSet;
	level::skin::SkinConfigType m_skinConfigs;
	utils::StringPairs          m_miscTextures; // pair.first = asset ID, pair.second = asset namespace
	tt::engine::EngineIDs       m_namespaceTextures;
};

// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_MAIN_LOADSTATE_LOADSTATEPRECACHE_H)
