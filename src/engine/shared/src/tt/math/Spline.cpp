#include <tt/math/Spline.h>


namespace tt {
namespace math {


Spline::Spline()
:
m_length(0.0),
m_knotsCount(0),
m_knots(),
m_knotDistances()
{
}


Spline::~Spline()
{
	// Clear knots container
	m_knots.clear();

	// Clear distances container
	m_knotDistances.clear();
}


bool Spline::load(tt::streams::BIStream& p_stream)
{
	// read the number of knots
	p_stream >> m_knotsCount;
	
	m_knots.reserve(static_cast<std::size_t>(m_knotsCount));
	m_knotDistances.reserve(static_cast<std::size_t>(m_knotsCount));
	for ( s32 i = 0; i < m_knotsCount; i++ )
	{
		SplineKnot knot;
		p_stream >> knot;
		//knot.load(p_stream);
		m_knots.push_back(knot);
	}
	
	// Generate the length
	recalculateLength(20);
	
	return true;
}


bool Spline::save(tt::streams::BOStream& p_stream) const
{
	// write the number of knots
	p_stream << m_knotsCount;
	
	// write the knots
	for ( KnotsCollection::const_iterator it = m_knots.begin(); it != m_knots.end(); ++it )
	{
		SplineKnot knot(*it);
		p_stream << *this;
		//(*it).save(p_stream);
	}
	return true;
}


//real Spline::getLength() const
//{
//	// Check it doesnt overflow
//	/*if ( std::abs(m_length.getValue() >> 8) > 0x7fffffff )
//	{
//		TT_PANIC("Spline length out of range");
//	}*/
//	return static_cast<real>(m_length);
//}


real Spline::recalculateLength(real /*p_step*/)
{
//	// Check the size
//	if ( p_step <= 0.0f || p_step >= 64.0f )
//	{
//		step = 5.0f;
//	}
//
//	m_length = 0;
//
//	// The first knot is 0 obviously
//	//step = 64;
//    // m_knotDistance is already allocated in contructor, memory leak
//	m_knotDistance = O3D_New real[m_knotsCount * 64 * 3];
//	m_knotDistance[0] = 0;
//	m_knotDistance[1] = 1;
//	m_knotDistance[2] = 0;
//
//	real index = 1;
//
//	real length = 0;
//
//	// Run through all the knots 
//	for ( int i = 1; i < m_knotsCount; i++ )
//	{
//		// Get the two knots
//		CO3DSplineKnot *k0 = &m_knots[i - 1];
//		CO3DSplineKnot *k1 = &m_knots[i];
//
//		// Get the starting position
//		Vector3 prev = k0->GetPosition();
//
//		for ( int s = 1; s <= step; s++ )
//		{
//			// Get the time
//			real time = (s << 20) / step;
//
//			// Precalc useable values
//			real T = time;
//			real fOneT = O3D_FIX20ONE - T;
//			real fOneT3 = O3D_FIX20MUL3(fOneT, fOneT, fOneT);
//			real f3TOneT = O3D_FIX20MUL3(T, O3D_TOFIX20(3.0), fOneT);
//			real f3TOneT2 = O3D_FIX20MUL(f3TOneT, fOneT);
//			real f3T2OneT = O3D_FIX20MUL(f3TOneT, T);
//			real fT3 = O3D_FIX20ONE - fOneT3 - f3TOneT2 - f3T2OneT;
//
//			// Calculate the point
////			Vector3 spline_point = (k0->GetPosition() * fOneT3) + (k0->GetOutVector() * O3D_FIXMUL(f3TOneT, fOneT)) + (k1->GetInVector() * O3D_FIXMUL(f3TOneT, time)) + (k1->GetPosition() * fT3);
//			Vector3 spline_point;
////		spline_point = (k0->GetPosition() * (fOneT3 >> 8)) + (k0->GetOutVector() * (f3TOneT2 >> 8)) + (k1->GetInVector() * (f3T2OneT >> 8)) + (k1->GetPosition() * (fT3 >> 8));
////		spline_point.x( spline_point.x >> 4 );
////		spline_point.y( spline_point.y >> 4 );
////		spline_point.z( spline_point.z >> 4 );
//			s64 p0x = ((s64)k0->GetPosition().x) << 8;
//			s64 p0y = ((s64)k0->GetPosition().y) << 8;
//			s64 p0z = ((s64)k0->GetPosition().z) << 8;
//			s64 p1x = ((s64)k1->GetPosition().x) << 8;
//			s64 p1y = ((s64)k1->GetPosition().y) << 8;
//			s64 p1z = ((s64)k1->GetPosition().z) << 8;
//			s64 t0x = ((s64)k0->GetOutVector().x) << 8;
//			s64 t0y = ((s64)k0->GetOutVector().y) << 8;
//			s64 t0z = ((s64)k0->GetOutVector().z) << 8;
//			s64 t1x = ((s64)k1->GetInVector().x) << 8;
//			s64 t1y = ((s64)k1->GetInVector().y) << 8;
//			s64 t1z = ((s64)k1->GetInVector().z) << 8;
//
//			fOneT3 = (fOneT3 >> 0);
//			f3TOneT2 = (f3TOneT2 >> 0);
//			f3T2OneT = (f3T2OneT >> 0);
//			fT3 = (fT3 >> 0);
//
//			spline_point.x( (((p0x * fOneT3) >> 20) + ((t0x * f3TOneT2) >> 20) + ((t1x * f3T2OneT) >> 20) + ((p1x * fT3) >> 20)) >> 8 );
//			spline_point.y( (((p0y * fOneT3) >> 20) + ((t0y * f3TOneT2) >> 20) + ((t1y * f3T2OneT) >> 20) + ((p1y * fT3) >> 20)) >> 8 );
//			spline_point.z( (((p0z * fOneT3) >> 20) + ((t0z * f3TOneT2) >> 20) + ((t1z * f3T2OneT) >> 20) + ((p1z * fT3) >> 20)) >> 8 );
//
////			spline_point.x( (((s64)k0->GetPosition().x * fOneT3) + ((s64)k0->GetOutVector().x * f3TOneT2) + ((s64)k1->GetInVector().x * f3T2OneT) + ((s64)k1->GetPosition().x * fT3)) >> 20 );
////			spline_point.y( (((s64)k0->GetPosition().y * fOneT3) + ((s64)k0->GetOutVector().y * f3TOneT2) + ((s64)k1->GetInVector().y * f3T2OneT) + ((s64)k1->GetPosition().y * fT3)) >> 20 );
////			spline_point.z( (((s64)k0->GetPosition().z * fOneT3) + ((s64)k0->GetOutVector().z * f3TOneT2) + ((s64)k1->GetInVector().z * f3T2OneT) + ((s64)k1->GetPosition().z * fT3)) >> 20 );
//
//			// Add the length
//			length += ((spline_point - prev).Length() << 8);
//
//			m_knotDistance[(index * 3) + 0] = length;
//			m_knotDistance[(index * 3) + 1] = i;
//			m_knotDistance[(index * 3) + 2] = time;
//			index++;
//
//			prev = spline_point;
//		}
//	}
//
//	m_length = length;
//
//	return (real)(m_length >> 8);
	return 0;
}


Vector3 Spline::model(real p_u, Spline::Mode p_mode) const
{
	Vector3 spline_point;

	// Handle start / end cases
	if (p_u <= 0.0f)
	{
		spline_point = m_knots[0].getPosition();
	}
	else if (p_u >= 1.0f)
	{
		spline_point = m_knots[static_cast<u32>(m_knotsCount-1)].getPosition();
	}
	else
	{
//		real64 seg = 0;
//		real64 time = 0;

		// Work out the segment and time we need 
		switch (p_mode)
		{
			case Spline::Mode_Time:
			{
				// Now find the segment
//				real64 frac = ((m_knotsCount - 1) * u);
//				seg = frac >> 20;
//				time = frac - (seg << 20);
				break;
			}
			
//
//		case SplineMode_Length:
//			{
//				// Get the length
//				s64 len = O3D_FIX20MUL(m_length, u);
//
//				// Find the segment for that length
//				for ( int i = 1; i < m_knotsCount * 64; i++ )
//				{
//					if ( m_knotDistance[(i * 3) + 0] >= len )
//					{
//						s64 seg_len = m_knotDistance[(i * 3) + 0] - m_knotDistance[((i - 1) * 3) + 0];
//						s64 frac = O3D_FIX20DIV(O3D_FIX20MUL((len - m_knotDistance[((i - 1) * 3) + 0]), O3D_FIX20ONE), seg_len);
//						seg = m_knotDistance[(i * 3) + 1] - 1;
//
//						if ( m_knotDistance[((i - 1) * 3) + 2] == 0x00100000 )
//						{
//							time = O3D_FIX20MUL(m_knotDistance[(i * 3) + 2], frac);
//						}
//						else
//						{
//							time = m_knotDistance[((i - 1) * 3) + 2] + O3D_FIX20MUL((m_knotDistance[(i * 3) + 2] - m_knotDistance[((i - 1) * 3) + 2]), frac);
//						}
//						break;
//					}
//				}
//			}
//			break;
			
		default:
			break;
		}
//
//		// Precalc useable values
//		real T = time;
//		real fOneT = O3D_FIX20ONE - T;
//		real fOneT3 = O3D_FIX20MUL3(fOneT, fOneT, fOneT);
//		real f3TOneT = O3D_FIX20MUL3(T, O3D_TOFIX20(3.0), fOneT);
//		real f3TOneT2 = O3D_FIX20MUL(f3TOneT, fOneT);
//		real f3T2OneT = O3D_FIX20MUL(f3TOneT, T);
//		real fT3 = O3D_FIX20ONE - fOneT3 - f3TOneT2 - f3T2OneT;
//
//		// Get the two knots
//		CO3DSplineKnot *k0 = &m_knots[seg];
//		CO3DSplineKnot *k1 = &m_knots[seg + 1];
//
//		// Calculate the point
////		spline_point = (k0->GetPosition() * (fOneT3 >> 8)) + (k0->GetOutVector() * (f3TOneT2 >> 8)) + (k1->GetInVector() * (f3T2OneT >> 8)) + (k1->GetPosition() * (fT3 >> 8));
//			s64 p0x = ((s64)k0->GetPosition().x) << 8;
//			s64 p0y = ((s64)k0->GetPosition().y) << 8;
//			s64 p0z = ((s64)k0->GetPosition().z) << 8;
//			s64 p1x = ((s64)k1->GetPosition().x) << 8;
//			s64 p1y = ((s64)k1->GetPosition().y) << 8;
//			s64 p1z = ((s64)k1->GetPosition().z) << 8;
//			s64 t0x = ((s64)k0->GetOutVector().x) << 8;
//			s64 t0y = ((s64)k0->GetOutVector().y) << 8;
//			s64 t0z = ((s64)k0->GetOutVector().z) << 8;
//			s64 t1x = ((s64)k1->GetInVector().x) << 8;
//			s64 t1y = ((s64)k1->GetInVector().y) << 8;
//			s64 t1z = ((s64)k1->GetInVector().z) << 8;
//
//			fOneT3 = (fOneT3 >> 0);
//			f3TOneT2 = (f3TOneT2 >> 0);
//			f3T2OneT = (f3T2OneT >> 0);
//			fT3 = (fT3 >> 0);
//
//			spline_point.x = ( (((p0x * fOneT3) >> 20) + ((t0x * f3TOneT2) >> 20) + ((t1x * f3T2OneT) >> 20) + ((p1x * fT3) >> 20)) >> 8 );
//			spline_point.y = ( (((p0y * fOneT3) >> 20) + ((t0y * f3TOneT2) >> 20) + ((t1y * f3T2OneT) >> 20) + ((p1y * fT3) >> 20)) >> 8 );
//			spline_point.z = ( (((p0z * fOneT3) >> 20) + ((t0z * f3TOneT2) >> 20) + ((t1z * f3T2OneT) >> 20) + ((p1z * fT3) >> 20)) >> 8 );
////		spline_point.x = ( spline_point.x >> 4 );
////		spline_point.y = ( spline_point.y >> 4 );
////		spline_point.z = ( spline_point.z >> 4 );
////		spline_point.x = ( (((s64)k0->GetPosition().x * fOneT3) + ((s64)k0->GetOutVector().x * f3TOneT2) + ((s64)k1->GetInVector().x * f3T2OneT) + ((s64)k1->GetPosition().x * fT3)) >> 20 );
////		spline_point.y = ( (((s64)k0->GetPosition().y * fOneT3) + ((s64)k0->GetOutVector().y * f3TOneT2) + ((s64)k1->GetInVector().y * f3T2OneT) + ((s64)k1->GetPosition().y * fT3)) >> 20 );
////		spline_point.z = ( (((s64)k0->GetPosition().z * fOneT3) + ((s64)k0->GetOutVector().z * f3TOneT2) + ((s64)k1->GetInVector().z * f3T2OneT) + ((s64)k1->GetPosition().z * fT3)) >> 20 );

	}

	return spline_point;
}


// Namespace end
}
}

