#if !defined(INC_TOKI_LEVEL_ATTRIBUTELAYERSECTION_H)
#define INC_TOKI_LEVEL_ATTRIBUTELAYERSECTION_H


#include <tt/math/Rect.h>
#include <tt/str/str_types.h>

#include <toki/level/fwd.h>


namespace toki {
namespace level {

/*! \brief Attribute storage with a specific position. */
class AttributeLayerSection
{
public:
	static AttributeLayerSectionPtr createFromLayerRect(
			const AttributeLayerPtr&   p_layerToCopyFrom,
			const tt::math::PointRect& p_rectToCopy);
	/*! \brief Create an AttributeLayerSection from the ASCII representation of collision tiles.
	    \note This only parses CollisionType from the passed text, no ThemeType! */
	static AttributeLayerSectionPtr createFromText(
			const tt::str::Strings& p_lines,
			const tt::math::Point2& p_pos);
	static AttributeLayerSectionPtr createFromRect(const tt::math::PointRect& p_tileRect,
	                                               level::CollisionType       p_type);
	static AttributeLayerSectionPtr createEmpty(const tt::math::PointRect& p_rect);
	~AttributeLayerSection();
	
	/*! \brief Parses the specified ASCII representation of theme tiles and applies them to this layer section.
	    \note The width and height of the ASCII representation must fit within the existing section. The section will not be resized. */
	void copyThemeTilesFromText(const tt::str::Strings& p_lines);
	
	/*! \brief Copies tiles in the rectangle covered by this section from the specified layer. */
	void copyFromLayer(const AttributeLayerPtr& p_sourceLayer);
	
	/*! \brief Applies the tiles from this section to the specified layer, at this section's position.
	    \param p_applyAirTiles Overwrite target tile if source tile is Air. */
	void applyToLayer(const AttributeLayerPtr& p_targetLayer,
	                  bool                     p_applyAirTiles = true) const;
	
	/*! \return The rectangle covered by this section. */
	tt::math::PointRect getRect() const;
	
	bool contains(const tt::math::Point2& p_pos) const;
	
	AttributeLayerSectionPtr clone() const;
	
	/*! \brief Prints details of this section to the debug output. */
#if !defined(TT_BUILD_FINAL)
	void debugPrint() const;
#else
	inline void debugPrint() const { }
#endif
	
	inline const tt::math::Point2& getPosition() const                        { return m_position;  }
	inline void                    setPosition(const tt::math::Point2& p_pos) { m_position = p_pos; }
	
	inline const AttributeLayerPtr& getAttributeLayer() const { return m_attributes; }
	
private:
	AttributeLayerSection(const tt::math::Point2&  p_position,
	                      const AttributeLayerPtr& p_attributes);
	
	// No copying
	AttributeLayerSection(const AttributeLayerSection&);
	AttributeLayerSection& operator=(const AttributeLayerSection&);
	
	
	tt::math::Point2  m_position;
	AttributeLayerPtr m_attributes;
};

// Namespace end
}
}


#endif  // !defined(INC_TOKI_LEVEL_ATTRIBUTELAYERSECTION_H)
