#include <tt/code/bufferutils.h>
#include <tt/pres/ParticleSpawner.h>
#include <tt/pres/ParticlesStack.h>
#include <tt/pres/PresentationMgr.h>
#include <tt/pres/PresentationObject.h>
#include <tt/pres/PresentationValue.h>

namespace tt {
namespace pres {


#define MAKE_VERSION(major, minor) (((major) << 8) | (minor))
#define GET_MAJOR_VERSION(version) ((version) >> 8)
#define GET_MINOR_VERSION(version) ((version) & 0xFF)

static const u16 s_version = MAKE_VERSION(1, 0);


//--------------------------------------------------------------------------------------------------
// Public member functions

ParticlesStack::ParticlesStack()
:
anim2d::StackBase<ParticleSpawner>()
{
}


ParticlesStack::ParticlesStack(const ParticlesStack& p_rhs)
:
anim2d::StackBase<ParticleSpawner>(p_rhs)
{
}


bool ParticlesStack::save( u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus ) const
{
	TT_ERR_CHAIN(bool, false, "saving particle stack");
		
	if(getBufferSize() > p_sizeOUT)
	{
		TT_ERR_AND_RETURN("Not enough space in buffer need " << getBufferSize() 
		                  << " got " << p_sizeOUT);
	}
	
	using namespace code::bufferutils;
	
	be_put(s_version, p_bufferOUT, p_sizeOUT);
	be_put(static_cast<u16>(m_allAnimations.size()), p_bufferOUT, p_sizeOUT);
	
	for(Stack::const_iterator it(m_allAnimations.begin()) ; it != m_allAnimations.end() ; ++it)
	{
		if((*it)->save(p_bufferOUT, p_sizeOUT, &errStatus) == false) 
		{
			TT_ERR_AND_RETURN("Saving of particleSpawner failed");
		}
	}
	
	return true;
}


bool ParticlesStack::load( const u8*& p_bufferOUT, size_t& p_sizeOUT, 
                           const DataTags& p_applyTags, const Tags& p_acceptedTags, 
                           const PresentationObjectPtr& p_object, 
                           u32 p_particleCategory,  
                           code::ErrorStatus* p_errStatus )
{
	TT_ERR_CHAIN(bool, false, "loading particle stack");
	
	using namespace code::bufferutils;
	
	const u16 version = be_get<u16>(p_bufferOUT, p_sizeOUT);
	TT_ERR_ASSERTMSG(version == s_version, "Invalid version: code "
		<< GET_MAJOR_VERSION(s_version) << "." << GET_MINOR_VERSION(s_version) << ", data "
		<< GET_MAJOR_VERSION(version)   << "." << GET_MINOR_VERSION(version)   <<
		" -- please update your converter.");
	
	TT_ERR_ASSERTMSG(p_sizeOUT >= sizeof(u16), "Buffer too small.");
	const u16 particleCount = be_get<u16>(p_bufferOUT, p_sizeOUT);
	for (u16 i = 0; i < particleCount ; ++i)
	{
		ParticleSpawnerPtr spawner(ParticleSpawner::loadBin(p_bufferOUT, p_sizeOUT, p_applyTags, 
		                                                    p_acceptedTags, p_object, 
		                                                    p_particleCategory, &errStatus));
		TT_ERR_RETURN_ON_ERROR();
		
		m_allAnimations.push_back(spawner);
		TT_ASSERT(m_activeAnimations.empty());
	}
	
	return true;
}


void ParticlesStack::stopParticles(bool p_kill)
{
	for(Stack::iterator it(m_activeAnimations.begin()) ; it != m_activeAnimations.end() ; ++it)
	{
		(*it)->stopParticles(p_kill);
	}
}


size_t ParticlesStack::getBufferSize() const
{
	size_t size(2 + 2); // number of animations + version
	for(Stack::const_iterator it(m_allAnimations.begin()) ; it != m_allAnimations.end() ; ++it)
	{
		size += (*it)->getBufferSize();
	}
	
	return size;
}


void ParticlesStack::appendStack(ParticlesStack& p_other)
{
	// just append the whole stack to the end
	m_allAnimations.insert(m_allAnimations.end(), p_other.m_allAnimations.begin(), p_other.m_allAnimations.end());
	p_other.m_allAnimations.clear();
	TT_ASSERT(p_other.m_activeAnimations.empty());
}


engine::renderer::TextureContainer ParticlesStack::getAndLoadAllUsedTextures() const
{
	using namespace engine::renderer;
	TextureContainer textures;
	for(Stack::const_iterator it(m_allAnimations.begin()) ; it != m_allAnimations.end() ; ++it)
	{
		TextureContainer temp((*it)->getAndLoadAllUsedTextures());
		textures.insert(textures.end(), temp.begin(), temp.end());
	}
	return textures;
}


bool ParticlesStack::isVisible() const
{
	for(Stack::const_iterator it(m_activeAnimations.begin()) ; it != m_activeAnimations.end() ; ++it)
	{
		const ParticleSpawnerPtr& ptr = *it;
		if (ptr->isVisible() && ptr->isActive()) return true;
	}
	return false;
}


//--------------------------------------------------------------------------------------------------
// Private member functions

void ParticlesStack::setFollowObject(const PresentationObjectPtr& p_followObject)
{
	for (Stack::iterator it(m_allAnimations.begin()) ; it != m_allAnimations.end() ; ++it)
	{
		(*it)->setFollowObject(p_followObject);
	}
}


void ParticlesStack::setFlipMask(u32 p_flipMask)
{
	for (Stack::iterator it(m_allAnimations.begin()) ; it != m_allAnimations.end() ; ++it)
	{
		(*it)->setFlipMask(p_flipMask);
	}
}


//namespace end
}
}
