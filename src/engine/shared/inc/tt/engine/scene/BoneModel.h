#if !defined(INC_TT_ENGINE_SCENE_BONEMODEL_H)
#define INC_TT_ENGINE_SCENE_BONEMODEL_H


#include <tt/platform/tt_types.h>
#include <tt/engine/scene/Model.h>
#include <tt/engine/scene/Bone.h>
#include <tt/engine/scene/fwd.h>


namespace tt {
namespace engine {
namespace scene {

class BoneModel : public Model
{

public:
	BoneModel();
	virtual ~BoneModel();

	void setBoneCount(s32 p_count);
	s32	 getBoneCount() const {return m_boneCount;}

	Bone* getBone(s32 p_bone) {return &m_bones[p_bone];}

	InstancePtr getBiped() const {return m_biped;}

	Vector3 getTransformedVector(s32 p_index) const
	{ 
		return m_transformedVectors[p_index];
	}

	Vector3 getMinVector() const	{return m_min;}
	Vector3 getMaxVector() const	{return m_max;}
	Vector3 getCenterVector() const	{return m_center;}

	Vector3 getFinalScale() const	{return m_finalScale;}

// Protected data
protected:
	virtual void update(Instance *instance, Animation *anim);
	virtual bool load(File* p_file);
	virtual void renderObject(Instance* p_instance = 0);
	virtual void calculateBoundingBox();

private:
	s32		m_boneCount;
	Bone*	m_bones;

	Vector3* m_transformedVectors;

	InstancePtr m_biped;

	Vector3	m_translate;
	Vector3	m_center;
	Vector3	m_min;
	Vector3 m_max;

	Vector3 m_finalScale;

	u32*	m_dataAddress;
	u32**	m_boneAddress;
	u32*	m_vertices;
	u32*	m_transVertices;
};


// Namespace end
}
}
}

#endif // INC_TT_ENGINE_SCENE_BONEMODEL_H
