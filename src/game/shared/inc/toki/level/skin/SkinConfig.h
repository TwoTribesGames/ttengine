#if !defined(INC_TOKI_LEVEL_SKIN_SKINCONFIG_H)
#define INC_TOKI_LEVEL_SKIN_SKINCONFIG_H


#include <map>
#include <string>
#include <vector>

#include <tt/code/ErrorStatus.h>
#include <tt/engine/scene2d/shoebox/shoebox_types.h>
#include <tt/math/Random.h>
#include <tt/xml/fwd.h>

#include <toki/level/skin/types.h>
#include <toki/level/fwd.h>


namespace toki {
namespace level {
namespace skin {

class SkinConfig
{
public:
	struct Plane
	{
		enum TexCoordMode
		{
			TexCoordMode_DoNotModify,   // use texture coordinate as specified in config
			TexCoordMode_UseWorldU,
			TexCoordMode_UseWorldV,
			TexCoordMode_UseWorldU_RandomOffset,
			TexCoordMode_UseWorldV_RandomOffset,
			TexCoordMode_Edge
		};
		
		
		tt::engine::scene2d::shoebox::PlaneData planeData;
		TexCoordMode                            texCoordModeU;
		TexCoordMode                            texCoordModeV;
		
		// For theme coloring: original colors as specified in skin XML
		tt::code::DefaultValue<tt::engine::renderer::ColorRGBA> colorWholeQuad;
		tt::code::DefaultValue<tt::engine::renderer::ColorRGBA> colorTopLeft;
		tt::code::DefaultValue<tt::engine::renderer::ColorRGBA> colorTopRight;
		tt::code::DefaultValue<tt::engine::renderer::ColorRGBA> colorBottomLeft;
		tt::code::DefaultValue<tt::engine::renderer::ColorRGBA> colorBottomRight;
		
		
		inline Plane()
		:
		planeData       (),
		texCoordModeU   (TexCoordMode_DoNotModify),
		texCoordModeV   (TexCoordMode_DoNotModify),
		colorWholeQuad  (planeData.colorWholeQuad),
		colorTopLeft    (planeData.colorTopLeft),
		colorTopRight   (planeData.colorTopRight),
		colorBottomLeft (planeData.colorBottomLeft),
		colorBottomRight(planeData.colorBottomRight)
		{ }
	};
	typedef std::vector<Plane> Planes;
	
	struct PlaneSet
	{
		bool   snapToUVAnchor;
		s32    anchorCount;
		bool   randomizeU;
		real   beginHalfTile;
		real   endHalfTile;
		Planes planes;
		
		
		inline PlaneSet()
		:
		snapToUVAnchor(false),
		anchorCount(4),
		randomizeU(true),
		beginHalfTile(0.5f),
		endHalfTile(0.5f),
		planes()
		{ }
	};
	
	
	SkinConfig();
	
	bool load(const std::string& p_filename);
	void clear();
	
	void resetRandomSeed();
	
	/*! \brief Modifies the vertex colors of all planes in this config so that the per-theme
	           vertex colors specified in the level data are taken into account. */
	void setPlaneColorsFromLevelData(const LevelDataPtr& p_levelData, SkinConfigType p_type);
	
	/*! \brief Returns all the texture pointers that are used by the planes. */
	tt::engine::renderer::TextureContainer getAllUsedTextures() const;
	
	const PlaneSet& getPlanes      (const TileMaterial& p_material, Shape p_shape) const;
	const PlaneSet& getEdgePlanes  (const TileMaterial& p_material, Shape p_shape) const;
	const PlaneSet& getCenterPlanes(const TileMaterial& p_material) const;
	
private:
	// NOTE: All of these nesting levels are their own structs so that they
	//       can be expanded with more functionality later on (if needed)
	typedef std::vector<PlaneSet> PlaneSets;
	
	struct ShapeConfig
	{
		PlaneSets planeSets;
	};
	
	struct MaterialConfig
	{
		ShapeConfig shapes[Shape_Count];
		ShapeConfig edges [Shape_Count];
		PlaneSets   centerPlanes;
	};
	
	
	void loadPlaneSet(const tt::xml::XmlNode* p_planeSetsNode,
	                  PlaneSets*              p_planeSets_OUT,
	                  const Plane&            p_defaultSettings,
	                  const std::string&      p_filename,
	                  tt::code::ErrorStatus*  p_errStatus);
	static bool parseTexCoordMode(const tt::xml::XmlNode* p_node,
	                              const std::string&      p_attributeName,
	                              Plane::TexCoordMode*    p_texCoordMode_OUT);
	const PlaneSet& getRandomPlaneSet(const PlaneSets& p_planeSets) const;
	
	void loadTextures(const PlaneSets&                        p_planeSets,
	                  tt::engine::renderer::TextureContainer* p_textures_OUT) const;
	void setVertexColors(PlaneSets&                             p_planeSets,
	                     const tt::engine::renderer::ColorRGBA& p_color);
	
	
	MaterialConfig m_materialConfig[MaterialType_Count][MaterialTheme_Count];
	
	mutable tt::math::Random m_skinRandom;
};

// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_LEVEL_SKIN_SKINCONFIG_H)
