#if !defined(INC_TT_ENGINE_EFFECT_EFFECTMANAGER_H)
#define INC_TT_ENGINE_EFFECT_EFFECTMANAGER_H

#include <set>
#include <string>
#include <tt/script/VirtualMachine.h>



namespace tt {
namespace engine {
namespace effect {

class Effect;
class EffectCollection;
typedef std::set<EffectCollection*> EffectCollections;

enum Group
{
	Group_Viewport1 = 0, // Main viewport
	Group_Viewport2 = 1,
	Group_HUD       = 2,

	Group_Count
};


class EffectManager
{
public:
	static void init(s32 p_randomSeed = 1);
	static void update(Group p_group = Group_Viewport1);
	static void computeParticles(Group);
	static void render(Group p_group = Group_Viewport1, bool p_enableDepthSort = true);
	static void shutdown();
	
	inline static script::VirtualMachinePtr getVM() {return ms_vm;}
	
private:
	EffectManager();
	~EffectManager();

	// Keep track of effect collections
	friend class EffectCollection;
	static void registerCollection  (EffectCollection* p_collection);
	static void unregisterCollection(EffectCollection* p_collection);

	static script::VirtualMachinePtr ms_vm;
	static EffectCollections ms_collections;
};


// Namespace end
}
}
}


#endif // INC_TT_ENGINE_EFFECT_EFFECTMANAGER_H
