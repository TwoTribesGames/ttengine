#if !defined(INC_TT_PRES_PRESENTATIONOBJECT_H)
#define INC_TT_PRES_PRESENTATIONOBJECT_H


#include <tt/pres/anim2d/fwd.h>
#include <tt/pres/anim2d/PositionAnimation2D.h>
#include <tt/engine/renderer/fwd.h>
#include <tt/engine/renderer/ColorRGBA.h>
#include <tt/math/Matrix44.h>
#include <tt/math/Quaternion.h>
#include <tt/math/Vector3.h>
#include <tt/pres/fwd.h>
#include <tt/pres/DataTags.h>
#include <tt/pres/PresentationValue.h>


#include <tt/pres/anim2d/AnimationStack2D.h>
#include <tt/pres/anim2d/ColorAnimationStack2D.h>
#include <tt/pres/FrameAnimationStack.h>
#include <tt/pres/ParticlesStack.h>
#include <tt/pres/TriggerStack.h>
#include <tt/pres/TimerStack.h>


namespace tt {
namespace pres {

struct DependencyData
{
	std::string type;
	std::string asset;
	bool        isLightMask;
	
	inline DependencyData()
	:
	type(),
	asset(),
	isLightMask(false)
	{ }
};


class PresentationObject
{
public:
	typedef std::vector<PresentationQuadPtr> OverlayQuads;
	typedef void (*removeFunction)(PresentationObject*);
	typedef std::map<std::string, real> CustomPresentationValues;
	
	~PresentationObject();
	
	/*! \brief Gets the current position of the presentationObject WITHOUT anim2d applied. */
	inline const math::Vector3& getPositionClean() const { return m_position; }
	
	/*! \brief Gets the current world position of the presentationObject with the anim2d transform applied.  */
	inline const math::Vector3& getWorldPosition() const { return m_worldPosition; }
	
	/*! \brief Sets the position of the presentation Object before the anim2d transform
	    \param p_position  Position to set to */ 
	void setPosition(const math::Vector3& p_position);
	
	/*! \brief Sets the world position where this Presentation object should start moving to 
	           when start is called. Can not be called when animation is active.
	    \param p_position  */ 
	void setEndPosition(const math::Vector3& p_position);
	
	/*! \brief Sets the rotation to use for the presentation object before the anim2d transform. */
	void setRotation(const math::Quaternion& p_rotation);
	
	/*! \brief Retrieves the rotation to use for the presentation object before the anim2d transform. */
	inline const math::Quaternion& getRotation() const { return m_rotation; }
	
	/*! \brief Sets the scale to use for the presentation object before the anim2d transform. */
	void setScale(const math::Vector3& p_scale);
	
	/*! \brief Retrieves the scale to use for the presentation object before the anim2d transform. */
	inline const math::Vector3& getScale() const { return m_scale; }
	
	/*! \brief Retrieves the Transform to apply after the anim2d transform.
	    \note Call resetCustomTransform() when the custom transform is no longer needed! */
	inline anim2d::Transform& modifyCustomTransform() { return m_anim2dStack.modifyCustomTransform(); }
	
	/*! \brief Reset the custom Transform; potentially do less work. */
	inline void resetCustomTransform() { m_anim2dStack.resetCustomTransform(); }
	
	/*! \brief Sets the root matrix interface. The matrix retrieved from this interface 
	           will be applied as root before the anim2d transform */
	inline void setRootMatrixInterface(const RootMatrixInterfacePtr& p_rootMtx) 
			{ m_rootMtx = p_rootMtx; }
	
	/*! \brief Resets the root matrix interface pointer back to empty. */
	inline void resetRootMatrixInterface() { m_rootMtx.reset(); }
	
	/*! \brief Returns the combined matrix for the presentation object,
	           containing the anim2d matrix, the PRS matrix set by client code and the optional RootMatrix.
	           This is the matrix used for rendering the presentation object. */
	inline const math::Matrix44& getCombinedMatrix() const { return m_combinedMatrix; }
	
	/*! \brief Adds a texture to overlay over the presentation. Will be affected by anim2d and coloranims.
	    \param p_texture texture to overlay.
	    \param p_positionOffset Offset for the overlay position relative to the presentation object position. */
	void addOverlay(const engine::renderer::TexturePtr& p_texture,
	                const math::Vector3&                p_positionOffset,
	                const engine::renderer::ColorRGBA&  p_color = engine::renderer::ColorRGB::white);
	
	/*! \brief Removes an overlay from the presentation.
	    \param p_texture texture to be removed */
	void removeOverlay(const engine::renderer::TexturePtr& p_texture);
	
	/*! \brief Gets the overlay quads. */
	inline const OverlayQuads& getOverlayQuads() const { return m_overlayQuads; }
	
	/*! \brief Removes all the overlays. */
	inline void clearOverlays() { m_overlayQuads.clear(); }
	
	/*! \brief Returns whether any of the elements are active.
	           If all Stack elements are looping this will always return true.*/
	bool isActive() const;
	
	/*! \brief Returns whether any of the active elements is looping */
	bool isLooping() const;
	
	/*! \brief Returns the currently active Tags or the last Active tags. */
	inline const Tags& getCurrentActiveTags() const { return m_activeTags; }
	
	/*! \brief Returns the currently active Tags or the last Active tags. */
	inline const std::string& getCurrentActiveName() const { return m_activeName; }
	
	/*! \brief Returns whether any of the animations are started with the tag and name filtering.
	           Used for knowing if this presentation object actualy has any effect. */
	bool hasNameOrTagMatch() const;
	
	/*! \brief Returns all the tags of this Object. */
	DataTags getTags() const;
	
	/*! \brief Returns all the render pass names of this Object. */
	str::StringSet getRenderPassNames() const;
	
	/*! \brief Returns whether this object was started with the hideAtEnd flag. */
	inline bool isHidingAtEnd() const { return m_hideAtEnd; }
	
	/*! \brief Starts the animations with the specified tags.
	    \param p_tags A container of Tags the animations will be started with.
	    \param p_hideEnd Whether the presentationObject should stop being rendered when it is finished. */
	void start(const Tags& p_tags, bool p_hideEnd = true, const std::string& p_name = "");
	
	/*! \brief Starts all the animations without tags.
	    \param p_hideEnd Whether the presentationObject should stop being rendered when it is finished. */
	void start(bool p_hideEnd = true, const std::string& p_name = "");
	
	/*! \brief Stops the animation. */
	void stop();
	
	/*! \brief Kills the particles. */
	void killParticles();
	
	/*! \brief Pauses the animation. */
	void pause();
	
	/*! \brief Resumes the animation. */
	void resume();
	
	/*! \brief Unpauses and stops the animation, sets time to 0. */
	void reset();
	
	/*! \brief Resets and clears this presentation object. */
	void resetAndClear();
	
	/*! \brief Scheduled reset can be called while updating and is only done when not looping through stacks. */
	void scheduleReset(real p_updateToTime) { TT_ASSERT(m_resetTriggerTime < 0.0f); m_resetTriggerTime = p_updateToTime; }
	
	/*! \brief calls getBuffersize on all stacks and returns the sum. */
	size_t getBufferSize() const;
	
	/*! \brief Returns true if both particle and frameanim stack are invisible */
	bool isMissingFrame() const;
	
	/*! \brief Gets the asset dependencies for this presentationObject. */
	Dependencies getDependencies() const;
	
	inline void setVisible(bool p_visible) { m_visible = p_visible; }
	inline bool isVisible() const { return m_visible; }
	
	/*! \brief Updates the presentation Object.*/
	void update(real p_deltaTime);
	void render() const;
	void renderPass(const std::string& p_passName) const;
	
	/*! \brief Returns a clone of the presentation object. */
	PresentationObjectPtr clone(removeFunction p_removeFunction, PresentationMgr* p_mgr) const;
	
	/*! \brief Returns all used textures of the presentation object. */
	engine::renderer::TextureContainer getAndLoadAllUsedTextures() const;
	
	/*! \brief Adds a custom Value which will replace the specified string.
	    \param p_name The name of the custom value to replace
	    \param p_value The value to replace the string with */
	void addCustomPresentationValue(const std::string& p_name, real p_value);
	void addPresetCustomPresentationValue(const std::string& p_name, const PresentationValue& p_value);
	
	bool getCustomValue(const std::string& p_name, real* p_valueOut) const;
	
	const CustomPresentationValues& getCustomValues() const { return m_customValues; }
	
	void addSyncedTrigger(const std::string& p_syncId, TriggerInterface* p_trigger);
	void addEndSyncedTrigger(const std::string& p_syncId, TriggerInterface* p_trigger);
	
	void triggerSync   (const std::string& p_id);
	void triggerEndSync(const std::string& p_id);
	
	std::string getUniqueCustomValue();
	
	inline const PresentationMgr* getManager() const { return m_mgr; }
	
	/*! \brief Returns the current callbackInterface pointer. */
	inline const CallbackTriggerInterfacePtr& getCallbackInterface() const { return m_callbackInterface; }
	/*! \brief Sets the current callbackInterface pointer. This interface is used by callback triggers. */
	inline void setCallbackInterface( const CallbackTriggerInterfacePtr& p_callbackInterface) { m_callbackInterface = p_callbackInterface; }

	enum FlipMask
	{
		FlipMask_None       = 0x0,
		FlipMask_Horizontal = 0x1,
		FlipMask_Vertical   = 0x2
	};
	inline u32 getFlipMask() const { return m_flipMask; }
	void       setFlipMask(u32 p_mask);
	
	inline void setPermanent(bool p_isPermanent) { m_isPermanent = p_isPermanent; }
	inline bool isPermanent() const { return m_isPermanent; }
	
	inline void setDontPrecache(bool p_dontPrecache) { m_dontPrecache = p_dontPrecache; }
	inline bool dontPrecache() const { return m_dontPrecache; }
	
	inline void setInScreenSpace(bool p_isInScreenSpace) { m_isInScreenSpace = p_isInScreenSpace; }
	inline bool isInScreenSpace() const { return m_isInScreenSpace; }
	
	inline void setIsCulled(bool p_isCulled) { m_isCulled = p_isCulled; }
	inline bool isCulled() const { return m_isCulled; }
	
	inline void setFixedTimestep(real p_fixedTimestep) { m_fixedTimestep = p_fixedTimestep; }
	inline real getFixedTimestep() const { return m_fixedTimestep; }
	
	inline void setObjectGroup(const GroupInterfacePtr& p_group) { m_group = p_group; }
	inline const GroupInterfacePtr& getObjectGroup() { return m_group; }
	inline void resetObjectGroup() { m_group.reset(); }
	
private:
	friend class PresentationMgr;    // needed to access m_group
	friend class PresentationCache;  // needed for ctor
	friend class PresentationLoader;
	friend class GroupInterface;     // Needed to set m_group.
	
	
	PresentationObject(PresentationMgr* p_mgr);
	PresentationObject(const PresentationObject& p_rhs, PresentationMgr* p_mgr);
	
	inline void updateWorldPosition()
	{
		m_worldPosition.setValues(m_anim2dTransform.m_41 + m_position.x,
		                          m_anim2dTransform.m_42 + m_position.y,
		                          m_anim2dTransform.m_43 + m_position.z);
	}
	void updatePrsMatrix();
	void updateTransform();
	void updatePresetCustomValues();
	
	// Group methods
	void addToGroup(const GroupInterfacePtr& p_group);
	void removeFromGroup();
	inline bool isInGroup() const { return m_group != 0; }
	
	// Disable assignment
	PresentationObject& operator=(const PresentationObject&);
	
	typedef std::multimap<std::string, TriggerInterface*> SyncedTriggers;
	typedef std::map<std::string, PresentationValue> PresetCustomValues;
	
	RootMatrixInterfaceWeakPtr m_rootMtx;
	math::Vector3              m_position;
	math::Vector3              m_worldPosition; // m_position with m_anim2dTransform applied.
	math::Quaternion           m_rotation;
	math::Vector3              m_scale;
	math::Matrix44             m_prsMatrix;
	math::Matrix44             m_anim2dTransform;
	math::Matrix44             m_combinedMatrix;  // The end result of all the matrices together (root matrix, anim2d matrix, client-set PRS), calculated in update()
	
	OverlayQuads               m_overlayQuads;
	
	anim2d::AnimationStack2D      m_anim2dStack;
	anim2d::ColorAnimationStack2D m_colorAnimStack;
	FrameAnimationStack           m_frameAnimStack;
	ParticlesStack                m_particleStack;
	TriggerStack                  m_triggerStack;
	TimerStack                    m_timerStack;
	
	CustomPresentationValues m_customValues;
	PresetCustomValues       m_presetCustomValues;
	
	Tags        m_activeTags;
	std::string m_activeName;
	bool        m_hideAtEnd;
	bool        m_shouldRender;
	u32         m_flipMask;
	real        m_resetTriggerTime;
	bool        m_isPermanent; // Once loaded, it should stay in memory
	bool        m_dontPrecache; // Precache code should NOT load this presentation
	real        m_fixedTimestep;
	
	// FIXME: This is used for the overlay quads, as they need to be scaled depending on whether the presentation object is in screenspace or not
	// this should be fixed properly and not be depending on some flag that we need to set
	bool        m_isInScreenSpace;
	
	PresentationMgr*  m_mgr;
	GroupInterfacePtr m_group; // Should ONLY be set by the parent group.
	
	SyncedTriggers m_syncedTriggers;
	SyncedTriggers m_endSyncedTriggers;
	
	CallbackTriggerInterfacePtr m_callbackInterface;
	
	bool m_isCulled;
	bool m_visible;
};


struct PresentationObjectLess
{
	inline bool operator()(PresentationObject* p_lhs,
	                       PresentationObject* p_rhs) const
	{
		TT_NULL_ASSERT(p_lhs);
		TT_NULL_ASSERT(p_rhs);
		if (p_lhs == 0) return false;
		if (p_rhs == 0) return true;
		
		const math::Vector3& lhsPos(p_lhs->getWorldPosition());
		const math::Vector3& rhsPos(p_rhs->getWorldPosition());
		
		if (lhsPos.z == rhsPos.z)
		{
			return p_lhs > p_rhs; // Compare pointers so it's 'random'.
		}
		return lhsPos.z < rhsPos.z;
	}
};

// Namespace end
}
}


#endif // !defined(INC_TT_PRES_PRESENTATIONOBJECT_H)
