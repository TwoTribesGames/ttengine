#if !defined(INC_TOKI_GAME_SCRIPT_WRAPPERS_EFFECTRECTWRAPPER_H)
#define INC_TOKI_GAME_SCRIPT_WRAPPERS_EFFECTRECTWRAPPER_H


#include <tt/code/fwd.h>
#include <tt/math/Vector2.h>

#include <toki/game/entity/effect/fwd.h>


namespace toki {
namespace game {
namespace script {
namespace wrappers {

/*! \brief 'EffectRect' in Squirrel. */
class EffectRectWrapper
{
public:
	EffectRectWrapper();
	EffectRectWrapper(const entity::effect::EffectRectHandle& p_effectRect);
	
	// Squirrel bindings:
	/*! \brief Set size with vector. */
	void setSizeVec(const tt::math::Vector2& p_size);
	/*! \brief Set size. */
	inline void setSize(real p_x, real p_y) { setSizeVec(tt::math::Vector2(p_x, p_y)); }
	
	/*! \brief Set border size with vector. */
	void setBorderVec(const tt::math::Vector2& p_borderSize);
	/*! \brief Set border size. */
	inline void setBorder(real p_x, real p_y) { setBorderVec(tt::math::Vector2(p_x, p_y)); }
	/*! \brief Set border size for all directions at once.*/
	void setBorderSize(  real p_size);
	/*! \brief Set Left border size. */
	void setLeftBorder(  real p_size);
	/*! \brief Set Right border size. */
	void setRightBorder( real p_size);
	/*! \brief Set Top border size. */
	void setTopBorder(   real p_size);
	/*! \brief Set Bottom border size. */
	void setBottomBorder(real p_size);
	
	/*! \brief Set base strength instantly. */
	void setBaseStrengthInstant(real p_strength);
	/*! \brief Set base strength. (Will do a transition from current base strength to new target.) */
	void setBaseStrength(real p_strength);
	
	void serialize  (tt::code::BufferWriteContext* p_context) const;
	void unserialize(tt::code::BufferReadContext*  p_context);
	
	static void bind(const tt::script::VirtualMachinePtr& p_vm);
	
	inline const entity::effect::EffectRectHandle& getHandle() const { return m_effectRect; }
	inline       entity::effect::EffectRectHandle& getHandle()       { return m_effectRect; }
	
private:
	entity::effect::EffectRectHandle m_effectRect;
};

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_SCRIPT_WRAPPERS_EFFECTRECTWRAPPER_H)
