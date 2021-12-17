#include <tt/code/ErrorStatus.h>
#include <tt/engine/scene2d/shoebox/shoebox.h>
#include <tt/fs/fs.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
#include <tt/str/str.h>
#include <tt/system/Time.h>
#include <tt/xml/util/parse.h>
#include <tt/xml/XmlDocument.h>
#include <tt/xml/XmlNode.h>

#include <toki/level/skin/SkinConfig.h>
#include <toki/level/skin/TileMaterial.h>
#include <toki/level/LevelData.h>


namespace toki {
namespace level {
namespace skin {

static const SkinConfig::PlaneSet g_emptyPlaneSet;


//--------------------------------------------------------------------------------------------------
// Public member functions

SkinConfig::SkinConfig()
:
m_skinRandom()
{
	resetRandomSeed();
}


bool SkinConfig::load(const std::string& p_filename)
{
	if (tt::fs::fileExists(p_filename) == false)
	{
		TT_PANIC("Cannot load level auto-dress (skin) config: file '%s' does not exist.", p_filename.c_str());
		return false;
	}
	
#if !defined(TT_BUILD_FINAL)
	const u64 timestampLoadStart = tt::system::Time::getInstance()->getMilliSeconds();
#endif
	
	using tt::xml::XmlNode;
	
	tt::xml::XmlDocument doc(p_filename);
	const XmlNode* rootNode = doc.getRootNode();
	if (rootNode == 0 || rootNode->getName() != "skin_config")
	{
		TT_PANIC("Cannot load level auto-dress (skin) config: file is not valid XML or root element is not 'skin_config'.\n"
		         "Root element: <%s>\nFilename: '%s'",
		         (rootNode == 0) ? "" : rootNode->getName().c_str(), p_filename.c_str());
		return false;
	}
	
	Plane defaultSettingsCenterPlane;
	Plane defaultSettingsShape;
	Plane defaultSettingsEdge;
	
	defaultSettingsCenterPlane.texCoordModeU = Plane::TexCoordMode_UseWorldU;
	defaultSettingsCenterPlane.texCoordModeV = Plane::TexCoordMode_UseWorldV;
	
	defaultSettingsEdge.texCoordModeU = Plane::TexCoordMode_Edge;
	
	// FIXME: This deep for-loop nesting looks ridiculous
	
	for (const XmlNode* materialNode = rootNode->getChild();
	     materialNode != 0; materialNode = materialNode->getSibling())
	{
		if (materialNode->getName() != "material")
		{
			TT_PANIC("Invalid level auto-dress (skin) config data: <skin_config> element has unsupported child <%s>. "
			         "Only <material> is supported.\nFilename: '%s'",
			         materialNode->getName().c_str(), p_filename.c_str());
			continue;
		}
		
		const std::string& materialTypeName(materialNode->getAttribute("type"));
		const MaterialType materialType = getMaterialTypeFromName(materialTypeName);
		if (isValidMaterialType(materialType) == false)
		{
			TT_PANIC("Invalid level auto-dress (skin) config data: "
			         "<material> element has unsupported material type: '%s'.\nFilename: '%s'",
			         materialTypeName.c_str(), p_filename.c_str());
			continue;
		}
		
		const std::string&  materialThemeName(materialNode->getAttribute("theme"));
		const MaterialTheme materialTheme = getMaterialThemeFromName(materialThemeName);
		if (isValidMaterialTheme(materialTheme) == false)
		{
			TT_PANIC("Invalid level auto-dress (skin) config data: "
			         "<material> element has unsupported theme: '%s'.\nFilename: '%s'",
			         materialThemeName.c_str(), p_filename.c_str());
			continue;
		}
		
		
		for (const XmlNode* shapeNode = materialNode->getChild();
		     shapeNode != 0; shapeNode = shapeNode->getSibling())
		{
			if (shapeNode->getName() == "center_plane")
			{
				TT_ERR_CREATE("Parse center plane from skin config file '" << p_filename << "'.");
				loadPlaneSet(
						shapeNode,
						&m_materialConfig[materialType][materialTheme].centerPlanes,
						defaultSettingsCenterPlane,
						p_filename,
						&errStatus);
				TT_ERR_ASSERT_ON_ERROR();
				continue;
			}
			else if (shapeNode->getName() != "shape" &&
			         shapeNode->getName() != "edge")
			{
				TT_PANIC("Invalid level auto-dress (skin) config data: <material> element has unsupported child <%s>. "
				         "Only <shape>, <edge> and <center_plane> are supported.\nFilename: '%s'",
				         shapeNode->getName().c_str(), p_filename.c_str());
				continue;
			}
			
			const bool isEdgeNode = (shapeNode->getName() == "edge");
			
			const std::string& shapeName(shapeNode->getAttribute("type"));
			const Shape        shape = getShapeFromName(shapeName);
			if (isValidShape(shape) == false)
			{
				TT_PANIC("Invalid level auto-dress (skin) config data: "
				         "<%s> element has unsupported type: '%s'.\nFilename: '%s'",
				         shapeNode->getName().c_str(), shapeName.c_str(), p_filename.c_str());
				continue;
			}
			
			// For <edge> nodes, only accept edge shapes
			if (isEdgeNode && isEdgeShape(shape) == false)
			{
				TT_PANIC("Invalid level auto-dress (skin) config data: "
				         "<%s> element has unsupported type: '%s'.\n"
				         "This element only allows edge types.\nFilename: '%s'",
				         shapeNode->getName().c_str(), shapeName.c_str(), p_filename.c_str());
				continue;
			}
			
			if (isEdgeNode)
			{
				TT_ERR_CREATE("Parse edge element from skin config file '" << p_filename << "'.");
				loadPlaneSet(
						shapeNode,
						&m_materialConfig[materialType][materialTheme].edges[shape].planeSets,
						defaultSettingsEdge,
						p_filename,
						&errStatus);
				TT_ERR_ASSERT_ON_ERROR();
			}
			else
			{
				TT_ERR_CREATE("Parse shape element from skin config file '" << p_filename << "'.");
				loadPlaneSet(
						shapeNode,
						&m_materialConfig[materialType][materialTheme].shapes[shape].planeSets,
						defaultSettingsShape,
						p_filename,
						&errStatus);
				TT_ERR_ASSERT_ON_ERROR();
			}
		}
	}
	
#if !defined(TT_BUILD_FINAL)
	const u64 timestampLoadEnd = tt::system::Time::getInstance()->getMilliSeconds();
	TT_Printf("SkinConfig::load: Load took %u ms (file '%s').\n",
	          u32(timestampLoadEnd - timestampLoadStart), p_filename.c_str());
#endif
	
	return true;
}


void SkinConfig::clear()
{
	for (s32 typeIndex = 0; typeIndex < MaterialType_Count; ++typeIndex)
	{
		for (s32 themeIndex = 0; themeIndex < MaterialTheme_Count; ++themeIndex)
		{
			for (s32 shapeIndex = 0; shapeIndex < Shape_Count; ++shapeIndex)
			{
				m_materialConfig[typeIndex][themeIndex].shapes[shapeIndex].planeSets.clear();
				m_materialConfig[typeIndex][themeIndex].edges [shapeIndex].planeSets.clear();
			}
			m_materialConfig[typeIndex][themeIndex].centerPlanes.clear();
		}
	}
}


void SkinConfig::resetRandomSeed()
{
	m_skinRandom.setSeed(1337u);
}


void SkinConfig::setPlaneColorsFromLevelData(const LevelDataPtr& p_levelData, SkinConfigType p_type)
{
	TT_NULL_ASSERT(p_levelData);
	
	for (s32 matTheme = 0; matTheme < MaterialTheme_Count; ++matTheme)
	{
		// Translate MaterialTheme to ThemeType for lookup in level data
		// (note: does not match all material themes)
		ThemeType themeType = ThemeType_Invalid;
		switch (matTheme)
		{
		case MaterialTheme_None:      themeType = ThemeType_DoNotTheme; break;
		case MaterialTheme_Sand:      themeType = ThemeType_Sand;       break;
		case MaterialTheme_Rocks:     themeType = ThemeType_Rocks;      break;
		case MaterialTheme_Beach:     themeType = ThemeType_Beach;      break;
		case MaterialTheme_DarkRocks: themeType = ThemeType_DarkRocks;  break;
		default:                      themeType = ThemeType_Invalid;    break;  // unsupported material theme
		}
		
		// Skip unsupported themes
		if (isValidThemeType(themeType) == false)
		{
			continue;
		}
		
		// Modify the vertex colors for all planes specified for this theme
		const tt::engine::renderer::ColorRGBA& color(p_levelData->getThemeColor(p_type, themeType));
		
		for (s32 matType = 0; matType < MaterialType_Count; ++matType)
		{
			for (s32 shapeIdx = 0; shapeIdx < Shape_Count; ++shapeIdx)
			{
				setVertexColors(m_materialConfig[matType][matTheme].shapes[shapeIdx].planeSets, color);
				setVertexColors(m_materialConfig[matType][matTheme].edges [shapeIdx].planeSets, color);
			}
			setVertexColors(m_materialConfig[matType][matTheme].centerPlanes, color);
		}
	}
}


tt::engine::renderer::TextureContainer SkinConfig::getAllUsedTextures() const
{
	tt::engine::renderer::TextureContainer textures;
	
	for (s32 typeIndex = 0; typeIndex < MaterialType_Count; ++typeIndex)
	{
		for (s32 themeIndex = 0; themeIndex < MaterialTheme_Count; ++themeIndex)
		{
			for (s32 shapeIndex = 0; shapeIndex < Shape_Count; ++shapeIndex)
			{
				loadTextures(m_materialConfig[typeIndex][themeIndex].shapes[shapeIndex].planeSets, &textures);
				loadTextures(m_materialConfig[typeIndex][themeIndex].edges [shapeIndex].planeSets, &textures);
			}
			
			loadTextures(m_materialConfig[typeIndex][themeIndex].centerPlanes, &textures);
		}
	}
	
	return textures;
}


const SkinConfig::PlaneSet& SkinConfig::getPlanes(const TileMaterial& p_material, Shape p_shape) const
{
	const MaterialType  type  = p_material.getMaterialType();
	const MaterialTheme theme = p_material.getMaterialTheme();
	TT_ASSERT(isValidMaterialType (type));
	TT_ASSERT(isValidMaterialTheme(theme));
	TT_ASSERT(isValidShape(p_shape));
	return getRandomPlaneSet(m_materialConfig[type][theme].shapes[p_shape].planeSets);
}


const SkinConfig::PlaneSet& SkinConfig::getEdgePlanes(const TileMaterial& p_material, Shape p_shape) const
{
	const MaterialType  type  = p_material.getMaterialType();
	const MaterialTheme theme = p_material.getMaterialTheme();
	TT_ASSERT(isValidMaterialType (type));
	TT_ASSERT(isValidMaterialTheme(theme));
	TT_ASSERT(isValidShape(p_shape));
	TT_ASSERT(isEdgeShape(p_shape));
	return getRandomPlaneSet(m_materialConfig[type][theme].edges[p_shape].planeSets);
}


const SkinConfig::PlaneSet& SkinConfig::getCenterPlanes(const TileMaterial& p_material) const
{
	const MaterialType  type  = p_material.getMaterialType();
	const MaterialTheme theme = p_material.getMaterialTheme();
	TT_ASSERT(isValidMaterialType (type));
	TT_ASSERT(isValidMaterialTheme(theme));
	return getRandomPlaneSet(m_materialConfig[type][theme].centerPlanes);
}


//--------------------------------------------------------------------------------------------------
// Private member functions

void SkinConfig::loadPlaneSet(const tt::xml::XmlNode* p_planeSetsNode,
                              PlaneSets*              p_planeSets_OUT,
                              const Plane&            p_defaultSettings,
                              const std::string&      p_filename,
                              tt::code::ErrorStatus*  p_errStatus)
{
	using tt::xml::XmlNode;
	
	TT_ERR_CHAIN_VOID("Parse plane set (from element <" << p_planeSetsNode->getName() << ">).");
	
	for (const XmlNode* planesNode = p_planeSetsNode->getChild();
	     planesNode != 0; planesNode = planesNode->getSibling())
	{
		if (planesNode->getName() != "planes")
		{
			TT_PANIC("Invalid level auto-dress (skin) config data: <%s> element has unsupported child <%s>. "
			         "Only <planes> is supported.\nFilename: '%s'",
			         p_planeSetsNode->getName().c_str(), planesNode->getName().c_str(), p_filename.c_str());
			continue;
		}
		
		PlaneSet planeSet;
		
		if (planesNode->hasAttribute("snap_to_uv_anchor"))
		{
			planeSet.snapToUVAnchor = tt::xml::util::parseBool(planesNode, "snap_to_uv_anchor", &errStatus);
			TT_ERR_RETURN_ON_ERROR();
		}
		
		if (planesNode->hasAttribute("anchor_count"))
		{
			planeSet.anchorCount = tt::xml::util::parseS32(planesNode, "anchor_count", &errStatus);
			TT_ERR_RETURN_ON_ERROR();
		}
		
		if (planesNode->hasAttribute("randomize_u"))
		{
			planeSet.randomizeU = tt::xml::util::parseBool(planesNode, "randomize_u", &errStatus);
			TT_ERR_RETURN_ON_ERROR();
		}
		
		if (planesNode->hasAttribute("begin_half_tile"))
		{
			planeSet.beginHalfTile = tt::xml::util::parseReal(planesNode, "begin_half_tile", &errStatus);
			TT_ERR_ASSERTMSG(planeSet.beginHalfTile >= 0.0f && planeSet.beginHalfTile <= 1.0f,
			                 "begin_half_tile needs to be <0.0 - 1.0> found: " << planeSet.beginHalfTile);
			TT_ERR_RETURN_ON_ERROR();
		}
		
		if (planesNode->hasAttribute("end_half_tile"))
		{
			planeSet.endHalfTile = tt::xml::util::parseReal(planesNode, "end_half_tile", &errStatus);
			TT_ERR_ASSERTMSG(planeSet.endHalfTile >= 0.0f && planeSet.endHalfTile <= 1.0f,
			                 "end_half_tile needs to be <0.0 - 1.0> found: " << planeSet.endHalfTile);
			TT_ERR_RETURN_ON_ERROR();
		}
		
		for (const XmlNode* singlePlaneNode = planesNode->getChild();
		     singlePlaneNode != 0; singlePlaneNode = singlePlaneNode->getSibling())
		{
			if (singlePlaneNode->getName() != "plane")
			{
				TT_PANIC("Invalid level auto-dress (skin) config data: <planes> element has unsupported child <%s>. "
				         "Only <plane> is supported.\nFilename: '%s'",
				         singlePlaneNode->getName().c_str(), p_filename.c_str());
				continue;
			}
			
			using tt::engine::scene2d::shoebox::PlaneData;
			Plane plane(p_defaultSettings);
			plane.planeData = PlaneData::parse(singlePlaneNode, &errStatus);
			
			TT_ERR_RETURN_ON_ERROR();
			
			// Save the vertex colors originally set for the plane
			plane.colorWholeQuad   = plane.planeData.colorWholeQuad;
			plane.colorTopLeft     = plane.planeData.colorTopLeft;
			plane.colorTopRight    = plane.planeData.colorTopRight;
			plane.colorBottomLeft  = plane.planeData.colorBottomLeft;
			plane.colorBottomRight = plane.planeData.colorBottomRight;
			
			std::string attribName;
			
			attribName = "tex_coord_mode_u";
			if (parseTexCoordMode(singlePlaneNode, attribName, &plane.texCoordModeU) == false)
			{
				TT_PANIC("Invalid level auto-dress (skin) config data: <%s> element "
				         "has unsupported value for attribute '%s': '%s'\nFilename: '%s'",
				         singlePlaneNode->getName().c_str(), attribName.c_str(),
				         singlePlaneNode->getAttribute(attribName).c_str(), p_filename.c_str());
			}
			
			attribName = "tex_coord_mode_v";
			if (parseTexCoordMode(singlePlaneNode, attribName, &plane.texCoordModeV) == false)
			{
				TT_PANIC("Invalid level auto-dress (skin) config data: <%s> element "
				         "has unsupported value for attribute '%s': '%s'\nFilename: '%s'",
				         singlePlaneNode->getName().c_str(), attribName.c_str(),
				         singlePlaneNode->getAttribute(attribName).c_str(), p_filename.c_str());
			}
			
			planeSet.planes.push_back(plane);
		}
		
		if (planeSet.planes.empty() == false)
		{
			p_planeSets_OUT->push_back(planeSet);
		}
	}
}


bool SkinConfig::parseTexCoordMode(const tt::xml::XmlNode* p_node,
                                   const std::string&      p_attributeName,
                                   Plane::TexCoordMode*    p_texCoordMode_OUT)
{
	TT_NULL_ASSERT(p_node);
	TT_NULL_ASSERT(p_texCoordMode_OUT);
	
	if (p_node->hasAttribute(p_attributeName) == false)
	{
		// Node does not have the requested attribute:
		// do not change the output value and silently ignore this attribute
		// (effectively using the default setting)
		return true;
	}
	
	const std::string& texCoordModeName(p_node->getAttribute(p_attributeName));
	
	if      (texCoordModeName == "do_not_modify")             *p_texCoordMode_OUT = Plane::TexCoordMode_DoNotModify;
	else if (texCoordModeName == "use_world_u")               *p_texCoordMode_OUT = Plane::TexCoordMode_UseWorldU;
	else if (texCoordModeName == "use_world_v")               *p_texCoordMode_OUT = Plane::TexCoordMode_UseWorldV;
	else if (texCoordModeName == "use_world_u_random_offset") *p_texCoordMode_OUT = Plane::TexCoordMode_UseWorldU_RandomOffset;
	else if (texCoordModeName == "use_world_v_random_offset") *p_texCoordMode_OUT = Plane::TexCoordMode_UseWorldV_RandomOffset;
	else if (texCoordModeName == "edge")                      *p_texCoordMode_OUT = Plane::TexCoordMode_Edge;
	else
	{
		// Unsupported attribute value
		return false;
	}
	
	return true;
}


const SkinConfig::PlaneSet& SkinConfig::getRandomPlaneSet(const PlaneSets& p_planeSets) const
{
	const PlaneSets::size_type setsSize = p_planeSets.size();
	if (setsSize == 0)
	{
		return g_emptyPlaneSet;
	}
	else if (setsSize == 1)
	{
		// Avoid the random number generator overhead if there is just one entry to return
		return p_planeSets.front();
	}
	
	const PlaneSets::size_type setIndex = static_cast<PlaneSets::size_type>(
			m_skinRandom.getNext(static_cast<u32>(setsSize)));
	return p_planeSets[setIndex];
}


void SkinConfig::loadTextures(const PlaneSets&                        p_planeSets,
                              tt::engine::renderer::TextureContainer* p_textures_OUT) const
{
	TT_NULL_ASSERT(p_textures_OUT);
	
	for (PlaneSets::const_iterator setIt = p_planeSets.begin(); setIt != p_planeSets.end(); ++setIt)
	{
		const Planes& planes((*setIt).planes);
		for (Planes::const_iterator planeIt = planes.begin(); planeIt != planes.end(); ++planeIt)
		{
			using namespace tt::engine::scene2d::shoebox;
			using tt::engine::renderer::EngineIDToTextures;
			EngineIDToTextures::value_type texEntry = createTextureCacheEntry(
					(*planeIt).planeData.textureFilename);
			if (texEntry.second != 0)
			{
				p_textures_OUT->push_back(texEntry.second);
			}
		}
	}
}


inline tt::engine::renderer::ColorRGBA blendColor(const tt::engine::renderer::ColorRGBA& p_colorA,
                                                  const tt::engine::renderer::ColorRGBA& p_colorB)
{
	tt::engine::renderer::ColorRGBA result;
	result.r = static_cast<u8>((static_cast<u32>(p_colorA.r) * p_colorB.r) / 255);
	result.g = static_cast<u8>((static_cast<u32>(p_colorA.g) * p_colorB.g) / 255);
	result.b = static_cast<u8>((static_cast<u32>(p_colorA.b) * p_colorB.b) / 255);
	result.a = static_cast<u8>((static_cast<u32>(p_colorA.a) * p_colorB.a) / 255);
	return result;
}


void SkinConfig::setVertexColors(PlaneSets&                             p_planeSets,
                                 const tt::engine::renderer::ColorRGBA& p_color)
{
	for (PlaneSets::iterator setIt = p_planeSets.begin(); setIt != p_planeSets.end(); ++setIt)
	{
		Planes& planes((*setIt).planes);
		for (Planes::iterator planeIt = planes.begin(); planeIt != planes.end(); ++planeIt)
		{
			Plane&                                   plane    (*planeIt);
			tt::engine::scene2d::shoebox::PlaneData& planeData(plane.planeData);
			
			if (plane.colorTopLeft    .isValid() ||
			    plane.colorTopRight   .isValid() ||
			    plane.colorBottomLeft .isValid() ||
			    plane.colorBottomRight.isValid())
			{
				planeData.colorTopLeft     = blendColor(plane.colorTopLeft,     p_color);
				planeData.colorTopRight    = blendColor(plane.colorTopRight,    p_color);
				planeData.colorBottomLeft  = blendColor(plane.colorBottomLeft,  p_color);
				planeData.colorBottomRight = blendColor(plane.colorBottomRight, p_color);
			}
			else
			{
				planeData.colorWholeQuad = blendColor(plane.colorWholeQuad, p_color);
			}
		}
	}
}

// Namespace end
}
}
}
