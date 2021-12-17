#include <tt/engine/scene/SceneObject.h>

#include <tt/engine/animation/Animation.h>
#include <tt/engine/debug/DebugRenderer.h>
#include <tt/engine/scene/Camera.h>
#include <tt/engine/scene/CameraTarget.h>
#include <tt/engine/scene/Dummy.h>
#include <tt/engine/scene/EventObject.h>
#include <tt/engine/scene/Instance.h>
#include <tt/engine/scene/Light.h>
#include <tt/engine/scene/LightTarget.h>
#include <tt/engine/scene/Model.h>
#include <tt/engine/scene/ShadowModel.h>
#include <tt/engine/scene/Spline.h>
#include <tt/engine/scene/UserProperty.h>
#include <tt/engine/renderer/FixedFunction.h>
#include <tt/engine/renderer/MatrixStack.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/file/FileUtils.h>
#include <tt/fs/File.h>
#include <tt/platform/tt_printf.h>


namespace tt {
namespace engine {
namespace scene {


SceneObject::SceneObject(Type p_type)
:
m_type(p_type),
m_id(0,0),
m_worldPivot(0,0,0),
m_matrix(),
m_worldMatrix(),
m_inverseWorldMatrix(),
m_materialAnim(),
m_parent(),
m_child(),
m_sibling(),
m_pivot(0,0,0),
m_position(0,0,0),
m_rotation(0,0,0),
m_scale(1,1,1),
m_defaultScaleFactor(1.0f),
m_defaultPose(),
m_defaultPosition(0,0,0),
m_defaultRotation(0,0,0),
m_defaultScale(1,1,1),
m_isVisible(true),
m_flags(0),
m_matrixID(0),
m_alpha(255),
m_renderAlpha(255),
m_visibility(1),
m_defaultVisibility(1),
m_shadowPass((p_type == Type_ShadowModel) ? u8(1) : u8(0)),
m_texAnimUpdated(false),
m_userProperties()
{
}


SceneObject::~SceneObject()
{
}


UserProperty* SceneObject::getUserProperty(const std::string& p_name)
{
	for(UserPropertyContainer::iterator it = m_userProperties.begin();
		it != m_userProperties.end(); ++it)
	{
		if(p_name == it->getName())
		{
			return &(*it);
		}
	}
	return 0;
}


SceneObject* SceneObject::create(const fs::FilePtr& p_file, const EngineID& p_id, u32 p_flags)
{
	s32 objectType(Type_Unknown);
	if (p_file->read(&objectType, sizeof(objectType)) != sizeof(objectType))
	{
		return 0;
	}

	SceneObject* object(0);

	// Create the object based on type
	switch (objectType)
	{
		case SceneObject::Type_Model:       object = new Model;       break;
		case SceneObject::Type_ShadowModel: object = new ShadowModel; break;
		case SceneObject::Type_Camera:
		{
			TT_PANIC("Create cameras through Camera class instead of SceneObject.");
			return 0;
		}
		case SceneObject::Type_CameraTarget:     object = new CameraTarget; break;
		case SceneObject::Type_Dummy:            object = new Dummy;        break;
		case SceneObject::Type_LightTarget:      object = new LightTarget;  break;
		case SceneObject::Type_Spline:           object = new Spline;       break;
		case SceneObject::Type_EventObject:      object = new EventObject;  break; 
		
		default:
		{
			TT_PANIC("Unknown object type %d\n", objectType);
			break;
		}
	}

	if(object != 0)
	{
		object->setFlags(p_flags);
		object->setEngineID(p_id);
	}
	return object;
}


bool SceneObject::load(const fs::FilePtr& p_file)
{
	// Get flags set by create() function, not the ones in the data
	u32 loadFlags(m_flags);

	{
		u32 flags(0);
		if(p_file->read(&flags, sizeof(flags)) != sizeof(flags))
		{
			return false;
		}
		m_flags |= flags;
	}

	{
		EngineID childID(0,0);
		if(childID.load(p_file) == false)
		{
			return false;
		}

		// Child
		if(childID.valid())
		{
			SceneObjectPtr child(SceneObjectCache::get(childID, true, loadFlags));
			if(child == 0)
			{
				return false;
			}
			m_child = child;
		}
	}

	{

		EngineID siblingID(0,0);
		if(siblingID.load(p_file) == false)
		{
			return false;
		}

		// Sibling
		if(siblingID.valid())
		{
			SceneObjectPtr sibling(SceneObjectCache::get(siblingID, true, loadFlags));
			if (sibling == 0)
			{
				return false;
			}
			m_sibling = sibling;
		}
	}
	
	// read the pivot information
	if(m_pivot.load(p_file) == false)
	{
		if(m_pivot != math::Vector3::zero)
		{
			setFlag(Flag_UsePivot);
		}
		return false;
	}

	// read the default info
	if(p_file->read(&m_defaultScaleFactor, sizeof(m_defaultScaleFactor)) != sizeof(m_defaultScaleFactor))
	{
		return false;
	}
	
	if(m_defaultPosition.load(p_file) == false)
	{
		return false;
	}
	m_position = m_defaultPosition;
	
	if(m_defaultRotation.load(p_file) == false)
	{
		return false;
	}
	m_rotation = m_defaultRotation;
	
	if(m_defaultScale.load(p_file) == false)
	{
		return false;
	}
	m_scale = m_defaultScale;

	// Compute default matrix
	m_defaultPose = math::Matrix44::getSRT(m_scale, m_rotation, m_position);

	if(p_file->read(&m_defaultVisibility, sizeof(m_defaultVisibility)) != sizeof(m_defaultVisibility))
	{
		return false;
	}

	// Load User Properties
	s32 userPropertyCount = 0;
	if(p_file->read(&userPropertyCount, sizeof(userPropertyCount)) != sizeof(userPropertyCount))
	{
		return false;
	}
	
	if(userPropertyCount > 0)
	{
		m_userProperties.resize(static_cast<UserPropertyContainer::size_type>(userPropertyCount));
	}

	for(s32 i = 0; i < userPropertyCount; ++i)
	{
		if(m_userProperties[static_cast<UserPropertyContainer::size_type>(i)].load(p_file) == false)
		{
			return false;
		}
	}

	// Set the initial update flag
	setFlag(Flag_InitialUpdate);

	// Recursive set the polygon id
	if(checkFlag(Flag_RootNode))
	{
		s32 matrices(0);
		setMatrixID(matrices);
	}

	return true;
}


void SceneObject::setMatrixID(s32& p_id)
{
	m_matrixID = p_id;
	++p_id;

	if(m_child != 0)
	{
		m_child->setMatrixID(p_id);
	}

	if(m_sibling != 0)
	{
		m_sibling->setMatrixID(p_id);
	}
}


void SceneObject::setFlagRecursive(Flag p_flag)
{
	// Set the flag for this object
	setFlag(p_flag);

	// Pass on to hierarchy
	if(m_child != 0)
	{
		m_child->setFlagRecursive(p_flag);
	}

	if(m_sibling != 0)
	{
		m_sibling->setFlagRecursive(p_flag);
	}
}


void SceneObject::resetFlagRecursive(Flag p_flag)
{
	// Set the flag for this object
	resetFlag(p_flag);

	// Pass on to hierarchy
	if(m_child != 0)
	{
		m_child->resetFlagRecursive(p_flag);
	}

	if(m_sibling != 0)
	{
		m_sibling->resetFlagRecursive(p_flag);
	}
}


void SceneObject::getPropertiesFrom(const SceneObject& p_object)
{
	m_defaultPose = p_object.m_defaultPose;
	m_flags       = p_object.m_flags;
	m_child       = p_object.m_child;
	m_sibling     = p_object.m_sibling;
}


void SceneObject::update(const animation::AnimationPtr& p_animation, Instance* p_instance)
{
	renderer::MatrixStack* stack(renderer::MatrixStack::getInstance());

	stack->push();

	if(p_animation == 0)
	{
		// Set to defaults
		stack->multiply44(m_defaultPose);
		m_visibility = m_defaultVisibility;
		resetFlag(Flag_AnimationNoRender);
	}
	else
	{
		// Load the animation matrix onto the stack
		if (p_animation->loadMatrix() == false)
		{
			setFlag(Flag_AnimationNoRender);
		}
		else
		{
			resetFlag(Flag_AnimationNoRender);
		}
		
		// TODO: add support for visibility controller
		m_visibility = m_defaultVisibility;
	}

	// Calculate alpha from visibility value
	m_renderAlpha = static_cast<u8>(m_alpha * m_visibility);
	if(m_renderAlpha < 255)
	{
		// Unfortunately we can not reset this flag anymore because the texture might
		// contain transparency --> need fix for this...
		setFlag(Flag_Transparent);
	}

	// If this object uses the PRS matrix, load it on the stack
	if (checkFlag(Flag_UsePRSMatrix))
	{
		if (checkFlag(Flag_UpdatePRSMatrix))
		{
			m_matrix = math::Matrix44::getSRT(m_scale, m_rotation, m_position);
			resetFlag(Flag_UpdatePRSMatrix);
		}
		stack->multiply44(m_matrix);
	}
	
	// Update our children
	if(m_child != 0)
	{
		m_child->update(p_animation == 0 ? p_animation : p_animation->getChild(), p_instance);
	}

	// Apply the pivot
	if(checkFlag(Flag_UsePivot))
	{
		stack->translate(m_pivot);
	}

	// Apply Model Scale
	if(math::realEqual(m_defaultScaleFactor, 1.0f) == false)
	{
		stack->uniformScale(m_defaultScaleFactor);
	}

	// Store world matrix in Instance
	TT_NULL_ASSERT(p_instance);
	stack->getCurrent(p_instance->getObjectMatrix(m_matrixID));

	// Pop the matrix
	stack->pop();

	// Handle any siblings
	if(m_sibling != 0)
	{
		m_sibling->update(p_animation == 0 ? p_animation : p_animation->getSibling(), p_instance);
	}
}


void SceneObject::updateTextureAnimations()
{
	// BUG: This should be done only once per frame

	if(m_materialAnim != 0 && m_texAnimUpdated == false)
	{
		m_materialAnim->update(renderer::Renderer::getInstance()->getDeltaTime());

		// Prevent multiple updates in 1 frame
		m_texAnimUpdated = true;
	}
}


void SceneObject::render(renderer::RenderContext& p_renderContext)
{
	renderer::MatrixStack* stack(renderer::MatrixStack::getInstance());
	renderer::Renderer* renderer(renderer::Renderer::getInstance());

	// Get animation time for texture animations
	if(m_materialAnim != 0)
	{
		p_renderContext.textureAnimationTime = m_materialAnim->getTime();

		// Make sure the texture animation is updated again
		m_texAnimUpdated = false;
	}

	// Handle non standard render passes
	if(p_renderContext.pass == renderer::RenderPass_Shadows ||
	   p_renderContext.pass == renderer::RenderPass_Transparents)
	{
		// NOTE: This is a bit hacky, we must prevent the whole hierarchy from being rendered
		//       We really want a {object, matrix} collection to be generated in the future

		renderObject(p_renderContext);
		return;
	}

	// Render children
	if(m_child != 0) m_child->render(p_renderContext);

	// Render self
	if(isVisible() && m_visibility > 0.0f && m_renderAlpha != 0 && checkFlag(Flag_AnimationNoRender) == false)
	{
		// Setup the stack
		stack->push();

		TT_NULL_ASSERT(p_renderContext.instance);
		stack->multiply44(p_renderContext.instance->getObjectMatrix(m_matrixID));

		// Handle transparent objects later
		if(checkFlag(Flag_Transparent) && p_renderContext.pass != renderer::RenderPass_ShadowVolumes)
		{
			// Get transform
			math::Matrix44 current;
			stack->getCurrent(current);
			
			const math::Vector3 objectPos(current.m_41, current.m_42, current.m_43);
			const math::Vector3 cameraPos(renderer->getActiveCamera()->getActualPosition());
			
			const real distance = math::distanceSquared(cameraPos, objectPos);
			renderer->addTransparentObject(distance, this, current);
		}
		else
		{
			// Now render ourselves
			renderObject(p_renderContext);

			if(getType() == Type_ShadowModel)
			{
				renderObject(p_renderContext);
			}
		}
		stack->pop();
	}

	// Render siblings
	if(m_sibling != 0) m_sibling->render(p_renderContext);
}


////////////////////////////
// Private

SceneObject::SceneObject(const SceneObject& p_object)
:
m_type(p_object.m_type),
#ifndef TT_BUILD_FINAL
m_id("clone of " + p_object.m_id.getName(), "clone from namespace " + p_object.m_id.getNamespace()),
#else
m_id(0,0), // Copies do not have a valid ID (not cached)
#endif
m_worldPivot(p_object.m_worldPivot),
m_matrix(p_object.m_matrix),
m_worldMatrix(p_object.m_worldMatrix),
m_inverseWorldMatrix(p_object.m_inverseWorldMatrix),
m_materialAnim(),
m_parent(p_object.m_parent),
m_child(p_object.m_child),
m_sibling(p_object.m_sibling),
m_pivot(p_object.m_pivot),
m_position(p_object.m_position),
m_rotation(p_object.m_rotation),
m_scale(p_object.m_scale),
m_defaultScaleFactor(p_object.m_defaultScaleFactor),
m_defaultPose(p_object.m_defaultPose),
m_defaultPosition(p_object.m_defaultPosition),
m_defaultRotation(p_object.m_defaultRotation),
m_defaultScale(p_object.m_defaultScale),
m_isVisible(p_object.m_isVisible),
m_flags(p_object.m_flags),
m_matrixID(p_object.m_matrixID),
m_alpha(p_object.m_alpha),
m_renderAlpha(p_object.m_renderAlpha),
m_visibility(p_object.m_visibility),
m_defaultVisibility(p_object.m_defaultVisibility),
m_shadowPass(p_object.m_shadowPass),
m_texAnimUpdated(p_object.m_texAnimUpdated),
m_userProperties(p_object.m_userProperties)
{
	// Create a copy of the texture anim control
	if(p_object.m_materialAnim != 0)
	{
		m_materialAnim.reset(new animation::AnimationControl(*p_object.m_materialAnim));
	}

	if(m_child != 0)
	{
		if(m_child->getType() == Type_Model)
		{
#if defined(TT_BUILD_FINAL)
			m_child = (tt_ptr_static_cast<Model>(m_child))->clone();
#else
			m_child = (tt_ptr_dynamic_cast<Model>(m_child))->clone();
#endif
		}
	}

	if(m_sibling != 0)
	{
		if(m_sibling->getType() == Type_Model)
		{
#if defined(TT_BUILD_FINAL)
			m_sibling = (tt_ptr_static_cast<Model>(m_sibling))->clone();
#else
			m_sibling = (tt_ptr_dynamic_cast<Model>(m_sibling))->clone();
#endif
		}
	}
}


// Namespace end
}
}
}
