#include <spark/SPK.h>

#include <tt/engine/cache/FileTextureCache.h>
#include <tt/engine/effect/Effect.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/script/ScriptEngine.h>
#include <tt/script/ScriptObject.h>

#include <tt/engine/debug/DebugRenderer.h>
#include <tt/engine/renderer/MatrixStack.h>
#include <tt/engine/renderer/Renderer.h>

//#define ENABLE_PARTICLES_DEBUG_RENDERING

namespace tt {
namespace engine {
namespace effect {


Effect::Effect(const std::string& p_name, Group p_group)
:
m_name(p_name),
m_group(p_group),
m_alive(true),
m_system(SPK::System::create())
{
	//TT_Printf("[EFFECT] Created: %s\n", m_name.c_str());
}


Effect::~Effect()
{
	//TT_Printf("[EFFECT] Destroyed: %s\n", m_name.c_str());
	SPK_Destroy(m_system.getSystem());
}


bool Effect::loadFromScript(const std::string& p_filename, const script::VirtualMachinePtr& p_vm)
{
	//TT_Printf("[EFFECT] Loading particle script: %s.nut\n", p_filename.c_str());
	const std::string fullFilename = p_filename + ".nut";
	if(fs::fileExists(fullFilename) == false)
	{
		TT_WARN("Particle script not found: %s\n", fullFilename.c_str());
		return false;
	}
	
	script::ScriptObject obj(p_vm->loadScriptAsObject(fullFilename));
	
	if (runScriptObject(obj, p_vm) == false)
	{
		TT_PANIC("Running script '%s' failed", fullFilename.c_str());
		return false;
	}
	return true;
}


bool Effect::runScriptObject(const script::ScriptObject& p_obj, const script::VirtualMachinePtr& p_vm)
{
	if (p_vm->runScriptObject(p_obj) == false)
	{
		TT_PANIC("Running script object failed");
		return false;
	}
	
	// Initialize the particle system through a script
	p_vm->callSqFun("setup", m_system);
	
	return true;
}


void Effect::setPosition(const math::Vector3& p_position)
{
	// Create translation matrix
	setTransform(math::Matrix44::getTranslation(p_position));
}


void Effect::setTransform(const math::Matrix44& p_transform)
{
	// Update the root matrix
	if(m_system.getSystem() != 0)
	{
		m_system.getSystem()->setTransform(reinterpret_cast<const float*>(&p_transform));
		m_system.getSystem()->updateTransform();
	}
}


bool Effect::isAlive() const
{
	if(m_system.getSystem() != 0)
	{
		return m_alive;
	}
	return false;
}


void Effect::update(real p_deltaTime, const script::VirtualMachinePtr& /*p_vm*/)
{
	if(m_system.getSystem() != 0)
	{
		// FIXME: Is this fast enough (should we call there is no update function?)
		// Update the particle system through a script
		//p_vm->callSqFun("update", m_system);

		//TT_Printf("[Effect] Updating effect '%s' \n", m_name.c_str());
		m_alive = m_system.getSystem()->update(p_deltaTime);
	}
}


void Effect::render() const
{
#ifdef ENABLE_PARTICLES_DEBUG_RENDERING
	renderDebug();
#endif

	if(m_system.getSystem() != 0)
	{
		//TT_Printf("[Effect] Rendering effect '%s' \n", m_name.c_str());
		m_system.getSystem()->render();
	}
}


void Effect::renderDebug() const
{
	// FIXME: Doesn't seem to work well if used with transformed systems

	if(m_system.getSystem() != 0)
	{
		using namespace tt::engine::renderer;
		debug::DebugRendererPtr dbgRenderer = Renderer::getInstance()->getDebug();

		// Transform system
		MatrixStack::getInstance()->push();
		MatrixStack::getInstance()->load44(
			*reinterpret_cast<const tt::math::Matrix44*>(m_system.getSystem()->getWorldTransform()));

		// Render AA Box for each group
		typedef std::vector<SPK::Group*> Groups;
		typedef std::vector<SPK::Emitter*> Emitters;

		Groups groups = m_system.getSystem()->getGroups();

		for(Groups::iterator it = groups.begin(); it != groups.end(); ++it)
		{
			(*it)->enableAABBComputing(true);
			(*it)->computeAABB();
			math::Vector3 aabbMin((*it)->getAABBMin().x, (*it)->getAABBMin().y, (*it)->getAABBMin().z);
			math::Vector3 aabbMax((*it)->getAABBMax().x, (*it)->getAABBMax().y, (*it)->getAABBMax().z);

			dbgRenderer->renderAABox(ColorRGB::red, aabbMin, aabbMax);

			// Draw a sphere for each emitter position
			Emitters emitters = (*it)->getEmitters();

			for(Emitters::iterator git = emitters.begin(); git != emitters.end(); ++git)
			{
				TT_NULL_ASSERT((*git)->getZone());
				SPK::Vector3D pos = (*git)->getZone()->getPosition();

				// Show emitter position as unit sphere
				dbgRenderer->renderSphere(math::Vector3(pos.x, pos.y, pos.z), 1.0f, false);
			}
		}

		MatrixStack::getInstance()->pop();
	}
}


// Namespace end
}
}
}
