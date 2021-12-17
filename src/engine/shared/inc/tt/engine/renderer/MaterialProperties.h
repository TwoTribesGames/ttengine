#if !defined(INC_TT_ENGINE_RENDERER_MATERIALPROPERTIES_H)
#define INC_TT_ENGINE_RENDERER_MATERIALPROPERTIES_H


#include <tt/engine/renderer/ColorRGBA.h>


namespace tt {
namespace engine {
namespace renderer {


/*! \brief Material properties that are related to lighting */
struct MaterialProperties
{
	ColorRGBA ambient;
	ColorRGBA diffuse;
	ColorRGBA emissive;
	ColorRGBA specular;
	real      specularPower;
	
	MaterialProperties(ColorRGBA p_ambient  = ColorRGB::white,
	                   ColorRGBA p_diffuse  = ColorRGB::white,
	                   ColorRGBA p_emissive = ColorRGB::black,
	                   ColorRGBA p_specular = ColorRGB::black,
	                   real p_specularPower = 0.0f)
	:
	ambient (p_ambient),
	diffuse (p_diffuse),
	emissive(p_emissive),
	specular(p_specular),
	specularPower(p_specularPower)
	{ }
};


}
}
}

#endif //INC_TT_ENGINE_RENDERER_MATERIALPROPERTIES_H
