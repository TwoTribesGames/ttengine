#include <tt/code/bufferutils.h>
#include <tt/fs/utils/utils.h>
#include <tt/pres/FrameAnimation.h>
#include <tt/pres/FrameAnimationStack.h>
#include <tt/pres/PresentationObject.h>
#include <tt/xml/XmlNode.h>
#include <tt/xml/util/parse.h>


namespace tt {
namespace pres {


#define MAKE_VERSION(major, minor) (((major) << 8) | (minor))
#define GET_MAJOR_VERSION(version) ((version) >> 8)
#define GET_MINOR_VERSION(version) ((version) & 0xFF)

static const u16 s_version = MAKE_VERSION(1, 2);


void FrameAnimationStack::createQuads() const
{
	for (Stack::const_iterator it = m_activeAnimations.begin(); it != m_activeAnimations.end(); ++it)
	{
		const FrameAnimationPtr& ptr(*it);
		TT_ASSERT(ptr->hasNameOrTagMatch());
		ptr->createQuad();
	}
	
	for (SequenceStack::const_iterator it = m_activeSequences.begin(); it != m_activeSequences.end(); ++it)
	{
		TT_ASSERT((*it)->hasNameOrTagMatch());
		
		const SequenceType::Stack& animations((*it)->getAnimations());
		for (SequenceType::Stack::const_iterator sequenceIt = animations.begin();
		     sequenceIt != animations.end(); ++sequenceIt)
		{
			(*sequenceIt)->createQuad();
		}
	}
}


void FrameAnimationStack::updateColors(const anim2d::ColorAnimationStack2D& p_coloranim)
{
	for (Stack::const_iterator it = m_activeAnimations.begin(); it != m_activeAnimations.end(); ++it)
	{
		(*it)->updateColor(p_coloranim);
	}
	
	for (SequenceStack::const_iterator it = m_activeSequences.begin(); it != m_activeSequences.end(); ++it)
	{
		(*it)->getActive()->updateColor(p_coloranim);
	}
}


void FrameAnimationStack::render(const math::Matrix44& p_transform) const
{
	for (Stack::const_iterator it = m_activeAnimations.begin(); it != m_activeAnimations.end(); ++it)
	{
		const FrameAnimationPtr& ptr = (*it);
		TT_ASSERT(ptr->hasNameOrTagMatch());
		ptr->renderWithLightPass(p_transform);
	}
	
	for (SequenceStack::const_iterator it = m_activeSequences.begin(); it != m_activeSequences.end(); ++it)
	{
		const SequencePtr& ptr = (*it);
		TT_ASSERT(ptr->isEmpty() == false);
		
		const FrameAnimationPtr& activeAnim(ptr->getActive());
		
		TT_ASSERT(ptr->hasNameOrTagMatch());
		activeAnim->renderWithLightPass(p_transform);
	}
}


// FIXME: Do not use strings to ID passes
void FrameAnimationStack::renderPass(const std::string& p_passName, const math::Matrix44& p_transform) const
{
	for (Stack::const_iterator it = m_activeAnimations.begin(); it != m_activeAnimations.end(); ++it)
	{
		const FrameAnimationPtr& ptr = (*it);
		TT_ASSERT(ptr->hasNameOrTagMatch());
		if (ptr->getRenderPass() == p_passName)
		{
			ptr->render(p_transform);
		}
	}
	
	for (SequenceStack::const_iterator it = m_activeSequences.begin(); it != m_activeSequences.end(); ++it)
	{
		const SequencePtr& ptr = (*it);
		TT_ASSERT(ptr->isEmpty() == false);
		
		const FrameAnimationPtr& activeAnim(ptr->getActive());
		
		TT_ASSERT(ptr->hasNameOrTagMatch());
		if (activeAnim->getRenderPass() == p_passName)
		{
			activeAnim->render(p_transform);
		}
	}
}


engine::renderer::TextureContainer FrameAnimationStack::getAndLoadAllUsedTextures() const
{
	engine::renderer::TextureContainer textures;
	
	for(Stack::const_iterator it(m_allAnimations.begin()) ; it != m_allAnimations.end() ; ++it)
	{
		(*it)->getAndLoadAllUsedTextures(textures);
	}
	
	for (SequenceStack::const_iterator it = m_allSequences.begin(); it != m_allSequences.end(); ++it)
	{
		const SequenceType::Stack& animations((*it)->getAnimations());
		
		for (SequenceType::Stack::const_iterator sequenceIt = animations.begin();
		     sequenceIt != animations.end(); ++sequenceIt)
		{
			(*sequenceIt)->getAndLoadAllUsedTextures(textures);
		}
		
	}
	return textures;
}


bool FrameAnimationStack::isVisible() const
{
	for(Stack::const_iterator it(m_activeAnimations.begin()) ; it != m_activeAnimations.end() ; ++it)
	{
		TT_ASSERT((*it)->hasNameOrTagMatch());
		if((*it)->isVisible()) return true;
	}
	
	for (SequenceStack::const_iterator it = m_activeSequences.begin(); it != m_activeSequences.end(); ++it)
	{
		TT_ASSERT((*it)->isEmpty() == false);
		TT_ASSERT((*it)->hasNameOrTagMatch());
		
		if ((*it)->getActive()->isVisible())
		{
			return true;
		}
	}
	return false;
}


bool FrameAnimationStack::isPassVisible(const std::string& p_passName)const
{
	for(Stack::const_iterator it(m_activeAnimations.begin()) ; it != m_activeAnimations.end() ; ++it)
	{
		const FrameAnimationPtr& ptr = (*it);
		TT_ASSERT(ptr->hasNameOrTagMatch());
		if (ptr->isVisible() && 
		    ptr->getRenderPass() == p_passName) return true;
	}
	
	for (SequenceStack::const_iterator it = m_activeSequences.begin(); it != m_activeSequences.end(); ++it)
	{
		TT_ASSERT((*it)->isEmpty() == false);
		
		const FrameAnimationPtr& activeAnim((*it)->getActive());
		
		TT_ASSERT((*it)->hasNameOrTagMatch());
		if (activeAnim->isVisible() &&
		    activeAnim->getRenderPass() == p_passName)
		{
			return true;
		}
	}
	return false;
}


bool FrameAnimationStack::load( const xml::XmlNode* p_node, const DataTags& p_applyTags, 
                                const Tags& p_acceptedTags, code::ErrorStatus* p_errStatus )
{
	TT_ERR_CHAIN(bool, false, "Loading Frame animation Stack");
	
	DataTags stackTags;
	stackTags.load(p_node, p_applyTags, p_acceptedTags, &errStatus);
	TT_ERR_RETURN_ON_ERROR();
	
	SequencePtr sequence;
	SequencePtr sequenceRenderPassCopy;
	if (p_node->getAttribute("type") == "sequence")
	{
		sequence.reset(new SequenceType());
		sequenceRenderPassCopy.reset(new SequenceType());
	}
	
	
	FrameAnimationPtr loopingAnim;
	FrameAnimationPtr loopingAnimRenderpassCopy;
	
	// iterate over children
	for (const xml::XmlNode* child = p_node->getChild(); child != 0; child = child->getSibling())
	{
		if(child->getName() == "frameanim")
		{
			FrameAnimationPtr frameAnim(new FrameAnimation);
			
			frameAnim->loadXml(child, stackTags, p_acceptedTags, &errStatus);
			TT_ERR_RETURN_ON_ERROR();
			
			if (sequence == 0)
			{
				m_allAnimations.push_back(frameAnim);
				TT_ASSERT(m_activeAnimations.empty());
			}
			else
			{
				// hold looping animation separete and add it to the end of the sequence
				if (frameAnim->isLooping())
				{
					TT_ERR_ASSERTMSG(loopingAnim == 0, 
							"Found multiple looping frame animations in a sequence frameanimstack with name: '" << 
							loopingAnim->getTags().getName() << "'. Only 1 allowed.");
					
					loopingAnim = frameAnim;
				}
				else
				{
					sequence->addAnimation(frameAnim);
				}
			}

			// TODO: Maybe default is to always have lightmask?? => Ask Meinte
			LightMaskType type(LightMaskType_None);

			// Handle lightmasks
			if(child->hasAttribute("lightmask"))
			{
				const std::string& lightMaskType = child->getAttribute("lightmask");

				if(lightMaskType == "use_lightmask")
				{
					type = LightMaskType_UseLightmaskTexture;
				}
				else if(lightMaskType == "use_alpha")
				{
					type = LightMaskType_UseAlphaChannel;
				}
				else if(lightMaskType == "none")
				{
					type = LightMaskType_None;
				}
				else
				{
					TT_ERR_AND_RETURN("Undefined lightmask property: " << lightMaskType <<
						" Valid options are: 'none', 'use_lightmask' and 'use_alpha'\n");
				}
				frameAnim->setLightMaskType(type);
			}
		}
		// FIXME: Hardcoded musthave/mustnothave; not ideal
		else if (child->getName() != "musthave" && child->getName() != "mustnothave")
		{
			TT_ERR_AND_RETURN("Unsuported child tag in Frame Animation Stack: " << child->getName());
		}
	}
	
	if (sequence != 0)
	{
		if (loopingAnim != 0)
		{
			sequence->addAnimation(loopingAnim);
		}
		
		if (sequence->isEmpty() == false)
		{
			m_allSequences.push_back(sequence);
			TT_ASSERT(m_activeSequences.empty());
		}
	}
	
	if (sequenceRenderPassCopy != 0)
	{
		if (loopingAnimRenderpassCopy != 0)
		{
			sequenceRenderPassCopy->addAnimation(loopingAnimRenderpassCopy);
		}
		
		if (sequenceRenderPassCopy->isEmpty() == false)
		{
			m_allSequences.push_back(sequenceRenderPassCopy);
			TT_ASSERT(m_activeSequences.empty());
		}
	}
	
	return true;
}


bool FrameAnimationStack::save( u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus ) const
{
	TT_ERR_CHAIN(bool, false, "Saving Frame animation Stack");
	
	if(getBufferSize() > p_sizeOUT)
	{
		TT_ERR_AND_RETURN("Not enough space in buffer need " << getBufferSize() 
		                  << " got " << p_sizeOUT);
	}
	
	using namespace code::bufferutils;
	
	be_put(s_version, p_bufferOUT, p_sizeOUT);
	be_put(static_cast<u16>(m_allAnimations.size()), p_bufferOUT, p_sizeOUT);
	
	for(Stack::const_iterator it(m_allAnimations.begin()) ; it != m_allAnimations.end() ; ++it)
	{
		if((*it)->save(p_bufferOUT, p_sizeOUT, &errStatus) == false) 
		{
			TT_ERR_AND_RETURN("Saving of frameAnim failed");
		}
	}
	
	be_put(static_cast<u16>(m_allSequences.size()), p_bufferOUT, p_sizeOUT);
	
	for(SequenceStack::const_iterator it(m_allSequences.begin()) ; it != m_allSequences.end() ; ++it)
	{
		const SequenceType::Stack& animations((*it)->getAnimations());
		
		be_put(static_cast<u16>(animations.size()), p_bufferOUT, p_sizeOUT);
		
		for (SequenceType::Stack::const_iterator sequenceIt = animations.begin();
		     sequenceIt != animations.end(); ++sequenceIt)
		{
			if((*sequenceIt)->save(p_bufferOUT, p_sizeOUT, &errStatus) == false) 
			{
				TT_ERR_AND_RETURN("Saving of frameAnim sequence failed");
			}
		}
	}
	return true;
}


bool FrameAnimationStack::load( const u8*& p_bufferOUT, size_t& p_sizeOUT, 
                                const DataTags& p_applyTags, const Tags& p_acceptedTags, 
                                code::ErrorStatus* p_errStatus )
{
	TT_ERR_CHAIN(bool, false, "Loading frameanim stack");
	
	using namespace code::bufferutils;
	
	const u16 version = be_get<u16>(p_bufferOUT, p_sizeOUT);
	TT_ERR_ASSERTMSG(version == s_version, "Invalid version: code "
		<< GET_MAJOR_VERSION(s_version) << "." << GET_MINOR_VERSION(s_version) << ", data "
		<< GET_MAJOR_VERSION(version)   << "." << GET_MINOR_VERSION(version)   <<
		" -- please update your converter.");
	
	const u16 frameAnimCount = be_get<u16>(p_bufferOUT, p_sizeOUT);
	for (u16 i = 0; i < frameAnimCount; ++i)
	{
		FrameAnimationPtr frameanim(new FrameAnimation);
		frameanim->load(p_bufferOUT, p_sizeOUT, p_applyTags, p_acceptedTags, &errStatus);
		
		TT_ERR_RETURN_ON_ERROR();
		
		m_allAnimations.push_back(frameanim);
		TT_ASSERT(m_activeAnimations.empty());
	}
	
	TT_ERR_ASSERTMSG(p_sizeOUT >= sizeof(u16), "Buffer too small.");
	const u16 sequenceCount = be_get<u16>(p_bufferOUT, p_sizeOUT);
	for (u16 i = 0; i < sequenceCount; ++i)
	{
		SequencePtr sequence(new SequenceType);
		
		TT_ERR_ASSERTMSG(p_sizeOUT >= sizeof(u16), "Buffer too small.");
		const u16 animationCount = be_get<u16>(p_bufferOUT, p_sizeOUT);
		
		for (u16 j = 0; j < animationCount; ++j)
		{
			FrameAnimationPtr frameanim(new FrameAnimation);
			frameanim->load(p_bufferOUT, p_sizeOUT, p_applyTags, p_acceptedTags, &errStatus);
			
			TT_ERR_RETURN_ON_ERROR();
			
			sequence->addAnimation(frameanim);
		}
		m_allSequences.push_back(sequence);
		TT_ASSERT(m_activeSequences.empty());
	}
	
	return true;
}


void FrameAnimationStack::appendStack(FrameAnimationStack& p_other)
{
	// just append the whole stack to the end
	m_allAnimations.insert(m_allAnimations.end(), p_other.m_allAnimations.begin(), p_other.m_allAnimations.end());
	TT_ASSERT(m_activeAnimations.empty());
	p_other.m_allAnimations.clear();
	p_other.m_activeAnimations.clear();
	
	m_allSequences.insert( m_allSequences.end(),  p_other.m_allSequences.begin(),  p_other.m_allSequences.end());
	TT_ASSERT(m_activeSequences.empty());
	p_other.m_allSequences.clear();
	p_other.m_activeSequences.clear();
}


str::StringSet FrameAnimationStack::getRenderPassNames() const
{
	str::StringSet names;
	for(Stack::const_iterator it(m_allAnimations.begin()) ; it != m_allAnimations.end() ; ++it)
	{
		std::string passName((*it)->getRenderPass());
		if (passName.empty() == false)
		{
			names.insert(passName);
		}
	}
	
	for (SequenceStack::const_iterator it = m_allSequences.begin(); it != m_allSequences.end(); ++it)
	{
		const SequenceType::Stack& animations((*it)->getAnimations());
		
		for (SequenceType::Stack::const_iterator sequenceIt = animations.begin();
		     sequenceIt != animations.end(); ++sequenceIt)
		{
			std::string passName((*sequenceIt)->getRenderPass());
			if (passName.empty() == false)
			{
				names.insert(passName);
			}
		}
	}
	
	return names;
}


const std::string createLightpassDirectory(const std::string& p_originalDirectory)
{
	std::string spriteDirectoryName(tt::fs::utils::getLastDirectoryName(p_originalDirectory));
	std::string spriteDirectoryPath(tt::fs::utils::getDirectory(p_originalDirectory));
	
	return spriteDirectoryPath + "lightmask_" + spriteDirectoryName;
}


Dependencies FrameAnimationStack::getDependencies() const
{
	Dependencies dependencies;
	for(Stack::const_iterator it(m_allAnimations.begin()) ; it != m_allAnimations.end() ; ++it)
	{
		if((*it)->getSpriteDirectory().empty() == false)
		{
			DependencyData data;
			data.type = "spritestrip";
			data.asset = (*it)->getSpriteDirectory(); 
			data.isLightMask = false;

			dependencies.push_back(data);

			if((*it)->getLightMaskType() == LightMaskType_UseLightmaskTexture)
			{
				data.isLightMask = true;
				data.asset = createLightpassDirectory(data.asset);
				dependencies.push_back(data);
			}
		}
	}
	
	for (SequenceStack::const_iterator it = m_allSequences.begin(); it != m_allSequences.end(); ++it)
	{
		const SequenceType::Stack& animations((*it)->getAnimations());
		
		for (SequenceType::Stack::const_iterator sequenceIt = animations.begin();
		     sequenceIt != animations.end(); ++sequenceIt)
		{
			if((*sequenceIt)->getSpriteDirectory().empty() == false)
			{
				DependencyData data;
				data.type = "spritestrip";
				data.asset = (*sequenceIt)->getSpriteDirectory();

				dependencies.push_back(data);

				if((*sequenceIt)->getLightMaskType() == LightMaskType_UseLightmaskTexture)
				{
					data.isLightMask = true;
					data.asset = createLightpassDirectory(data.asset);
					dependencies.push_back(data);
				}
			}
		}
	}

	return dependencies;
}


size_t FrameAnimationStack::getBufferSize() const
{
	size_t size(2 + 2); // number of animations + version
	for(Stack::const_iterator it(m_allAnimations.begin()) ; it != m_allAnimations.end() ; ++it)
	{
		size += (*it)->getBufferSize(); 
	}
	
	size += 2; // number of sequences
	for (SequenceStack::const_iterator it = m_allSequences.begin(); it != m_allSequences.end(); ++it)
	{
		size += 2; // number of animations
		const SequenceType::Stack& animations((*it)->getAnimations());
		
		for (SequenceType::Stack::const_iterator sequenceIt = animations.begin();
		     sequenceIt != animations.end(); ++sequenceIt)
		{
			size += (*sequenceIt)->getBufferSize(); 
		}
	}
	return size;
}


void FrameAnimationStack::makeDefault()
{
	FrameAnimationPtr frameAnim(new FrameAnimation);
	frameAnim->makeDefault();
	m_allAnimations.push_back(frameAnim);
	TT_ASSERT(m_activeAnimations.empty());
}


FrameAnimationStack::FrameAnimationStack(const FrameAnimationStack& p_rhs)
:
anim2d::StackBase<FrameAnimation>(p_rhs)
{
}

//namespace end
}
}
