#include <vector>

#include <tt/engine/animation/TexMatrixController.h>
#include <tt/engine/debug/DebugRenderer.h>
#include <tt/engine/file/FileUtils.h>
#include <tt/engine/file/ResourceHeader.h>
#include <tt/engine/renderer/enums.h>
#include <tt/engine/renderer/FixedFunction.h>
#include <tt/engine/renderer/Material.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/engine/renderer/MultiTexture.h>
#include <tt/engine/renderer/RenderContext.h>
#include <tt/engine/scene/Scene.h>
#include <tt/fs/File.h>


namespace tt {
namespace engine {
namespace renderer {


struct TextureInfo
{
	u32 wrapS;
	u32 wrapT;
	u32 minFilter;
	u32 magFilter;
};


struct TextureConfig
{
	s32 texCoordIndex;
	s32 texMatrixIndex;
	s32 blendOperation;
	s32 alphaOperation;
};


struct TexMatrixSRT
{
	real scaleS;
	real scaleT;
	real rotate;
	real translateS;
	real translateT;
};

struct TextureMatrix
{
	math::Matrix44 value;
	animation::TexMatrixControllerPtr controller;
};
typedef std::vector<TextureMatrix> TexMatrixContainer;



Material::Material(const EngineID& p_id, u32 p_flags)
:
m_id(p_id),
m_flags(p_flags),
m_properties(),
m_fogIndex(-1),
m_lightSetIndex(-1),
m_srcFactor(BlendFactor_SrcAlpha),
m_dstFactor(BlendFactor_InvSrcAlpha),
m_animationTime(0)
{
}


Material::Material(const ColorRGBA& p_diffuse,
                   const ColorRGBA& p_ambient,
                   const ColorRGBA& p_emissive,
                   const ColorRGBA& p_specular,
                   real p_power)
:
m_id(0,0),
m_flags(0),
m_properties(p_ambient, p_diffuse, p_emissive, p_specular, p_power),
m_fogIndex(-1),
m_lightSetIndex(-1),
m_srcFactor(BlendFactor_SrcAlpha),
m_dstFactor(BlendFactor_InvSrcAlpha),
m_animationTime(0)
{
}


Material::Material(const Material& p_rhs)
:
m_id(p_rhs.m_id),
m_flags(p_rhs.m_flags),
m_properties(p_rhs.m_properties),
m_fogIndex(p_rhs.m_fogIndex),
m_lightSetIndex(p_rhs.m_lightSetIndex),
m_srcFactor(p_rhs.m_srcFactor),
m_dstFactor(p_rhs.m_dstFactor),
m_animationTime(p_rhs.m_animationTime)
{
}


Material* Material::create(const fs::FilePtr&, const EngineID& p_id, u32 p_flags)
{
	return new Material(p_id, p_flags);
}


bool Material::load(const fs::FilePtr& p_file)
{
	u16 len = 0;
	if (p_file->read(&len, sizeof(len)) != sizeof(len))
	{
		return false;
	}
	
	std::vector<char> name(len);
	if (p_file->read(&name[0], len) != len)
	{
		return false;
	}
	
	u32 textureCount(0);
	if (p_file->read(&textureCount, sizeof(textureCount)) != sizeof(textureCount))
	{
		return false;
	}
	
	typedef std::vector<TextureConfig> Configs;
	Configs configs;
	
	for (u32 i = 0; i < textureCount; ++i)
	{
		EngineID textureID(0,0);
		if(textureID.load(p_file) == false)
		{
			return false;
		}
		
		TexturePtr current;
		
		if (checkFlag(Flag_DoNotLoadTexture) && textureID.valid() == false)
		{
			//textureID.setName(name);
			current = TextureCache::get(textureID, true);
			
			if(current == 0)
			{
				current = Renderer::getInstance()->getDebug()->getDummyTexture();
			}
		}
		
		TextureInfo texInfo;
		if (p_file->read(&texInfo, sizeof(texInfo)) != sizeof(texInfo))
		{
			return false;
		}
		
		if(current != 0)
		{
			current->setAddressMode(static_cast<AddressMode>(texInfo.wrapS),
			                        static_cast<AddressMode>(texInfo.wrapT));
			
			// FIXME: Add support for mipmap filter
			current->setMinificationFilter (static_cast<FilterMode>(texInfo.minFilter));
			current->setMagnificationFilter(static_cast<FilterMode>(texInfo.magFilter));
		}
		
		TextureConfig config;
		
		// Do not read alpha op (not in data, but is computed from blendOp)
		// FIXME: Add to data
		tt::fs::size_type sizeToRead = sizeof(config) - sizeof(config.alphaOperation);
		
		if (p_file->read(&config, sizeToRead) != sizeToRead)
		{
			return false;
		}
		
		// Add texture
		//m_textures.push_back(current);
		configs.push_back(config);
		
		TextureStageData textureStage;
		textureStage.setTexture(current);
		textureStage.setTexCoordIndex(config.texCoordIndex);

		// FIXME: Directly use and load the texture stage info
		switch(static_cast<BlendOp>(config.blendOperation))
		{
		case BlendOp_Modulate:
			textureStage.setColorBlendOperation(TextureBlendOperation_Modulate);
			textureStage.setAlphaBlendOperation(TextureBlendOperation_Modulate);
			break;
		case BlendOp_Modulate2X:
			textureStage.setColorBlendOperation(TextureBlendOperation_Modulate2X);
			textureStage.setAlphaBlendOperation(TextureBlendOperation_Modulate);
			break;
		case BlendOp_Add:
			textureStage.setColorBlendOperation(TextureBlendOperation_Add);
			textureStage.setAlphaBlendOperation(TextureBlendOperation_Add);
			break;
		case BlendOp_Decal:
			textureStage.setColorBlendOperation(TextureBlendOperation_Decal);
			textureStage.setAlphaBlendOperation(TextureBlendOperation_Add);
			break;
		default:
			textureStage.setColorBlendOperation(TextureBlendOperation_Modulate);
			textureStage.setAlphaBlendOperation(TextureBlendOperation_Modulate);
			break;
		}
		m_textureStages.push_back(textureStage);
	}

	// Load the rest of the material details
	if (p_file->read(&m_properties.ambient, sizeof(ColorRGBA)) != sizeof(ColorRGBA))
	{
		return false;
	}

	if (p_file->read(&m_properties.diffuse, sizeof(ColorRGBA)) != sizeof(ColorRGBA))
	{
		return false;
	}

	if (p_file->read(&m_properties.emissive, sizeof(ColorRGBA)) != sizeof(ColorRGBA))
	{
		return false;
	}

	if (p_file->read(&m_properties.specular, sizeof(ColorRGBA)) != sizeof(ColorRGBA))
	{
		return false;
	}
	
	if (p_file->read(&m_properties.specularPower, sizeof(m_properties.specularPower)) !=
		sizeof(m_properties.specularPower))
	{
		return false;
	}

	if (p_file->read(&m_fogIndex, sizeof(m_fogIndex)) != sizeof(m_fogIndex))
	{
		return false;
	}

	if (p_file->read(&m_lightSetIndex, sizeof(m_lightSetIndex)) != sizeof(m_lightSetIndex))
	{
		return false;
	}

	u32 flags = 0;
	if (p_file->read(&flags, sizeof(flags)) != sizeof(flags))
	{
		return false;
	}

	// Mask in the flags
	m_flags |= flags;

	
	// Load texture matrices
	s32 matrixCount(0);
	if (p_file->read(&matrixCount, sizeof(matrixCount)) != sizeof(matrixCount))
	{
		return false;
	}

	TexMatrixContainer texMatrices;
	for (s32 i = 0; i < matrixCount; ++i)
	{
		TexMatrixSRT srt;
		if (p_file->read(&srt, sizeof(srt)) != sizeof(srt))
		{
			return false;
		}
		
		TextureMatrix texMatrix;
		
		texMatrix.value = math::Matrix44::getMayaTextureMatrix(srt.scaleS, srt.scaleT, srt.rotate,
		                                                       srt.translateS, srt.translateT);
		texMatrices.push_back(texMatrix);
	}
	
	s32 stage(0);
	for (TextureStages::iterator it = m_textureStages.begin(); it != m_textureStages.end(); ++it, ++stage)
	{
		it->setMatrix(texMatrices[configs[stage].texMatrixIndex].value);
	}
	
	// Load texture matrices
	s32 animCount(0);
	if (p_file->read(&animCount, sizeof(animCount)) != sizeof(animCount))
	{
		return false;
	}

	for(s32 i = 0; i < animCount; ++i)
	{
		// Load texture matrices
		s32 matrixIndex(0);
		if (p_file->read(&matrixIndex, sizeof(matrixIndex)) != sizeof(matrixIndex))
		{
			return false;
		}

		using animation::TexMatrixController;

		if(matrixIndex < static_cast<s32>(texMatrices.size()))
		{
			TT_ERR_CREATE("Loading Texture Matrix Animation ");
			
			TT_ERR_ADD_LOC(matrixIndex);

			// Create a texture matrix controller
			texMatrices[matrixIndex].controller.reset(new TexMatrixController);
			texMatrices[matrixIndex].controller->load(p_file, &errStatus);
			setFlag(Flag_HasTextureAnim);

			TT_ERR_ASSERT_ON_ERROR();
		}
	}
	// FIXME: Connect controllers to texture stages
	m_texMatrixControllers.resize(m_textureStages.size());
	for (s32 i = 0; i < static_cast<s32>(m_texMatrixControllers.size()); ++i)
	{
		m_texMatrixControllers[i] = texMatrices[configs[i].texMatrixIndex].controller;
	}

	// Load Blend Mode settings
	u32 srcFactor(0);
	if (p_file->read(&srcFactor, sizeof(srcFactor)) != sizeof(srcFactor))
	{
		return false;
	}
	m_srcFactor = static_cast<BlendFactor>(srcFactor);

	u32 dstFactor(0);
	if (p_file->read(&dstFactor, sizeof(dstFactor)) != sizeof(dstFactor))
	{
		return false;
	}
	m_dstFactor = static_cast<BlendFactor>(dstFactor);

	return true;
}


MaterialPtr Material::clone() const
{
	return MaterialPtr(new Material(*this));
}


const TexturePtr& Material::getTexture(s32 p_channel) const
{
	return m_textureStages[p_channel].getTexture();
}


void Material::setTexture(const TexturePtr& p_texture, s32 p_channel)
{
	if (static_cast<s32>(m_textureStages.size()) <= p_channel)
	{
		m_textureStages.resize(p_channel + 1);
		m_texMatrixControllers.resize(p_channel + 1);
		TextureStageData stage;
		stage.setColorBlendOperation(TextureBlendOperation_Modulate);
		stage.setAlphaBlendOperation(TextureBlendOperation_Modulate);
		m_textureStages[p_channel] = stage;
	}
	m_textureStages[p_channel].setTexture(p_texture);
}


const math::Matrix44& Material::getTextureMatrix(s32 p_channel) const
{
	TT_MINMAX_ASSERT(p_channel, 0, static_cast<s32>(MultiTexture::getChannelCount()));
	return m_textureStages[p_channel].getMatrix();
}


void Material::setTextureMatrix(const math::Matrix44& p_matrix, s32 p_channel)
{
	TT_MINMAX_ASSERT(p_channel, 0, static_cast<s32>(MultiTexture::getChannelCount()));
	m_textureStages[p_channel].setMatrix(p_matrix);
}


void Material::select(RenderContext& p_renderContext)
{
	// Activate scene settings (light + fog)
	if(p_renderContext.scene != 0)
	{
		p_renderContext.scene->setFog(m_fogIndex);
		p_renderContext.scene->setLightSet(m_lightSetIndex);
	}

	if (checkFlag(Flag_HasTextureAnim))
	{
		updateAnimation(p_renderContext.textureAnimationTime);
	}
	
	// Activate material
	if (Renderer::getInstance()->isLightingEnabled())
	{
		FixedFunction::setMaterial(m_properties);
	}
	
	s32 stage(0);
	for (TextureStages::iterator it = m_textureStages.begin(); it != m_textureStages.end(); ++it, ++stage)
	{
		FixedFunction::setTextureStage(stage, *it);
	}
	FixedFunction::resetTextureStages(stage);
	
	// Set blend mode
	Renderer::getInstance()->setCustomBlendMode(m_srcFactor, m_dstFactor);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////
// Private


void Material::updateAnimation(real p_time)
{
	if (math::realEqual(p_time, m_animationTime))
	{
		// Already up-to-date
		return;
	}
	m_animationTime = p_time;
	
	TT_ASSERT(m_texMatrixControllers.size() == m_textureStages.size());
	for (s32 stage = 0; stage < static_cast<s32>(m_textureStages.size()); ++stage)
	{
		if (m_texMatrixControllers[stage] != 0)
		{
			m_texMatrixControllers[stage]->getValue(m_animationTime, m_textureStages[stage].modifyMatrix());
		}
	}
}


// Namespace end
}
}
}

