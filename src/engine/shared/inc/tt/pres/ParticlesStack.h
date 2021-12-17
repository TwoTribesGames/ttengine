#if !defined(INC_TT_PRES_PARTICLESSTACK_H)
#define INC_TT_PRES_PARTICLESSTACK_H
#include <vector>

#include <tt/code/ErrorStatus.h>
#include <tt/pres/anim2d/StackBase.h>
#include <tt/pres/fwd.h>
#include <tt/pres/ParticleSpawner.h>


namespace tt {
namespace pres {

class ParticlesStack : public anim2d::StackBase<ParticleSpawner>
{
public:
	ParticlesStack();
	virtual ~ParticlesStack(){}
	ParticlesStack(const ParticlesStack& p_rhs);
	
	inline void push_back(const ParticleSpawnerPtr& p_spawner)
	{
		m_allAnimations.push_back(p_spawner);
		TT_ASSERT(m_activeAnimations.empty());
	}
	
	/*! \brief Saves the stack to a buffer.*/
	virtual bool save( u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus ) const;
	
	/*! \brief Loads the particle stack from a buffer.
	    \param p_bufferOUT The buffer to load from, will be updated.
	    \param p_sizeOUT The size of the buffer, will be updated
	    \param p_applyTags tags that all loaded animations should be applied to
	    \param p_acceptedTags List of accepted tags where loaded tags should be checked against.
	    \param p_object PresentationObject this stack is created with.
	    \param p_particleCategory Category to add the particles to.
	    \return Whether loading succeeded or not.*/
	bool load( const u8*& p_bufferOUT, size_t& p_sizeOUT, 
	           const DataTags& p_applyTags, const Tags& p_acceptedTags, 
	           const PresentationObjectPtr& p_object, 
	           u32 p_particleCategory, 
	           code::ErrorStatus* p_errStatus);
	
	/* \brief Stops the spawning of all the particles. Should be called together with stop and reset.*/
	void stopParticles(bool p_kill = false);
	
	/*! \brief Returns the size of the buffer needed to load or save this stack.
	    \return The size of the buffer.*/
	virtual size_t getBufferSize() const;
	
	/*! \brief adds two stacks together. .*/
	void appendStack(ParticlesStack& p_other);
	
	/*! \brief Loads all the texures for the particles in this stack. And returns them.
	        used for precaching the textures. */ 
	engine::renderer::TextureContainer getAndLoadAllUsedTextures() const;
	
	bool isVisible() const;
	
private:
	ParticlesStack& operator=(const ParticlesStack&); // Disable assignment
	
	//FIXME: The presentation loader does this on creation, this goes wrong with cloning
	friend class PresentationLoader;
	friend class PresentationObject;
	void setFollowObject(const PresentationObjectPtr& p_followObject);
	
	void setFlipMask(u32 p_flipMask);
	
	// Will set startscale to all ParticleSpawners in this stack
	// only used by PresentationLoader
	void setStartScale(const PresentationValue& p_scale);
};


//namespace end
}
}

#endif // !defined(INC_TT_PRES_PARTICLESSTACK_H)
