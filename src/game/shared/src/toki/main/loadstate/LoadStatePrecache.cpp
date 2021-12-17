#include <tt/engine/particles/ParticleTrigger.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/fs/utils/utils.h>
#include <tt/fs/Dir.h>
#include <tt/fs/DirEntry.h>
#include <tt/fs/fs.h>
#include <tt/platform/tt_printf.h>
#include <tt/str/str.h>
#include <tt/system/Time.h>

#include <toki/game/entity/graphics/PowerBeamGraphic.h>
#include <toki/game/fluid/graphics_helpers.h>
#include <toki/game/movement/MovementSet.h>
#include <toki/game/types.h>
#include <toki/level/skin/SkinConfig.h>
#include <toki/main/loadstate/LoadStatePrecache.h>
#include <toki/AppGlobal.h>
#include <toki/cfg.h>


namespace toki {
namespace main {
namespace loadstate {

// Set this to 1 to enable timing of the precache loading steps
#define TT_ENABLE_PRECACHE_TIMING 0


//--------------------------------------------------------------------------------------------------
// Public member functions

std::string LoadStatePrecache::getName() const
{
	if (m_filenamesParticles.empty() == false)
	{
		return "Precache - Particles";
	}
	else if (m_filenamesPresentation.empty() == false)
	{
		return "Precache - Presentation";
	}
	else if (m_filenamesMovementSet.empty() == false)
	{
		return "Precache - Movement Sets";
	}
	else if (m_skinConfigs != level::skin::SkinConfigType_Count)
	{
		return "Precache - Skin Textures";
	}
	else if (m_miscTextures.empty() == false)
	{
		return "Precache - Misc Textures";
	}
	else if (m_namespaceTextures.empty() == false)
	{
		return "Precache - Namespace Textures";
	}
	else
	{
		return "Precache";
	}
}


s32 LoadStatePrecache::getEstimatedStepCount() const
{
	return static_cast<s32>(m_filenamesParticles.size()    +
	                        m_filenamesPresentation.size() +
	                        m_filenamesMovementSet.size()  +
	                        level::skin::SkinConfigType_Count +
	                        m_miscTextures.size()          +
	                        m_namespaceTextures.size());
}


void LoadStatePrecache::doLoadStep()
{
	// NOTE: Possible loading speed improvement: continue loading files until a predetermined
	//       amount of time was taken. This way, loads taking less than one "frame time"
	//       don't waste the rest of the time for that frame.
	//       This does require StateLoadApp::update to be changed, so that LoadState::doLoadStep
	//       can indicate it performed more than one step.
	
	// Particles
	if (m_filenamesParticles.empty() == false)
	{
		const std::string& filename(m_filenamesParticles.front());
		
#if TT_ENABLE_PRECACHE_TIMING
		const u64 loadStart = tt::system::Time::getInstance()->getMilliSeconds();
#endif
		
		using namespace tt::engine;
		particles::ParticleTrigger trigger(static_cast<particles::WorldObject*>(0), 0);
		trigger.load(filename);
		renderer::TextureContainer textures = trigger.getAndLoadAllUsedTextures();
		AppGlobal::addTexturesToPrecache(textures);
		
#if TT_ENABLE_PRECACHE_TIMING
		const u64 loadEnd = tt::system::Time::getInstance()->getMilliSeconds();
		TT_Printf("LoadStatePrecache::doLoadStep: [Particle    ] [%4u ms] '%s'\n", u32(loadEnd - loadStart), filename.c_str());
#endif
		
		m_filenamesParticles.erase(m_filenamesParticles.begin());
		return;
	}
	
	// Presentation files
	if (m_filenamesPresentation.empty() == false)
	{
		const std::string& filename(m_filenamesPresentation.front());
		
#if TT_ENABLE_PRECACHE_TIMING
		const u64 loadStart = tt::system::Time::getInstance()->getMilliSeconds();
#endif
		
		AppGlobal::addPresentationToPrecache(filename);
		
#if TT_ENABLE_PRECACHE_TIMING
		const u64 loadEnd = tt::system::Time::getInstance()->getMilliSeconds();
		TT_Printf("LoadStatePrecache::doLoadStep: [Presentation] [%4u ms] '%s'\n", u32(loadEnd - loadStart), filename.c_str());
#endif
		
		m_filenamesPresentation.erase(m_filenamesPresentation.begin());
		return;
	}
	
	// Movement sets
	if (m_filenamesMovementSet.empty() == false)
	{
		const std::string& filename(m_filenamesMovementSet.front());
		
#if TT_ENABLE_PRECACHE_TIMING
		const u64 loadStart = tt::system::Time::getInstance()->getMilliSeconds();
#endif
		
		AppGlobal::addMovementSetToPrecache(game::movement::MovementSet::create(filename));
		
#if TT_ENABLE_PRECACHE_TIMING
		const u64 loadEnd = tt::system::Time::getInstance()->getMilliSeconds();
		TT_Printf("LoadStatePrecache::doLoadStep: [MovementSet ] [%4u ms] '%s'\n", u32(loadEnd - loadStart), filename.c_str());
#endif
		
		m_filenamesMovementSet.erase(m_filenamesMovementSet.begin());
		return;
	}
	
	// Skin configs (or rather, their textures)
	if (m_skinConfigs != level::skin::SkinConfigType_Count)
	{
#if TT_ENABLE_PRECACHE_TIMING
		const u64 loadStart = tt::system::Time::getInstance()->getMilliSeconds();
#endif
		
		level::skin::SkinConfigPtr skinConfig = AppGlobal::getSkinConfig(m_skinConfigs);
		
		//AppGlobal::addTexturesToPrecache(skinConfig->getAllUsedTextures());
		
#if TT_ENABLE_PRECACHE_TIMING
		const u64 loadEnd = tt::system::Time::getInstance()->getMilliSeconds();
		TT_Printf("LoadStatePrecache::doLoadStep: [SkinConfig  ] [%4u ms] %d\n", u32(loadEnd - loadStart), m_skinConfigs);
#endif
		
		m_skinConfigs = static_cast<level::skin::SkinConfigType>(m_skinConfigs + 1);
		return;
	}
	
	// Miscellaneous textures
	if (m_miscTextures.empty() == false)
	{
		const utils::StringPair& fullAssetID(m_miscTextures.front());
		
#if TT_ENABLE_PRECACHE_TIMING
		const u64 loadStart = tt::system::Time::getInstance()->getMilliSeconds();
#endif
		
		tt::engine::renderer::TexturePtr tex = tt::engine::renderer::TextureCache::get(
				fullAssetID.first, fullAssetID.second, false);
		if (tex != 0)
		{
			AppGlobal::addTexturesToPrecache(tt::engine::renderer::TextureContainer(1, tex));
		}
		
#if TT_ENABLE_PRECACHE_TIMING
		const u64 loadEnd = tt::system::Time::getInstance()->getMilliSeconds();
		TT_Printf("LoadStatePrecache::doLoadStep: [Misc Texture] [%4u ms] '%s'\n", u32(loadEnd - loadStart), fullAssetID.first.c_str());
#endif
		
		m_miscTextures.erase(m_miscTextures.begin());
		return;
	}
	
	if (m_namespaceTextures.empty() == false)
	{
#if TT_ENABLE_PRECACHE_TIMING
		const u64 loadStart = tt::system::Time::getInstance()->getMilliSeconds();
#endif
		tt::engine::EngineID engineID(m_namespaceTextures.front());
		tt::engine::renderer::TexturePtr tex = tt::engine::renderer::TextureCache::get(engineID, false);
		if (tex != 0)
		{
			AppGlobal::addTexturesToPrecache(tt::engine::renderer::TextureContainer(1, tex));
		}
		
#if TT_ENABLE_PRECACHE_TIMING
		const u64 loadEnd = tt::system::Time::getInstance()->getMilliSeconds();
		TT_Printf("LoadStatePrecache::doLoadStep: [Misc Texture] [%4u ms] '%s'\n", u32(loadEnd - loadStart), engineID.toString().c_str());
#endif
		
		m_namespaceTextures.erase(m_namespaceTextures.begin());
		return;
	}
}


bool LoadStatePrecache::isDone() const
{
	return m_filenamesParticles.empty()             &&
	       m_filenamesPresentation.empty()          &&
	       m_filenamesMovementSet.empty()           &&
	       m_skinConfigs == level::skin::SkinConfigType_Count &&
	       m_miscTextures.empty()                   &&
	       m_namespaceTextures.empty();
}


//--------------------------------------------------------------------------------------------------
// Private member functions

LoadStatePrecache::LoadStatePrecache()
:
m_filenamesParticles(),
m_filenamesPresentation(),
m_filenamesMovementSet(),
m_skinConfigs(static_cast<level::skin::SkinConfigType>(0)),
m_miscTextures(),
m_namespaceTextures()
{
#if TT_ENABLE_PRECACHE_TIMING
	const u64 loadStart = tt::system::Time::getInstance()->getMilliSeconds();
#endif
	
	gatherFilenames("particles/",    "trigger", false, m_filenamesParticles);
	gatherFilenames("movement/",     "ttms",    false, m_filenamesMovementSet);
	
	// Precache all presentations
	gatherFilenames("presentation/", "pres", true, m_filenamesPresentation);
	// Don't precache all textures
	//gatherEngineIDs("textures" , m_namespaceTextures);
	
	m_skinConfigs = static_cast<level::skin::SkinConfigType>(0);
	
#if defined(TT_PLATFORM_WIN)
	m_miscTextures.push_back(utils::StringPair("attributeview", "textures"));
	m_miscTextures.push_back(utils::StringPair("debugview"    , "textures"));
#endif
	
	game::entity::graphics::PowerBeamGraphic::getNeededTextureIDs(&m_miscTextures);
	
	gatherEngineIDs("color_grading" , m_namespaceTextures);
	
#if TT_ENABLE_PRECACHE_TIMING
	const u64 loadEnd = tt::system::Time::getInstance()->getMilliSeconds();
	TT_Printf("LoadStatePrecache::LoadStatePrecache: Gathering filenames took %u ms\n", u32(loadEnd - loadStart));
#endif
}


void LoadStatePrecache::gatherFilenames(const std::string& p_path,
                                        const std::string& p_fileExtension,
                                        bool               p_stripExtension,
                                        tt::str::Strings&  p_filenames_OUT)
{
	if (tt::fs::dirExists(p_path) == false)
	{
		return;
	}
	
	tt::fs::DirPtr dir(tt::fs::openDir(p_path));
	if (dir == 0)
	{
		return;
	}
	
	tt::fs::DirEntry entry;
	while (dir->read(entry))
	{
		if (entry.isDirectory())
		{
			if (entry.getName() != "." && entry.getName() != "..")
			{
				gatherFilenames(p_path + entry.getName() + "/", p_fileExtension,
				                p_stripExtension, p_filenames_OUT);
			}
		}
		else if (tt::fs::utils::getExtension(entry.getName()) == p_fileExtension)
		{
			const std::string filename(p_stripExtension ? tt::fs::utils::getFileTitle(entry.getName()) : entry.getName());
			p_filenames_OUT.push_back(p_path + filename);
		}
	}
}


void LoadStatePrecache::gatherEngineIDs(const std::string&     p_namespace,
                                        tt::engine::EngineIDs& p_engineIDs_OUT)
{
	const tt::engine::file::FileType filetype = tt::engine::file::FileType_Texture;
	const char* extension = tt::engine::file::getFileTypeExtension(filetype);
	
	std::string path(p_namespace);
	tt::str::replace(path, ".", "/");
	
	if (tt::fs::dirExists(path) == false)
	{
		return;
	}
	
	tt::fs::DirPtr dir(tt::fs::openDir(path));
	if (dir == 0)
	{
		return;
	}
	
	tt::fs::DirEntry entry;
	while (dir->read(entry))
	{
		if (entry.isDirectory())
		{
			if (entry.getName() != "." && entry.getName() != "..")
			{
				gatherEngineIDs(p_namespace + "." + entry.getName(), p_engineIDs_OUT);
			}
		}
		else if (tt::fs::utils::getExtension(entry.getName()) == extension)
		{
			const std::string filename(tt::fs::utils::getFileTitle(entry.getName()));
			if (filename.length() < 16) // We're expecting a 64 bit hex number here.
			{
				TT_WARN("We expect filenames of 64 bit hex values. '%s' is not long enough to be one.",
				        filename.c_str());
				continue;
			}
			const std::string crc1Str(filename.substr(0, 8));
			const std::string crc2Str(filename.substr(8, 8));
			
			TT_ERR_CREATE("GatherEngineIDs for namespace '" << p_namespace << "'");
			const u32 crc1 = tt::str::parseU32Hex(crc1Str, &errStatus);
			const u32 crc2 = tt::str::parseU32Hex(crc2Str, &errStatus);
			if (errStatus.hasError())
			{
				TT_ERR_ASSERT_ON_ERROR();
				continue;
			}
			tt::engine::EngineID id(crc1, crc2);
			TT_ASSERTMSG(tt::engine::file::FileUtils::getInstance()->exists(id, filetype),
			             "The EngineID '%s' created for file: '%s' namespace: '%s' does not exist?",
			             id.toString().c_str(), filename.c_str(), p_namespace.c_str());
			p_engineIDs_OUT.push_back(id);
		}
	}
}


// Namespace end
}
}
}
