#if !defined(INC_TT_ENGINE_SCENE_BONE_H)
#define INC_TT_ENGINE_SCENE_BONE_H

#include <string>

#include <tt/platform/tt_types.h>
#include <tt/math/Matrix44.h>
#include <tt/engine/scene/BoneWeight.h>
#include <tt/engine/scene/fwd.h>


namespace tt {
namespace engine {
namespace scene {


class Bone
{
public:
	Bone();
	virtual ~Bone();
	
	/*! \brief Get the name of the bone
	    \return A Pointer to a null terminated string of the name */
	inline const std::string& getName() const {return m_name;}
	
	/*! \brief Get the Pose matrix
	    \return Gets the default Pose matrix as a math::Matrix44. */
	inline const math::Matrix44& getDefaultMatrix() const {return m_defaultMatrix;}
	inline const math::Matrix44& getWorldMatrix() const   {return m_worldMatrix;}
	
	inline void setWorldMatrix(const math::Matrix44& p_world) {m_worldMatrix = p_world;}
	
	inline s16         getBoneWeightCount() const {return m_weightCount;}
	inline BoneWeight* getBoneWeight(s32 p_index) const {return &m_boneWeights[p_index];}
	
	bool update();
	
	virtual bool load(const fs::FilePtr& p_file);
	
private:
	// No copying
	Bone(const Bone&);
	Bone& operator=(const Bone&);
	
	
	std::string    m_name;
	SceneObjectPtr m_sceneObject;
	math::Matrix44 m_defaultMatrix;
	math::Matrix44 m_worldMatrix;
	
	s16         m_weightCount;
	BoneWeight* m_boneWeights;
};


// Namespace end
}
}
}


#endif // INC_TT_ENGINE_SCENE_BONE_H
