#include <tt/engine/scene/Dummy.h>
#include <tt/fs/File.h>


namespace tt {
namespace engine {
namespace scene {


Dummy::Dummy() 
: 
SceneObject(Type_Dummy)
{
}


Dummy::~Dummy()
{
}


bool Dummy::load(const fs::FilePtr& p_file)
{
	// Load the base part of the object
	if (SceneObject::load(p_file) == false)
	{
		return false;
	}

	return true;
}


void Dummy::renderObject(renderer::RenderContext&)
{
	// We dont really render a dummy
}


// Namespace end
}
}
}

