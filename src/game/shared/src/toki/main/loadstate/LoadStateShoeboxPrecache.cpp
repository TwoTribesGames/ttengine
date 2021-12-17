#include <tt/engine/scene2d/shoebox/fwd.h>
#include <tt/engine/scene2d/shoebox/shoebox.h>
#include <tt/fs/Dir.h>
#include <tt/fs/DirEntry.h>
#include <tt/fs/fs.h>
#include <tt/platform/tt_error.h>

#include <toki/main/loadstate/LoadStateShoeboxPrecache.h>
#include <toki/AppGlobal.h>


namespace toki {
namespace main {
namespace loadstate {

//--------------------------------------------------------------------------------------------------
// Public member functions

s32 LoadStateShoeboxPrecache::getEstimatedStepCount() const
{
	return static_cast<s32>(m_filenamesShoeboxes.size());
}


void LoadStateShoeboxPrecache::doLoadStep()
{
	if (m_filenamesShoeboxes.empty() == false)
	{
		const std::string& filename(m_filenamesShoeboxes.front());
		
		tt::engine::scene2d::shoebox::ShoeboxPtr result;
		
		if (tt::fs::fileExists(filename))
		{
			using tt::engine::scene2d::shoebox::Shoebox;
			result.reset(new Shoebox);
			if (result->load(filename, 1, 1, 1.0f, tt::math::Vector3::zero, 0, false))
			{
				using tt::engine::renderer::EngineIDToTextures;
				EngineIDToTextures usedTextures(result->getAllUsedTextures(true));
				
				tt::engine::renderer::TextureContainer texturesToCache;
				texturesToCache.reserve(usedTextures.size());
				for (EngineIDToTextures::iterator it = usedTextures.begin();
				     it != usedTextures.end(); ++it)
				{
					texturesToCache.push_back((*it).second);
				}
				
				AppGlobal::addTexturesToPrecache(texturesToCache);
			}
			else
			{
				TT_WARN("Precaching shoebox '%s' failed (load of shoebox failed).", filename.c_str());
			}
		}
		else
		{
			TT_PANIC("Trying to precache non-existing shoebox '%s'", filename.c_str());
		}
		
		m_filenamesShoeboxes.erase(m_filenamesShoeboxes.begin());
		return;
	}
}


bool LoadStateShoeboxPrecache::isDone() const
{
	return m_filenamesShoeboxes.empty();
}

//--------------------------------------------------------------------------------------------------
// Private member functions

LoadStateShoeboxPrecache::LoadStateShoeboxPrecache()
:
m_filenamesShoeboxes()
{
	const std::string rootPath("levels/");
	
	tt::fs::DirPtr dir(tt::fs::openDir(rootPath, "menu_*.shoebox"));
	if (dir == 0)
	{
		return;
	}
	
	tt::fs::DirEntry entry;
	while (dir->read(entry))
	{
		if (entry.isDirectory() == false)
		{
			m_filenamesShoeboxes.push_back(rootPath + entry.getName());
			//TT_Printf("SHOEBOX  %s\n", (*m_filenamesShoeboxes.rbegin()).c_str());
		}
	}
}

// Namespace end
}
}
}
