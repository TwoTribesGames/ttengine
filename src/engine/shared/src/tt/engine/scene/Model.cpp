#include <tt/engine/scene/Model.h>

#include <tt/engine/animation/AnimationControl.h>
#include <tt/engine/debug/DebugStats.h>
#include <tt/engine/renderer/FixedFunction.h>
#include <tt/engine/renderer/MatrixStack.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/SubModel.h>
#include <tt/engine/scene/Camera.h>
#include <tt/fs/File.h>
#include <tt/code/ErrorStatus.h>


namespace tt {
namespace engine {
namespace scene {


// define this to render wireframe bounding sphere
//#define MODEL_RENDER_BOUNDING_SPHERE


Model::Model() 
: 
SceneObject(Type_Model),
m_sphere(1.0f),
m_renderNormals(false),
m_subModels()
{
}


Model::Model(SceneObject::Type p_type)
: 
SceneObject(p_type),
m_sphere(1.0f),
m_renderNormals(false),
m_subModels()
{
}


ModelPtr Model::clone() const
{
	return ModelPtr(new Model(*this));
}


renderer::SubModel* Model::getSubModel(s32 p_index)
{
	return &m_subModels.at(static_cast<SubModelContainer::size_type>(p_index));
}


bool Model::load(const fs::FilePtr& p_file)
{
	// Load the base part of the object
	if (SceneObject::load(p_file) == false)
	{
		return false;
	}

	// Now we can load our information
	s32 subModelCount(0);
	if (p_file->read(&subModelCount, sizeof(subModelCount)) != sizeof(subModelCount))
	{
		return false;
	}

	if(subModelCount == 0)
	{
		// Nothing more to do
		return true;
	}

	// Load bounding volume
	{
		math::Vector3 min;
		math::Vector3 max;

		if(min.load(p_file) == false || max.load(p_file) == false)
		{
			return false;
		}
		
		// Choose a sphere based on the aabb
		m_sphere.setPosition((min + max) / 2.0f);
		m_sphere.setRadius  ((min - m_sphere.getPosition()).length());
	}

	// We can now load all the data in for these
	m_subModels.resize(static_cast<SubModelContainer::size_type>(subModelCount));

	for(SubModelContainer::iterator it = m_subModels.begin(); it != m_subModels.end(); ++it)
	{
		if (it->load(p_file, this) == false)
		{
			return false;
		}
	}

	if(checkFlag(Flag_RootNode))
	{
		code::ErrorStatus errStatus("Loading Material Animation Control");

		s32 animFlag(0);
		if (p_file->read(&animFlag, sizeof(animFlag)) != sizeof(animFlag))
		{
			return false;
		}

		if(animFlag == 1)
		{
			// Read anim control info for materials
			m_materialAnim.reset(new animation::AnimationControl());
			m_materialAnim->load(p_file, &errStatus);
		}
	}

	return true;
}


void Model::renderObject(renderer::RenderContext& p_renderContext)
{
	using renderer::Renderer;
	using tt::engine::renderer::MatrixStack;

	CameraPtr cam(Renderer::getInstance()->getActiveCamera());
	TT_NULL_ASSERT(cam);

	{
		///////////////////////
		// Do frustrum culling

		// set sphere position to worldspace
		math::Matrix44 current;
		renderer::MatrixStack::getInstance()->getCurrent(current);

		// Compute world position of model
		math::Vector3 worldPos(m_sphere.getPosition() * current);

		// Create a normalized vector along all axis with length 1
		static const math::Vector4 unit(0.57735f, 0.57735f, 0.57735f, 0);

		// Apply the current matrix to it
		math::Vector4 scaling(unit * current);
		
		// Multiply length of scale vector with the radius
		real radius(scaling.length() * m_sphere.getRadius());

		// NOTE: the correct way of computing the sphere radius would be to compute
		//		 the scaling along all three axis and take the largest value.

#if defined(MODEL_RENDER_BOUNDING_SPHERE) && defined(TT_PLATFORM_WIN)
		/////////////////////////////////
		// Bounding Sphere rendering

		// FIXME: No shared sphere debug rendering

		// Model Space sphere
		/*Renderer::getInstance()->getDebug()->renderSphere(
			m_sphere.getPosition(), m_sphere.getRadius());*/

		// World space spheres -- Only useful on windows
		//MatrixStack::getInstance()->push();
		//MatrixStack::getInstance()->setIdentity();
		//Renderer::getInstance()->getDebug()->renderSphere(worldPos, radius);
		//MatrixStack::getInstance()->pop();
#endif

		// Transform position to view space
		worldPos = worldPos * cam->getViewMatrix();
		
		// Actual worldposition and real radius known now check frustrum culling
		if (cam->isVisible(worldPos, radius) == false)
		{
			debug::DebugStats::addToObjectsCulled(1);
			return;
		}
	}
	
	// Render all our sub objects
	for(SubModelContainer::iterator it = m_subModels.begin(); it != m_subModels.end(); ++it)
	{
		it->render(p_renderContext);
	}

#if !defined(TT_BUILD_FINAL)
	if (m_renderNormals)
	{
		renderNormals();
	}
#endif
}


void Model::renderNormals()
{
	for(SubModelContainer::iterator it = m_subModels.begin(); it != m_subModels.end(); ++it)
	{
		it->renderNormals();
	}
}


////////////////////////////////
// Private

Model::Model(const Model& p_model)
:
SceneObject    (p_model),
m_sphere       (p_model.m_sphere),
m_renderNormals(p_model.m_renderNormals),
m_subModels    (p_model.m_subModels)
{
	// Set submodel pointers
	for(SubModelContainer::iterator it = m_subModels.begin(); it != m_subModels.end(); ++it)
	{
		it->setOwner(this);
	}
}


// Namespace end
}
}
}

