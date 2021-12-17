#if !defined(INC_TOKI_LEVEL_SKIN_GROWEDGE_H)
#define INC_TOKI_LEVEL_SKIN_GROWEDGE_H

#include <tt/engine/scene2d/shoebox/fwd.h>

#include <toki/level/skin/fwd.h>
#include <toki/level/skin/types.h>
#include <toki/level/skin/TileMaterial.h>
#include <toki/level/fwd.h>
#include <toki/level/types.h>

#define TT_SINGLE_SIDE_EDGES 1 // 1 to have edges on every side. 0 if two sides are needed to get an edge.


namespace toki {
namespace level {
namespace skin {
namespace impl {


struct EdgeShape
{
public:
	Shape            shape;
	tt::math::Point2 startTile;
	s32              distance;
	bool             startHalf;
	bool             endHalf;
	TileMaterial     material;
	
	EdgeShape()
	:
	shape(Shape_None),
	startTile(0,0),
	distance(0),
	startHalf(false),
	endHalf(false),
	material()
	{}
	
	inline bool isValid() const { return material.isNone() == false; }
	inline void invalidate()    { material.makeNone();               }
};


struct GrowEdge : public EdgeShape
{
public:
#if !TT_SINGLE_SIDE_EDGES
	Shape previousTileShape; // Used to check two connected tiles for a shape.
#endif
	
	GrowEdge()
	:
	EdgeShape()
#if !TT_SINGLE_SIDE_EDGES
	,
	previousTileShape(Shape_None)
#endif
	{}
	
	void stop(bool p_endHalf,  const tt::math::Point2& p_pos, const tt::math::Point2& p_step,
	          const SkinConfig& p_config, tt::engine::scene2d::shoebox::ShoeboxData* p_shoebox_OUT);
	
	void stopAtLevelBorder(const tt::math::Point2& p_pos, const tt::math::Point2& p_step,
	                       const SkinConfig& p_config, tt::engine::scene2d::shoebox::ShoeboxData* p_shoebox_OUT);
	
	void start(Shape p_shape, const TileMaterial& p_material, bool p_startHalf, const tt::math::Point2& p_pos);
	
	void startAtLevelBorder(Shape p_shape, const TileMaterial& p_material, const tt::math::Point2& p_pos,
	                        const tt::math::Point2& p_step);
	
#if !TT_SINGLE_SIDE_EDGES
	void stopAndContinue(Shape p_shape, const TileMaterial& p_material, const tt::math::Point2& p_pos,
	                     const tt::math::Point2& p_step, Shape p_previousTileShapeValid,
	                     const SkinConfig& p_config,
	                     tt::engine::scene2d::shoebox::ShoeboxData* p_shoebox_OUT, bool p_stop = false,
	                     bool p_stopWithEndHalf = false);
#endif
};

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_LEVEL_SKIN_GROWEDGE_H)
