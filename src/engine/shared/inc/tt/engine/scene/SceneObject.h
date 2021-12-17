#if !defined(INC_TT_ENGINE_SCENE_SCENEOBJECT_H)
#define INC_TT_ENGINE_SCENE_SCENEOBJECT_H


#include <string>

#include <tt/platform/tt_types.h>
#include <tt/platform/tt_error.h>
#include <tt/math/Vector3.h>
#include <tt/math/Matrix44.h>
#include <tt/engine/EngineID.h>
#include <tt/engine/physics/BoundingBox.h>
#include <tt/engine/scene/UserProperty.h>
#include <tt/engine/scene/fwd.h>
#include <tt/engine/physics/fwd.h>
#include <tt/engine/animation/fwd.h>
#include <tt/engine/cache/ResourceCache.h>
#include <tt/engine/renderer/RenderContext.h>


namespace tt {
namespace engine {
namespace scene {


// Resource Management
typedef cache::ResourceCache<SceneObject> SceneObjectCache;


class SceneObject
{
public:
	static const file::FileType fileType = file::FileType_Object;
	static const bool hasResourceHeader = true;
	
	enum Type
	{
		Type_Unknown = 0,
		Type_Model,
		Type_Camera,
		Type_CameraTarget,
		Type_Dummy,
		Type_Light,
		Type_DirectionalLight,
		Type_LightTarget,
		Type_Sprite,
		Type_ShadowModel,
		Type_Spline,
		Type_BoneModel,
		Type_EventObject,
		
		Type_UserModel
	};
	
	enum Flag
	{
		Flag_UpdatePRSMatrix      = (1 << 0),
		Flag_UsePRSMatrix         = (1 << 1),
		Flag_DisableLighting      = (1 << 2),
		Flag_DoNotPack            = (1 << 3),
		Flag_CastShadow           = (1 << 4),
		Flag_ReceiveShadow        = (1 << 5),
		Flag_InverseMatrixValid   = (1 << 6),
		Flag_DynamicObject        = (1 << 7),
		Flag_InitialUpdate        = (1 << 8),
		Flag_UsePivot             = (1 << 9),
		Flag_Transparent          = (1 << 10),
		Flag_EnableObjectCulling  = (1 << 11),
		Flag_RootNode             = (1 << 12),
		
		Flag_ITCMHack             = (1 << 13),
		Flag_AnimationNoRender    = (1 << 14)  // Based on current animation result, don't render. (e.g. zero scale)
	};
	
	
	explicit SceneObject(Type p_type);
	virtual ~SceneObject();
	
	/*! \brief Get the object type.
	    \return The Object type as defined by SceneObject::Type */
	inline Type getType() const {return m_type;}
	
	/*! \brief Gets the flags set on the object.
	    \return The flags currently in use, as defined by SceneObject::FlagS */
	inline u32 getFlags() const { return m_flags; }
	
	/*! \brief Sets a specific flag.
	    \param flag - The flag to set */
	inline void setFlag(Flag p_flag)  {m_flags |= p_flag;}
	inline void setFlags(u32 p_flags) {m_flags |= p_flags;}
	
	/*! \brief Resets a specific flag 
	    \param flag - The flag to reset */
	inline void resetFlag(Flag p_flag) {m_flags &= ~p_flag;}
	
	/*! \brief Checks if a flag is currently set
	    \param flag - The flag to check
	    \return True if the flag is set else false */
	inline bool checkFlag(Flag p_flag) const {return ((m_flags & p_flag) != 0);}
	
	void setFlagRecursive(Flag p_flag);
	void resetFlagRecursive(Flag p_flag);
	
	/*! \brief Get the parent object
	    \return The pointer to the parent SceneObject or 0 */
	inline SceneObjectPtr getParent() const {return m_parent.lock();}
	
	/*! \brief Get the child object
	    \return The pointer to the child SceneObject or 0 */
	inline const SceneObjectPtr& getChild() const {return m_child;}
	
	/*! \brief Get the sibling object
	    \return The pointer to the sibling SceneObject or 0 */
	inline const SceneObjectPtr& getSibling() const {return m_sibling;}
	
	inline const math::Vector3& getPivot() const {return m_pivot;}
	
	/*! \brief Get the position of the object
	    \return The math::Vector3 containing the objects position */
	inline const math::Vector3& getPosition() const {return m_position;}
	
	/*! \brief Get the default scale of the object
	    \return The default scale applied to the object */
	inline real getDefaultScaleFactor() const {return m_defaultScaleFactor;}
	
	/*! \brief Get the default position of the objects pose matrix
	    \return The Position from the pose matrix */
	inline const math::Vector3& getDefaultPosition() const {return m_defaultPosition;}
	
	/*! \brief Get the default rotation of the objects pose matrix
	    \return The rotation from the pose matrix */
	inline const math::Vector3& getDefaultRotation() const {return m_defaultRotation;}
	
	/*! \brief Get the rotation of the object
	    \return The math::Vector3 containing the rotation of the object */
	inline const math::Vector3& getRotation() const {return m_rotation;}
	
	/*! \brief Get the scale of the object
	    \return The math::Vector3 containing the scale of the object */
	inline const math::Vector3& getScale() const {return m_scale;}
	
	/*! \brief Set this objects parent
	    \param parent - The parent object */
	inline void setParent(const SceneObjectPtr& p_parent){m_parent = p_parent;}
	
	/*! \brief Set this objects sibling
	    \param sibling - The sibling object */
	inline void setSibling(const SceneObjectPtr& p_sibling) {m_sibling = p_sibling;}
	
	/*! \brief Set this objects child
	    \param sibling - The child object */
	inline void setChild(const SceneObjectPtr& p_child) {m_child = p_child;}
	
	/*! \brief Set this objects position
	    \param p_pos - A reference to a math::Vector3 containing the new position */
	inline void setPosition(const math::Vector3& p_pos)
	{
		m_position = p_pos;
		setFlag(Flag_UpdatePRSMatrix);
	}
	
	/*! \brief Set this objects position
	    \param x - The new X position of the object
	    \param y - The new Y position of the object
	    \param z - The new Z position of the object */
	inline void setPosition(real p_x, real p_y, real p_z)
	{
		m_position.setValues(p_x, p_y, p_z);
		setFlag(Flag_UpdatePRSMatrix);
	}
	
	/*! \brief Set this objects X position
	    \param x - The new X position of the object */
	inline void setPositionX(real p_x)
	{
		m_position.x = p_x;
		setFlag(Flag_UpdatePRSMatrix);
	}
	
	/*! \brief Set this objects Y position
	    \param y - The new Y position of the object */
	inline void setPositionY(real p_y)
	{
		m_position.y = p_y;
		setFlag(Flag_UpdatePRSMatrix);
	}
	
	/*! \brief Set this objects Z position
	    \param z - The new Z position of the object */
	inline void setPositionZ(real p_z)
	{
		m_position.z = p_z;
		setFlag(Flag_UpdatePRSMatrix);
	}
	
	/*! \brief Set this objects rotation
	    \param p_rotation - A reference to a math::Vector3 containing the new rotation. */
	inline void setRotation(const math::Vector3& p_rotation)
	{
		m_rotation = p_rotation;
		setFlag(Flag_UpdatePRSMatrix);
	}
	
	/*! \brief Set this objects rotation
	    \param x - The new X rotation of the object
	    \param y - The new Y rotation of the object
	    \param z - The new Z rotation of the object */
	inline void setRotation(real p_x, real p_y, real p_z)
	{
		m_rotation.setValues(p_x, p_y, p_z);
		setFlag(Flag_UpdatePRSMatrix);
	}
	
	/*! \brief Set this objects X rotation
	    \param x - The new X rotation of the object */
	inline void setRotationX(real p_x)
	{
		m_rotation.x = p_x;
		setFlag(Flag_UpdatePRSMatrix);
	}
	/*! \brief Set this objects Y rotation
	    \param y - The new Y rotation of the object */
	inline void setRotationY(real p_y)
	{
		m_rotation.y = p_y;
		setFlag(Flag_UpdatePRSMatrix);
	}
	
	/*! \brief Set this objects Z rotation
	    \param z - The new Z rotation of the object */
	inline void setRotationZ(real p_z)
	{
		m_rotation.z = p_z;
		setFlag(Flag_UpdatePRSMatrix);
	}
	
	/*! \brief Set this objects scale
	    \param v - A reference to a math::Vector3 containing the new object scale */
	inline void setScale(const math::Vector3& p_scale)
	{
		m_scale = p_scale;
		setFlag(Flag_UpdatePRSMatrix);
	}
	
	/*! \brief Set this objects scale
	    \param x - The new X scale of the object
	    \param y - The new Y scale of the object
	    \param z - The new Z scale of the object */
	inline void setScale(real p_x, real p_y, real p_z)
	{
		m_scale.setValues(p_x, p_y, p_z);
		setFlag(Flag_UpdatePRSMatrix);
	}
	
	/*! \brief Set this objects X scale
	    \param x - The new X scale of the object */
	inline void setScaleX(real p_x)
	{
		m_scale.x = p_x;
		setFlag(Flag_UpdatePRSMatrix);
	}
	
	/*! \brief Set this objects Y scale
	    \param y - The new Y scale of the object */
	inline void setScaleY(real p_y)
	{
		m_scale.y = p_y;
		setFlag(Flag_UpdatePRSMatrix);
	}
	
	/*! \brief Set this objects Z scale
	    \param z - The new Z scale of the object */
	inline void setScaleZ(real p_z)
	{
		m_scale.z = p_z;
		setFlag(Flag_UpdatePRSMatrix);
	}
	
	/*! \brief Checks if object is visible. NOTE: This does not mean on screen
	    \return true if the object is visible else false */
	inline bool isVisible() const {return m_isVisible;}
	
	/*! \brief Set whether the object is visible
	    \param vis - true if the object is visible else false */
	inline void setVisible(bool p_isVisible) {m_isVisible = p_isVisible;}
	
	/*! \brief Gets the Alpha of the object
	    \return The current alpha value of the object */
	inline u8 getAlpha() {return m_alpha;}
	
	/*! \brief Sets the alpha of the object
	    \param alpha - The alpha value to set on the object */
	inline void setAlpha(u8 p_alpha) {m_alpha = p_alpha;}
	
	/*! \brief Gets the number of user properties for this object
	    \return The number of user properties available */
	inline u32 getUserPropertyCount() const {return static_cast<u32>(m_userProperties.size());}
	
	/*! \brief Gets the given user property
	    \param index - The index number of the property to get
	    \return The pointer to the UserProperty */
	inline UserProperty* getUserProperty(u32 p_index) {return &m_userProperties[p_index];}
	UserProperty* getUserProperty(const std::string& p_name);
	
	/*! \brief Get the Pose matrix
	    \return Gets the default Pose matrix as a math::Matrix44. */
	inline const math::Matrix44& getDefaultMatrix() const {return m_defaultPose;}
	
	/*! \brief Get the World matrix
	    \return Gets the World Matrix as a math::Matrix44. */
	inline const math::Matrix44& getWorldMatrix() const {return m_worldMatrix;}
	
	/*! \brief Set the World matrix
	    \param matrix - The World Matrix 
	    \return Nothing*/
	inline void setWorldMatrix(const math::Matrix44& p_world) {m_worldMatrix = p_world;}
	
	inline void setPRSMatrix(const math::Matrix44& p_matrix) {m_matrix = p_matrix;}
	
	inline void setDefaultMatrix(const math::Matrix44& p_default) {m_defaultPose = p_default;}
	
	virtual Model* getModel() {return 0;}
	
	inline void setEngineID(const EngineID& p_id) {m_id = p_id;}
	inline const EngineID& getEngineID() const {return m_id;}
	
	void getPropertiesFrom(const SceneObject& p_object);
	
	void render(renderer::RenderContext& p_renderContext);
	
	virtual void update(const animation::AnimationPtr& p_animation, Instance* p_instance);
	
	void updateTextureAnimations();
	
	static SceneObject* create(const fs::FilePtr& p_file, const EngineID& p_id, u32 p_flags);
	
	
	virtual bool load(const fs::FilePtr& p_file);
	
	inline virtual s32 getMemSize() const {return sizeof(SceneObject);}
	
	inline s32 getMatrixID() const {return m_matrixID;}
	
	
protected:
	virtual void renderObject(renderer::RenderContext& p_renderContext)
	{
		(void)p_renderContext;
		TT_PANIC("This function should only be called on derived classes (type=%d)", m_type);
	}
	
	inline void setDefaultScale(real p_scale) {m_defaultScaleFactor = p_scale;}
	
	void setPolygonAttributes(bool p_secondShadow = false);
	void setHierarchyID(u8 p_id);
	void setMatrixID(s32& p_id);
	
	SceneObject(const SceneObject& p_rhs);
	
protected:
	// Members that are accesible by derived classses
	Type                 m_type;
	EngineID             m_id;
	
	// FIXME: Are these all in use?
	math::Vector3        m_worldPivot;
	math::Matrix44       m_matrix;
	math::Matrix44       m_worldMatrix;
	math::Matrix44       m_inverseWorldMatrix;
	
	animation::AnimationControlPtr m_materialAnim;
	
private:
	SceneObjectWeakPtr m_parent;
	SceneObjectPtr     m_child;
	SceneObjectPtr     m_sibling;
	
	math::Vector3 m_pivot;
	math::Vector3 m_position;
	math::Vector3 m_rotation;
	math::Vector3 m_scale;
	
	real           m_defaultScaleFactor;
	math::Matrix44 m_defaultPose;
	math::Vector3  m_defaultPosition;
	math::Vector3  m_defaultRotation;
	math::Vector3  m_defaultScale;
	
	bool m_isVisible;
	u32  m_flags;
	s32  m_matrixID;
	
	u8 m_alpha;
	u8 m_renderAlpha;
	
	real m_visibility;
	real m_defaultVisibility;
	
	u8   m_shadowPass;
	bool m_texAnimUpdated;
	
	typedef std::vector<UserProperty> UserPropertyContainer;
	UserPropertyContainer m_userProperties;
};

// Namespace end
}
}
}


#endif  // !defined(INC_TT_ENGINE_SCENE_SCENEOBJECT_H)
