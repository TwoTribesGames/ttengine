#if !defined(INC_TT_ENGINE_EFFECT_EFFECTCOLLECTION_H)
#define INC_TT_ENGINE_EFFECT_EFFECTCOLLECTION_H

#include <tt/engine/effect/EffectManager.h>
#include <tt/math/Vector3.h>
#include <tt/math/Matrix44.h>


namespace tt {
namespace engine {
namespace effect {


class Effect;
typedef tt_ptr<Effect>::shared EffectPtr;

class EffectCollection;
typedef tt_ptr<EffectCollection>::shared EffectCollectionPtr;

typedef std::vector<EffectPtr> Effects;


class EffectCollection
{
public:
	~EffectCollection();

	EffectPtr spawn(const std::string& p_name, Group p_group = Group_Viewport1);
	EffectPtr spawn(const std::string& p_name, const math::Vector3&,  Group p_group = Group_Viewport1);
	EffectPtr spawn(const std::string& p_name, const math::Matrix44&, Group p_group = Group_Viewport1);
	
	bool remove(const EffectPtr& p_effect);
	
	s32 size() const;
	
	void update(real p_deltaTime, Group p_group = Group_Viewport1);
	void render(Group p_group) const;
	
	static EffectCollectionPtr load(const std::string& p_filename);
	
	static void updateDeathRow();

private:
	struct DyingEffect
	{
		Effect* effect;
		s32 timeToLive;
		
		DyingEffect(Effect* p_effect)
		:
		effect(p_effect),
		timeToLive(1)
		{
		}
	};
	typedef std::map<std::string, tt::script::ScriptObject> ScriptObjects;
	typedef std::vector<DyingEffect> DyingEffects;
	explicit EffectCollection(const std::string& p_dirname);
	void precache(std::string p_dirName, bool p_recursive);
	static void scheduleForRemove(Effect* p_effect);
	
	// Keep track of all effectptrs used
	std::string  m_dirname;
	Effects      m_effects[Group_Count];
	ScriptObjects m_scriptCache;
	
	static DyingEffects ms_deathRow;
};

// Namespace end
}
}
}


#endif // INC_TT_ENGINE_EFFECT_EFFECTCOLLECTION_H
