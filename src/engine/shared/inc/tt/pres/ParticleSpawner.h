#if !defined(INC_TT_PRES_PARTICLESPAWNER_H)
#define INC_TT_PRES_PARTICLESPAWNER_H


#include <string>

#include <tt/pres/TriggerBase.h>
#include <tt/engine/particles/fwd.h>
#include <tt/engine/particles/WorldObject.h>
#include <tt/engine/renderer/fwd.h>
#include <tt/math/Vector3.h>
#include <tt/pres/fwd.h>


namespace tt {
namespace pres {


class ParticleSpawner : public TriggerBase, public engine::particles::WorldObject
{
public:
	enum PositionType
	{
		PositionType_FollowOffset,  //!< Follow the presentation object with specified Offset.
		PositionType_RelativeOffset,//!< Spawn relative to current position. No follow.
		PositionType_WorldPosition, //!< Spawn in world position.
		
		PositionType_Invalid        //!< Invalid PositionType.
	};
	
	virtual ~ParticleSpawner() { }
	
	/*! \brief creates and loads a Particlespawner from buffer */
	static ParticleSpawnerPtr loadBin(const u8*& p_bufferOUT, size_t& p_sizeOUT,
	                                  const DataTags& p_applyTags, 
	                                  const Tags& p_acceptedTags, 
	                                  const PresentationObjectPtr& p_followObject,
	                                  u32 p_particleCategory,
	                                  code::ErrorStatus* p_errStatus);
	
	/*! \brief Loads the particlespawner from an xml node.
	    \param p_node The xml node to load from.
	    \param p_applyTags tags that the particlespawner should be applied to
	    \param p_acceptedTags List of accepted tags where loaded tags should be checked against.
	    \param p_object PresentationObject this stack is created with.
	    \param p_particleCategory Category to add the particles to.
	    \return Whether loading succeeded or not.*/
	static ParticleSpawnerPtr loadXml(const xml::XmlNode* p_node, const DataTags& p_applyTags, 
	                                  const Tags& p_acceptedTags, 
	                                  const PresentationObjectPtr& p_object, 
	                                  u32 p_particleCategory,
	                                  code::ErrorStatus* p_errStatus);
	
	virtual math::Vector3 getPosition() const;
	virtual real          getScale() const;
	inline virtual real   getScaleForParticles() const { return getScale(); }
	virtual bool          isCulled() const;
	
	virtual int getSortWeight() const;
	
	virtual void trigger();
	virtual void presentationEnded();
	virtual void reTrigger();
	
	bool isVisible() const;
	
	/*! \brief Saves the spawner to a buffer.*/
	bool save(u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus);
	
	void stopParticles(bool p_kill);
	
	/*! \brief Returns the size of the buffer needed to load or save this stack.
	    \return The size of the buffer.*/
	virtual size_t getBufferSize() const;
	
	/*! \brief Returns a clone of the particle spawner object.*/
	inline virtual ParticleSpawner* clone() const { return new ParticleSpawner(*this); }
	
	/*! \brief Creates and returns the texture used by this particle.*/
	engine::renderer::TextureContainer getAndLoadAllUsedTextures() const;
	
	static void setTrySubDir(const std::string& p_subDir) { ms_trySubDir = p_subDir; }
	
	void setFlipMask(u32 p_flipMask);
	
private:
	ParticleSpawner(const TriggerInfo&           p_triggerInfo,
	                const std::string&           p_triggerFile, 
	                PositionType                 p_positionType,
	                const PresentationValue&     p_positionOffsetX,
	                const PresentationValue&     p_positionOffsetY,
	                const PresentationValue&     p_positionOffsetZ,
	                const PresentationValue&     p_scale,
	                const PresentationObjectPtr& p_followObject,
	                u32 p_particleCategory, s32 p_renderGroup, const std::string& p_renderGroupName);
	ParticleSpawner(const ParticleSpawner& p_rhs);
	
	// Disable assignment
	ParticleSpawner& operator=(const ParticleSpawner&);
	
	virtual void setRanges(PresentationObject* p_presObj);
	
	/*! \brief Calculates the world position to spawn particles at, based on the follow object's PRS matrix. */
	math::Vector3 getWorldPositionWithFollowObject() const;
	
	//FIXME: The presentation loader does this on creation, this goes wrong with cloning
	friend class ParticlesStack;
	void setFollowObject(const PresentationObjectPtr& p_followObject);
	
	inline math::Vector3 getPositionOffset() const
	{
		return math::Vector3(m_positionOffsetX, m_positionOffsetY, m_positionOffsetZ);
	}
	
	//FIXME: Uncomment this when particles have implemented weakPtrs instead of RawPtrs
	//ParticleSpawnerWeakPtr              m_this;
	
	std::string                          m_triggerFile;
	
	PresentationValue                    m_positionOffsetX;
	PresentationValue                    m_positionOffsetY;
	PresentationValue                    m_positionOffsetZ;
	PositionType                         m_positionType;
	PresentationValue                    m_scale;
	
	engine::particles::ParticleEffectPtr m_particleEffect;
	u32                                  m_particleCategory;
	s32                                  m_renderGroup;
	std::string                          m_renderGroupName;
	PresentationObjectWeakPtr            m_followObject;
	TriggerInfo                          m_triggerInfo;
	u32                                  m_flipMask;
	bool                                 m_useObjectFlip; // Should this particle flip if the parent object flips. (Can also been seen as a enable flip flag.)
	
	static std::string ms_trySubDir; // Try finding particle file in this subdir and load it using that if possbile.
	                                 // Used for low, medium and high devices performance modes.
};

// Namespace end
}
}


#endif  // !defined(INC_TT_PRES_PARTICLESPAWNER_H)
