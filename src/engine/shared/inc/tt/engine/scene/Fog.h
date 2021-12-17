#if !defined(INC_TT_ENGINE_SCENE_FOG_H)
#define INC_TT_ENGINE_SCENE_FOG_H


#include <tt/platform/tt_types.h>
#include <tt/engine/renderer/ColorRGBA.h>
#include <tt/engine/animation/fwd.h>
#include <tt/fs/types.h>

namespace tt {
namespace engine {
namespace scene {


class Fog
{
public:
	enum Type
	{
		Type_None,
		Type_Linear,
		Type_Exponential,
		Type_ExponentialSquared,
		Type_ReverseExponential,
		Type_ReverseExponentialSquared
	};

	Fog();
	inline ~Fog() { }

	inline void setType(Type p_type) {m_type = p_type;}
	
	inline Type getType()      const {return m_type;}
	inline s32  getReference() const {return m_reference;}
	
	renderer::ColorRGBA getColor() const;
	real                getStart() const;
	real                getEnd()   const;

	bool load(const fs::FilePtr& p_file);

private:
	Type m_type;
	s32  m_reference;

	animation::HermiteFloatControllerPtr m_red;
	animation::HermiteFloatControllerPtr m_green;
	animation::HermiteFloatControllerPtr m_blue;
	
	animation::HermiteFloatControllerPtr m_start;
	animation::HermiteFloatControllerPtr m_end;
};

// Namespace end
}
} 
}

#endif // INC_TT_ENGINE_SCENE_FOG_H
