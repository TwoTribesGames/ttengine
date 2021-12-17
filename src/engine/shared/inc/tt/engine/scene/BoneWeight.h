#if !defined(INC_TT_ENGINE_SCENE_BONEWEIGHT_H)
#define INC_TT_ENGINE_SCENE_BONEWEIGHT_H


#include <tt/platform/tt_types.h>


namespace tt {
namespace engine {
namespace scene {

class BoneWeight
{
public:
	BoneWeight()
	:
	m_index(0),
	m_weight(0)
	{}

	BoneWeight(s16 p_index, real p_weight)
	:
	m_index(p_index),
	m_weight(p_weight)
	{}

	~BoneWeight() {}

	inline s16  getIndex() const  {return m_index;}
	inline real getWeight() const {return m_weight;}

	inline void setIndex(s16 p_index)    {m_index  = p_index;}
	inline void setWeight(real p_weight) {m_weight = p_weight;}

private:
	s16	 m_index;
	real m_weight;
};


// Namespace end
}
}
}


#endif // INC_TT_ENGINE_SCENE_BONEWEIGHT_H
