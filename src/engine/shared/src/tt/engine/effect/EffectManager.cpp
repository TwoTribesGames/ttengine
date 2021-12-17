

#include <spark/SPK.h>

#include <tt/engine/effect/EffectManager.h>
#include <tt/engine/effect/EffectCollection.h>
#include <tt/engine/effect/SparkRenderer.h>

#include <tt/engine/effect/wrapper/SystemWrapper.h>
#include <tt/engine/effect/wrapper/GroupWrapper.h>
#include <tt/engine/effect/wrapper/ModelWrapper.h>

#include <tt/engine/effect/wrapper/ZoneWrapper.h>
#include <tt/engine/effect/wrapper/AABoxWrapper.h>
#include <tt/engine/effect/wrapper/LineWrapper.h>
#include <tt/engine/effect/wrapper/PlaneWrapper.h>
#include <tt/engine/effect/wrapper/PointWrapper.h>
#include <tt/engine/effect/wrapper/RingWrapper.h>
#include <tt/engine/effect/wrapper/SphereWrapper.h>
#include <tt/engine/effect/wrapper/ZoneIntersectionWrapper.h>
#include <tt/engine/effect/wrapper/ZoneUnionWrapper.h>

#include <tt/engine/effect/wrapper/ModifierWrapper.h>
#include <tt/engine/effect/wrapper/CollisionWrapper.h>
#include <tt/engine/effect/wrapper/DestroyerWrapper.h>
#include <tt/engine/effect/wrapper/LinearForceWrapper.h>
#include <tt/engine/effect/wrapper/ModifierGroupWrapper.h>
#include <tt/engine/effect/wrapper/ObstacleWrapper.h>
#include <tt/engine/effect/wrapper/PointMassWrapper.h>
#include <tt/engine/effect/wrapper/RotatorWrapper.h>
#include <tt/engine/effect/wrapper/VortexWrapper.h>

#include <tt/engine/effect/wrapper/EmitterWrapper.h>
#include <tt/engine/effect/wrapper/NormalEmitterWrapper.h>
#include <tt/engine/effect/wrapper/RandomEmitterWrapper.h>
#include <tt/engine/effect/wrapper/SphericEmitterWrapper.h>
#include <tt/engine/effect/wrapper/StaticEmitterWrapper.h>
#include <tt/engine/effect/wrapper/StraightEmitterWrapper.h>

#include <tt/engine/effect/wrapper/RendererWrapper.h>
#include <tt/engine/effect/wrapper/PointRendererWrapper.h>
#include <tt/engine/effect/wrapper/QuadRendererWrapper.h>
#include <tt/engine/effect/wrapper/LineRendererWrapper.h>
#include <tt/engine/effect/wrapper/LineTrailRendererWrapper.h>

#include <tt/engine/renderer/MatrixStack.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/scene/Camera.h>
#include <tt/script/ScriptEngine.h>


// Bind enums
SQBIND_INTEGER(SPK::BlendingMode);
SQBIND_INTEGER(SPK::ForceFactor);
SQBIND_INTEGER(SPK::ModelParam);
SQBIND_INTEGER(SPK::ModelParamFlag);
SQBIND_INTEGER(SPK::ModifierTrigger);
SQBIND_INTEGER(SPK::RenderingHint);
SQBIND_INTEGER(SPK::TexturingMode);
SQBIND_INTEGER(SPK::InterpolationType);

SQBIND_INTEGER(SPK::LookOrientation);
SQBIND_INTEGER(SPK::UpOrientation);
SQBIND_INTEGER(SPK::LockedAxis);
SQBIND_INTEGER(SPK::OrientationPreset);


namespace tt {
namespace engine {
namespace effect {


EffectCollections            EffectManager::ms_collections;
script::VirtualMachinePtr    EffectManager::ms_vm;


void EffectManager::init(s32 p_randomSeed)
{
	// Setup effect system

	// random seed
	SPK::randomSeed = p_randomSeed;

	// Sets the update step
	SPK::System::setClampStep(true, 0.05f);		 // Clamps the update step to 50 ms
	
	// NOTE: According to the documentation this allows for framerate fluctuation, but in practice
	//       it seems to cause stuttering framerates, using constant timestep now
	//SPK::System::useAdaptiveStep(1 / 30.0f, 1 / 60.0f); // use an adaptive step from 16ms to 33ms (60 to 30fps)
	
	SPK::System::useConstantStep(1 / 60.0f);

	// Setup scripting
	ms_vm = script::ScriptEngine::createVM("");
	ms_vm->disableCaching_HACK();

	/////////////////////////////////////////
	// Bind all effect wrappers

	// Model
	wrapper::ModelWrapper::bind(ms_vm);

	// Zones
	wrapper::ZoneWrapper::bind(ms_vm);
	wrapper::AABoxWrapper::bind(ms_vm);
	wrapper::LineWrapper::bind(ms_vm);
	wrapper::PlaneWrapper::bind(ms_vm);
	wrapper::PointWrapper::bind(ms_vm);
	wrapper::RingWrapper::bind(ms_vm);
	wrapper::SphereWrapper::bind(ms_vm);
	wrapper::ZoneIntersectionWrapper::bind(ms_vm);
	wrapper::ZoneUnionWrapper::bind(ms_vm);

	// Modifiers
	wrapper::ModifierWrapper::bind(ms_vm);
	wrapper::CollisionWrapper::bind(ms_vm);
	wrapper::DestroyerWrapper::bind(ms_vm);
	wrapper::LinearForceWrapper::bind(ms_vm);
	wrapper::ModifierGroupWrapper::bind(ms_vm);
	wrapper::ObstacleWrapper::bind(ms_vm);
	wrapper::PointMassWrapper::bind(ms_vm);
	wrapper::RotatorWrapper::bind(ms_vm);
	wrapper::VortexWrapper::bind(ms_vm);

	// Emitters
	wrapper::EmitterWrapper::bind(ms_vm);
	wrapper::NormalEmitterWrapper::bind(ms_vm);
	wrapper::RandomEmitterWrapper::bind(ms_vm);
	wrapper::SphericEmitterWrapper::bind(ms_vm);
	wrapper::StaticEmitterWrapper::bind(ms_vm);
	wrapper::StraightEmitterWrapper::bind(ms_vm);
	
	// Renderers
	wrapper::RendererWrapper::bind(ms_vm);
#if SPARK_USE_DX9POINT
	wrapper::PointRendererWrapper::bind(ms_vm);
#endif // SPARK_USE_DX9POINT
	wrapper::QuadRendererWrapper::bind(ms_vm);
#if SPARK_USE_DX9LINETRAIL
	wrapper::LineRendererWrapper::bind(ms_vm);
#endif // SPARK_USE_DX9LINETRAIL
#if SPARK_USE_DX9LINETRAIL 
	wrapper::LineTrailRendererWrapper::bind(ms_vm);
#endif // SPARK_USE_DX9LINETRAIL 
	
	// Group
	wrapper::GroupWrapper::bind(ms_vm);
	wrapper::SystemWrapper::bind(ms_vm);

	// Bind enum values
	using namespace SPK;
	SQBIND_CONSTANT(ms_vm->getVM(), ALWAYS);
	SQBIND_CONSTANT(ms_vm->getVM(), INSIDE_ZONE);
	SQBIND_CONSTANT(ms_vm->getVM(), OUTSIDE_ZONE);
	SQBIND_CONSTANT(ms_vm->getVM(), INTERSECT_ZONE);
	SQBIND_CONSTANT(ms_vm->getVM(), ENTER_ZONE);
	SQBIND_CONSTANT(ms_vm->getVM(), EXIT_ZONE);

	SQBIND_CONSTANT(ms_vm->getVM(), PARAM_RED);
	SQBIND_CONSTANT(ms_vm->getVM(), PARAM_GREEN);
	SQBIND_CONSTANT(ms_vm->getVM(), PARAM_BLUE);
	SQBIND_CONSTANT(ms_vm->getVM(), PARAM_ALPHA);
	SQBIND_CONSTANT(ms_vm->getVM(), PARAM_SIZE);
	SQBIND_CONSTANT(ms_vm->getVM(), PARAM_MASS);
	SQBIND_CONSTANT(ms_vm->getVM(), PARAM_ANGLE);
	SQBIND_CONSTANT(ms_vm->getVM(), PARAM_TEXTURE_INDEX);
	SQBIND_CONSTANT(ms_vm->getVM(), PARAM_ROTATION_SPEED);
	SQBIND_CONSTANT(ms_vm->getVM(), PARAM_CUSTOM_0);
	SQBIND_CONSTANT(ms_vm->getVM(), PARAM_CUSTOM_1);
	SQBIND_CONSTANT(ms_vm->getVM(), PARAM_CUSTOM_2);

	SQBIND_CONSTANT(ms_vm->getVM(), FLAG_NONE);
	SQBIND_CONSTANT(ms_vm->getVM(), FLAG_RED);
	SQBIND_CONSTANT(ms_vm->getVM(), FLAG_GREEN);
	SQBIND_CONSTANT(ms_vm->getVM(), FLAG_BLUE);
	SQBIND_CONSTANT(ms_vm->getVM(), FLAG_ALPHA);
	SQBIND_CONSTANT(ms_vm->getVM(), FLAG_SIZE);
	SQBIND_CONSTANT(ms_vm->getVM(), FLAG_MASS);
	SQBIND_CONSTANT(ms_vm->getVM(), FLAG_ANGLE);
	SQBIND_CONSTANT(ms_vm->getVM(), FLAG_TEXTURE_INDEX);
	SQBIND_CONSTANT(ms_vm->getVM(), FLAG_ROTATION_SPEED);

	SQBIND_CONSTANT(ms_vm->getVM(), FACTOR_NONE);
	SQBIND_CONSTANT(ms_vm->getVM(), FACTOR_LINEAR);
	SQBIND_CONSTANT(ms_vm->getVM(), FACTOR_SQUARE);

	SQBIND_CONSTANT(ms_vm->getVM(), BLENDING_NONE);
	SQBIND_CONSTANT(ms_vm->getVM(), BLENDING_ADD);
	SQBIND_CONSTANT(ms_vm->getVM(), BLENDING_ALPHA);
	SQBIND_CONSTANT(ms_vm->getVM(), BLENDING_MULTIPLY);

	SQBIND_CONSTANT(ms_vm->getVM(), ALPHA_TEST);
	SQBIND_CONSTANT(ms_vm->getVM(), DEPTH_TEST);
	SQBIND_CONSTANT(ms_vm->getVM(), DEPTH_WRITE);

	SQBIND_CONSTANT(ms_vm->getVM(), TEXTURE_NONE);
	SQBIND_CONSTANT(ms_vm->getVM(), TEXTURE_2D);
	//SQBIND_CONSTANT(ms_vm->getVM(), TEXTURE_3D);

	SQBIND_CONSTANT(ms_vm->getVM(), INTERPOLATOR_LIFETIME);
	SQBIND_CONSTANT(ms_vm->getVM(), INTERPOLATOR_AGE);
	SQBIND_CONSTANT(ms_vm->getVM(), INTERPOLATOR_PARAM);
	SQBIND_CONSTANT(ms_vm->getVM(), INTERPOLATOR_VELOCITY);
	
	SQBIND_CONSTANT(ms_vm->getVM(), LOOK_CAMERA_PLANE);
	SQBIND_CONSTANT(ms_vm->getVM(), LOOK_CAMERA_POINT);
	SQBIND_CONSTANT(ms_vm->getVM(), LOOK_AXIS);
	SQBIND_CONSTANT(ms_vm->getVM(), LOOK_POINT);
	
	SQBIND_CONSTANT(ms_vm->getVM(), UP_CAMERA);
	SQBIND_CONSTANT(ms_vm->getVM(), UP_DIRECTION);
	SQBIND_CONSTANT(ms_vm->getVM(), UP_AXIS);
	SQBIND_CONSTANT(ms_vm->getVM(), UP_POINT);
	
	SQBIND_CONSTANT(ms_vm->getVM(), LOCK_LOOK);
	SQBIND_CONSTANT(ms_vm->getVM(), LOCK_UP);
	
	SQBIND_CONSTANT(ms_vm->getVM(), CAMERA_PLANE_ALIGNED);
	SQBIND_CONSTANT(ms_vm->getVM(), CAMERA_POINT_ALIGNED);
	SQBIND_CONSTANT(ms_vm->getVM(), DIRECTION_ALIGNED);
	SQBIND_CONSTANT(ms_vm->getVM(), AROUND_AXIS);
	SQBIND_CONSTANT(ms_vm->getVM(), TOWARDS_POINT);
	SQBIND_CONSTANT(ms_vm->getVM(), FIXED_ORIENTATION);
	
	// Initialize platform specific renderer
	SparkRenderer::initialize();
}


void EffectManager::update(Group p_group)
{
	real deltaTime(tt::engine::renderer::Renderer::getInstance()->getDeltaTime());

	scene::CameraPtr camera;

	switch(p_group)
	{
	case Group_Viewport1:
		camera = renderer::Renderer::getInstance()->getMainCamera(renderer::ViewPortID_1);
		break;

	case Group_Viewport2:
		camera = renderer::Renderer::getInstance()->getMainCamera(renderer::ViewPortID_2);
		break;

	case Group_HUD:
		camera = renderer::Renderer::getInstance()->getHudCamera();
		break;

	default:
		TT_PANIC("Invalid Group!");
	}

	// Set camera
	math::Vector3 camPos(camera->getPosition());
	SPK::System::setCameraPosition(SPK::Vector3D(camPos.x, camPos.y, camPos.z));

	for(EffectCollections::const_iterator it = ms_collections.begin();
		it != ms_collections.end(); ++it)
	{
		TT_NULL_ASSERT(*it);
		(*it)->update(deltaTime, p_group);
	}

	EffectCollection::updateDeathRow();
}


void EffectManager::computeParticles(Group)
{
}


void EffectManager::render(Group p_group, bool /*p_enableDepthSort*/)
{
	renderer::MatrixStack* stack(renderer::MatrixStack::getInstance());
	renderer::Renderer*    rendererPtr(renderer::Renderer::getInstance());
	
	// Reset the texture matrix (ensure that the particle textures are rendered correctly)
	stack->resetTextureMatrix();
	
	// Configure world matrix
	stack->setMode(renderer::MatrixStack::Mode_Position);
	stack->push();
	stack->updateWorldMatrix();
	
	// Disable lighting
	const bool oldLightSetting = rendererPtr->isLightingEnabled();
	rendererPtr->setLighting(false);
	
	const bool oldCullingSetting = rendererPtr->isCullingEnabled();
	rendererPtr->setCullingEnabled(false);
	
	// Draw effects
	for (EffectCollections::const_iterator it = ms_collections.begin();
	     it != ms_collections.end(); ++it)
	{
		TT_NULL_ASSERT(*it);
		(*it)->render(p_group);
	}
	
	stack->pop();
	
	// Restore states
	rendererPtr->reset();
	rendererPtr->setLighting(oldLightSetting);
	rendererPtr->setCullingEnabled(oldCullingSetting);
}


void EffectManager::shutdown()
{
	// NOTE: Client code must unload all effect collections and release all Effect references
	//       before calling this function!

	// Platform specific renderer shutdown
	SparkRenderer::shutdown();

	// Shutdown scripting
	ms_vm.reset();
	script::ScriptEngine::destroy();

	// Destroy all resources
	SPK::SPKFactory::getInstance().destroyAll();
	SPK::SPKFactory::destroyInstance();
}



/////////////////////////////////
//  Private

void EffectManager::registerCollection(EffectCollection* p_collection)
{
	TT_NULL_ASSERT(p_collection);
	TT_ASSERTMSG(ms_collections.find(p_collection) == ms_collections.end(), "Already registered");

	ms_collections.insert(p_collection);

	//TT_Printf("[EFFECT] Collection %p registered\n", p_collection);
}

void EffectManager::unregisterCollection(EffectCollection* p_collection)
{
	TT_NULL_ASSERT(p_collection);

	EffectCollections::iterator it = ms_collections.find(p_collection);
	if(it != ms_collections.end())
	{
		ms_collections.erase(it);
	}
	else
	{
		TT_PANIC("Collection pointer not found!");
	}

	//TT_Printf("[EFFECT] Collection %p unregistered\n", p_collection);
}


// Namespace end
}
}
}
