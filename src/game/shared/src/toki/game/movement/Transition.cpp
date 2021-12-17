#include <tt/code/bufferutils.h>
#include <tt/xml/XmlNode.h>
#include <tt/xml/util/parse.h>

#include <toki/game/movement/Transition.h>
#include <toki/serialization/utils.h>


namespace toki {
namespace game {
namespace movement {

//--------------------------------------------------------------------------------------------------
// Public member functions


TransitionPtr Transition::createFromXML(const tt::xml::XmlNode* p_node, tt::code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN(TransitionPtr, TransitionPtr(), "load (turn)transition from XML.");
	
	TT_ERR_ASSERT(p_node->getName() == "transition" || p_node->getName() == "turn_transition");
	
	TransitionPtr result(new Transition(p_node->getName() == "turn_transition"));
	
	using tt::xml::XmlNode;
	
	// Presentation animation tags (optional; plays an animation with the specified tags if specified)
	{
		const XmlNode* node = p_node->getFirstChild("animation_tags");
		
		if (node != 0)
		{
			for (const XmlNode* child = node->getChild(); child != 0; child = child->getSibling())
			{
				TT_ERR_ASSERT(child->getName() == "tag");
				TT_ERR_ASSERT(child->getData().empty() == false);
				result->m_animationTags.insert(tt::pres::Tag(child->getData()));
			}
		}
	}
	
	// Presentation animation name
	{
		const XmlNode* node = p_node->getFirstChild("animation_name");
		
		if (node != 0)
		{
			result->m_animationName = node->getData();
		}
	}
	
	// Callback names (optional; no callback triggered if no callback name specified)
	{
		const XmlNode* node = p_node->getFirstChild("callbacks");
		
		if (node != 0)
		{
			const XmlNode* startNode = node->getFirstChild("start");
			if (startNode != 0)
			{
				result->m_startCallback = startNode->getData();
			}
			
			const XmlNode* endNode = node->getFirstChild("end");
			if (endNode != 0)
			{
				result->m_endCallback = endNode->getData();
			}
		}
	}
	
	// Speed
	{
		const XmlNode* node = p_node->getFirstChild("speed");
		if (node != 0)
		{
			result->m_speed = getTransitionSpeedFromName(node->getData());
			TT_ERR_ASSERTMSG(isValidTransitionSpeed(result->m_speed),
			                 "Invalid speed value found: '" << node->getData() << "'");
		}
	}
	
	TT_ERR_RETURN_ON_ERROR();
	
	return result;
}


void Transition::serialize(const ConstTransitionPtr& p_transition, tt::code::BufferWriteContext* p_context)
{
	TT_NULL_ASSERT(p_context);
	namespace bu = tt::code::bufferutils;
	
	const bool exists = (p_transition != 0);
	bu::put(exists, p_context);
	
	if (exists)
	{
		// Start with isTurn so we can use it for ctor in unserialize.
		bu::put(p_transition->m_isTurn,        p_context); 
		
		bu::put(                         p_transition->m_animationName, p_context);
		serialization::serializePresTags(p_transition->m_animationTags, p_context);
		bu::put(                         p_transition->m_startCallback, p_context);
		bu::put(                         p_transition->m_endCallback,   p_context);
		bu::putEnum<u8, TransitionSpeed>(p_transition->m_speed,         p_context);
	}
}


TransitionPtr Transition::unserialize(tt::code::BufferReadContext* p_context)
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	const bool exists = bu::get<bool>(p_context);
	if (exists == false)
	{
		return TransitionPtr();
	}
	
	const bool isTurn = bu::get<bool>(p_context);
	TransitionPtr result(new Transition(isTurn));
	
	result->m_animationName = bu::get<std::string   >(p_context);
	result->m_animationTags = serialization::unserializePresTags(p_context);
	result->m_startCallback = bu::get<std::string   >(p_context);
	result->m_endCallback   = bu::get<std::string   >(p_context);
	result->m_speed         = bu::getEnum<u8, TransitionSpeed>(p_context);
	
	return result;
}


//--------------------------------------------------------------------------------------------------
// Private member functions



// Namespace end
}
}
}
