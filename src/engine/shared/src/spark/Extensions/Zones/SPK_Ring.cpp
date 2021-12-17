//////////////////////////////////////////////////////////////////////////////////
// SPARK particle engine														//
// Copyright (C) 2008-2009 - Julien Fryer - julienfryer@gmail.com				//
//																				//
// This software is provided 'as-is', without any express or implied			//
// warranty.  In no event will the authors be held liable for any damages		//
// arising from the use of this software.										//
//																				//
// Permission is granted to anyone to use this software for any purpose,		//
// including commercial applications, and to alter it and redistribute it		//
// freely, subject to the following restrictions:								//
//																				//
// 1. The origin of this software must not be misrepresented; you must not		//
//    claim that you wrote the original software. If you use this software		//
//    in a product, an acknowledgment in the product documentation would be		//
//    appreciated but is not required.											//
// 2. Altered source versions must be plainly marked as such, and must not be	//
//    misrepresented as being the original software.							//
// 3. This notice may not be removed or altered from any source distribution.	//
//////////////////////////////////////////////////////////////////////////////////

#include <spark/Extensions/Zones/SPK_Ring.h>
#include <spark/Core/SPK_Particle.h>

namespace SPK
{
	Ring::Ring(const Vector3D& p_position,const Vector3D& p_normal,float p_minRadius,float p_maxRadius) :
		Zone(p_position)
	{
		setNormal(p_normal);
		setRadius(p_minRadius, p_maxRadius);
	}

	void Ring::setRadius(float p_minRadius,float p_maxRadius)
	{
		if (p_minRadius < 0.0f) p_minRadius = -p_minRadius;
		if (p_maxRadius < 0.0f) p_maxRadius = -p_maxRadius;
		if (p_minRadius > p_maxRadius) std::swap(p_minRadius, p_maxRadius);
		minRadius = p_minRadius;
		maxRadius = p_maxRadius;
		sqrMinRadius = p_minRadius * p_minRadius;
		sqrMaxRadius = p_maxRadius * p_maxRadius;
	}

	void Ring::generatePosition(Particle& particle,bool /*full*/) const
	{
		Vector3D tmp;
		do tmp = Vector3D(random(-1.0f,1.0f),random(-1.0f,1.0f),random(-1.0f,1.0f));
		while (tmp.getSqrNorm() > 1.0f);
		
		crossProduct(tNormal,tmp,particle.position());
		normalizeOrRandomize(particle.position());

		particle.position() *= std::sqrt(random(sqrMinRadius,sqrMaxRadius)); // to have a uniform distribution
		particle.position() += getTransformedPosition();
	}

	bool Ring::intersects(const Vector3D& p_v0,const Vector3D& p_v1,Vector3D* p_intersection,Vector3D* p_normal) const
	{
		float dist0 = dotProduct(tNormal,p_v0 - getTransformedPosition());
		float dist1 = dotProduct(tNormal,p_v1 - getTransformedPosition());

		if ((dist0 <= 0.0f) == (dist1 <= 0.0f)) // both points are on the same side
			return false;

		if (dist0 <= 0.0f)
			dist0 = -dist0;
		else
			dist1 = -dist1;

		float ti = dist0 / (dist0 + dist1);

		Vector3D vDir(p_v1 - p_v0);
		float norm = vDir.getNorm();

		norm *= ti;
		ti = norm < APPROXIMATION_VALUE ? 0.0f : ti * (norm - APPROXIMATION_VALUE) / norm;

		vDir *= ti;
		Vector3D inter(p_v0 + vDir);

		float distFromCenter = getSqrDist(inter,getTransformedPosition());
		if (distFromCenter > sqrMaxRadius || distFromCenter < sqrMinRadius) // intersection is not in the ring
			return false;

		if (p_intersection != NULL)
		{
			*p_intersection = inter;
			if (p_normal != NULL)
				*p_normal = tNormal;
		}

		return true;
	}
	
	void Ring::moveAtBorder(Vector3D& v,bool /*inside*/) const
	{
		float dist = dotProduct(tNormal,v - getTransformedPosition());	
		v += tNormal * -dist;

		float distFromCenter = getSqrDist(v,getTransformedPosition());

		if (distFromCenter > sqrMaxRadius)
		{
			distFromCenter = std::sqrt(distFromCenter);
			Vector3D vDir(v - getTransformedPosition());
			vDir *= maxRadius / distFromCenter;
			v = getTransformedPosition() + vDir;
		}
		else if (distFromCenter < sqrMinRadius)
		{
			distFromCenter = std::sqrt(distFromCenter);
			Vector3D vDir(v - getTransformedPosition());
			vDir *= minRadius / distFromCenter;
			v = getTransformedPosition() + vDir;
		}
	}

	void Ring::innerUpdateTransform()
	{
		Zone::innerUpdateTransform();
		transformDir(tNormal,normal);
		tNormal.normalize();
	}
}
