#include <visualboy/VisualBoy.h>

#include <tt/code/bufferutils.h>
#include <tt/code/DefaultValue.h>
#include <tt/code/OptionalValue.h>
#include <tt/pres/anim2d/TweenTypes.h>
#include <tt/engine/renderer/MatrixStack.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/engine/renderer/VertexBuffer.h>
#include <tt/engine/scene/Camera.h>

#include <tt/pres/anim2d/ColorAnimationStack2D.h>
#include <tt/pres/FrameAnimation.h>
#include <tt/pres/PresentationQuad.h>
#include <tt/pres/PresentationObject.h>
#include <tt/pres/PresentationValue.h>
#include <tt/pres/SpriteStrip.h>

#include <tt/xml/util/parse.h>
#include <tt/xml/XmlNode.h>


namespace tt {
namespace pres {


#define MAKE_VERSION(major, minor) (((major) << 8) | (minor))
#define GET_MAJOR_VERSION(version) ((version) >> 8)
#define GET_MINOR_VERSION(version) ((version) & 0xFF)

static const u16 s_version = MAKE_VERSION(1, 17);

bool FrameAnimation::ms_doHudScale = false;

//--------------------------------------------------------------------------------------------------
// Helper functions

engine::renderer::FilterMode readFilterMode(
	const xml::XmlNode* p_node, const std::string& p_name, code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN(engine::renderer::FilterMode, engine::renderer::FilterMode_Invalid,
		"Reading engine filter mode");

	const std::string& filterModeStr(p_node->getAttribute(p_name));

	if (filterModeStr.empty() == false)
	{
		engine::renderer::FilterMode filterMode = engine::renderer::getFilterModeFromName(filterModeStr);

		if (engine::renderer::isValidFilterMode(filterMode) == false)
		{
			TT_ERR_AND_RETURN("Found unknown value '" << filterModeStr
			                  << "' in attribute 'filtermode' in node '"
			                  << p_node->getName() << "'.");
		}

		return filterMode;
	}

	return engine::renderer::FilterMode_Invalid;
}


//--------------------------------------------------------------------------------------------------
// Class Definition

FrameAnimation::FrameAnimation()
:
m_beginFrame(0),
m_endFrame(-1),
m_frameDifference(0),
m_frameBegin(0),
m_totalFrames(0),
m_frameCount(0,0),
m_frameSize(0,0),
m_frameSizeX(0),
m_frameSizeY(0),
m_quadSizeX(0),
m_quadSizeY(0),
m_fps(1),
m_holdFirstFrame(true),
m_holdLastFrame(true),
m_flip(Flip_None),
m_minFilter(engine::renderer::FilterMode_Invalid),
m_magFilter(engine::renderer::FilterMode_Invalid),
m_mipFilter(engine::renderer::FilterMode_Invalid),
m_blendMode(engine::renderer::BlendMode_Invalid),
m_flags(),
m_passName(),
m_spritestripID(0,0),
m_lightmaskID(0,0),
m_spriteDirectory(),
m_usingDirectory(false),
m_texture(),
m_isTextureForVisualBoy(false),
m_lightmask(),
m_quad(),
m_translationX(0.0),
m_translationY(0.0),
m_translationZ(0.0),
m_scaleX(1.0f),
m_scaleY(1.0f),
m_isUniformScale(false),
m_rotation(0.0f),
m_texAnimU(0),
m_texAnimV(0),
m_loaded(false),
m_usingDuration(false),
m_frameDuration(),
m_durationTimeLeft(0),
m_animationUV(0,0),
m_offsetUV(0,0),
m_cameraSpaceScale(0,0),
m_lightMaskType(LightMaskType_None)
{
}


void FrameAnimation::createQuad()
{
	TT_ASSERTMSG(m_loaded, "load the Frameanimation before creating a quad");
	
	if(m_quad == 0)
	{
		using namespace tt::engine::renderer;

		{
			// VisualBoy hack
			if (m_spritestripID.getValue() == VisualBoy::getVisualBoyTextureCRC())
			{
				TT_NULL_ASSERT(VisualBoy::getOutputTexture());
				m_texture =  VisualBoy::getOutputTexture();
				m_isTextureForVisualBoy = true;
			}
			else
			{
				m_texture = TextureCache::get(m_spritestripID, true);
				m_isTextureForVisualBoy = false;
			}
		}
		TT_NULL_ASSERT(m_texture);

		if(m_lightMaskType == LightMaskType_UseLightmaskTexture)
		{
			m_lightmask = TextureCache::get(m_lightmaskID, false);
			TT_NULL_ASSERT(m_lightmask);
		}
		
		if(engine::renderer::isValidFilterMode(m_minFilter))
		{
			m_texture->setMinificationFilter(m_minFilter);
		}
		if(engine::renderer::isValidFilterMode(m_magFilter))
		{
			m_texture->setMagnificationFilter(m_magFilter);
		}
		if(engine::renderer::isValidFilterMode(m_mipFilter))
		{
			m_texture->setMipmapFilter(m_mipFilter);
		}
		
		m_quad.reset(new PresentationQuad(m_texture, engine::renderer::ColorRGBA()));
		
		updateTextureAndQuadWithPresentationValues();
	}
	else
	{
		m_quad->setColor(m_quad->getOriginalColor());
	}
}


void FrameAnimation::updateColor(const anim2d::ColorAnimationStack2D& p_coloranim)
{
	if (m_quad != 0)
	{
		m_quad->setColor(p_coloranim.getColor(m_quad->getOriginalColor()));
	}
}


void FrameAnimation::update(real p_delta)
{
	// VisualBoy hack; check if texture has been updated
	if (m_isTextureForVisualBoy && m_texture != VisualBoy::getOutputTexture())
	{
		m_quad.reset();
		createQuad();
	}

	if (m_usingDuration)
	{
		m_durationTimeLeft -= p_delta;
		if (m_durationTimeLeft < 0)
		{
			stop();
		}
	}

	m_offsetUV += m_animationUV * p_delta;
	
	Animation2D::update(p_delta);
}


void FrameAnimation::render(const math::Matrix44& p_transform) const
{
	if (m_quad != 0)
	{
		engine::renderer::MatrixStack* stack = engine::renderer::MatrixStack::getInstance();
		engine::renderer::Renderer* renderer = engine::renderer::Renderer::getInstance();

		// Do not cull objects in HUD space
		if(renderer->isRenderingHud() == false)
		{
			// Culling in view space

			// Transform quad position to world space
			math::Vector3 center = m_quad->getTranslation() * p_transform;

			// Transform to view space
			const engine::scene::CameraPtr& activeCam(renderer->getActiveCamera());
			center = center * activeCam->getViewMatrix();

			// Compute radius

			// Create a normalized vector along all axis with length 1
			static const math::Vector4 unit(0.57735f, 0.57735f, 0.57735f,0);

			// Apply the current matrix to it
			math::Vector4 scaling(unit * p_transform);

			const real radius = m_quad->getRadius() * scaling.length();

			if(activeCam->isVisible(center, radius) == false)
			{
				//TT_Printf("Presentation Culled!!\n ");
				return;
			}
		}

		if(isVisible())
		{
			stack->setMode(engine::renderer::MatrixStack::Mode_Position);
			stack->push();
			
			if (ms_doHudScale && renderer->isRenderingHud())
			{
				static const real defaultAspectRatio = 16.0f / 9.0f;
				const real aspectRatio = renderer->getHudCamera()->getAspectRatio();
				
				const real scaleHudX = aspectRatio / defaultAspectRatio;
				
				math::Matrix44 transform(p_transform);
				transform.m_41 *= scaleHudX;
				stack->multiply44(transform);
			}
			else
			{
				stack->multiply44(p_transform);
			}
		
			// Setup texture transform
			stack->setMode(engine::renderer::MatrixStack::Mode_Texture);
			stack->load44(getTextureMatrix());

			if (math::realEqual(m_cameraSpaceScale.x, 0.0f) == false ||
				math::realEqual(m_cameraSpaceScale.y, 0.0f) == false)
			{
				// Texture scrolls in camera space
				const math::Vector3 cameraPos = renderer->getActiveCamera()->getActualPosition();

				stack->translate(math::Vector3(cameraPos.x * m_cameraSpaceScale.x, -cameraPos.y * m_cameraSpaceScale.y, 0));
			}
			
			const bool fogWasEnabled = renderer->isFogEnabled();
			if (m_flags.checkFlag(Flag_IgnoreFog))
			{
				renderer->setFogEnabled(false);
			}
			
			stack->updateTextureMatrix();

			// Regular rendering
			// Texture setting and quad position offset is done in the quad.
			m_quad->render();

			// Lightmask rendering
			if(m_lightMaskType != LightMaskType_None)
			{
				using namespace tt::engine::renderer;
				renderer->setColorMask(ColorMask_Alpha);
				renderer->setCustomBlendMode(BlendFactor_One, BlendFactor_One);
				renderer->resetCustomBlendModeAlpha();

				if(m_lightMaskType == LightMaskType_UseAlphaChannel)
				{
					m_quad->render();
				}
				else if(m_lightMaskType == LightMaskType_UseLightmaskTexture)
				{
					TT_NULL_ASSERT(m_lightmask);
					m_quad->renderWithTexture(m_lightmask);
				}

				renderer->setColorMask(ColorMask_All);
				renderer->setBlendMode(BlendMode_Blend);
				renderer->setCustomBlendModeAlpha(BlendFactor_Zero, BlendFactor_InvSrcAlpha);
			}
			
			if (m_flags.checkFlag(Flag_IgnoreFog))
			{
				renderer->setFogEnabled(fogWasEnabled);
			}
			
			stack->resetTextureMatrix();
			
			stack->setMode(engine::renderer::MatrixStack::Mode_Position);
			stack->pop();
		}
	}
}


void FrameAnimation::getAndLoadAllUsedTextures(engine::renderer::TextureContainer& p_textures) const
{
	p_textures.push_back(engine::renderer::TextureCache::get(m_spritestripID, true));

	if(m_lightMaskType == LightMaskType_UseLightmaskTexture)
	{
		p_textures.push_back(engine::renderer::TextureCache::get(m_lightmaskID, true));
	}
}


math::Matrix44 FrameAnimation::getTextureMatrix() const
{
	TT_ASSERTMSG(m_loaded, "load the Frameanimation before getting the texture matrix");
	TT_NULL_ASSERT(m_texture);
	
	// calc current frame
	s32 frame = static_cast<s32>(math::floor(getTime() * (m_frameDifference + 1))) + m_frameBegin;
	math::clamp(frame, m_frameBegin, m_frameBegin + m_frameDifference);
	
	math::Vector3 textureTranslation;
	if(frame != 0)
	{
		// use this to calc the texture translation
		if(m_spriteDirectory.empty())
		{
			TT_ASSERT(m_frameCount.x > 0);
			textureTranslation.setValues(
				(frame % m_frameCount.x) * m_frameSize.x, (frame / m_frameCount.x) * m_frameSize.y, 0);
		}
		else
		{
			TT_ASSERT(m_frameCount.y > 0);
			textureTranslation.setValues(
				(frame / m_frameCount.y) * m_frameSize.x, (frame % m_frameCount.y) * m_frameSize.y, 0);
		}
	}

	textureTranslation.x += m_offsetUV.x;
	textureTranslation.y += m_offsetUV.y;
	
	return math::Matrix44::getTranslation(textureTranslation);
}


bool FrameAnimation::isVisible() const
{
	if (isActive())
	{
		// at the begin of the animation and active. Means that we are in delay time
		if (isDelaying()) // Are we delaying?
		{
			switch (getDirection())
			{
			case DirectionType_Forward:
			case DirectionType_PingPong:
				return m_holdFirstFrame; 
				
			case DirectionType_Backward:
			case DirectionType_ReversePingPong:
				return m_holdLastFrame; 
				
			default:
				TT_PANIC("Unhandled direction type %d\n", getDirection());
				break;
			}
		}
		return true;
	}
	
	// else (not active)
	switch (getDirection())
	{
	case DirectionType_Forward:
	case DirectionType_ReversePingPong:
		if (getTime() >= 1.0f) 
		{
			return m_holdLastFrame; 
		}
		break;
		
	case DirectionType_Backward:
	case DirectionType_PingPong:
		if (getTime() <= 0.0f) 
		{
			return m_holdFirstFrame; 
		}
		break;
		
	default:
		TT_PANIC("Unhandled direction type %d\n", getDirection());
		break;
	}
	
	return false;
}


bool FrameAnimation::isActive() const
{
	if (m_usingDuration)
	{
		return m_durationTimeLeft > 0.0f;
	}
	else return Animation2D::isActive();
}


bool FrameAnimation::isLooping() const
{
	// when using the duration, the animation can be looping but it will end when the duration is over.
	// so it's not looping
	if (m_usingDuration)
	{
		return false;
	}
	else return Animation2D::isLooping();
}


bool FrameAnimation::loadXml( const xml::XmlNode* p_node, const DataTags& p_applyTags, 
                           const Tags& p_acceptedTags, code::ErrorStatus* p_errStatus )
{
	TT_ERR_CHAIN(bool, false, "Loading Frame animation");
	TT_ERR_ASSERT(p_node->getName() == "frameanim");
	
	// apply the tags
	getDataTags().load(p_node, p_applyTags, p_acceptedTags, &errStatus);
	
	// iterate over children
	for (const xml::XmlNode* child = p_node->getChild(); child != 0; child = child->getSibling())
	{
		if(child->getName() == "translation")
		{
			m_translationX =  parseOptionalPresentationValue(child, "x", 0.0f, &errStatus);
			m_translationY =  parseOptionalPresentationValue(child, "y", 0.0f, &errStatus);
			m_translationZ =  parseOptionalPresentationValue(child, "z", 0.0f, &errStatus);
		}
		else if(child->getName() == "scale")
		{
			if (child->hasAttribute("scale"))
			{
				m_isUniformScale = true;
				m_scaleX = parsePresentationValue(child, "scale", 1.0f, &errStatus);
				// m_scaleY will be same as m_scaleX
			}
			else
			{
				m_isUniformScale = false;
				m_scaleX =  parseOptionalPresentationValue(child, "x", 1.0f, &errStatus);
				m_scaleY =  parseOptionalPresentationValue(child, "y", 1.0f, &errStatus);
			}
		}
		else if(child->getName() == "rotation")
		{
			m_rotation = parseOptionalPresentationValue(child, "value", 0.0f, &errStatus);
		}
		else if(child->getName() == "begin" || child->getName() == "end")
		{
			// parse begin and end elements 
			PresentationValue frame(parsePresentationValue(child, "frame", &errStatus));
			code::OptionalValue<bool> hold(xml::util::parseOptionalBool(child, "hold", &errStatus));
			
			TT_ERR_RETURN_ON_ERROR();
			
			if(child->getName() == "begin")
			{
				m_beginFrame = frame;
				if(hold.isValid()) m_holdFirstFrame = hold.get();
			}
			else if(child->getName() == "end")
			{
				m_endFrame = frame;
				if(hold.isValid()) m_holdLastFrame = hold.get();
			}
		}
		else
		{
			TT_ERR_AND_RETURN("Unsuported child tag in Frame Animation: " << child->getName());
		}
	}
	
	TT_ERR_ASSERTMSG(m_endFrame.getMax() >= 0, "No end frame specified or end frame is -1");
	if (m_beginFrame.isValid() && m_endFrame.isValid() && m_beginFrame.get() > m_endFrame.get())
	{
		TT_ERR_AND_RETURN("Beginframe '" << m_beginFrame << "' should be less or equal than endframe '" << m_endFrame << "'");
	}
	
	// image: path to the image strip (when using this the framewidth and height have to be specified)
	code::OptionalValue<std::string> image(xml::util::parseOptionalStr(p_node, "image", &errStatus));
	if(image.isValid())
	{
		//image_namespace: namespace of the image strip
		code::OptionalValue<std::string> imageNamespace(xml::util::parseOptionalStr(p_node, 
		                                                "image_namespace", &errStatus));
		TT_ERR_RETURN_ON_ERROR();
		TT_ERR_ASSERTMSG(imageNamespace.isValid(), "When using image, image_namespace must be set.");
		
		
		// frame_height: width of a single frame (only needed when using image instead of directory)
		code::OptionalValue<PresentationValue> frameHeight(
				parseOptionalPresentationValue(p_node, "frame_height", &errStatus));
		TT_ERR_RETURN_ON_ERROR();
		TT_ERR_ASSERTMSG(frameHeight.isValid(), "When using image, frame_height must be set.");
		
		
		// frame_width: height of a single frame (only needed when using image instead of directory)
		code::OptionalValue<PresentationValue> frameWidth(
				parseOptionalPresentationValue(p_node, "frame_width", &errStatus));
		TT_ERR_RETURN_ON_ERROR();
		TT_ERR_ASSERTMSG(frameWidth.isValid(), "When using image, frame_width must be set.");
		
		// Get asset id
		m_spritestripID = engine::EngineID(image.get(), imageNamespace.get());
		m_lightmaskID   = engine::EngineID("lightmask_" + image.get(), imageNamespace.get());
		
		m_frameSizeX = frameWidth.get();
		m_frameSizeY = frameHeight.get();

		// Optional UV animations
		m_texAnimU = parseOptionalPresentationValue(p_node, "texture_anim_u", 0.0f, &errStatus);
		m_texAnimV = parseOptionalPresentationValue(p_node, "texture_anim_v", 0.0f, &errStatus);

		if(m_texAnimU.isValid()) m_animationUV.x = m_texAnimU.get();
		if(m_texAnimV.isValid()) m_animationUV.y = m_texAnimV.get();

		code::OptionalValue<real> cameraUScale =
			xml::util::parseOptionalReal(p_node, "camera_space_u_scale", &errStatus);
		code::OptionalValue<real> cameraVScale =
			xml::util::parseOptionalReal(p_node, "camera_space_v_scale", &errStatus);

		if(cameraUScale.isValid()) m_cameraSpaceScale.x = cameraUScale.get();
		if(cameraVScale.isValid()) m_cameraSpaceScale.y = cameraVScale.get();
		
		m_usingDirectory = false;
	}
	else
	{
		// directory: path to the directory with separate frame images
		code::OptionalValue<std::string> directory(xml::util::parseOptionalStr(p_node, "directory", &errStatus));
		TT_ERR_RETURN_ON_ERROR();
		TT_ERR_ASSERTMSG(directory.isValid(), "Image or directory attributes must be set.");
		
		parseSpriteStripDirectory(directory.get(), &errStatus);
		TT_ERR_RETURN_ON_ERROR();
	}
	
	// quad_height: width of the quad 
	PresentationValue quadHeight = parsePresentationValue(p_node, "quad_height", &errStatus);
	
	// quad_width: height of the quad 
	PresentationValue quadWidth = parsePresentationValue(p_node, "quad_width", &errStatus);
	
	m_quadSizeX = quadWidth;
	m_quadSizeY = quadHeight;
	
	m_flip = Flip_None;
	// flip_vertical
	code::DefaultValue<bool> flipVertical(false);
	flipVertical = xml::util::parseOptionalBool(p_node, "flip_vertical", &errStatus);
	if(flipVertical.get())
	{
		m_flip |= Flip_Vertical;
	}
	
	// flip_horizontal
	code::DefaultValue<bool> flipHorizontal(false);
	flipHorizontal = xml::util::parseOptionalBool(p_node, "flip_horizontal", &errStatus);
	if(flipHorizontal.get())
	{
		m_flip |= Flip_Horizontal;
	}
	
	// Texture Filtering
	m_minFilter = readFilterMode(p_node, "filter_minify",  &errStatus);
	m_magFilter = readFilterMode(p_node, "filter_magnify", &errStatus);
	m_mipFilter = readFilterMode(p_node, "filter_mipmap",  &errStatus);
	
	// blendmode (one of the engine::renderer::BlendMode values)
	m_blendMode = engine::renderer::BlendMode_Invalid;
	if (p_node->getAttribute("blendmode").empty() == false)
	{
		const std::string& blendModeStr(p_node->getAttribute("blendmode"));
		m_blendMode = engine::renderer::getBlendModeFromName(blendModeStr);
		
		TT_ERR_ASSERTMSG(engine::renderer::isValidBlendMode(m_blendMode),
		                 "Found unknown value '" << blendModeStr
		                  << "' in attribute 'blendmode' in node '"
		                  << p_node->getName() << "'.");
	}
	
	// Flags
	m_flags.resetAllFlags(); // Start with clean flags.
	
	code::OptionalValue<bool> ignore_fog(xml::util::parseOptionalBool(p_node, "ignore_fog", &errStatus));
	if (ignore_fog.isValid())
	{
		m_flags.setFlag(Flag_IgnoreFog);
	}
	
	// looping: Boolean
	code::OptionalValue<bool> loop(xml::util::parseOptionalBool(p_node, "looping", &errStatus));
	TT_ERR_RETURN_ON_ERROR();
	if(loop.isValid())
	{
		setLooping(loop.get());
	}
	
	// delay: delay in seconds before the frame animation starts
	setDelayRange(parseOptionalPresentationValue(p_node, "delay", 0, &errStatus));
	
	// preset: see anim2d preset
	const std::string& preset = p_node->getAttribute("preset");
	if (preset.empty() == false)
	{
		if      (preset == "oneshot")     setPreset(Preset_OneShot);
		else if (preset == "looping")     setPreset(Preset_Looping);
		else if (preset == "pingpong")    setPreset(Preset_PingPong);
		else if (preset == "oscillating") setPreset(Preset_Oscillating);
		else
		{
			TT_ERR_AND_RETURN("Found unknown value '" << preset 
			                  << "' in attribute 'preset' in node '"
			                  << p_node->getName() << "'.");
		}
	}
	else
	{
		// direction: see anim2d
		setDirection(directionTypeFromString(   parseOptionalString(p_node, "direction", "forward",
		                                                           &errStatus)));
		
		// timing: see anim2d timing types
		setTimeType (timeTypeFromString(        parseOptionalString(p_node, "timing",    "linear",
		                                                           &errStatus)));
		
		// tweening: see anim2d tweentypes
		setTweenType(anim2d::Tween::tweenTypeFromString(parseOptionalString(p_node, "tweening",  "quad",
		                                                           &errStatus)));
	}
	setId(p_node->getAttribute("id"));
	
	m_passName = p_node->getAttribute("render_pass");
	
	// fps: the frame rate of the images (instead of duration)
	// get fps as range
	m_fps = parsePresentationValue(p_node, "fps", 1.0f, &errStatus);
	TT_ERR_RETURN_ON_ERROR();
	TT_ERR_ASSERTMSG(m_fps.getMin() > 0, "fps can not be 0");
	
	if (p_node->hasAttribute("duration"))
	{
		m_usingDuration = true;
		m_frameDuration = parsePresentationValue(p_node, "duration", &errStatus);
	}
	else
	{
		m_usingDuration = false;
	}
	
	m_loaded = true;
	
	return true;
}


bool FrameAnimation::parseSpriteStripDirectory(const std::string& p_directory, code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN(bool, false, "Parsing SpriteStrip Directory");
	
	m_spriteDirectory = p_directory;

	// HACK: Special case to use the DRC live framebuffer as a texture in presentation
	if(m_spriteDirectory == "use_drc_framebuffer")
	{
		m_usingDirectory = false;
		m_spritestripID = engine::EngineID("drc_framebuffer","drc");
		m_spriteDirectory.clear();
		m_frameSizeX.setValue(1.0f);
		m_frameSizeY.setValue(1.0f);
		return true;
	}
	
	SpriteStrip::SpriteStripData spriteData;
	if(SpriteStrip::load(m_spriteDirectory, spriteData, &errStatus) == false)
	{
		TT_ERR_RETURN_ON_ERROR();
	}
	else
	{
		m_frameSizeX.setValue(spriteData.frameSize.x);
		m_frameSizeY.setValue(spriteData.frameSize.y);

		m_frameSize.setValues(m_frameSizeX, m_frameSizeY);

		m_totalFrames = spriteData.totalFrameCount;
		TT_ERR_ASSERTMSG(m_beginFrame.getMax() < m_totalFrames, 
			"Begin frame "<< m_beginFrame.toStr() << "(" << m_beginFrame.get() << ")" <<
			" exceeds total number of frames available: " << m_totalFrames);
		TT_ERR_ASSERTMSG(m_endFrame.getMax() < m_totalFrames, 
			"End frame " << m_endFrame.toStr() << "(" << m_endFrame.get() << ")" <<
			" exceeds total number of frames available: " << m_totalFrames);
	}
	
	m_spritestripID = engine::EngineID(spriteData.spriteStripName, spriteData.spriteStripNamespace);
	m_lightmaskID   = engine::EngineID("lightmask_" + spriteData.spriteStripName, spriteData.spriteStripNamespace);
	
	m_usingDirectory = true;
	
	return true;
}


bool FrameAnimation::save( u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus )
{
	TT_ERR_CHAIN(bool, false, "Saving Frame animation");
	
	TT_ERR_ASSERTMSG(getBufferSize() <= p_sizeOUT, "Not enough space in buffer need " <<
	                                               getBufferSize() << " got " << p_sizeOUT);
	
	using namespace code::bufferutils;
	
	be_put(s_version, p_bufferOUT, p_sizeOUT);
	
	m_beginFrame.save(p_bufferOUT, p_sizeOUT, &errStatus);
	m_endFrame.save(p_bufferOUT, p_sizeOUT, &errStatus);
	
	m_quadSizeX.save(p_bufferOUT, p_sizeOUT, &errStatus);
	m_quadSizeY.save(p_bufferOUT, p_sizeOUT, &errStatus);
	
	be_put(m_holdFirstFrame, p_bufferOUT, p_sizeOUT);
	be_put(m_holdLastFrame,  p_bufferOUT, p_sizeOUT);
	
	be_put(m_usingDirectory, p_bufferOUT, p_sizeOUT);
	
	be_put(m_flip, p_bufferOUT, p_sizeOUT);
	
	be_put(static_cast<u8>(m_minFilter), p_bufferOUT, p_sizeOUT);
	be_put(static_cast<u8>(m_magFilter), p_bufferOUT, p_sizeOUT);
	be_put(static_cast<u8>(m_mipFilter), p_bufferOUT, p_sizeOUT);
	be_put(static_cast<u8>(m_blendMode), p_bufferOUT, p_sizeOUT);
	// NOTE: Added specific template type so we know when type changes in bitmask (Format changed.)
	be_put<u32>(m_flags.getFlags()     , p_bufferOUT, p_sizeOUT);
	be_put(m_passName, p_bufferOUT, p_sizeOUT);
	
	m_fps.save(p_bufferOUT, p_sizeOUT, &errStatus);
	
	m_translationX.save(p_bufferOUT, p_sizeOUT, &errStatus);
	m_translationY.save(p_bufferOUT, p_sizeOUT, &errStatus);
	m_translationZ.save(p_bufferOUT, p_sizeOUT, &errStatus);
	m_scaleX      .save(p_bufferOUT, p_sizeOUT, &errStatus);
	m_scaleY      .save(p_bufferOUT, p_sizeOUT, &errStatus);
	be_put(m_isUniformScale, p_bufferOUT, p_sizeOUT);
	m_rotation    .save(p_bufferOUT, p_sizeOUT, &errStatus);
	m_texAnimU    .save(p_bufferOUT, p_sizeOUT, &errStatus);
	m_texAnimV    .save(p_bufferOUT, p_sizeOUT, &errStatus);
	
	if(m_usingDirectory)
	{
		be_put(m_spriteDirectory, p_bufferOUT, p_sizeOUT);
	}
	else
	{
		m_frameSizeX.save(p_bufferOUT, p_sizeOUT, &errStatus);
		m_frameSizeY.save(p_bufferOUT, p_sizeOUT, &errStatus);

		be_put(m_spritestripID.crc1, p_bufferOUT, p_sizeOUT);
		be_put(m_spritestripID.crc2, p_bufferOUT, p_sizeOUT);
		be_put(m_lightmaskID.crc1,   p_bufferOUT, p_sizeOUT);
		be_put(m_lightmaskID.crc2,   p_bufferOUT, p_sizeOUT);
	}
	
	be_put(m_usingDuration, p_bufferOUT, p_sizeOUT);
	if (m_usingDuration)
	{
		m_frameDuration.save(p_bufferOUT, p_sizeOUT, &errStatus);
	}
	
	be_put(m_cameraSpaceScale, p_bufferOUT, p_sizeOUT);
	be_put(static_cast<u8>(m_lightMaskType), p_bufferOUT, p_sizeOUT);
	
	TT_ERR_RETURN_ON_ERROR();
	
	return Animation2D::save(p_bufferOUT, p_sizeOUT, &errStatus);
}


bool FrameAnimation::load(const u8*& p_bufferOUT, size_t& p_sizeOUT, const DataTags& p_applyTags,
                          const Tags& p_acceptedTags, code::ErrorStatus* p_errStatus )
{
	TT_ERR_CHAIN(bool, false, "loading frameAnimation");
	
	using namespace code::bufferutils;
	
	u16 version = be_get<u16>(p_bufferOUT, p_sizeOUT);
	TT_ERR_ASSERTMSG(version == s_version, "Invalid FrameAnimation version, code "
			<< GET_MAJOR_VERSION(s_version) << "." <<GET_MINOR_VERSION(s_version)<<", data "
			<< GET_MAJOR_VERSION(version)   << "." <<GET_MINOR_VERSION(version)  <<
			", Please update your presentation converter");
	
	m_beginFrame.load(p_bufferOUT, p_sizeOUT, &errStatus);
	m_endFrame.setValue(0.0f);
	m_endFrame  .load(p_bufferOUT, p_sizeOUT, &errStatus);
	
	m_quadSizeX.load(p_bufferOUT, p_sizeOUT, &errStatus);
	m_quadSizeY.load(p_bufferOUT, p_sizeOUT, &errStatus);
	
	m_holdFirstFrame = be_get<bool>(p_bufferOUT, p_sizeOUT);
	m_holdLastFrame  = be_get<bool>(p_bufferOUT, p_sizeOUT);
	
	m_usingDirectory = be_get<bool>(p_bufferOUT, p_sizeOUT);
	
	m_flip = be_get<u32>(p_bufferOUT, p_sizeOUT);
	
	m_minFilter  = static_cast<engine::renderer::FilterMode>(be_get<u8>(p_bufferOUT, p_sizeOUT));
	m_magFilter  = static_cast<engine::renderer::FilterMode>(be_get<u8>(p_bufferOUT, p_sizeOUT));
	m_mipFilter  = static_cast<engine::renderer::FilterMode>(be_get<u8>(p_bufferOUT, p_sizeOUT));
	m_blendMode  = static_cast<engine::renderer::BlendMode> (be_get<u8>(p_bufferOUT, p_sizeOUT));
	m_flags      = Flags(                                    be_get<u32>(p_bufferOUT, p_sizeOUT));
	
	m_passName  = be_get<std::string>(p_bufferOUT, p_sizeOUT);
	
	m_fps.load(p_bufferOUT, p_sizeOUT, &errStatus);
	
	m_translationX.load(p_bufferOUT, p_sizeOUT, &errStatus);
	m_translationY.load(p_bufferOUT, p_sizeOUT, &errStatus);
	m_translationZ.load(p_bufferOUT, p_sizeOUT, &errStatus);
	m_scaleX      .load(p_bufferOUT, p_sizeOUT, &errStatus);
	m_scaleY      .load(p_bufferOUT, p_sizeOUT, &errStatus);
	m_isUniformScale = be_get<bool>(p_bufferOUT, p_sizeOUT);
	m_rotation    .load(p_bufferOUT, p_sizeOUT, &errStatus);
	m_texAnimU    .load(p_bufferOUT, p_sizeOUT, &errStatus);
	m_texAnimV    .load(p_bufferOUT, p_sizeOUT, &errStatus);
	
	if(m_usingDirectory)
	{
		m_spriteDirectory = be_get<std::string>(p_bufferOUT, p_sizeOUT);
		
		// loading sprite strip and meta data
		SpriteStrip::SpriteStripData spriteData;
		if(SpriteStrip::load(m_spriteDirectory, spriteData, &errStatus) == false)
		{
			TT_ERR_RETURN_ON_ERROR();
		}
		m_frameSizeX.setValue(spriteData.frameSize.x);
		m_frameSizeY.setValue(spriteData.frameSize.y);
		m_totalFrames = spriteData.totalFrameCount;
		
		TT_ERR_ASSERTMSG(m_beginFrame.getMin() >= 0 && m_beginFrame.getMax() < m_totalFrames, 
			"Sprite strip: " << spriteData.spriteStripName <<
			"\nBegin frame: '"<< m_beginFrame << "'" <<
			" has to be in the range [0.." << (m_totalFrames-1) << "]");
		TT_ERR_ASSERTMSG(m_endFrame.getMin() >= 0 && m_endFrame.getMax() < m_totalFrames, 
			"Sprite strip: " << spriteData.spriteStripName <<
			"\nEnd frame: '" << m_endFrame << "'" <<
			" has to be in the range [0.." << (m_totalFrames-1) << "]");
		
		m_spritestripID = engine::EngineID(spriteData.spriteStripName, spriteData.spriteStripNamespace);
		m_lightmaskID   = engine::EngineID("lightmask_" + spriteData.spriteStripName, spriteData.spriteStripNamespace);
	}
	else
	{
		m_frameSizeX.load(p_bufferOUT, p_sizeOUT, &errStatus);
		m_frameSizeY.load(p_bufferOUT, p_sizeOUT, &errStatus);
		
		m_spritestripID.crc1      = be_get<u32>(p_bufferOUT, p_sizeOUT);
		m_spritestripID.crc2      = be_get<u32>(p_bufferOUT, p_sizeOUT);
		m_lightmaskID.crc1        = be_get<u32>(p_bufferOUT, p_sizeOUT);
		m_lightmaskID.crc2        = be_get<u32>(p_bufferOUT, p_sizeOUT);
	}
	
	m_usingDuration = be_get<bool>(p_bufferOUT, p_sizeOUT);
	if (m_usingDuration)
	{
		m_frameDuration.load(p_bufferOUT, p_sizeOUT, &errStatus);
	}

	m_cameraSpaceScale = be_get<math::Vector2>(p_bufferOUT, p_sizeOUT);

	m_lightMaskType = static_cast<LightMaskType>(be_get<u8>(p_bufferOUT, p_sizeOUT));
	
	TT_ERR_RETURN_ON_ERROR();
	
	Animation2D::load(p_bufferOUT, p_sizeOUT, p_applyTags, p_acceptedTags, &errStatus);
	TT_ERR_RETURN_ON_ERROR();
	
	m_loaded = true;
	return true;
}


size_t FrameAnimation::getBufferSize() const
{
	// beginFrame + endFrame + 
	size_t size(m_beginFrame.getBufferSize() + m_endFrame.getBufferSize() +
		// quadsize + hold last frame + hold first frame + 
		m_quadSizeX.getBufferSize() + m_quadSizeY.getBufferSize() + 1 + 1);
	
	//animation2d + version + useDirectory + flip bitmask + filtermode/blendmode bytes + fps +
	size += Animation2D::getBufferSize() + 2 + 1 + 4 + 4 + m_fps.getBufferSize() +
		// Flag bitmask
		4 +
		// passname + size
		2 + m_passName.size() +
		// position offset x, y & z
		m_translationX.getBufferSize() + m_translationY.getBufferSize() + m_translationZ.getBufferSize() +
		m_scaleX.getBufferSize() + m_scaleY.getBufferSize() + sizeof(m_isUniformScale) + 
		m_rotation.getBufferSize() + m_texAnimU.getBufferSize() + m_texAnimV.getBufferSize();
	
	if(m_usingDirectory)
	{
		// spritedirectoryStringsize + spriteDirectory
		size += 2 + m_spriteDirectory.size();
	}
	else
	{
		// frameSize + 
		size += m_frameSizeX.getBufferSize() + m_frameSizeY.getBufferSize() +
			// EngineID CRCs for lightmask & spritestrip
			4 * sizeof(u32);
	}
	// usingDuration
	size += 1;
	if (m_usingDuration) size += m_frameDuration.getBufferSize();

	size += sizeof(m_cameraSpaceScale) + 1; // 1 byte for m_lightMaskType
	
	return size;
}


void FrameAnimation::makeDefault()
{
	m_totalFrames = 1;
	m_holdLastFrame = true;
	m_holdFirstFrame = true;
	m_beginFrame.setValue(0);
	m_endFrame.setValue(0);
	m_frameDifference = 0;
	m_frameBegin      = 0;
	m_frameCount.setValues(1,1);
	m_texture = engine::renderer::TextureCache::getDefault();
	m_frameSizeX.setValue(1);
	m_frameSizeY.setValue(1);
	m_frameSize.setValues(1,1);
	
	m_quad.reset(new PresentationQuad(m_texture, engine::renderer::ColorRGBA()));
	updateTextureAndQuadWithPresentationValues();
	
	m_quadSizeX.setValue(0.0f);
	m_quadSizeY.setValue(0.0f);
	
	m_fps.setValue(0.0f);
	
	m_translationX.setValue(0.0f);
	m_translationY.setValue(0.0f);
	m_translationZ.setValue(0.0f);
	m_scaleX      .setValue(1.0f);
	m_scaleY      .setValue(1.0f);
	m_isUniformScale = false;
	m_rotation    .setValue(0.0f);
	m_texAnimU    .setValue(0.0f);
	m_texAnimV    .setValue(0.0f);

	m_animationUV .setValues(0.0f, 0.0f);
	m_offsetUV    .setValues(0.0f, 0.0f);
	
	m_usingDirectory = false;
	
	m_loaded = true;
	
	m_usingDuration = false;
}


std::string FrameAnimation::parseOptionalString( const xml::XmlNode* p_node, 
                                                 const std::string& p_attribute, 
                                                 const std::string& p_default, 
                                                 code::ErrorStatus* p_errStatus )
{
	TT_ERR_CHAIN(std::string, p_default, "Parsing string " << p_attribute);
	code::OptionalValue<std::string> range(xml::util::parseOptionalStr(p_node, p_attribute, &errStatus));
	TT_ERR_RETURN_ON_ERROR();
	if(range.isValid())
	{
		return range.get();
	}
	else return p_default;
}


void FrameAnimation::setRanges(PresentationObject* p_presObj)
{
	m_frameSizeX.updateValue(p_presObj);
	m_frameSizeY.updateValue(p_presObj);
	m_frameSize.setValues(m_frameSizeX, m_frameSizeY);
	m_quadSizeX.updateValue(p_presObj);
	m_quadSizeY.updateValue(p_presObj);
	
	m_fps.updateValue(p_presObj);
	
	m_beginFrame.updateValue(p_presObj);
	m_endFrame.updateValue(p_presObj);
	m_frameDifference = static_cast<s32>(m_endFrame.get() - m_beginFrame.get());
	m_frameBegin      = static_cast<s32>(m_beginFrame.get());
	
	m_translationX.updateValue(p_presObj);
	m_translationY.updateValue(p_presObj);
	m_translationZ.updateValue(p_presObj);
	m_scaleX      .updateValue(p_presObj);
	if (m_isUniformScale)
	{
		m_scaleY.setValue(m_scaleX.get());
	}
	else
	{
		m_scaleY.updateValue(p_presObj);
	}
	
	m_rotation    .updateValue(p_presObj);
	m_texAnimU    .updateValue(p_presObj);
	m_texAnimV    .updateValue(p_presObj);
	
	updateTextureAndQuadWithPresentationValues();
	
	if (m_usingDuration)
	{
		m_frameDuration.updateValue(p_presObj);
		m_durationTimeLeft = m_frameDuration.get();
	}
	
	// calc duration from fps
	real fps = m_fps;
	
	fps = (getFrameDifference() + 1) / fps;

	// Get UV animation speed
	m_animationUV.x = m_texAnimU;
	m_animationUV.y = m_texAnimV;

	m_offsetUV.setValues(0,0);

	// take double speed in pinpong in to account and double the duration
	if (getDirection() == Animation2D::DirectionType_PingPong || 
	    getDirection() == Animation2D::DirectionType_ReversePingPong)
	{
		fps *= 2;
	}
	
	setDuration(fps);
	
	Animation2D::setRanges(p_presObj);
}


void FrameAnimation::updateTextureAndQuadWithPresentationValues()
{
	if (m_texture != 0)
	{
		using namespace tt::engine::renderer;
		
		m_frameCount.x = static_cast<s32>(1.0f / m_frameSizeX);
		m_frameCount.y = static_cast<s32>(1.0f / m_frameSizeY);
		
		// If the frame is animating or larger than the texture, allow repeating
		AddressMode addressModeU(AddressMode_Clamp);
		AddressMode addressModeV(AddressMode_Clamp);

		if (math::realEqual(m_animationUV.x,      0.0f) == false ||
			math::realEqual(m_cameraSpaceScale.x, 0.0f) == false ||
			m_frameCount.x == 0)
		{
			addressModeU = AddressMode_Wrap;
		}

		if (math::realEqual(m_animationUV.y,      0.0f) == false ||
			math::realEqual(m_cameraSpaceScale.y, 0.0f) == false ||
			m_frameCount.y == 0)
		{
			addressModeV = AddressMode_Wrap;
		}

		m_texture->setAddressMode(addressModeU, addressModeV);

		if(m_lightmask != 0)
		{
			m_lightmask->setAddressMode(addressModeU, addressModeV);
		}
	}
	
	if (m_quad != 0)
	{
		if(m_quadSizeX != 0 && m_quadSizeY != 0)
		{
			m_quad->setWidth(m_quadSizeX);
			m_quad->setHeight(m_quadSizeY);

			// This will probably be problematic if createQuad is called multiple times...
			m_cameraSpaceScale.x *= 1.0f / m_quadSizeX;
			m_cameraSpaceScale.y *= 1.0f / m_quadSizeY;
		}
		else
		{
			m_quad->setWidth (m_frameSize.x * m_texture->getWidth());
			m_quad->setHeight(m_frameSize.y * m_texture->getHeight());
		}
		
		if(engine::renderer::isValidBlendMode(m_blendMode))
		{
			m_quad->setBlendMode(m_blendMode);
		}
		
		math::Vector2 topLeft(0,0);
		math::Vector2 bottomRight(math::Vector2(m_frameSizeX, m_frameSizeY));
		
		if((m_flip & Flip_Horizontal) != 0)
		{
			// Swap x coordinates
			std::swap(topLeft.x, bottomRight.x);
		}
		if((m_flip & Flip_Vertical) != 0)
		{
			// Swap y coordinates
			std::swap(topLeft.y, bottomRight.y);
		}
		
		m_quad->setTexcoords(topLeft, bottomRight);
		m_quad->setRotation(getRotation());
		m_quad->setScale(getScale());
		m_quad->setTranslation(getTranslation());
	}
}


FrameAnimation::FrameAnimation(const FrameAnimation& p_rhs)
:
anim2d::Animation2D(p_rhs),
m_beginFrame(p_rhs.m_beginFrame),
m_endFrame(p_rhs.m_endFrame),
m_frameDifference(p_rhs.m_frameDifference),
m_frameBegin(p_rhs.m_frameBegin),
m_totalFrames(p_rhs.m_totalFrames),
m_frameCount(p_rhs.m_frameCount),
m_frameSize(p_rhs.m_frameSize),
m_frameSizeX(p_rhs.m_frameSizeX),
m_frameSizeY(p_rhs.m_frameSizeY),
m_quadSizeX(p_rhs.m_quadSizeX),
m_quadSizeY(p_rhs.m_quadSizeY),
m_fps(p_rhs.m_fps),
m_holdFirstFrame(p_rhs.m_holdFirstFrame),
m_holdLastFrame(p_rhs.m_holdLastFrame),
m_flip(p_rhs.m_flip),
m_minFilter(p_rhs.m_minFilter),
m_magFilter(p_rhs.m_magFilter),
m_mipFilter(p_rhs.m_mipFilter),
m_blendMode(p_rhs.m_blendMode),
m_flags(p_rhs.m_flags),
m_passName(p_rhs.m_passName),
m_spritestripID(p_rhs.m_spritestripID),
m_lightmaskID(p_rhs.m_lightmaskID),
m_spriteDirectory(p_rhs.m_spriteDirectory),
m_usingDirectory(p_rhs.m_usingDirectory),
m_texture(p_rhs.m_texture),
m_lightmask(p_rhs.m_lightmask),
m_quad(p_rhs.m_quad == 0 ? PresentationQuadPtr() : p_rhs.m_quad->clone()),
m_translationX(p_rhs.m_translationX),
m_translationY(p_rhs.m_translationY),
m_translationZ(p_rhs.m_translationZ),
m_scaleX(p_rhs.m_scaleX),
m_scaleY(p_rhs.m_scaleY),
m_isUniformScale(p_rhs.m_isUniformScale),
m_rotation(p_rhs.m_rotation),
m_texAnimU(p_rhs.m_texAnimU),
m_texAnimV(p_rhs.m_texAnimV),
m_loaded(p_rhs.m_loaded),
m_usingDuration(p_rhs.m_usingDuration),
m_frameDuration(p_rhs.m_frameDuration),
m_durationTimeLeft(p_rhs.m_durationTimeLeft),
m_offsetUV(p_rhs.m_offsetUV),
m_cameraSpaceScale(p_rhs.m_cameraSpaceScale),
m_lightMaskType(p_rhs.m_lightMaskType)
{
}

//namespace end
}
}
