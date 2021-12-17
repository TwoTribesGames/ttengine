#if !defined(INC_TT_ENGINE_EFFECT_EFFECT_H)
#define INC_TT_ENGINE_EFFECT_EFFECT_H

#include <tt/engine/renderer/fwd.h>
#include <tt/engine/effect/wrapper/SystemWrapper.h>
#include <tt/engine/effect/EffectManager.h>
#include <tt/math/Vector3.h>
#include <tt/math/Matrix44.h>
#include <tt/script/VirtualMachine.h>

namespace SPK {
class System;
}

namespace tt {
namespace engine {
namespace effect {


class Effect
{
public:
	~Effect();

	bool loadFromScript( const std::string& p_filename,     const script::VirtualMachinePtr& p_vm);
	bool runScriptObject(const script::ScriptObject& p_obj, const script::VirtualMachinePtr& p_vm);

	void setPosition(const math::Vector3&);
	void setTransform(const math::Matrix44&);
	
	bool isAlive() const;

	void update(real p_deltaTime, const script::VirtualMachinePtr& p_vm);
	void render() const;
	void renderDebug() const;

	inline std::string getName() const {return m_name;}
	inline Group getGroup() const {return m_group;}
	
private:
	friend class EffectCollection;
	
	explicit Effect(const std::string& p_name, Group p_group);
	
	// Name of the effect
	std::string m_name;
	Group       m_group;
	bool        m_alive;

	wrapper::SystemWrapper m_system;
};

// Namespace end
}
}
}


#endif // INC_TT_ENGINE_EFFECT_EFFECTCOLLECTION_H
