#if !defined(INC_TT_ENGINE_RENDERER_MATERIAL_H)
#define INC_TT_ENGINE_RENDERER_MATERIAL_H


#include <vector>

#include <tt/platform/tt_types.h>
#include <tt/fs/types.h>
#include <tt/math/Matrix44.h>
#include <tt/engine/renderer/ColorRGBA.h>
#include <tt/engine/EngineID.h>
#include <tt/engine/renderer/fwd.h>
#include <tt/engine/renderer/MaterialProperties.h>
#include <tt/engine/renderer/TextureStageData.h>
#include <tt/engine/cache/ResourceCache.h>
#include <tt/engine/file/FileType.h>
#include <tt/engine/animation/fwd.h>


namespace tt {
namespace engine {
namespace renderer {

// Resource Management
typedef cache::ResourceCache<Material> MaterialCache;


class Material
{
public:
	static const file::FileType fileType = file::FileType_Material;
	static const bool hasResourceHeader = true;
	
	enum Flag
	{
		Flag_DoNotLoadTexture   = (1 << 0),
		Flag_Transparent        = (1 << 1),
		Flag_DoubleSided        = (1 << 2),
		Flag_UMirror            = (1 << 3),
		Flag_VMirror            = (1 << 4),
		Flag_DisableLighting    = (1 << 5),
		Flag_NeedsUpdate        = (1 << 6),
		Flag_HasTextureAnim     = (1 << 7)
	};
	
public:
	Material(const EngineID& p_id, u32 p_flags = 0);
	Material(const ColorRGBA& p_diffuse,
		     const ColorRGBA& p_ambient  = ColorRGB::black,
			 const ColorRGBA& p_specular = ColorRGB::black,
			 const ColorRGBA& p_emissive = ColorRGB::black,
			 real p_power = 0.0f);
	Material(const Material& p_rhs);
	~Material() {}
	
	static Material* create(const fs::FilePtr& p_file, const EngineID& p_id, u32 p_flags);
	bool load(const fs::FilePtr& p_file);
	
	MaterialPtr clone() const;
	
	inline const EngineID& getEngineID() const {return m_id;}
	
	// Material Flag Handling
	inline u32  getFlags() const             { return m_flags; }
	inline void setFlag(Flag p_flag)         { m_flags |= p_flag; }
	inline void resetFlag(Flag p_flag)       { m_flags &= ~p_flag; }
	inline bool checkFlag(Flag p_flag) const { return (m_flags & p_flag) == u32(p_flag); }
	
	inline void setProperties(const MaterialProperties& p_properties) { m_properties = p_properties; }
	inline const MaterialProperties& getProperties   () const { return m_properties; }
	inline       MaterialProperties& modifyProperties()       { return m_properties; }
	
	inline void setBlendMode(BlendFactor p_source, BlendFactor p_dest)
	{
		m_srcFactor = p_source;
		m_dstFactor = p_dest;
	}
	
	inline s32 getTextureStageCount() const { return static_cast<s32>(m_textureStages.size()); }
	inline const TextureStageData& getTextureStage(s32 p_stage) const { return m_textureStages[p_stage]; }
	
	const TexturePtr& getTexture(s32 p_channel = 0) const;
	void              setTexture(const TexturePtr& p_texture, s32 p_channel = 0);
	
	const math::Matrix44& getTextureMatrix(s32 p_channel = 0) const;
	void                  setTextureMatrix(const math::Matrix44& p_matrix, s32 p_channel = 0);
	
	void select(RenderContext& p_renderContext);
	
	inline s32 getMemSize() const {return sizeof(Material);}
	
private:
	void updateAnimation(real p_time);
	
	EngineID m_id;
	u32 m_flags;
	
	MaterialProperties m_properties;
	
	s32 m_fogIndex;
	s32 m_lightSetIndex;
	
	BlendFactor m_srcFactor;
	BlendFactor m_dstFactor;
	
	typedef std::vector<TextureStageData> TextureStages;
	TextureStages m_textureStages;
	
	typedef std::vector<animation::TexMatrixControllerPtr> TexMatrixControllers;
	TexMatrixControllers m_texMatrixControllers;

	real m_animationTime;
};

// Namespace end
}
}
}

#endif // INC_TT_ENGINE_RENDERER_MATERIAL_H
