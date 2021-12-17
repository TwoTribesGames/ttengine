#include <tt/engine/effect/Effect.h>
#include <tt/engine/effect/EffectCollection.h>
#include <tt/fs/Dir.h>
#include <tt/fs/utils/utils.h>
#include <tt/script/ScriptObject.h>
#include <tt/str/str.h>


namespace tt {
namespace engine {
namespace effect {


EffectCollection::DyingEffects EffectCollection::ms_deathRow;


EffectCollection::~EffectCollection()
{
	EffectManager::unregisterCollection(this);
}


EffectPtr EffectCollection::spawn(const std::string& p_name, Group p_group)
{
	return spawn(p_name, math::Matrix44::identity, p_group);
}


EffectPtr EffectCollection::spawn(const std::string& p_name, const math::Vector3& p_position, Group p_group)
{
	return spawn(p_name, math::Matrix44::getTranslation(p_position), p_group);
}


EffectPtr EffectCollection::spawn(const std::string& p_name, const math::Matrix44& p_transform, Group p_group)
{
	// Create effect
	EffectPtr effect(new Effect(p_name, p_group), scheduleForRemove);
	
	std::string file(m_dirname + p_name + ".nut");
	
	if(fs::fileExists(file) == false)
	{
		TT_WARN("Particle script not found: %s\n", file.c_str());
	}
	else
	{
		tt::script::ScriptObject scriptObject(EffectManager::getVM()->getVM());
		
		ScriptObjects::iterator it = m_scriptCache.find(file);
		if (it == m_scriptCache.end())
		{
			TT_PANIC("Effect '%s' wasn't precached!", file.c_str());
			
			scriptObject = EffectManager::getVM()->loadScriptAsObject(file);
			m_scriptCache.insert(std::make_pair(file, scriptObject));
		}
		else
		{
			scriptObject = it->second;
		}
		if (effect->runScriptObject(scriptObject, EffectManager::getVM()) == false)
		{
			TT_PANIC("Running script '%s' failed", file.c_str());
		}
	}
	
	effect->setTransform(p_transform);
	
	m_effects[p_group].push_back(effect);

	//TT_Printf("[EFFECT] Spawning effect '%s' at position [%f, %f, %f] in group %d\n",
	//	p_name.c_str(), p_transform.m_41, p_transform.m_42, p_transform.m_43, p_group);
	
	return effect;
}


bool EffectCollection::remove(const EffectPtr& p_effect)
{
	TT_NULL_ASSERT(p_effect);

	Group group = p_effect->getGroup();
	
	for(Effects::iterator it = m_effects[group].begin(); it != m_effects[group].end(); ++it)
	{
		if ((*it) == p_effect)
		{
			// remove from list
			m_effects[group].erase(it);
			
			return true;
		}
	}
	
	// Not found, effect is already removed because it died --> No reason to panic ;-)
	//TT_PANIC("Couldn't find effect '%s'", p_effect->m_name.c_str());
	return false;
}


s32 EffectCollection::size() const
{
	s32 result(0);

	for(s32 i = 0; i < Group_Count; ++i)
	{
		result += static_cast<s32>(m_effects[i].size());
	}

	return result;
}


void EffectCollection::update(real p_deltaTime, Group p_group)
{
	for(Effects::iterator it = m_effects[p_group].begin(); it != m_effects[p_group].end(); )
	{
		if((*it)->isAlive())
		{
			(*it)->update(p_deltaTime, EffectManager::getVM());
			++it;
		}
		else
		{
			// Effect has finished!
			it = m_effects[p_group].erase(it);
		}
	}
}


void EffectCollection::render(Group p_group) const
{
	for(Effects::const_iterator it = m_effects[p_group].begin(); it != m_effects[p_group].end(); ++it)
	{
		(*it)->render();
	}
}


EffectCollectionPtr EffectCollection::load(const std::string& p_filename)
{
	return EffectCollectionPtr(new EffectCollection(p_filename));
}


void EffectCollection::updateDeathRow()
{
	for (DyingEffects::iterator it = ms_deathRow.begin(); it != ms_deathRow.end(); )
	{
		it->timeToLive--;
		if (it->timeToLive < 0)
		{
			delete it->effect;
			it = ms_deathRow.erase(it);
		}
		else
		{
			++it;
		}
	}
}


//////////////////////////////////
// Private

EffectCollection::EffectCollection(const std::string& p_dirname)
:
m_dirname(p_dirname),
m_scriptCache()
{
	m_dirname += "/";
	EffectManager::registerCollection(this);
	
	precache(m_dirname, true);
}



void EffectCollection::precache(std::string p_dirName, bool p_recursive)
{
	tt::str::replace(p_dirName, "\\", "/");
	{
		const std::string dirSeparator("/");
		
		if (tt::str::endsWith(p_dirName, dirSeparator) == false)
		{
			p_dirName += dirSeparator;
		}
	}
	
	if (tt::fs::dirExists(p_dirName) == false)
	{
		TT_PANIC("directory: '%s' not found.", p_dirName.c_str());
		return;
	}
	
	tt::fs::DirPtr dir = tt::fs::openDir(p_dirName);
	if (dir == 0)
	{
		TT_PANIC("Couldn't open dir: '%s'.", p_dirName.c_str());
		return;
	}
	

	tt::fs::DirEntry entry;
	while (dir->read(entry))
	{
		if(entry.isDirectory())
		{
			if(entry.getName() == "." || entry.getName() == ".." || p_recursive == false)
				continue;
			precache(p_dirName + entry.getName(), p_recursive);
		}
		else
		{
			std::string fileName(entry.getName());
			std::string extension(tt::fs::utils::getExtension(fileName));
			if(extension == "nut" && fileName.length() > extension.length() + 1)
			{
				std::string path(p_dirName + fileName);
				
				tt::script::ScriptObject scriptObject(EffectManager::getVM()->getVM());
				scriptObject = EffectManager::getVM()->loadScriptAsObject(path);
				
				if (scriptObject.isValid())
				{
					m_scriptCache.insert(std::make_pair(path, scriptObject));
				}
				else
				{
					TT_PANIC("EffectCollection::precache - Failed to load effect: '%s'\n", path.c_str());
				}
			}
		}
	}
}


void EffectCollection::scheduleForRemove(Effect* p_effect)
{
	ms_deathRow.push_back(DyingEffect(p_effect));
}


// Namespace end
}
}
}
