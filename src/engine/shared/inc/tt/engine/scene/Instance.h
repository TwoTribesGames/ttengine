#if !defined(INC_TT_ENGINE_SCENE_INSTANCE_H)
#define INC_TT_ENGINE_SCENE_INSTANCE_H


#include <string>

#include <tt/platform/tt_types.h>
#include <tt/platform/tt_error.h>
#include <tt/engine/scene/SceneObject.h>
#include <tt/engine/animation/Animation.h>
#include <tt/engine/renderer/fwd.h>
#include <tt/engine/animation/fwd.h>
#include <tt/engine/scene/fwd.h>
#include <tt/math/Vector3.h>


namespace tt {
namespace engine {
namespace scene {

class Instance
{
public:
	enum Flag
	{
		Flag_UpdatePRSMatrix          = (1 << 0),
		Flag_CameraAlign              = (1 << 7),
		Flag_CameraPositionBased      = (1 << 8),
		Flag_UpdateStaticObject       = (1 << 9),
		
		Flag_UseAlpha                 = (1 << 10),
		Flag_Invisible                = (1 << 11),
		
		Flag_Offscreen                = (1 << 12),
		Flag_Transparent              = (1 << 13),
		Flag_AnimationRemoved         = (1 << 14)
	};
	
	
	Instance();
	~Instance() {}
	
	inline void setSceneObject(const SceneObjectPtr& p_object) {m_object = p_object;}
	
	bool setAnimation(const animation::AnimationPtr& p_animation,
	                  animation::Mode                p_mode = animation::Mode_PlayHold,
	                  real                           p_offset = 0);
	
	void setFollowupAnimation(const animation::AnimationPtr& p_animation,
	                          animation::Mode                p_mode = animation::Mode_PlayHold);
	
	inline void setPlaybackSpeed(real p_speed) {m_animationControl.setSpeed(p_speed);}
	
	/*! \brief Gets the object.
	    \return The object linked to this instance */
	inline const SceneObjectPtr& getSceneObject() const {return m_object;}
	
	/*! \brief Gets the animation
	    \return The current animation set on this object,*/
	inline const animation::AnimationPtr& getAnimation() const {return m_animation;}
	
	// Shortcut to obtain a Model pointer (preventing a cast)
	inline Model* getModel()
	{
		TT_NULL_ASSERT(m_object);
		return m_object->getModel();
	}
	
	void update();
	void render(renderer::RenderContext* p_renderContext = 0);
	
	/*! \brief Gets the flags set on the instance.
	    \return The flags currently in use, as defined by Instance::FlagS */
	inline u32 getFlags() const {return m_flags;}
	
	/*! \brief Sets a specific flag.
	    \param flag - The flag to set */
	inline void setFlag(Flag p_flag) { setFlags(p_flag); }
	
	/*! \brief Sets multiple flags.
	    \param flags - The flags to set */
	inline void setFlags(u32 p_flags)
	{
		m_flags |= p_flags;
		
		// HACK: Update static objects twice
		if ((p_flags & Flag_UpdateStaticObject) != 0 || (p_flags & Flag_UpdatePRSMatrix) != 0)
		{
			m_updateMatrixCount = 4;
		}
	}
	
	/*! \brief Resets a specific flag 
	    \param flag - The flag to reset */
	inline void resetFlag(Flag p_flag) {m_flags &= ~p_flag;}
	
	/*! \brief Checks if a flag is currently set
	    \param flag - The flag to check
	    \return True if the flag is set else false */
	inline bool checkFlag(Flag p_flag) const {return ((m_flags & p_flag) != 0);}
	
	math::Vector3 getActualPosition() const;
	
	/*! \brief Get the position of the instance
	    \return The math::Vector3 containing the instances position */
	inline const math::Vector3& getPosition() const {return m_position;}
	
	/*! \brief Get the rotation of the instance
	    \return The math::Vector3 containing the rotation of the instance */
	inline const math::Vector3& getRotation() const {return m_rotation;}
	
	/*! \brief Get the scale of the instance
	    \return The math::Vector3 containing the scale of the instance */
	inline const math::Vector3& getScale() const {return m_scale;}
	
	/*! \brief Set this instances position
	    \param v - A reference to a math::Vector3 containing the new position */
	inline void setPosition(const math::Vector3& p_pos)
	{
		m_position = p_pos;
		setFlags(Flag_UpdatePRSMatrix | Flag_UpdateStaticObject);
	}
	
	/*! \brief Set this instances position
	    \param x - The new X position of the instance
	    \param y - The new Y position of the instance
	    \param z - The new Z position of the instance */
	inline void setPosition(real p_x, real p_y, real p_z)
	{
		m_position.setValues(p_x, p_y, p_z);
		setFlags(Flag_UpdatePRSMatrix | Flag_UpdateStaticObject);
	}
	
	/*! \brief Set this instances X position
	    \param x - The new X position of the instance */
	inline void setPositionX(real p_x)
	{
		m_position.x = p_x;
		setFlags(Flag_UpdatePRSMatrix | Flag_UpdateStaticObject);
	}
	
	/*! \brief Set this instances Y position
	    \param y - The new Y position of the instance */
	inline void setPositionY(real p_y)
	{
		m_position.y = p_y;
		setFlags(Flag_UpdatePRSMatrix | Flag_UpdateStaticObject);
	}
	
	/*! \brief Set this instances Z position
	    \param z - The new Z position of the instance */
	inline void setPositionZ(real p_z)
	{
		m_position.z = p_z;
		setFlags(Flag_UpdatePRSMatrix | Flag_UpdateStaticObject);
	}
	
	/*! \brief Set this instances rotation
	    \param v - A reference to a math::Vector3 containing the new rotation */
	inline void setRotation(const math::Vector3& p_rotation)
	{
		m_rotation = p_rotation;
		setFlags(Flag_UpdatePRSMatrix | Flag_UpdateStaticObject);
	}
	
	/*! \brief Set this instances rotation
	    \param x - The new X rotation of the instance
	    \param y - The new Y rotation of the instance
	    \param z - The new Z rotation of the instance */
	inline void setRotation(real p_x, real p_y, real p_z)
	{
		m_rotation.setValues(p_x, p_y, p_z);
		setFlags(Flag_UpdatePRSMatrix | Flag_UpdateStaticObject);
	}
	
	/*! \brief Set this instances X rotation
	    \param x - The new X rotation of the instance */
	inline void setRotationX(real p_x)
	{
		m_rotation.x = p_x;
		setFlags(Flag_UpdatePRSMatrix | Flag_UpdateStaticObject);
	}
	
	/*! \brief Set this instances Y rotation
	    \param y - The new Y rotation of the instance */
	inline void setRotationY(real p_y)
	{
		m_rotation.y = p_y;
		setFlags(Flag_UpdatePRSMatrix | Flag_UpdateStaticObject);
	}
	
	/*! \brief Set this instances Z rotation
	    \param z - The new Z rotation of the instance */
	inline void setRotationZ(real p_z)
	{
		m_rotation.z = p_z;
		setFlags(Flag_UpdatePRSMatrix | Flag_UpdateStaticObject);
	}
	
	/*! \brief Set this instances scale
	    \param v - A reference to a math::Vector3 containing the new scale */
	inline void setScale(const math::Vector3& p_scale)
	{
		m_scale = p_scale;
		setFlags(Flag_UpdatePRSMatrix | Flag_UpdateStaticObject);
	}
	
	/*! \brief Set this instances scale
	    \param x - The new X scale of the instance
	    \param y - The new Y scale of the instance
	    \param z - The new Z scale of the instance */
	inline void setScale(real p_x, real p_y, real p_z)
	{
		m_scale.setValues(p_x, p_y, p_z);
		setFlags(Flag_UpdatePRSMatrix | Flag_UpdateStaticObject);
	}
	
	/*! \brief Set this instances X scale
	    \param x - The new X scale of the instance */
	inline void setScaleX(real p_x)
	{
		m_scale.x = p_x;
		setFlags(Flag_UpdatePRSMatrix | Flag_UpdateStaticObject);
	}
	
	/*! \brief Set this instances Y scale
	    \param y - The new Y scale of the instance */
	inline void setScaleY(real p_y)
	{
		m_scale.y = p_y;
		setFlags(Flag_UpdatePRSMatrix | Flag_UpdateStaticObject);
	}
	
	/*! \brief Set this instances Z scale
	    \param z - The new Z scale of the instance */
	inline void setScaleZ(real p_z)
	{
		m_scale.z = p_z;
		setFlags(Flag_UpdatePRSMatrix | Flag_UpdateStaticObject);
	}
	
	inline void setMatrix(const math::Matrix44& p_transform)
	{
		m_matrix = p_transform;
		setFlag(Flag_UpdateStaticObject);
		resetFlag(Flag_UpdatePRSMatrix);
		
		// NOTE: Only use this if not using the setPosition / Scale / Rotate functions
		//       Otherwise the matrix will be overridden by the PRS matrix
	}
	
	inline const math::Matrix44& getMatrix() const {return m_matrix;}
	
	// Loading
	/*! \param p_useDefault Whether to load the default model if specified model does not exist.
	                        If false, returns a null pointer if model does not exist. */
	static InstancePtr get(const std::string& p_name,
	                       const std::string& p_namespace,
	                       u32                p_flags      = 0,
	                       bool               p_useDefault = true);
	
	static bool exists(const std::string& p_name, const std::string& p_namespace);
	
	// TODO: Alpha should be a model property!!
	inline void setAlpha(u8 p_alpha) {m_alpha = p_alpha;}
	
	/*! \brief Forces an update no matter what. */
	void forceUpdate();
	
	/*! \brief Clone the scene object (only works for models) */
	InstancePtr clone() const;
	
	math::Matrix44& getObjectMatrix(s32 p_matrixID);
	
	void pauseAnimation();
	void continueAnimation();
	inline bool hasPausedAnimation() const { return m_animationControl.isPaused(); }
	bool hasAnimationEnded() const;
	void setAnimationMode(animation::Mode p_mode);
	inline animation::Mode getAnimationMode() const { return m_animationControl.getMode(); }
	
	inline void setAnimationTime(real p_time) { m_animationControl.setTime(p_time); }
	inline real getAnimationTime() const { return m_animationControl.getTime(); }
	
private:
	void setRecursive(const SceneObjectPtr& p_object, SceneObject::Flag p_flag);
	
	// No copying (use clone())
	Instance(const Instance&);
	Instance& operator=(const Instance&);
	
	
	SceneObjectPtr              m_object;
	animation::AnimationPtr     m_animation;
	animation::AnimationControl m_animationControl;
	
	u32 m_flags;
	
	// TODO: This should ideally be replaced by a Transform component
	math::Vector3  m_position;
	math::Vector3  m_rotation;
	math::Vector3  m_scale;
	math::Matrix44 m_matrix;
	
	// Objects can store there instance specific matrices here (Instance Transform + Object Transform + Animation)
	typedef std::vector<math::Matrix44> MatrixCollection;
	MatrixCollection m_objectMatrices;
	
	// Can be used to override model alpha. TODO: Do we really need it?
	u8  m_alpha;
	
	// TODO: Debug update logic, make sure it works correctly for all platforms
	real m_updateTime;
	
	// HACK: Make sure the PRS and static objects are updated twice
	s32 m_updateMatrixCount;
};

// Namespace end
}
}
}


#endif  // !defined(INC_TT_ENGINE_SCENE_INSTANCE_H)
