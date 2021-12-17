#if !defined(INC_TT_ENGINE_PARTICLES_PARTICLE)
#define INC_TT_ENGINE_PARTICLES_PARTICLE

#include <tt/math/Vector3.h>
#include <tt/math/Vector2.h>
#include <tt/engine/renderer/ColorRGBA.h>

namespace tt {
namespace engine {
namespace particles {

// Particle structure
struct Particle
{
	// Current properties
	math::Vector2 position;
	math::Vector2 velocity;
	real velocity_friction;
	real energy;
	real size;
	math::Vector2 scale;
	real weight;
	real rotation;
	real rotation_friction;
	
	// Update properties
	real lifetime;          // Particle lifetime (in seconds)
	real start_size;        // Original size
	real end_size;          // Size of the particle
	real start_weight;      // Original weight
	real end_weight;        // Determines effect of forces
	real start_rotation;
	real rotation_speed;
	real rotation_force;
	
	
	// Animation
	std::size_t anim_idx;
	s32         frame;
	real        animation_time;
	
	math::Vector3 tex_transform;
	
	renderer::ColorRGBA	start_color;	// Start color of the particle
	renderer::ColorRGBA	end_color;		// Final color of the particle
	
	real fade_in;
	real fade_out;
	
	
	inline Particle()
	:
	position(math::Vector2::zero),
	velocity(math::Vector2::zero),
	velocity_friction(0.0f),
	energy(0.0f),
	size(0.0f),
	scale(math::Vector2::allOne),
	weight(0.0f),
	rotation(0.0f),
	rotation_friction(0.0f),
	lifetime(0.0f),
	start_size(0.0f),
	end_size(0.0f),
	start_weight(0.0f),
	end_weight(0.0f),
	start_rotation(0.0f),
	rotation_speed(0.0f),
	rotation_force(0.0f),
	anim_idx(0),
	frame(0),
	animation_time(0.0f),
	tex_transform(math::Vector3::zero),
	start_color(renderer::ColorRGB::black),
	end_color(renderer::ColorRGB::black),
	fade_in(0.0f),
	fade_out(0.0f)
	{
	}
};

// Namespace end
} 
}
}

#endif // !defined(INC_TT_ENGINE_PARTICLES_PARTICLE)
