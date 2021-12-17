#include <toki/level/types.h>


namespace toki {
namespace level {

// All collision types that represent air
const CollisionTypes g_collisionTypesAir(
		CollisionTypes(CollisionType_Air      ) |
		CollisionTypes(CollisionType_AirPrefer) |
		CollisionTypes(CollisionType_AirAvoid ));

// All collision types that are solid:
const CollisionTypes g_collisionTypesSolid(
		CollisionTypes(CollisionType_Solid                   ) |
		CollisionTypes(CollisionType_Solid_Allow_Pathfinding ) |
		CollisionTypes(CollisionType_Solid_FluidKill         ) |
		CollisionTypes(CollisionType_Crystal_Solid           ));

// All collision types that are solid for pathfinding:
const CollisionTypes g_collisionTypesSolidForPathfinding(
		CollisionTypes(CollisionType_Solid                   ) |
		CollisionTypes(CollisionType_Solid_FluidKill         ) |
		CollisionTypes(CollisionType_Crystal_Solid           ));

// All collision types that block light:
const CollisionTypes g_collisionTypesLightBlocking(
		CollisionTypes(CollisionType_Solid                  ) |
		CollisionTypes(CollisionType_Solid_Allow_Pathfinding) |
		CollisionTypes(CollisionType_Solid_FluidKill        ));

// All collision types that block sound:
const CollisionTypes g_collisionTypesSoundBlocking(
		CollisionTypes(CollisionType_Solid                   ) |
		CollisionTypes(CollisionType_Solid_Allow_Pathfinding ) |
		CollisionTypes(CollisionType_Solid_FluidKill         ) |
		CollisionTypes(CollisionType_Crystal_Solid           ) |
		CollisionTypes(CollisionType_Water_Still             ) |
		CollisionTypes(CollisionType_Lava_Still              ));

// All collision types representing fluids:
const CollisionTypes g_collisionTypesFluids(
		CollisionTypes(CollisionType_Water_Source            ) |
		CollisionTypes(CollisionType_Water_Still             ) |
		CollisionTypes(CollisionType_Lava_Source             ) |
		CollisionTypes(CollisionType_Lava_Still              ) |
		CollisionTypes(CollisionType_Solid_FluidKill         ) |
		CollisionTypes(CollisionType_FluidKill               ));

// All collision types representing still fluids:
const CollisionTypes g_collisionTypesFluidsStill(
		CollisionTypes(CollisionType_Water_Still             ) |
		CollisionTypes(CollisionType_Lava_Still              ));

// All collision types representing fluid sources:
const CollisionTypes g_collisionTypesFluidsSource(
		CollisionTypes(CollisionType_Water_Source) |
		CollisionTypes(CollisionType_Lava_Source ));

// All collision types representing crystals:
const CollisionTypes g_collisionTypesCrystal(CollisionType_Crystal_Solid);


// Namespace end
}
}
