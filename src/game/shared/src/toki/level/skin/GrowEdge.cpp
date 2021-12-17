#include <tt/platform/tt_printf.h>

#include <toki/level/skin/functions.h>
#include <toki/level/skin/GrowEdge.h>


namespace toki {
namespace level {
namespace skin {
namespace impl {


void GrowEdge::stop(bool p_endHalf,  const tt::math::Point2& p_pos, const tt::math::Point2& p_step,
                    const SkinConfig& p_config,
                    tt::engine::scene2d::shoebox::ShoeboxData* p_shoebox_OUT)
{
	if (isValid())
	{
		if (shape != Shape_None)
		{
#if !TT_SINGLE_SIDE_EDGES
			if (previousTileShape != Shape_None)
			{
				stopAndContinue(previousTileShape, material, p_pos, p_step,
				                static_cast<Shape>(previousTileShape & shape), p_config, p_shoebox_OUT, true, p_endHalf);
				
				// Did have nothing to continue?
				if (shape == Shape_None)
				{
					invalidate();
					return;
				}
			}
			else
#else
			(void)p_step;
			(void)p_pos;
#endif
			{
				++distance;
			}
			endHalf = p_endHalf;
			addShoeboxPlane(p_config, *this, p_shoebox_OUT);
		}
		invalidate();
	}
}


void GrowEdge::stopAtLevelBorder(const tt::math::Point2& p_pos, const tt::math::Point2& p_step,
                                 const SkinConfig& p_config,
                                 tt::engine::scene2d::shoebox::ShoeboxData* p_shoebox_OUT)
{
	if (isValid() == false)
	{
		return;
	}
	
#if !TT_SINGLE_SIDE_EDGES
	if (shape == Shape_None)
	{
		if (previousTileShape == Shape_None)
		{
			invalidate();
			return;
		}
		else
		{
			shape = previousTileShape;
			previousTileShape = Shape_None;
		}
	}
	
	if (previousTileShape != Shape_None)
	{
		stopAndContinue(previousTileShape, material, p_pos, p_step, previousTileShape, 
		                p_config, p_shoebox_OUT, true);
		
		// Did have nothing to continue?
		if (shape == Shape_None)
		{
			if (previousTileShape == Shape_None)
			{
				invalidate();
				return;
			}
			else
			{
				shape = previousTileShape;
				previousTileShape = Shape_None;
			}
		}
	}
	else
#else
	(void)p_step;
	(void)p_pos;
#endif
	{
		++distance;
	}
	
	distance += Constants_LevelExtensionSize;
	
	endHalf = false;
	addShoeboxPlane(p_config, *this, p_shoebox_OUT);
	
	invalidate();
}


void GrowEdge::start(Shape p_shape, const TileMaterial& p_material, bool p_startHalf, const tt::math::Point2& p_pos)
{
#if TT_SINGLE_SIDE_EDGES
	shape             = p_shape;
#else
	shape             = Shape_None;
	previousTileShape = p_shape;
#endif
	material          = p_material;
	startHalf         = p_startHalf;
	startTile         = p_pos;
	distance          = 0;
}


void GrowEdge::startAtLevelBorder(Shape p_shape, const TileMaterial& p_material, const tt::math::Point2& p_pos,
                                  const tt::math::Point2& p_step)
{
	shape             = p_shape;
#if !TT_SINGLE_SIDE_EDGES
	previousTileShape = Shape_None;
#endif
	material          = p_material;
	startHalf         = false;
	startTile         = p_pos - (p_step * Constants_LevelExtensionSize);
	distance          = Constants_LevelExtensionSize;
}


#if !TT_SINGLE_SIDE_EDGES
void GrowEdge::stopAndContinue(Shape p_shape, const TileMaterial& p_material, const tt::math::Point2& p_pos,
                               const tt::math::Point2& p_step, Shape p_previousTileShapeValid,
                               const SkinConfig& p_config,
                               tt::engine::scene2d::shoebox::ShoeboxData* p_shoebox_OUT, bool p_stop,
                               bool p_stopWithEndHalf)
{
	/*
	Here we check three different shapes to see how they fit together.
	We have the old shape (shape), the center shape (previousTileShape) and the new shape (p_shape).
	*/
	
	// The center shape may keep the sides that are the same sides as new or old.
	const Shape centerShape = static_cast<Shape>((shape & previousTileShape) | ((p_stop) ? 0 : p_previousTileShapeValid));
	
	const bool newConnected = (p_previousTileShapeValid == centerShape/* && p_stop == false*/); //((centerShape & p_previousTileShapeValid) == p_previousTileShapeValid);
	
	tt::math::Point2 newPos = p_pos - p_step;
	
	
	const bool oldConnected = (shape == centerShape);
	
	if (oldConnected == false)
	{
		--distance;
	}
	
	previousTileShape = Shape_None;
	const TileMaterial continueMaterial = material; // Restore material after stop.
	stop(p_stopWithEndHalf && oldConnected, p_pos, p_step, p_config, p_shoebox_OUT);
	material = continueMaterial;
	
	if (newConnected == false &&
	    oldConnected == false)
	{
		EdgeShape centerEdgeShape;
		centerEdgeShape.shape     = centerShape;
		centerEdgeShape.startTile = newPos;
		centerEdgeShape.distance  = 1;
		centerEdgeShape.startHalf = false;
		centerEdgeShape.endHalf   = false;
		centerEdgeShape.material  = material;
		addShoeboxPlane(p_config, centerEdgeShape, p_shoebox_OUT);
		newPos += p_step; // back to p_pos.
	}
	start(p_previousTileShapeValid /*p_shape*/, p_material, false /*newHalf*/, newPos);
	
	shape             = p_previousTileShapeValid;
	previousTileShape = (p_shape == p_previousTileShapeValid) ? Shape_None : p_shape;
	
	if (newConnected)
	{
		++distance;
	}
	
	return;
}
#endif

// Namespace end
}
}
}
}
