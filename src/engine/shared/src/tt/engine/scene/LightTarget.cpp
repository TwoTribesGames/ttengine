#include <tt/engine/scene/LightTarget.h>
#include <tt/fs/File.h>


namespace tt {
namespace engine {
namespace scene {

LightTarget::LightTarget() 
: 
SceneObject(Type_LightTarget)
{
}


LightTarget::~LightTarget()
{
}


bool LightTarget::load(const fs::FilePtr& p_file)
{
	// Load the base part of the object
	if (SceneObject::load(p_file) == false)
	{
		return false;
	}
	return true;
}


void LightTarget::renderObject(renderer::RenderContext&)
{
	// We dont really render a light target
}


// Namespace end
}
}
}

