#include <tt/code/bufferutils.h>
#include <tt/engine/renderer/ColorRGBA.h>
#include <tt/engine/renderer/MatrixStack.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/engine/renderer/TrianglestripBuffer.h>
#include <tt/fs/fs.h>
#include <tt/math/Vector3.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>


#include <toki/game/light/LightShape.h>
#include <toki/game/light/Polygon.h>


namespace toki {
namespace game {
namespace light {


//--------------------------------------------------------------------------------------------------
// Public member functions


LightShape::LightShape(const tt::math::Vector2& p_centerPos,
             real p_radius, 
             const tt::engine::renderer::ColorRGB& p_color)
:
m_centerPos(p_centerPos),
m_radius(p_radius),
m_direction(0.0f),
m_halfSpread(tt::math::pi),
m_color(p_color),
m_textureOverride(),
m_shadows(),
m_distanceSort(),
m_toRender(),
m_toAddToRenderOutSideLoop(),
m_shadowsScratch(),
m_backFacingScratch()
#ifdef USE_DEBUG_SHAPE
,
m_debug(p_centerPos, p_radius, tt::engine::renderer::ColorRGB::blue)
#endif
{
	using namespace tt::engine::renderer;
	m_shadows.reserve(256);
	m_distanceSort.reserve(256);
	m_toRender.reserve(256);
	m_toAddToRenderOutSideLoop.reserve(256);
	m_shadowsScratch.reserve(256);
	m_backFacingScratch.reserve(64);
	
	//calculateCircle();
}


LightShape::~LightShape()
{
}


void LightShape::setTexture(const std::string& p_textureName)
{
	m_textureOverride = p_textureName;
	
	if (m_trianglestripBuffer != 0)
	{
		tt::engine::renderer::TexturePtr texture = tt::engine::renderer::TextureCache::get( (m_textureOverride.empty()) ? "lightintensity" : m_textureOverride, "textures.lights", false);
		TT_ASSERTMSG(texture != 0, "Failed to load texture '%s' 'textures.lights'.", m_textureOverride.c_str());
		texture->setAddressMode(tt::engine::renderer::AddressMode_Mirror, tt::engine::renderer::AddressMode_Mirror);
		
		m_trianglestripBuffer->setTexture(texture);
	}
}


void LightShape::update(real /*p_elapsedTime*/, const Polygons& p_occluders, u8 p_centerAlpha)
{
	/*
	BufferVtxUV<1> graphicsVtx;
	typedef std::vector<BufferVtxUV<1> > Graphics;
	Graphics graphcis;
	graphicsVtx.setColor(m_color);
	graphicsVtx.setPosition(m_centerPos.x, m_centerPos.y, 0.0f);
	graphics.push_back(graphicsVtx);
	// */
	
	const real radiusSquared = m_radius * m_radius;
	m_shadowsScratch.clear();
	Shadows& shadows = m_shadowsScratch;
	
	for (Polygons::const_iterator occluderIt = p_occluders.begin(); occluderIt != p_occluders.end(); ++occluderIt)
	{
#define USE_CIRCLE_FOR_SHADOW 0
		
#if USE_CIRCLE_FOR_SHADOW
		
		const tt::math::Vector2& midPoint = (*occluderIt)->getBoundingSphereMidPoint();
		const tt::math::Vector2 ray(midPoint - m_centerPos);
		const real length = ray.length();
		
		/*
		TT_Printf("Light::update - midPoint x: %f, y: %f. Light pos x:%f, y: %f. Ray x: %f, y: %f, length: %f\n",
				  midPoint.x, midPoint.y, m_centerPos.x, m_centerPos.y, ray.x, ray.y, length);
		// */
		
		if (length - (*occluderIt)->getBoundingSphereRadius() <= getRadius())
		{
			tt::math::Vector2 perpendicular(ray.y, -ray.x);
			perpendicular.normalize();
			perpendicular *= (*occluderIt)->getBoundingSphereRadius();
			const tt::math::Vector2 leftPoint( midPoint   - perpendicular);
			const tt::math::Vector2 rightPoint(midPoint   + perpendicular);
			const tt::math::Vector2 left(      leftPoint  - m_centerPos);
			const tt::math::Vector2 right(     rightPoint - m_centerPos);
			
			/*
			TT_Printf("Light::update - leftPoint x: %f, y: %f. (left x: %f, y: %f). rightPoint x:%f, y: %f. (right: x: %f, y: %f).\n",
					  leftPoint.x, leftPoint.y, left.x, left.y, rightPoint.x, rightPoint.y, right.x, right.y);
			// */
			
			TT_ASSERT(tt::math::realEqual(left.length(), right.length()));
			Shadow shadow(left, right);
			
			/*
			TT_Printf("Light::update - shadow leftAngle: %f, rightAngle: %f, leftDistance: %f, rightDistance: %f\n",
			          shadow.leftAngle, shadow.rightAngle, shadow.leftDistance, shadow.rightDistance);
			// */
			
			shadows.push_back(shadow);
		}
		
#else // USE_CIRCLE_FOR_SHADOW
		
		const tt::math::Vector2& midPoint = (*occluderIt)->getBoundingSphereMidPoint();
		const tt::math::Vector2 ray(midPoint - m_centerPos);
		const real length = ray.length(); // distance between both center points.
		
		/*
		TT_Printf("Light::update - midPoint x: %f, y: %f. Light pos x:%f, y: %f. Ray x: %f, y: %f, length: %f\n",
		midPoint.x, midPoint.y, m_centerPos.x, m_centerPos.y, ray.x, ray.y, ray.length());
		// */
		
		// Culling of occluders which are outside the range of this light.
		if (length > (*occluderIt)->getBoundingSphereRadius() + getRadius())
		{
			// The distance between the two center points is larger than both radii.
			continue;
		}
		
		const Vertices& vertices = (*occluderIt)->getVerticesWorldSpace();
		const Vertices& normals  = (*occluderIt)->getNormals();
		TT_ASSERT(vertices.size() == normals.size());
		
		m_backFacingScratch.clear();
		BoolVector& backFacing = m_backFacingScratch;
		//backFacing.reserve(normals.size());
		
		{
			Vertices::const_iterator vtxIt = vertices.begin();
			for (Vertices::const_iterator nmlIt = normals.begin();
			     nmlIt != normals.end() && vtxIt != vertices.end();
				 ++nmlIt, ++vtxIt)
			{
				const tt::math::Vector2 vtxRay((*vtxIt) - m_centerPos);
				const real dot = tt::math::dotProduct((*nmlIt), vtxRay);
				backFacing.push_back(dot > 0);
				/*
				TT_Printf("Ligth::update - dot: %f, backFacing: %d (vtx x: %f, y: %f) (vtxRay x: %f, y: %f)\n",
						  dot, dot > 0, (*nmlIt).x, (*nmlIt).y, vtxRay.x, vtxRay.y);
				// */
			}
		}
		
		if (vertices.empty() == false)
		{
			Vertices::const_iterator vtxIt = vertices.begin();
			tt::math::Vector2 previousVtx  = vertices.back();
			Vertices::const_iterator nmlIt = normals.begin();
			
			for (BoolVector::const_iterator backFaceIt = backFacing.begin();
			     backFaceIt != backFacing.end();
			     ++vtxIt, ++backFaceIt, ++nmlIt)
			{
				TT_ASSERT(vtxIt != vertices.end());
				TT_ASSERT(nmlIt != normals.end());
				
// HACK: This define can be used to switch between shadow till front of back faces.
#define TT_LIGHT_USE_BACKFACE 0 // 1 is 'normal', light continues 'inside' objects. (0 stops at front face)
				
				if (*backFaceIt
#if TT_LIGHT_USE_BACKFACE == 0
					== false
#endif
					)
				{
					const tt::math::Vector2& currentVtx = (*vtxIt);
					
					// Use front instead of back faces.
					const tt::math::Vector2 left( currentVtx  - m_centerPos);
					const tt::math::Vector2 right(previousVtx - m_centerPos);
					
					/*
					TT_Printf("Light::update - currentVtx x: %f, y: %f. (left x: %f, y: %f). "
							  "previousVtx x:%f, y: %f. (right x: %f, y: %f).\n",
							  currentVtx.x,  currentVtx.y,  left.x,  left.y, 
							  previousVtx.x, previousVtx.y, right.x, right.y);
					// */
					
#if TT_LIGHT_USE_BACKFACE
					Shadow shadow(left, right,  (*nmlIt));
#else
					// Invert everything for front face shadows.
					Shadow shadow(right, left, -(*nmlIt));
#endif
					/*
					TT_Printf("Light::update - shadow leftAngle: %f, rightAngle: %f, leftDistance: %f, rigthDistance: %f, minimumDistance: %f\n",
							  shadow.leftAngle, shadow.rightAngle, shadow.leftDistance, shadow.rightDistance, shadow.minimumDistance);
					// */
					if (shadow.minimumDistance <= m_radius)
					{
						// Only add shadows which are in range.
						
						bool shadowIsValid = true;
						
						if (shadow.leftDistance  > m_radius ||
						    shadow.rightDistance > m_radius)
						{
							// Calculate how far from the projection on the shadow line we need to move to get a distance of m_radius.
							real correctionDistance = radiusSquared - shadow.projectionDistance * shadow.projectionDistance;
							correctionDistance = tt::math::sqrt(correctionDistance);
							
							tt::math::Vector2 correctionVec(shadow.normal.getNormalized() * correctionDistance);
							
							if (shadow.leftDistance > m_radius)
							{
								// Move left so it's on the radius distance
								tt::math::Vector2 newPos(shadow.pointClosestToLightOnLine.x - correctionVec.y,
								                         shadow.pointClosestToLightOnLine.y + correctionVec.x);
								const real newLeft = newPos.getAngleWithUnitX();
								if (shadow.angleIsInbetweenRightAndLeft(newLeft) == false)
								{
									shadowIsValid = false;
								}
								shadow.setLeftAngle(newLeft);
							}
							if (shadow.rightDistance > m_radius)
							{
								// Move right so it's on the radius distance
								tt::math::Vector2 newPos(shadow.pointClosestToLightOnLine.x + correctionVec.y,
								                         shadow.pointClosestToLightOnLine.y - correctionVec.x);
								const real newRight = newPos.getAngleWithUnitX();
								if (shadow.angleIsInbetweenRightAndLeft(newRight) == false)
								{
									shadowIsValid = false;
								}
								shadow.setRightAngle(newRight);
							}
						}
						
						if (shadowIsValid)
						{
							shadows.push_back(shadow);
						}
					}
					previousVtx = currentVtx;
				}
				else if (vtxIt != vertices.end())
				{
					previousVtx = (*vtxIt);
				}
			}
		}
#endif
	}
	
	if (m_halfSpread < tt::math::pi)
	{
		const real smallDistance =
#if TT_LIGHT_USE_BACKFACE // Flip the distance so the normal ends up reversed.
		                          -
#endif
		                          0.01f;
		
		// Get the start and end angle for the spotlight.
		// Note that we will be creating shadow for all other angles.
		// We're doing this by creating a 'box' with a gap the size of the beam.
		// We put the corners of this box at 0, halfPi, pi and oneAndHalfPi angles.
		// There are less corners if they are inside the spotlight.
		real startAngle = m_direction - m_halfSpread;
		//TT_Printf("lightShape::update - startAngle: %f <- dir: %f, halfSpread: %f.\n", startAngle, m_direction, m_halfSpread);
		if (startAngle < 0.0f)
		{
			startAngle += tt::math::twoPi;
			//TT_Printf("lightShape::update - startAngle: %f\n", startAngle);
		}
		TT_ASSERT(startAngle >= 0.0f && startAngle < tt::math::twoPi);
		const real endAngle   = startAngle + m_halfSpread + m_halfSpread;
		TT_ASSERT(startAngle <= endAngle);
		
		real shadowAngle = 0.0f;
		
#define SHADOW_SPOTLIGHT 0
		
		if (endAngle > tt::math::twoPi) // Cone goes over 0.
		{
			shadowAngle = endAngle - tt::math::twoPi;
			
#if SHADOW_SPOTLIGHT 
			shadows.push_back(Shadow(0.0f      , shadowAngle    , smallDistance));
			shadows.push_back(Shadow(startAngle, tt::math::twoPi, smallDistance));
			TT_Printf("LightShape::update start: %f, end: %f, fixedEndAngle: %f\n", 
			          startAngle, endAngle, shadowAngle);
#endif
		}
#if SHADOW_SPOTLIGHT 
		else
		{
			shadows.push_back(Shadow(startAngle, endAngle, smallDistance));
			
			TT_Printf("LightShape::update start: %f, end: %f\n", startAngle, endAngle);
		}
#endif
		
#if !SHADOW_SPOTLIGHT 
		while (shadowAngle < tt::math::twoPi)
		{
			const real nextGap = shadowAngle + tt::math::halfPi;
			if (nextGap > startAngle && shadowAngle < endAngle) // Will the next shadow intersect the spotlight?
			{
				shadows.push_back(Shadow(shadowAngle, startAngle, smallDistance));
				shadowAngle = endAngle;
			}
			else if (shadowAngle < tt::math::halfPi)
			{
				shadows.push_back(Shadow(shadowAngle, tt::math::halfPi, smallDistance));
				shadowAngle = tt::math::halfPi;
			}
			else if (shadowAngle < tt::math::pi)
			{
				shadows.push_back(Shadow(shadowAngle, tt::math::pi, smallDistance));
				shadowAngle = tt::math::pi;
			}
			else if (shadowAngle < tt::math::oneAndAHalfPi)
			{
				shadows.push_back(Shadow(shadowAngle, tt::math::oneAndAHalfPi, smallDistance));
				shadowAngle = tt::math::oneAndAHalfPi;
			}
			else
			{
				shadows.push_back(Shadow(shadowAngle, tt::math::twoPi, smallDistance));
				shadowAngle = tt::math::twoPi;
			}
		}
#endif
	}
	
	calculateCircle(shadows, p_centerAlpha);
}


void LightShape::render() const
{
	if (m_trianglestripBuffer != 0)
	{
		m_trianglestripBuffer->applyChanges();
		m_trianglestripBuffer->render();
	}
}


bool LightShape::inSpread(const tt::math::Vector2& p_centerPos, const tt::math::Vector2& p_otherPos) const
{
	if (m_halfSpread >= tt::math::pi)
	{
		return true;
	}
	
	const tt::math::Vector2 localPos(p_otherPos - p_centerPos);
	real angle = tt::math::atan2(localPos.y, localPos.x);
	if (angle < 0.0f)
	{
		angle += tt::math::twoPi;
	}
	TT_ASSERT(angle >= 0.0f && angle < tt::math::twoPi);
	real startAngle = m_direction - m_halfSpread;
	if (startAngle < 0.0f)
	{
		startAngle += tt::math::twoPi;
	}
	TT_ASSERT(startAngle >= 0.0f && startAngle < tt::math::twoPi);
	const real endAngle   = startAngle + m_halfSpread + m_halfSpread;
	TT_ASSERT(startAngle <= endAngle);
	
	return (angle >= startAngle && angle <= endAngle) ||
	       (endAngle > tt::math::twoPi && (angle + tt::math::twoPi) <= endAngle); // The cone goes over 0.
}




void LightShape::serialize(tt::code::BufferWriteContext* p_context) const
{
	namespace bu = tt::code::bufferutils;
	
	bu::put(m_direction      , p_context);
	bu::put(m_halfSpread     , p_context);
	bu::put(m_color          , p_context);
	bu::put(m_textureOverride, p_context);
}


void LightShape::unserialize(tt::code::BufferReadContext* p_context)
{
	namespace bu = tt::code::bufferutils;
	
	m_direction       = bu::get<real                          >(p_context);
	m_halfSpread      = bu::get<real                          >(p_context);
	m_color           = bu::get<tt::engine::renderer::ColorRGB>(p_context);
	m_textureOverride = bu::get<std::string                   >(p_context);
}


// -------------------------------------------------------------------------------------------------
// Private functions


void LightShape::calculateCircle(const Shadows& p_shadows, u8 p_centerAlpha)
{
	using namespace tt::engine::renderer;
	
	const real subdivisionAngle = tt::math::twoPi / segments;
	
	m_rays.clear();
	
	// Get rays which collide
	// Remove overlaping traingles.
	
	m_shadows.clear();
	{
		m_distanceSort.clear();
		
		// Copy p_shadows and split those shadows that overlap 0 degrees because that's easier later.
		for (Shadows::const_iterator it = p_shadows.begin(); it != p_shadows.end(); ++it)
		{
			const Shadow& current = (*it);
			if (current.rightAngle > current.leftAngle) // Rays go over 0 degrees.
			{
				Shadow leftHalf(current);
				leftHalf.setRightAngle(0.0f);
				Shadow rightHalf(current);
				rightHalf.setLeftAngle(tt::math::twoPi);
				
				// Make sure the angles are on the right side of the 0.
				// This was the reason for the split.
				TT_ASSERT(leftHalf.rightAngle == 0.0f);
				m_distanceSort.push_back(leftHalf);
				TT_ASSERT(rightHalf.leftAngle == tt::math::twoPi);
				m_distanceSort.push_back(rightHalf);
			}
			else
			{
				m_distanceSort.push_back(*it);
			}
			
		}
		
		// Make sure we're sorted by distance so the closes shadows are handled first.
		std::sort(m_distanceSort.begin(), m_distanceSort.end(), Shadow::sortDistance);
		
		for (Shadows::iterator it = m_distanceSort.begin(); it != m_distanceSort.end(); ++it)
		{
			/*
			TT_Printf("adding to shadow list left: %f, right: %f, leftDistance: %f, rightDistance: %f\n",
			          (*it).leftAngle, (*it).rightAngle, (*it).leftDistance, (*it).rightDistance);
			// */
			m_shadows.push_back(*it);
		}
	}
	
	//TT_Printf("Light::calculateCircle - shadow count: %d --------------------------------\n", m_shadows.size());
	
	m_toRender.clear();
	
	m_toAddToRenderOutSideLoop.clear();
	
	while (m_shadows.empty() == false)
	{
		Shadow current = m_shadows.front();
		m_shadows.erase(m_shadows.begin());
		
		/*
		TT_Printf("Light::calculateCircle - (%d) current angle left: %f, right: %f, leftDistance: %f, rightDistance: %f. --------\n",
		          m_shadows.size(), current.leftAngle, current.rightAngle, current.leftDistance, current.rightDistance);
		// */
		
		real gapRightAngle = 0.0f;
		bool doneWithCurrent = false;
		
		for (Shadows::iterator it = m_toRender.begin(); it != m_toRender.end();)
		{
			Shadow& other = (*it);
			/*
			TT_Printf("Light::calculateCircle - other angle left: %f, right: %f, leftDistance: %f, rightDistance: %f. - gapRightAngle: %f\n",
			          other.leftAngle, other.rightAngle, other.leftDistance, other.rightDistance, gapRightAngle);
			// */
			
			if (current.leftAngle <= other.rightAngle) // Current starts in front of other.
			{
				if (current.rightAngle < gapRightAngle) // Doesn't fit on the right.
				{
					current.setRightAngle(gapRightAngle); // Clamp
				}
				doneWithCurrent = true;
				it = insertRightAngleSorted(m_toRender, current); // Add to render
				break; // Done with current.
			}
			else if (current.rightAngle >= other.leftAngle) // Current not inside other. (Current start after other)
			{
				gapRightAngle = other.leftAngle; // Move gap indicater.
				++it;
				continue; // Examen next other.
			}
			// Current is (partially) overlapping with other.
			else if (current.leftAngle <= other.leftAngle) // Current does not pass other.
			{
				if (current.rightAngle < other.rightAngle) // Part of current is sticking out to the right.
				{
					// FIXME: This doesn't work if current and other intersect with the shadow part of their lines.
					//        If they intersect the angle to the intersection point needs to be set to both.
					if (current.leftDistance - 0.001f >= other.getIntersectionPoint(current.leftAngle).length())
					{
						const real prevDistance = current.minimumDistance;
						// current is behind the other so it must get smaller.
						current.setLeftAngle(other.rightAngle);
						if (prevDistance != current.minimumDistance)
						{
							// The distance changed, need to re-sort.
							m_toAddToRenderOutSideLoop.push_back(current);
							doneWithCurrent = true;
							break;
						}
					}
					else
					{
						other.setRightAngle(current.leftAngle);
					}
					if (current.rightAngle < gapRightAngle) // Make sure we stay in the gap.
					{
						current.setRightAngle(gapRightAngle);
					}
					it = insertRightAngleSorted(m_toRender, current); // Add to render.
					doneWithCurrent = true;
					break;
				}
				
				// Check if current is infront of other.
				if (current.leftDistance - 0.001f < other.getIntersectionPoint(current.leftAngle).length())
				{
					
					if (current.rightDistance - 0.001f < other.getIntersectionPoint(current.rightAngle).length())
					{
						// Current right is infront of other.
						
						// Check if other sticks out of current to the right, because then it needs to be split.
						if (other.rightAngle < current.rightAngle)
						{
							// Create new split of other.
							Shadow rightSplit(other);
							rightSplit.setLeftAngle(current.rightAngle);
							m_toAddToRenderOutSideLoop.push_back(rightSplit);
						}
						// current is in front of the other.
						other.setRightAngle(current.leftAngle); // Clip the other
						++it;
						continue; // move to next.
					}
					else
					{
						// Current is full blocked. (dismiss.)
						doneWithCurrent = true;
						break;
					}
				}
				
				doneWithCurrent = true;
				break; // Done with current.
			}
			else // Current is partially overlapping with other, we'll need to split it.
			{
				// Check if right sticks out.
				if (current.rightAngle < other.rightAngle)
				{
					// Create a new Shadow with left = other.rightAngle and right = current.rightAngle
					Shadow rightSplit(current);
					// FIXME: This doesn't work if current and other interface with the shadow part of their lines.
					//if (rightSplit.leftDistance >= other.rightDistance)
					{
						// RightSplit is behind the other so it must get smaller.
						rightSplit.setLeftAngle(other.rightAngle);
					}
					//else
					//{
					//	other.setRightAngle(rightSplit.leftAngle);
					//}
					if (rightSplit.rightAngle < gapRightAngle)
					{
						rightSplit.setRightAngle(gapRightAngle);
					}
					gapRightAngle = other.leftAngle; // Move gap before other is invalid. (by inserting rigthSplit in m_toRender.)
					/*
					TT_Printf("Light::calculateCircle - rightSplit angle left: %f, right: %f, leftDistance: %f, rigthDistance: %f.\n",
					          rightSplit.leftAngle, rightSplit.rightAngle, rightSplit.leftDistance, rightSplit.rightDistance);
					// */
					m_toAddToRenderOutSideLoop.push_back(rightSplit);
					
					current.setRightAngle(gapRightAngle); // Clamp left split to known gap.
					// Current needs to be reinserted into m_shadows with a distance sort.
					Shadows::iterator shadowIt = m_shadows.begin();
					for (; shadowIt != m_shadows.end(); ++shadowIt)
					{
						if (Shadow::sortDistance((*shadowIt), current) == false)
						{
							shadowIt = m_shadows.insert(shadowIt, current);
							break;
						}
					}
					if (shadowIt == m_shadows.end()) // Was current inserted?
					{
						m_shadows.push_back(current);
					}
					doneWithCurrent = true;
					break; // Can't continue with current. Select new shadow from m_shadows (need to continue with the closest.)
				}
				else
				{
					if (current.rightAngle < other.leftAngle) // Part of current is sticking out to the left of other.
					{
						// Current Right and Other Left need to fit.
						
						if (current.rightDistance - 0.001f >= other.getIntersectionPoint(current.right).length())
						{
							// current is behind the other so it must get smaller.
							current.setRightAngle(other.leftAngle);
						}
						else if (current.getIntersectionPoint(other.leftAngle).length() - 0.001f >= other.leftDistance)
						{
							current.setRightAngle(other.leftAngle);
						}
						else
						{
							other.setLeftAngle(current.rightAngle);
						}
					}
					gapRightAngle = other.leftAngle;
				}
				current.setRightAngle(gapRightAngle); // Clamp to known gap.
				++it;
				continue; // Continue to next other.
			}
		}
		// Done checking the others, is there still room?
		if (doneWithCurrent == false && gapRightAngle < tt::math::twoPi)
		{
			if (current.rightAngle < gapRightAngle)
			{
				current.setRightAngle(gapRightAngle);
			}
			TT_ASSERT(current.leftAngle <= tt::math::twoPi);
			insertRightAngleSorted(m_toRender, current); // Add to render
			gapRightAngle = current.leftAngle;
		}
		for (Shadows::iterator it = m_toAddToRenderOutSideLoop.begin(); it != m_toAddToRenderOutSideLoop.end(); ++it)
		{
			insertDistanceSorted(m_shadows, (*it));
		}
		m_toAddToRenderOutSideLoop.clear();
	}
	
	for (Shadows::iterator it = m_toRender.begin(); it != m_toRender.end(); ++it)
	{
		/*
		TT_Printf("final m_toRender list (pre radius clamp) left: %f, right: %f, leftDistance: %f, rightDistance: %f\n",
				  (*it).leftAngle, (*it).rightAngle, (*it).leftDistance, (*it).rightDistance);
		// */
		tt::math::clamp((*it).leftDistance,  0.0f, m_radius);
		tt::math::clamp((*it).rightDistance, 0.0f, m_radius);
	}
	
#if 1 // 1 to run new ray generation code; 0 to run old ray code.
	
#define FILL_GAPS 1
	
	//TT_Printf("Light::calculateCircle - fill gaps with 'normal' rays. ------------------\n");
	
	// Fill in gaps with 'normal' rays.
	
	real angle = 0.0f;
	bool lastRayWasShadow = false;
	{
		Shadows::const_iterator it = m_toRender.begin();
		
		for (; it != m_toRender.end(); ++it)
		{
			const Shadow& shadow = (*it);
			
			if (shadow.rightAngle > angle)
			{
				// We're not starting in the shadow.
#if FILL_GAPS
				//TT_Printf("Light::calculateCircle - preshadow light angle: %f - m_radius: %f\n", angle, m_radius);
				m_rays.push_back(tt::math::Vector2(tt::math::cos(angle) * m_radius,
				                                   tt::math::sin(angle) * m_radius));
#else
				m_rays.push_back(tt::math::Vector2(0.0f, 0.0f));
#endif
				lastRayWasShadow = false;
			}
			
			// Found (big enough) gap, fill it.
			while (angle + subdivisionAngle < shadow.rightAngle)
			{
				angle = tt::math::floor(((angle + subdivisionAngle) / subdivisionAngle) + 0.001f) * subdivisionAngle;
#if FILL_GAPS
				//TT_Printf("Light::calculateCircle - gap %f - m_radius: %f\n", angle, m_radius);
				m_rays.push_back(tt::math::Vector2(tt::math::cos(angle) * m_radius,
				                                   tt::math::sin(angle) * m_radius));
#endif
			}
			angle = shadow.rightAngle;
			//TT_Printf("Light::calculateCircle - right %f - m_radius: %f rightDistance: %f\n", angle, m_radius, shadow.rightDistance);
#if FILL_GAPS
			if (lastRayWasShadow == false)
			{
				m_rays.push_back(tt::math::Vector2(tt::math::cos(angle) * m_radius,
				                                   tt::math::sin(angle) * m_radius));
			}
#endif
			
			m_rays.push_back(tt::math::Vector2(tt::math::cos(angle) * shadow.rightDistance,
			                                   tt::math::sin(angle) * shadow.rightDistance));
			lastRayWasShadow = true;
			
			// Found (big enough) gap with the shadow, fill it.
			while (angle + subdivisionAngle < shadow.leftAngle)
			{
				angle = tt::math::floor(((angle + subdivisionAngle) / subdivisionAngle) + 0.001f) * subdivisionAngle;
#if FILL_GAPS
				//TT_Printf("Light::calculateCircle - in shadow gap %f - distance: %f\n", angle, distance);
				const real distance = shadow.getIntersectionPoint(angle).length();
				m_rays.push_back(tt::math::Vector2(tt::math::cos(angle) * distance,
				                                   tt::math::sin(angle) * distance));
#endif
			}
			
			angle = shadow.leftAngle;
			//TT_Printf("Light::calculateCircle - left %f - shadow.leftDistance: %f, m_radius: %f\n", angle, shadow.leftDistance, m_radius);
			m_rays.push_back(tt::math::Vector2(tt::math::cos(angle) * shadow.leftDistance,
			                                   tt::math::sin(angle) * shadow.leftDistance));
		}
	}
	
	if (/*lastRayWasShadow &&*/ angle < tt::math::twoPi)
	{
#if FILL_GAPS
		//TT_Printf("Light::calculateCircle - first angle: %f - m_radius: %f\n", angle, m_radius);
		m_rays.push_back(tt::math::Vector2(tt::math::cos(angle) * m_radius,
		                                   tt::math::sin(angle) * m_radius));
#else
		m_rays.push_back(tt::math::Vector2(0.0f, 0.0f));
#endif
	}
	
#if FILL_GAPS
	// Found (big enough) gap, fill it.
	while (angle + subdivisionAngle < tt::math::twoPi)
	{
		angle = tt::math::floor(((angle + subdivisionAngle) / subdivisionAngle) + 0.001f) * subdivisionAngle;
		//TT_Printf("Light::calculateCircle - ending gap %f - m_radius: %f\n", angle, m_radius);
		m_rays.push_back(tt::math::Vector2(tt::math::cos(angle) * m_radius,
		                                   tt::math::sin(angle) * m_radius));
	}
	
	if (angle < tt::math::twoPi)
	{
		//TT_Printf("Light::calculateCircle - twoPi %f - m_radius: %f\n", tt::math::twoPi, m_radius);
		m_rays.push_back(tt::math::Vector2(tt::math::cos(tt::math::twoPi) * m_radius,
		                                   tt::math::sin(tt::math::twoPi) * m_radius));
	}
#endif
	
#else // The old render code
	
#define FILL_GAPS 1
	
	//TT_Printf("Light::calculateCircle - fill gaps with 'normal' rays. ------------------\n");
	
	// Fill in gaps with 'normal' rays.
	
	real angle = 0.0f;
	{
		ShadowsList::const_iterator it = m_toRender.begin();
		
#if FILL_GAPS
		//TT_Printf("Light::calculateCircle - first angle: %f\n", angle);
		m_rays.push_back(tt::math::Vector2(tt::math::cos(angle) * m_radius,
		                                   tt::math::sin(angle) * m_radius));
#else
		m_rays.push_back(tt::math::Vector2(0.0f, 0.0f));
#endif
		
		for (; it != m_toRender.end(); ++it)
		{
			const Shadow& shadow = (*it);
			
			// Found (big enough) gap, fill it.
			while (angle + subdivisionAngle < shadow.rightAngle)
			{
				angle = tt::math::floor(((angle + subdivisionAngle) / subdivisionAngle) + 0.001f) * subdivisionAngle;
#if FILL_GAPS
				//TT_Printf("Light::calculateCircle - gap %f\n", angle);
				m_rays.push_back(tt::math::Vector2(tt::math::cos(angle) * m_radius,
				                                   tt::math::sin(angle) * m_radius));
#endif
			}
			angle = shadow.rightAngle;
			//TT_Printf("Light::calculateCircle - right %f\n", angle);
#if FILL_GAPS
			m_rays.push_back(tt::math::Vector2(tt::math::cos(angle) * m_radius,
			                                   tt::math::sin(angle) * m_radius));
#endif
			
			m_rays.push_back(tt::math::Vector2(tt::math::cos(angle) * shadow.rightDistance,
			                                   tt::math::sin(angle) * shadow.rightDistance));
			
			// Found (big enough) gap with the shadow, fill it.
			while (angle + subdivisionAngle < shadow.leftAngle)
			{
				angle = tt::math::floor(((angle + subdivisionAngle) / subdivisionAngle) + 0.001f) * subdivisionAngle;
#if FILL_GAPS
				const real distance = shadow.getIntersectionPoint(angle).length();
				m_rays.push_back(tt::math::Vector2(tt::math::cos(angle) * distance,
				                                   tt::math::sin(angle) * distance));
#endif
			}
			
			angle = shadow.leftAngle;
			//TT_Printf("Light::calculateCircle - left %f\n", angle);
			m_rays.push_back(tt::math::Vector2(tt::math::cos(angle) * shadow.leftDistance,
			                                   tt::math::sin(angle) * shadow.leftDistance));
#if FILL_GAPS
			m_rays.push_back(tt::math::Vector2(tt::math::cos(angle) * m_radius,
			                                   tt::math::sin(angle) * m_radius));
#else
			m_rays.push_back(tt::math::Vector2(0.0f, 0.0f));
#endif
		}
	}
	
#if FILL_GAPS
	// Found (big enough) gap, fill it.
	while (angle + subdivisionAngle < tt::math::twoPi)
	{
		angle = tt::math::floor(((angle + subdivisionAngle) / subdivisionAngle) + 0.001f) * subdivisionAngle;
		//TT_Printf("Light::calculateCircle - ending gap %f\n", angle);
		m_rays.push_back(tt::math::Vector2(tt::math::cos(angle) * m_radius,
										   tt::math::sin(angle) * m_radius));
	}
	
	//TT_Printf("Light::calculateCircle - twoPi %f\n", tt::math::twoPi);
	m_rays.push_back(tt::math::Vector2(tt::math::cos(tt::math::twoPi) * m_radius,
									   tt::math::sin(tt::math::twoPi) * m_radius));
#endif
	
#endif // End of old code.
	
	if (m_rays.empty())
	{
		if (m_trianglestripBuffer != 0)
		{
			m_trianglestripBuffer.reset();
			return;
		}
	}
	
	const s32 vertexCount = static_cast<s32>(m_rays.size() + 1); // Add center point
	if (m_trianglestripBuffer == 0)
	{
		m_trianglestripBuffer.reset(new TrianglestripBuffer(vertexCount,
															1,
															TexturePtr(),
															BatchFlagTrianglestrip_UseVertexColor,
															tt::engine::renderer::TrianglestripBuffer::PrimitiveType_TriangleFan));
		
		setTexture(m_textureOverride);
	}
	else if (m_trianglestripBuffer->getTotalVerticesCount() != vertexCount)
	{
		m_trianglestripBuffer->resizeBuffers(vertexCount);
	}
	
	BufferVtxUV<1> defaultValue;
	defaultValue.setColor(tt::engine::renderer::ColorRGBA(0, 0, 0, 0));
	m_trianglestripBuffer->resize<1>(vertexCount, defaultValue);
	
	// Center position
	{
		BufferVtxUV<1>& vtx = m_trianglestripBuffer->modifyVtx<1>(0);
		vtx.setPosition(m_centerPos.x, m_centerPos.y, 0.0f);
		vtx.setColor(tt::engine::renderer::ColorRGBA(m_color, p_centerAlpha));
		vtx.setTexCoord(0.0f, 0.0f);
		/*
		const tt::math::Vector3&               pos   = vtx.getPosition();
		const tt::engine::renderer::ColorRGBA& color = vtx.getColor();
		TT_Printf("Light::calculateCircle - center pos x: %f, y: %f, z: %f, color r:%u, g:%u, b:%u, a:%u\n", 
				  pos.x, pos.y, pos.z, color.r, color.g, color.b, color.a);
		// */
	}
	
	s32 vtxIdx = 1;
	for (Vertices::iterator it = m_rays.begin(); it != m_rays.end(); ++it, ++vtxIdx)
	{
		BufferVtxUV<1>& vtx = m_trianglestripBuffer->modifyVtx<1>(vtxIdx);
		const tt::math::Vector2& vec = (*it);
		vtx.setPosition(m_centerPos.x + vec.x, m_centerPos.y + vec.y, 0.0f);
		vtx.setTexCoord(vec.x / m_radius, vec.y / m_radius);
		/*
		const real length = vec.length();
		// HACK: Give the light circle a clear edge by not going to zero but to half center alpha.
		real alphaNormalized = (m_radius - (length * 0.75f)) / m_radius;
		// HACK: was: real alphaNormalized = (m_radius - length) / m_radius;
		tt::math::clamp(alphaNormalized, 0.0f, 1.0f);
		*/
		const u8 alpha = p_centerAlpha; // TEMP HACK static_cast<u8>(alphaNormalized * p_centerAlpha);
		vtx.setColor(tt::engine::renderer::ColorRGBA(m_color, alpha));
		
		/*
		const tt::math::Vector3&               pos   = vtx.getPosition();
		const tt::engine::renderer::ColorRGBA& color = vtx.getColor();
		TT_Printf("Light::calculateCircle - pos x: %f, y: %f, z: %f, color r:%u, g:%u, b:%u, a:%u\n", 
		pos.x, pos.y, pos.z, color.r, color.g, color.b, color.a);
		// */
	}
}


Shadows::iterator LightShape::insertRightAngleSorted(Shadows& p_list, const Shadow& p_shadow)
{
	s32 index = 0;
	for (Shadows::iterator it = p_list.begin(); it != p_list.end(); ++it)
	{
		if (Shadow::sortRightAngle(p_shadow, (*it)))
		{
			/*
			TT_Printf("Light::insertRightAngleSorted - inserted current at index: %d. (l: %f, r: %f, ld: %f, rd: %f).\n",
					  index, p_shadow.leftAngle, p_shadow.rightAngle, p_shadow.leftDistance, p_shadow.rightDistance);
			// */
			return p_list.insert(it, p_shadow);
		}
		++index;
	}
	
	p_list.push_back(p_shadow);
	/*
	TT_Printf("Light::insertRightAngleSorted - inserted current at back. (l: %f, r: %f, ld: %f, rd: %f).\n",
			  p_shadow.leftAngle, p_shadow.rightAngle, p_shadow.leftDistance, p_shadow.rightDistance);
	// */
	return p_list.end();
}



Shadows::iterator LightShape::insertDistanceSorted(Shadows& p_list, const Shadow& p_shadow)
{
	for (Shadows::iterator it = p_list.begin(); it != p_list.end(); ++it)
	{
		if (Shadow::sortDistance(p_shadow, (*it)))
		{
			return p_list.insert(it, p_shadow);
		}
	}
	
	p_list.push_back(p_shadow);
	return p_list.end();
}

// Namespace end
}
}
}
