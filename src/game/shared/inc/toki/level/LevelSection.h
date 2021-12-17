#if !defined(INC_TOKI_LEVEL_LEVELSECTION_H)
#define INC_TOKI_LEVEL_LEVELSECTION_H


#include <tt/math/Rect.h>

#include <toki/level/entity/EntityInstance.h>
#include <toki/level/fwd.h>


namespace toki {
namespace level {

class LevelSection;
typedef tt_ptr<LevelSection>::shared LevelSectionPtr;

/*! \brief Level data storage with a specific position. */
class LevelSection
{
public:
	enum Type
	{
		Type_Entities   = 0x01,
		Type_Attributes = 0x02,
		Type_All        = 0xffff
	};
	
	static LevelSectionPtr createFromLevelRect(
		const LevelDataPtr&        p_levelToCopyFrom,
		const tt::math::PointRect& p_rectToCopy,
		Type                       p_type = Type_All);
	
	static LevelSectionPtr createEmpty(const tt::math::PointRect& p_rect,
	                                   Type                       p_type = Type_All);
	~LevelSection();
	
	/*! \brief Applies the level data from this section to the specified level, at this section's position. */
	void applyToLevel(const LevelDataPtr& p_targetLevel) const;
	
	/*! \return The rectangle covered by this section. */
	tt::math::PointRect getRect() const;
	
	LevelSectionPtr clone() const;
	
	void addEntity  (const entity::EntityInstancePtr& p_entity);
	void addEntities(const entity::EntityInstances&   p_entities);
	
	/*! \brief Prints details of this section to the debug output. */
#if !defined(TT_BUILD_FINAL)
	void debugPrint() const;
#else
	inline void debugPrint() const { }
#endif
	
	inline const tt::math::Point2& getPosition() const                        { return m_position;  }
	inline void                    setPosition(const tt::math::Point2& p_pos) { m_position = p_pos; }
	
	inline const AttributeLayerPtr&       getAttributeLayer() const { return m_attributes; }
	inline const entity::EntityInstances& getEntities()       const { return m_entities;   }
	
private:
	LevelSection(const tt::math::Point2&  p_position,
	             const AttributeLayerPtr& p_attributes,
	             Type                     p_type);
	LevelSection(const LevelSection& p_rhs);
	
	/*! \brief Copies level data in the rectangle covered by this section from the specified level. */
	void copyFromLevel(const LevelDataPtr& p_sourceLevel);
	
	// No assignment
	LevelSection& operator=(const LevelSection&);
	
	
	tt::math::Point2        m_position;
	AttributeLayerPtr       m_attributes;
	entity::EntityInstances m_entities;
	Type                    m_type;
};

// Namespace end
}
}


#endif  // !defined(INC_TOKI_LEVEL_LEVELSECTION_H)
