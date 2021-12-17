#if !defined(INC_TT_ENGINE_ANIM2D_PARTICLEANIMATION2D_H)
#define INC_TT_ENGINE_ANIM2D_PARTICLEANIMATION2D_H

#include <tt/engine/anim2d/PositionAnimation2D.h>
#include <tt/engine/particles/fwd.h>
#include <tt/math/Vector3.h>
#include <tt/xml/fwd.h>



namespace tt {
namespace engine {
namespace anim2d {

class ParticleAnimation2D : public PositionAnimation2D
{
public:
	ParticleAnimation2D();
	virtual ~ParticleAnimation2D();
	
	virtual inline void applyTransform(Transform*) const {} // FIXME: Why does ParticleAnimation2D derive from PositionAnimation2D if it doesn't implement this function?
	
	/*! \brief Returns whether or not the animation has a Z animation.
	    \return Whether or not the animation has a Z animation.*/
	virtual bool hasZAnimation() const { return false; }
	
	/*! \brief Returns whether or not the animation has a Rotation animation.
	    \return Whether or not the animation has a Rotation animation.*/
	virtual inline bool hasRotationAnimation() const { return false; }
	
	/*! \brief Returns the sorting weight.
	    \return The sorting weight.*/
	virtual int getSortWeight() const;
	
	
	inline void setWorldObject(tt::engine::particles::WorldObject* p_object) { m_worldObject = p_object; }
	
	/*! \brief Update logic of this animation.
	    \param p_delta Time passed in seconds.*/
	virtual void update(real p_delta);
	
	/*! \brief Loads the animation from an xml node.
	    \param p_node The xml node to load from.
	    \return Whether loading succeeded or not.*/
	virtual bool load(const xml::XmlNode* p_node);
	
	/*! \brief Stores the animation to an xml node.
	    \param p_node The xml node to save to.
	    \return Whether storing succeeded or not.*/
	virtual bool save(xml::XmlNode* p_node) const;
	
	/*! \brief Loads the animation from a file.
	    \param p_file The file to load from.
	    \return Whether loading succeeded or not.*/
	virtual bool load(const fs::FilePtr& p_file);
	
	/*! \brief Stores the animation to a file.
	    \param p_file The file to save to.
	    \return Whether storing succeeded or not.*/
	virtual bool save(const fs::FilePtr& p_file) const;
	
	/*! \brief Loads the animation from memory.
	    \param p_bufferOUT The buffer to load from, will be updated.
	    \param p_sizeOUT The size of the buffer, will be updated
	    \return Whether loading succeeded or not.*/
	virtual bool load(const u8*& p_bufferOUT, size_t& p_sizeOUT);
	
	/*! \brief Save the animation to memory.
	    \param p_bufferOUT The buffer to save to, will be updated.
	    \param p_sizeOUT The size of the buffer, will be updated
	    \return Whether saving succeeded or not.*/
	virtual bool save(u8*& p_bufferOUT, size_t& p_sizeOUT) const;
	
	/*! \brief Returns the size of the buffer needed to load or save this animation.
	    \return The size of the buffer.*/
	virtual size_t getBufferSize() const;
	
	/*! \brief Returns the type of the animation
	    \return AnimationType */ 
	virtual AnimationType getAnimationType() const 
	{ 
		return PositionAnimation2D::AnimationType_Particles; 
	}
	
	using Animation2D::load; // prevent hiding of Tags overload of 'load'
	
	/*! \brief Returns a clone of the position animation object.*/
	inline virtual ParticleAnimation2D* clone() const { return new ParticleAnimation2D(*this); }
	
private:
	virtual void setRanges();
	
	
	std::string m_particlesFile;
	tt::engine::particles::WorldObject* m_worldObject;
	tt::engine::particles::ParticleTrigger* m_trigger;
	
	
	//Enable copy / disable assignment
	ParticleAnimation2D(const ParticleAnimation2D& p_rhs);
	ParticleAnimation2D& operator=(const ParticleAnimation2D&);
};


//namespace end
}
}
}

#endif // !defined(INC_TT_ENGINE_ANIM2D_PARTICLEANIMATION2D_H)
