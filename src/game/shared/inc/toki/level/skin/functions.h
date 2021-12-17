#if !defined(INC_TOKI_LEVEL_SKIN_FUNCTIONS_H)
#define INC_TOKI_LEVEL_SKIN_FUNCTIONS_H


#include <tt/engine/scene2d/shoebox/shoebox_types.h>
#include <tt/math/Point2.h>

#include <toki/level/skin/fwd.h>
#include <toki/level/skin/types.h>
#include <toki/level/fwd.h>
#include <toki/level/types.h>


namespace toki {
namespace level {
namespace skin {

void generateSkinShoebox(const SkinContextPtr&                      p_context,
                         const LevelDataPtr&                        p_level,
                         ThemeType                                  p_defaultLevelTheme,
                         const ThemeTiles&                          p_overriddenThemeTiles,
                         tt::engine::scene2d::shoebox::ShoeboxData* p_shoebox_OUT);

namespace impl {


void generateShoebox(const SkinConfig&                          p_config,
                     const MaterialCache&                       p_tiles,
                     const EdgeCache&                           p_edges,
                     impl::GrowEdge*                            p_edgeScratch,
                     s32                                        p_edgeScratchSize,
                     BlobData&                                  p_blobData,
                     tt::engine::scene2d::shoebox::ShoeboxData* p_shoebox_OUT);

void addShoeboxPlanesFromBlobData(const BlobData&                            p_blobData,
                                  const SkinConfig&                          p_config,
                                  tt::engine::scene2d::shoebox::ShoeboxData* p_shoebox_OUT);

void doGrowEdge(Shape p_shape, const TileMaterial& p_material,
                TileSideBit p_toCheckMask, GrowEdge& p_growEdge,
                const tt::math::Point2& p_pos, const tt::math::Point2& p_step,
                const SkinConfig&                          p_config,
                tt::engine::scene2d::shoebox::ShoeboxData* p_shoebox_OUT);


void checkForInsideCornersLeft(const tt::math::Point2& p_pos, const MaterialCache& p_tiles,
                               const EdgeCache& p_edges, const SkinConfig& p_config,
                               tt::engine::scene2d::shoebox::ShoeboxData* p_shoebox_OUT);


void checkForInsideCornersRight(const tt::math::Point2& p_pos, const MaterialCache& p_tiles,
                                const EdgeCache& p_edges, const SkinConfig& p_config,
                                tt::engine::scene2d::shoebox::ShoeboxData* p_shoebox_OUT);


void addShoeboxPlane(const SkinConfig&                          p_config,
                     Shape                                      p_shape,
                     const tt::math::Point2&                    p_tile,
                     const TileMaterial&                        p_material,
                     tt::engine::scene2d::shoebox::ShoeboxData* p_shoebox_OUT);


void addShoeboxPlane(const SkinConfig&                          p_config,
                     const EdgeShape&                           p_edge,
                     tt::engine::scene2d::shoebox::ShoeboxData* p_shoebox_OUT);

void addShoeboxPlaneQuad(const tt::math::Point2& p_min, const tt::math::Point2& p_max,
                         const TileMaterial& p_material, const SkinConfig& p_config,
                         tt::engine::scene2d::shoebox::ShoeboxData* p_shoebox_OUT);

// impl namespace end
}

// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_LEVEL_SKIN_FUNCTIONS_H)
