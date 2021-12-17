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

#include <spark/Extensions/Modifiers/SPK_Vortex.h>
#include <spark/Core/SPK_Particle.h>

namespace SPK
{
	Vortex::Vortex(const Vector3D& p_position,const Vector3D& p_direction,float p_rotationSpeed,float p_attractionSpeed) :
		Modifier(ALWAYS | INSIDE_ZONE | OUTSIDE_ZONE,ALWAYS,false,false,NULL),
		rotationSpeed(p_rotationSpeed),
		attractionSpeed(p_attractionSpeed),
		angularSpeedEnabled(false),
		linearSpeedEnabled(false),
		eyeRadius(0.0f),
		killingParticleEnabled(false)
	{
		setPosition(p_position);
		setDirection(p_direction);
	}

	void Vortex::modify(Particle& particle,float deltaTime) const
	{
		// Distance of the projection point from the position of the vortex
		float dist = dotProduct(tDirection,particle.position() - tPosition);
		
		// Position of the rotation center (orthogonal projection of the particle)
		Vector3D rotationCenter = tDirection;
		rotationCenter *= dist;
		rotationCenter += tPosition;

		// Distance of the particle from the eye of the vortex
		dist = getDist(rotationCenter,particle.position());

		if (dist <= eyeRadius)
		{
			if (killingParticleEnabled)
				particle.kill();
			return;
		}

		float angle = angularSpeedEnabled ? rotationSpeed * deltaTime : rotationSpeed * deltaTime / dist;

		// Computes ortho base
		Vector3D particleNormal = (particle.position() - rotationCenter) / dist;
        Vector3D tangent = crossProduct(tDirection,particleNormal);

        float endRadius = linearSpeedEnabled ? dist * (1.0f - attractionSpeed * deltaTime) : dist - attractionSpeed * deltaTime;
        if (endRadius <= eyeRadius)
		{
		    endRadius = eyeRadius;
		    if (killingParticleEnabled)
                particle.kill();
		}

        particle.position() = rotationCenter + particleNormal * endRadius * std::cos(angle) + tangent * endRadius * std::sin(angle);
	}

	void Vortex::innerUpdateTransform()
	{
		Modifier::innerUpdateTransform();
		transformPos(tPosition,position);
		transformDir(tDirection,direction);
		tDirection.normalize();
	}
}
