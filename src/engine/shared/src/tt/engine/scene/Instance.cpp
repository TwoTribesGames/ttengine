#include <tt/engine/scene/Instance.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/MatrixStack.h>
#include <tt/engine/renderer/RenderContext.h>
#include <tt/engine/scene/SceneObject.h>
#include <tt/engine/scene/Camera.h>
#include <tt/engine/scene/Model.h>
#include <tt/engine/file/FileUtils.h>


namespace tt {
namespace engine {
namespace scene {


Instance::Instance()
:
m_object(),
m_animation(),
m_animationControl(),
m_flags(0),
m_position(math::Vector3::zero),
m_rotation(math::Vector3::zero),
m_scale(math::Vector3::allOne),
m_matrix(),
m_objectMatrices(),
m_alpha(255),
m_updateTime(std::numeric_limits<real>::min()),
m_updateMatrixCount(0)
{
}


bool Instance::setAnimation(const animation::AnimationPtr& p_animation,
							animation::Mode p_mode, real p_offset)
{
	// FIXME: Instancing is not ideally implemented at the moment
	//        Must separate instance & shared data more consistently
	
	if(p_animation == 0 && m_animation != 0)
	{
		// Remove animation
		if(m_animationControl.getMode() != animation::Mode_PlayHold)
		{
			setFlag(Instance::Flag_UpdateStaticObject);
		}
	}
	else if(p_animation != 0)
	{
		// Copy control structure
		m_animationControl = *p_animation->getControl();

		// FIXME: Think of a more reasonable way to handle this 
		//        (in the conversion process for instance...)
		/*real endTime(0);
		p_animation->resolveEndTime(endTime);

		m_animationControl.setStartTime(0);
		m_animationControl.setEndTime(endTime);*/

		// Set instance mode, type & offset
		m_animationControl.setMode(p_mode);

		// TODO: move this to anim control
		m_animationControl.setLoop(p_mode == animation::Mode_Repeat);

		m_animationControl.setTime(p_offset);
	}

	// Set the new
	m_animation = p_animation;

	resetFlag(Flag_AnimationRemoved);

	return true;
}


void Instance::setFollowupAnimation(const animation::AnimationPtr& p_animation, animation::Mode p_mode)
{
	setAnimation(p_animation, p_mode, m_animationControl.getLeftOverTime());
}


InstancePtr Instance::get(const std::string& p_name, const std::string& p_namespace, u32 p_flags,
                          bool p_useDefault)
{
	SceneObjectCache::ResourcePtr sceneObj(SceneObjectCache::get(p_name, p_namespace, p_useDefault, p_flags));
	if (sceneObj == 0)
	{
		return InstancePtr();
	}
	
	InstancePtr instance(new Instance);
	TT_NULL_ASSERT(instance);
	
	instance->setSceneObject(sceneObj);
	instance->setFlag(Flag_UpdateStaticObject);
	
	return instance;
}


bool Instance::exists(const std::string& p_name, const std::string& p_namespace)
{
	return SceneObjectCache::exists(EngineID(p_name, p_namespace));
}


math::Vector3 Instance::getActualPosition() const
{
	MatrixCollection::size_type index = static_cast<MatrixCollection::size_type>(m_object->getMatrixID());

	if(m_object != 0 && index < m_objectMatrices.size())
	{
		// Return position of root object
		const math::Matrix44& transform = m_objectMatrices[index];

		return math::Vector3(transform.m_41, transform.m_42, transform.m_43);
	}

	return m_position;
}


void Instance::update()
{
	// Must have an object to update
	if(m_object == 0) return;

	// Always update texture animations
	m_object->updateTextureAnimations();

	//////////////////////////////////////////////////////////////////////////
	// Optimization for static objects
	// FIXME: Enable this optimization again, but make sure the models don't
	//        look like dots. Marco please look at the shadow volumes on the NDS.
	// TODO: Need to create a test case to solve this

	if (m_animation == 0 &&
		checkFlag(Flag_UpdateStaticObject) == false)
	{
		// NOTE: Removed optimization, because this way static objects cannot cast shadow
		//       We need a way to check if any object in the hierarchy can cast shadows
		//return;
	}
	//
	/////////////////////////////////////////////////////////////////////////

	// Animation update logic
	if(m_animation != 0)
	{
		if(hasAnimationEnded())
		{
			// Remove animation once it has finished
			setAnimation(animation::AnimationPtr());
			setFlag(Flag_AnimationRemoved);
		}
		else
		{
			m_animationControl.update(renderer::Renderer::getInstance()->getDeltaTime());
			m_animation->setTimeRecursive(m_animationControl.getTime());
		}
	}

	if(checkFlag(Flag_UpdatePRSMatrix))
	{
		m_matrix = math::Matrix44::getSRT(m_scale, m_rotation, m_position);
	}

	/////////////////////////////////////////////////////
	// HACK : Update static objects twice
	// FIXME: matrix can be invalid causing dots to appear, this needs to be fixed! (See above)
	if(m_updateMatrixCount > 0)
	{
		--m_updateMatrixCount;
	}
	else
	{
		resetFlag(Flag_UpdateStaticObject);
		resetFlag(Flag_UpdatePRSMatrix);
	}
	//
	/////////////////////////////////////////////////////
	
	// Load our matrix on the stack
	renderer::MatrixStack* stack = renderer::MatrixStack::getInstance();
	stack->load44(m_matrix);
	
	// Now calculcate the world matrices
	m_object->update(m_animation, this);
	
	stack->setIdentity();
}


void Instance::render(renderer::RenderContext* p_renderContext)
{
	// Dont bother if we dont have an object
	if (m_object == 0 || checkFlag(Flag_Invisible)) return;

	renderer::RenderContext defaultRC;
	if(p_renderContext == 0)
	{
		p_renderContext = &defaultRC;
	}

	// Store pointer to instance in render context
	p_renderContext->instance = this;

	if (checkFlag(Flag_UseAlpha))
	{
		// Store object alpha & use instance alpha
		u8 alpha(m_object->getAlpha());
		m_object->setAlpha(m_alpha);

		// Render with instance alpha
		m_object->render(*p_renderContext);

		// Restore original value
		m_object->setAlpha(alpha);
	}
	else
	{
		// Render object with object alpha
		m_object->render(*p_renderContext);
	}
}


void Instance::forceUpdate()
{
	if (m_object != 0)
	{
		m_updateTime = std::numeric_limits<real>::min();

		m_object->setFlagRecursive(SceneObject::Flag_InitialUpdate);
		
		update();
	}
}	


InstancePtr Instance::clone() const
{
	InstancePtr clonedInstance(new Instance);

	clonedInstance->setSceneObject(m_object);
	clonedInstance->setAnimation(m_animation, m_animationControl.getMode());

	// Clone scene object
	if(m_object != 0 && m_object->getType() == SceneObject::Type_Model)
	{
		clonedInstance->setSceneObject(
			tt_ptr_dynamic_cast<Model>(clonedInstance->getSceneObject())->clone());
	}

	// Force update after clone
	clonedInstance->setFlag(Instance::Flag_UpdateStaticObject);

	return clonedInstance;
}


void Instance::pauseAnimation()
{
	m_animationControl.setPaused(true);
}


void Instance::continueAnimation()
{
	m_animationControl.setPaused(false);
}


bool Instance::hasAnimationEnded() const
{
	return m_animationControl.isFinished();
}


void Instance::setAnimationMode(animation::Mode p_mode)
{
	m_animationControl.setMode(p_mode);

	// TODO: must be handled by Animation Control
	m_animationControl.setLoop(p_mode == animation::Mode_Repeat);	
}


math::Matrix44& Instance::getObjectMatrix(s32 p_matrixID)
{
	MatrixCollection::size_type index = static_cast<MatrixCollection::size_type>(p_matrixID);

	if(index < m_objectMatrices.size())
	{
		return m_objectMatrices[index];
	}

	// Add another transformation matrix
	m_objectMatrices.resize(index + 1);
	return m_objectMatrices[index];
}


// Namespace end
}
}
}

