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


#include <spark/Extensions/Modifiers/SPK_Collision.h>
#include <spark/Core/SPK_Group.h>


namespace SPK
{
	Collision::Collision(float p_scale,float p_elasticity) :
		Modifier(),
		scale(p_scale)
	{
		setElasticity(p_elasticity);
	}

	void Collision::modify(Particle& particle,float /*deltaTime*/) const
	{
		size_t index = particle.getIndex();
		float radius1 = particle.getParamCurrentValue(PARAM_SIZE) * scale * 0.5f;
		float m1 = particle.getParamCurrentValue(PARAM_MASS);
		Group& group = *particle.getGroup();

		// Tests collisions with all the particles that are stored before in the pool
		for (size_t i = 0; i < index; ++i)
		{
			Particle& particle2 = group.getParticle(i);
			float radius2 = particle2.getParamCurrentValue(PARAM_SIZE) * scale * 0.5f;				
			
			float sqrRadius = radius1 + radius2;
			sqrRadius *= sqrRadius;

			// Gets the normal of the collision plane
			Vector3D collisionNormal = particle.position();
			collisionNormal -= particle2.position();
			float sqrDist = collisionNormal.getSqrNorm();

			if (sqrDist < sqrRadius) // particles are intersecting each other
			{
				Vector3D delta = particle.velocity();
				delta -= particle2.velocity();

				if (dotProduct(collisionNormal,delta) < 0.0f) // particles are moving towards each other
				{
					float oldSqrDist = getSqrDist(particle.oldPosition(),particle2.oldPosition());
					if (oldSqrDist > sqrDist)
					{
						// Disables the move from this frame
						particle.position() = particle.oldPosition();
						particle2.position() = particle2.oldPosition();

						collisionNormal = particle.position();
						collisionNormal -= particle2.position();

						if (dotProduct(collisionNormal,delta) >= 0.0f)
							continue;
					}

					collisionNormal.normalize();

					// Gets the normal components of the velocities
					Vector3D normal1(collisionNormal);
					Vector3D normal2(collisionNormal);
					normal1 *= dotProduct(collisionNormal,particle.velocity());
					normal2 *= dotProduct(collisionNormal,particle2.velocity());

					// Resolves collision
					float m2 = particle2.getParamCurrentValue(PARAM_MASS);

					if (oldSqrDist < sqrRadius && sqrDist < sqrRadius)
					{
						// Tweak to separate particles that intersects at both t - deltaTime and t
						// In that case the collision is no more considered as punctual
						if (dotProduct(collisionNormal,normal1) < 0.0f)
						{
							particle.velocity() -= normal1;
							particle2.velocity() += normal1;
						}

						if (dotProduct(collisionNormal,normal2) > 0.0f)
						{
							particle2.velocity() -= normal2;
							particle.velocity() += normal2;
						}
					}
					else
					{
						// Else classic collision equations are applied
						// Tangent components of the velocities are left untouched
						particle.velocity() -= (1.0f + (elasticity * m2 - m1) / (m1 + m2)) * normal1;
						particle2.velocity() -= (1.0f + (elasticity * m1 - m2) / (m1 + m2)) * normal2;

						normal1 *= ((1.0f + elasticity) * m1) / (m1 + m2);
						normal2 *= ((1.0f + elasticity) * m2) / (m1 + m2);

						particle.velocity() += normal2;
						particle2.velocity() += normal1;
					}
				}
			}
		}
	}
}
