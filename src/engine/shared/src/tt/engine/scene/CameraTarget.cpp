#include <tt/engine/scene/CameraTarget.h>
#include <tt/fs/File.h>


namespace tt {
namespace engine {
namespace scene {


CameraTarget::CameraTarget() 
: 
SceneObject(Type_CameraTarget)
{
}


CameraTarget::~CameraTarget()
{
}


bool CameraTarget::load(const fs::FilePtr& p_file)
{
	// Load the base part of the object
	if (SceneObject::load(p_file) == false)
	{
		return false;
	}

	return true;
}


void CameraTarget::renderObject(renderer::RenderContext&)
{
	// We dont really render a Camera Target
}


// Namespace end
}
}
}

