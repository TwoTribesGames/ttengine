#include <tt/code/bufferutils.h>
#include <tt/code/ErrorStatus.h>
#include <tt/code/helpers.h>
#include <tt/engine/anim2d/AnimationStack2D.h>
#include <tt/engine/anim2d/ColorAnimationStack2D.h>
#include <tt/engine/anim2d/TranslationAnimation2D.h>
#include <tt/engine/scene2d/shoebox/shoebox_types.h>
#include <tt/engine/scene2d/shoebox/shoebox.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/engine/fwd.h>
#include <tt/fs/utils/utils.h>
#include <tt/fs/File.h>
#include <tt/fs/fs.h>
#include <tt/str/str.h>
#include <tt/xml/util/check.h>
#include <tt/xml/util/parse.h>
#include <tt/xml/util/store.h>
#include <tt/xml/XmlNode.h>
#include <tt/xml/XmlFileWriter.h>


namespace tt {
namespace engine {
namespace scene2d {
namespace shoebox {

#define MAKE_VERSION(major, minor) (((major) << 8) | (minor))
#define GET_MAJOR_VERSION(version) ((version) >> 8)
#define GET_MINOR_VERSION(version) ((version) & 0xFF)

static const u16 g_shoeboxDataVersion = MAKE_VERSION(1, 15);


//--------------------------------------------------------------------------------------------------
// Helper functions (TODO: Move this to engine/ xml parse)

renderer::FilterMode readFilterMode(
		const xml::XmlNode* p_node,
		const std::string&  p_name,
		code::ErrorStatus*  p_errStatus)
{
	TT_ERR_CHAIN(renderer::FilterMode, renderer::FilterMode_Invalid, "Reading engine filter mode");
	
	const std::string& filterModeStr(p_node->getAttribute(p_name));
	
	if (filterModeStr.empty() == false)
	{
		renderer::FilterMode filterMode = renderer::getFilterModeFromName(filterModeStr);
		
		TT_ERR_ASSERTMSG(renderer::isValidFilterMode(filterMode),
		                 "Found unknown value '" << filterModeStr
		                 << "' in attribute '" << p_name
		                 << "' in XML element <" << p_node->getName() << ">.");
		
		return filterMode;
	}
	
	return renderer::FilterMode_Invalid;
}


//--------------------------------------------------------------------------------------------------
// PositionData

void PositionData::loadPosition(const xml::XmlNode* p_node, code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN_VOID("Parsing Position Data");
	position = xml::util::parseVector3(p_node, &errStatus);
}


void PositionData::savePosition(xml::XmlNode* p_node, code::ErrorStatus* p_errStatus) const
{
	TT_ERR_CHAIN_VOID("Storing Position Data");
	xml::util::store(p_node, position, &errStatus);
}


void PositionData::loadPosition(const u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN_VOID("Loading Position Data From Memory");
	TT_ERR_ASSERTMSG(p_sizeOUT >= s_size, "Buffer too small");
	TT_ERR_RETURN_ON_ERROR();
	
	position = code::bufferutils::be_get<math::Vector3>(p_bufferOUT, p_sizeOUT);
}


void PositionData::savePosition(u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus) const
{
	TT_ERR_CHAIN_VOID("Saving Position Data To Memory");
	TT_ERR_ASSERTMSG(p_sizeOUT >= s_size, "Buffer too small");
	TT_ERR_RETURN_ON_ERROR();
	
	code::bufferutils::be_put(position, p_bufferOUT, p_sizeOUT);
}


//--------------------------------------------------------------------------------------------------
// ParticleData

ParticleData ParticleData::parse(const xml::XmlNode* p_node, code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN(ParticleData, ParticleData(), "Parsing Particle Data from XmlNode");
	ParticleData ret;
	ret.loadParticle(p_node, &errStatus);
	return ret;
}


ParticleData ParticleData::parse(const u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN(ParticleData, ParticleData(), "Parsing Particle Data from a buffer");
	ParticleData ret;
	ret.loadParticle(p_bufferOUT, p_sizeOUT, &errStatus);
	return ret;
}


void ParticleData::loadParticle(const xml::XmlNode* p_node, code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN_VOID("Parsing Particle Data");
	loadPosition(p_node, &errStatus);
	
	namespace ut = xml::util;
	ut::checkName(p_node, "particle", errStatus);
	particleFilename = ut::parseStr(         p_node, "effect_file", &errStatus);
	scale            = ut::parseOptionalReal(p_node, "scale"      , &errStatus);
	parentID         = ut::parseOptionalStr( p_node, "parent_id"  , &errStatus);
	hidden           = ut::parseOptionalBool(p_node, "hidden"     , &errStatus);
	
	for (const xml::XmlNode* child = p_node->getChild(); child != 0; child = child->getSibling())
	{
		if (child->getName() == "tag")
		{
			if (child->getAttribute("name").empty())
			{
				TT_ERR_AND_RETURN("Expected 'name' attribute in 'tag'");
			}
			tags.push_back(child->getAttribute("name"));
		}
	}
}


void ParticleData::saveParticle(xml::XmlNode* p_node, code::ErrorStatus* p_errStatus) const
{
	TT_ERR_CHAIN_VOID("Storing Particle Data");
	savePosition(p_node, &errStatus);
	if (p_node != 0)
	{
		p_node->setName("particle");
	}
	namespace ut = xml::util;
	ut::store(p_node, "effect_file", particleFilename, &errStatus);
	ut::store(p_node, "scale"      , scale           , &errStatus);
	ut::store(p_node, "parent_id"  , parentID        , &errStatus);
	ut::store(p_node, "hidden"     , hidden          , &errStatus);
	
	for (Tags::const_iterator it = tags.begin(); it != tags.end(); ++it)
	{
		xml::XmlNode* node = new xml::XmlNode("tag");
		xml::util::store(node, "name", (*it), &errStatus);
	}
}


void ParticleData::loadParticle(const u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN_VOID("Loading Particle Data From Memory");
	loadPosition(p_bufferOUT, p_sizeOUT, &errStatus);
	TT_ERR_RETURN_ON_ERROR();
	
	namespace bu = code::bufferutils;
	particleFilename = bu::be_get<std::string>(p_bufferOUT, p_sizeOUT);
	scale            = bu::be_get<real       >(p_bufferOUT, p_sizeOUT);
	parentID         = bu::be_get<std::string>(p_bufferOUT, p_sizeOUT);
	hidden           = bu::be_get<bool       >(p_bufferOUT, p_sizeOUT);
	
	TT_ERR_ASSERTMSG(p_sizeOUT >= 2, "Buffer too small");
	const u16 tagCount = bu::be_get<u16>(p_bufferOUT, p_sizeOUT);
	tags.resize(static_cast<Tags::size_type>(tagCount));
	for (Tags::iterator it = tags.begin(); it != tags.end(); ++it)
	{
		*it = bu::be_get<std::string>(p_bufferOUT, p_sizeOUT);
	}
}


void ParticleData::saveParticle(u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus) const
{
	TT_ERR_CHAIN_VOID("Saving Particle Data To Memory");
	savePosition(p_bufferOUT, p_sizeOUT, &errStatus);
	TT_ERR_RETURN_ON_ERROR();
	
	namespace bu = code::bufferutils;
	bu::be_put(particleFilename, p_bufferOUT, p_sizeOUT);
	bu::be_put(scale.get(),      p_bufferOUT, p_sizeOUT);
	bu::be_put(parentID.get(),   p_bufferOUT, p_sizeOUT);
	bu::be_put(hidden.get(),     p_bufferOUT, p_sizeOUT);
	
	TT_ERR_ASSERTMSG(p_sizeOUT >= 2, "Buffer too small");
	bu::be_put(static_cast<u16>(tags.size()), p_bufferOUT, p_sizeOUT);
	for (Tags::const_iterator it = tags.begin(); it != tags.end(); ++it)
	{
		bu::be_put(*it, p_bufferOUT, p_sizeOUT);
	}
}


//--------------------------------------------------------------------------------------------------
// RectData

void RectData::loadRect(const xml::XmlNode* p_node, code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN_VOID("Parsing Rectangle Data");
	loadPosition(p_node, &errStatus);
	
	rotation = xml::util::parseOptionalReal(p_node, "rotation", &errStatus);
	width    = xml::util::parseOptionalReal(p_node, "width",    &errStatus);
	height   = xml::util::parseOptionalReal(p_node, "height",   &errStatus);
}


void RectData::saveRect(xml::XmlNode* p_node, code::ErrorStatus* p_errStatus) const
{
	TT_ERR_CHAIN_VOID("Storing Rectangle Data");
	savePosition(p_node, &errStatus);
	
	xml::util::store(p_node, "rotation", rotation, &errStatus);
	xml::util::store(p_node, "width",    width,    &errStatus);
	xml::util::store(p_node, "height",   height,   &errStatus);
}


void RectData::loadRect(const u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN_VOID("Loading Rectangle Data From Memory");
	loadPosition(p_bufferOUT, p_sizeOUT, &errStatus);
	TT_ERR_ASSERTMSG(p_sizeOUT >= s_size, "Buffer too small");
	TT_ERR_RETURN_ON_ERROR();
	
	namespace bu = code::bufferutils;
	rotation = bu::be_get<real>(p_bufferOUT, p_sizeOUT);
	width    = bu::be_get<real>(p_bufferOUT, p_sizeOUT);
	height   = bu::be_get<real>(p_bufferOUT, p_sizeOUT);
}


void RectData::saveRect(u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus) const
{
	TT_ERR_CHAIN_VOID("Saving Rectangle Data To Memory");
	savePosition(p_bufferOUT, p_sizeOUT, &errStatus);
	TT_ERR_ASSERTMSG(p_sizeOUT >= s_size, "Buffer too small");
	TT_ERR_RETURN_ON_ERROR();
	
	namespace bu = code::bufferutils;
	bu::be_put(rotation.get(), p_bufferOUT, p_sizeOUT);
	bu::be_put(width.get(),    p_bufferOUT, p_sizeOUT);
	bu::be_put(height.get(),   p_bufferOUT, p_sizeOUT);
}


//--------------------------------------------------------------------------------------------------
// ScriptTriggerData

ScriptTriggerData ScriptTriggerData::parse(const xml::XmlNode* p_node, code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN(ScriptTriggerData, ScriptTriggerData(), "Parsing Script Trigger Data from XmlNode");
	ScriptTriggerData data;
	data.loadTrigger(p_node, &errStatus);
	return data;
}


ScriptTriggerData ScriptTriggerData::parse(const u8*& p_bufferOUT, size_t& p_sizeOUT,
                                           code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN(ScriptTriggerData, ScriptTriggerData(), "Parsing Script Trigger Data from a buffer");
	ScriptTriggerData data;
	data.loadTrigger(p_bufferOUT, p_sizeOUT, &errStatus);
	return data;
}


void ScriptTriggerData::print() const
{
	TT_Printf("ScriptTriggerData::print - rect.left: %d, rect.top: %d, rect.width: %d, rect.height: %d, "
	          "triggerName: '%s', scriptFilename: '%s', tagName(%s): '%s'\n",
	          rect.getLeft(), rect.getTop(), rect.getWidth(), rect.getHeight(),
	          triggerName.c_str(), scriptFilename.c_str(),
	          ( (tagName.isValid()) ? "entered" : "default" ),
	          tagName.get().c_str());
}


void ScriptTriggerData::loadTrigger(const xml::XmlNode* p_node, code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN_VOID("Parsing Script Trigger Data");
	
	TT_ERR_NULL_ASSERT(p_node);
	xml::util::checkName(p_node, "script_trigger", errStatus);
	
	rect           = xml::util::parsePointRect(p_node,                &errStatus);
	// In shoebox space the position of a rect is its center.
	rect.setCenterPosition(rect.getPosition());
	triggerName    = xml::util::parseStr        (p_node, "trigger_name",    &errStatus);
	scriptFilename = xml::util::parseStr        (p_node, "script_filename", &errStatus);
	tagName        = xml::util::parseOptionalStr(p_node, "tag_name",        &errStatus);
}


void ScriptTriggerData::saveTrigger(xml::XmlNode* p_node, code::ErrorStatus* p_errStatus) const
{
	TT_ERR_CHAIN_VOID("Storing Script Trigger Data");
	
	TT_ERR_NULL_ASSERT(p_node);
	if (p_node != 0)
	{
		p_node->setName("script_trigger");
	}
	
	// In shoebox space the position of a rect is its center.
	math::PointRect r(rect);
	r.setPosition(r.getCenterPosition());
	xml::util::store(p_node, r, &errStatus);
	
	xml::util::store(p_node, "trigger_name",    triggerName,    &errStatus);
	xml::util::store(p_node, "script_filename", scriptFilename, &errStatus);
	xml::util::store(p_node, "tag_name",        tagName,        &errStatus);
}


void ScriptTriggerData::loadTrigger(const u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN_VOID("Loading Script Trigger Data From Memory");
	TT_ERR_ASSERTMSG(p_sizeOUT >= s_size, "Buffer too small");
	TT_ERR_RETURN_ON_ERROR();
	
	namespace bu = code::bufferutils;
	const s32 left   = bu::be_get<s32>(p_bufferOUT, p_sizeOUT);
	const s32 top    = bu::be_get<s32>(p_bufferOUT, p_sizeOUT);
	const s32 width  = bu::be_get<s32>(p_bufferOUT, p_sizeOUT);
	const s32 height = bu::be_get<s32>(p_bufferOUT, p_sizeOUT);
	
	rect.setWidth(width);
	rect.setHeight(height);
	rect.setPosition(math::Point2(left, top));
	
	TT_ERR_ASSERTMSG(p_sizeOUT >= 2, "Buffer too small");
	triggerName    = bu::be_get<std::string>(p_bufferOUT, p_sizeOUT);
	TT_ERR_ASSERTMSG(p_sizeOUT >= 2, "Buffer too small");
	scriptFilename = bu::be_get<std::string>(p_bufferOUT, p_sizeOUT);
	TT_ERR_ASSERTMSG(p_sizeOUT >= 2, "Buffer too small");
	tagName        = bu::be_get<std::string>(p_bufferOUT, p_sizeOUT);
}


void ScriptTriggerData::saveTrigger(u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus) const
{
	TT_ERR_CHAIN_VOID("Saving Script Trigger Data To Memory");
	TT_ERR_ASSERTMSG(p_sizeOUT >= s_size, "Buffer too small");
	TT_ERR_RETURN_ON_ERROR();
	
	namespace bu = code::bufferutils;
	bu::be_put(rect.getLeft(),   p_bufferOUT, p_sizeOUT);
	bu::be_put(rect.getTop(),    p_bufferOUT, p_sizeOUT);
	bu::be_put(rect.getWidth(),  p_bufferOUT, p_sizeOUT);
	bu::be_put(rect.getHeight(), p_bufferOUT, p_sizeOUT);
	
	TT_ERR_ASSERTMSG(p_sizeOUT >= (2 + triggerName.length()), "Buffer too small");
	bu::be_put(triggerName, p_bufferOUT, p_sizeOUT);
	TT_ERR_ASSERTMSG(p_sizeOUT >= (2 + scriptFilename.length()), "Buffer too small");
	bu::be_put(scriptFilename, p_bufferOUT, p_sizeOUT);
	TT_ERR_ASSERTMSG(p_sizeOUT >= (2 + tagName.get().length()), "Buffer too small");
	bu::be_put(tagName.get(), p_bufferOUT, p_sizeOUT);
}


//--------------------------------------------------------------------------------------------------
// PopupData

PopupData PopupData::parse(const xml::XmlNode* p_node, code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN(PopupData, PopupData(), "Parsing Popup Data from XmlNode");
	PopupData data;
	data.loadPopup(p_node, &errStatus);
	return data;
}


PopupData PopupData::parse(const u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN(PopupData, PopupData(), "Parsing Popup Data from a buffer");
	PopupData data;
	data.loadPopup(p_bufferOUT, p_sizeOUT, &errStatus);
	return data;
}


void PopupData::print() const
{
	TT_Printf("PopupData::print - rect.left: %d, rect.top: %d, rect.width: %d, rect.height: %d, "
	          "popupName(%s): '%s', scriptFilename(%s): '%s', locID: '%s'\n",
	          rect.getLeft(), rect.getTop(), rect.getWidth(), rect.getHeight(),
	          popupName.isValid() ? "entered" : "default", popupName.get().c_str(),
	          scriptFilename.isValid() ? "entered" : "default", scriptFilename.get().c_str(),
	          locID.c_str());
}


void PopupData::loadPopup(const xml::XmlNode* p_node, code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN_VOID("Parsing Popup Data");
	
	TT_ERR_NULL_ASSERT(p_node);
	xml::util::checkName(p_node, "popup", errStatus);
	
	rect           = xml::util::parsePointRect  (p_node,                    &errStatus);
	// In shoebox space the position of a rect is its center.
	rect.setCenterPosition(rect.getPosition());
	popupName      = xml::util::parseOptionalStr(p_node, "popup_name",      &errStatus);
	scriptFilename = xml::util::parseOptionalStr(p_node, "script_filename", &errStatus);
	locID          = xml::util::parseStr        (p_node, "loc_id",          &errStatus);
	
	for (const xml::XmlNode* child = p_node->getChild(); child != 0; child = child->getSibling())
	{
		if (child->getName() == "tag")
		{
			if (child->getAttribute("name").empty())
			{
				TT_ERR_AND_RETURN("Expected 'name' attribute in 'tag'");
			}
			tags.push_back(child->getAttribute("name"));
		}
	}
}


void PopupData::savePopup(xml::XmlNode* p_node, code::ErrorStatus* p_errStatus) const
{
	TT_ERR_CHAIN_VOID("Storing Popup Data");
	
	TT_ERR_NULL_ASSERT(p_node);
	if (p_node != 0)
	{
		p_node->setName("popup");
	}
	
	// In shoebox space the position of a rect is it's center.
	math::PointRect r(rect);
	r.setPosition(r.getCenterPosition());
	xml::util::store(p_node, r, &errStatus);
	
	xml::util::store(p_node, "popup_name",      popupName,      &errStatus);
	xml::util::store(p_node, "script_filename", scriptFilename, &errStatus);
	xml::util::store(p_node, "loc_id",          locID,          &errStatus);
	
	for (Tags::const_iterator it = tags.begin(); it != tags.end(); ++it)
	{
		xml::XmlNode* node = new xml::XmlNode("tag");
		xml::util::store(node, "name", *it, &errStatus);
		p_node->addChild(node);
	}
}


void PopupData::loadPopup(const u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN_VOID("Loading Popup Data From Memory");
	TT_ERR_ASSERTMSG(p_sizeOUT >= s_size, "Buffer too small");
	TT_ERR_RETURN_ON_ERROR();
	
	
	namespace bu = code::bufferutils;
	const s32 left   = bu::be_get<s32>(p_bufferOUT, p_sizeOUT);
	const s32 top    = bu::be_get<s32>(p_bufferOUT, p_sizeOUT);
	const s32 width  = bu::be_get<s32>(p_bufferOUT, p_sizeOUT);
	const s32 height = bu::be_get<s32>(p_bufferOUT, p_sizeOUT);
	
	rect.setWidth(width);
	rect.setHeight(height);
	rect.setPosition(math::Point2(left, top));
	
	TT_ERR_ASSERTMSG(p_sizeOUT >= 2, "Buffer too small");
	popupName      = bu::be_get<std::string>(p_bufferOUT, p_sizeOUT);
	TT_ERR_ASSERTMSG(p_sizeOUT >= 2, "Buffer too small");
	scriptFilename = bu::be_get<std::string>(p_bufferOUT, p_sizeOUT);
	TT_ERR_ASSERTMSG(p_sizeOUT >= 2, "Buffer too small");
	locID          = bu::be_get<std::string>(p_bufferOUT, p_sizeOUT);
	
	TT_ERR_ASSERTMSG(p_sizeOUT >= 2, "Buffer too small");
	u16 tagCount = bu::be_get<u16>(p_bufferOUT, p_sizeOUT);
	tags.resize(static_cast<Tags::size_type>(tagCount));
	for (Tags::iterator it = tags.begin(); it != tags.end(); ++it)
	{
		*it = bu::be_get<std::string>(p_bufferOUT, p_sizeOUT);
	}
}


void PopupData::savePopup(u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus) const
{
	TT_ERR_CHAIN_VOID("Saving Popup Data To Memory");
	TT_ERR_ASSERTMSG(p_sizeOUT >= s_size, "Buffer too small");
	TT_ERR_RETURN_ON_ERROR();
	
	namespace bu = code::bufferutils;
	bu::be_put(rect.getLeft(),   p_bufferOUT, p_sizeOUT);
	bu::be_put(rect.getTop(),    p_bufferOUT, p_sizeOUT);
	bu::be_put(rect.getWidth(),  p_bufferOUT, p_sizeOUT);
	bu::be_put(rect.getHeight(), p_bufferOUT, p_sizeOUT);
	
	TT_ERR_ASSERTMSG(p_sizeOUT >= (2 + popupName.get().length()), "Buffer too small");
	bu::be_put(popupName.get(), p_bufferOUT, p_sizeOUT);
	TT_ERR_ASSERTMSG(p_sizeOUT >= (2 + scriptFilename.get().length()), "Buffer too small");
	bu::be_put(scriptFilename.get(), p_bufferOUT, p_sizeOUT);
	TT_ERR_ASSERTMSG(p_sizeOUT >= (2 + locID.length()), "Buffer too small");
	bu::be_put(locID, p_bufferOUT, p_sizeOUT);
	
	TT_ERR_ASSERTMSG(p_sizeOUT >= 2, "Buffer too small");
	bu::be_put(static_cast<u16>(tags.size()), p_bufferOUT, p_sizeOUT);
	for (Tags::const_iterator it = tags.begin(); it != tags.end(); ++it)
	{
		bu::be_put(*it, p_bufferOUT, p_sizeOUT);
	}
}


//--------------------------------------------------------------------------------------------------
// PlaneData

PlaneData PlaneData::parse(const xml::XmlNode* p_node, code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN(PlaneData, PlaneData(), "Parsing Plane Data from XmlNode");
	PlaneData ret;
	ret.loadPlane(p_node, &errStatus);
	return ret;
}


PlaneData PlaneData::parse(const u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN(PlaneData, PlaneData(), "Parsing Plane Data from a buffer");
	PlaneData ret;
	ret.loadPlane(p_bufferOUT, p_sizeOUT, &errStatus);
	return ret;
}


void PlaneData::loadPlane(const xml::XmlNode* p_node, code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN_VOID("Parsing Plane Data");
	loadRect(p_node, &errStatus);
	
	using namespace xml::util;
	
	TT_ERR_NULL_ASSERT(p_node);
	checkName(p_node, "plane", errStatus);
	
	id.clear();
	if (p_node->hasAttribute("id"))
	{
		id = p_node->getAttribute("id");
	}
	
	textureFilename = xml::util::parseStr(p_node, "texture", &errStatus);
	
	if (p_node->getFirstChild("animations") != 0)
	{
		animations.reset(new anim2d::AnimationStack2D);
		
		// NOTE: only for ingame loading, we need to invert the Y axis on translations
		// this is not needed for the converter
		if (animations->load(p_node->getFirstChild("animations"), true) == false)
		{
			animations.reset();
		}
	}
	
	if (p_node->getFirstChild("textureanimations") != 0)
	{
		textureanimations.reset(new anim2d::AnimationStack2D);
		if (textureanimations->load(p_node->getFirstChild("textureanimations"), false) == false)
		{
			textureanimations.reset();
		}
	}
	
	if (p_node->getFirstChild("coloranimations") != 0)
	{
		coloranimations.reset(new anim2d::ColorAnimationStack2D);
		if (coloranimations->load(p_node->getFirstChild("coloranimations")) == false)
		{
			coloranimations.reset();
		}
	}
	
	for (const xml::XmlNode* child = p_node->getChild(); child != 0; child = child->getSibling())
	{
		if (child->getName() == "tag")
		{
			if (child->getAttribute("name").empty())
			{
				TT_ERR_AND_RETURN("Expected 'name' attribute in 'tag'");
			}
			tags.push_back(child->getAttribute("name"));
		}
	}
	
	using renderer::Renderer;
	{
		code::OptionalValue<std::string> blendModeStr(parseOptionalStr(p_node, "blend_mode", &errStatus));
		if (blendModeStr.isValid())
		{
			blendMode = renderer::getBlendModeFromName(blendModeStr.get());
			
			if (renderer::isValidBlendMode(blendMode) == false)
			{
				TT_ERR_AND_RETURN("Found unknown value '" << blendModeStr.get() <<
				                  "' in attribute 'blend_mode' in node '" << p_node->getName() << "'.");
			}
		}
		
		minFilter = readFilterMode(p_node, "filter_minify",  &errStatus);
		magFilter = readFilterMode(p_node, "filter_magnify", &errStatus);
		mipFilter = readFilterMode(p_node, "filter_mipmap",  &errStatus);
	}
	
	screenSpace           = parseOptionalBool(p_node, "screenspace",               &errStatus);
	hidden                = parseOptionalBool(p_node, "hidden",                    &errStatus);
	ignoreFog             = parseOptionalBool(p_node, "ignore_fog",                &errStatus);
	texTopLeftU           = parseOptionalReal(p_node, "texture_top_left_u",        &errStatus);
	texTopLeftV           = parseOptionalReal(p_node, "texture_top_left_v",        &errStatus);
	texTopRightU          = parseOptionalReal(p_node, "texture_top_right_u",       &errStatus);
	texTopRightV          = parseOptionalReal(p_node, "texture_top_right_v",       &errStatus);
	texBottomRightU       = parseOptionalReal(p_node, "texture_bottom_right_u",    &errStatus);
	texBottomRightV       = parseOptionalReal(p_node, "texture_bottom_right_v",    &errStatus);
	texBottomLeftU        = parseOptionalReal(p_node, "texture_bottom_left_u",     &errStatus);
	texBottomLeftV        = parseOptionalReal(p_node, "texture_bottom_left_v",     &errStatus);
	texAnimU              = parseOptionalReal(p_node, "texture_anim_u",            &errStatus);
	texAnimV              = parseOptionalReal(p_node, "texture_anim_v",            &errStatus);
	texOffsetScale        = parseOptionalReal(p_node, "camera_uv_offset_scale",    &errStatus);
	offsetAnimXMove       = parseOptionalReal(p_node, "offset_anim_x_move",        &errStatus);
	offsetAnimXDuration   = parseOptionalReal(p_node, "offset_anim_x_duration",    &errStatus);
	offsetAnimXTimeOffset = parseOptionalReal(p_node, "offset_anim_x_time_offset", &errStatus);
	offsetAnimYMove       = parseOptionalReal(p_node, "offset_anim_y_move",        &errStatus);
	offsetAnimYDuration   = parseOptionalReal(p_node, "offset_anim_y_duration",    &errStatus);
	offsetAnimYTimeOffset = parseOptionalReal(p_node, "offset_anim_y_time_offset", &errStatus);
	cameraSpaceUScale     = parseOptionalReal(p_node, "camera_space_u_scale",      &errStatus);
	cameraSpaceVScale     = parseOptionalReal(p_node, "camera_space_v_scale",      &errStatus);
	
	if (offsetAnimXMove.isValid())
	{
		TT_ERR_ASSERTMSG(offsetAnimXDuration.isValid(), "offset_anim_x_move is set with missing duration node.");
		TT_ERR_ASSERTMSG(offsetAnimXDuration.get() != 0, "offset_anim_x_move is set with duration 0.");
	}
	
	if (offsetAnimYMove.isValid())
	{
		TT_ERR_ASSERTMSG(offsetAnimYDuration.isValid(), "offset_anim_y_move is set with missing duration node.");
		TT_ERR_ASSERTMSG(offsetAnimYDuration.get() != 0, "offset_anim_y_move is set with duration 0.");
	}
	
	colorWholeQuad   = parseOptionalColorRGBA(p_node, "color_whole_quad",   &errStatus);
	colorTopLeft     = parseOptionalColorRGBA(p_node, "color_top_left",     &errStatus);
	colorTopRight    = parseOptionalColorRGBA(p_node, "color_top_right",    &errStatus);
	colorBottomLeft  = parseOptionalColorRGBA(p_node, "color_bottom_left",  &errStatus);
	colorBottomRight = parseOptionalColorRGBA(p_node, "color_bottom_right", &errStatus);
	
	priority = parseOptionalS32(p_node, "priority", &errStatus);
}


void PlaneData::savePlane(xml::XmlNode* p_node, code::ErrorStatus* p_errStatus) const
{
	TT_ERR_CHAIN_VOID("Storing Plane Data");
	saveRect(p_node, &errStatus);
	
	TT_ERR_NULL_ASSERT(p_node);
	if (p_node != 0)
	{
		p_node->setName("plane");
	}
	
	xml::util::store(p_node, "id",      id,              &errStatus);
	xml::util::store(p_node, "texture", textureFilename, &errStatus);
	
	if (animations != 0)
	{
		xml::XmlNode* node = new xml::XmlNode("animations");
		if (animations->save(node) == false)
		{
			delete node;
			TT_ERR_AND_RETURN("Error saving animations");
		}
		p_node->addChild(node);
	}
	
	if (textureanimations != 0)
	{
		xml::XmlNode* node = new xml::XmlNode("textureanimations");
		if (textureanimations->save(node) == false)
		{
			delete node;
			TT_ERR_AND_RETURN("Error saving textureanimations");
		}
		p_node->addChild(node);
	}
	
	if (coloranimations != 0)
	{
		xml::XmlNode* node = new xml::XmlNode("coloranimations");
		if (coloranimations->save(node) == false)
		{
			delete node;
			TT_ERR_AND_RETURN("Error saving coloranimations");
		}
		p_node->addChild(node);
	}
	
	for (Tags::const_iterator it = tags.begin(); it != tags.end(); ++it)
	{
		xml::XmlNode* node = new xml::XmlNode("tag");
		xml::util::store(node, "name", (*it), &errStatus);
	}
	
	if (renderer::isValidBlendMode(blendMode))
	{
		xml::util::store(p_node, "blend_mode", renderer::getBlendModeName(blendMode), &errStatus);
	}
	
	
	xml::util::store(p_node, "screenspace",               screenSpace,           &errStatus);
	xml::util::store(p_node, "hidden",                    hidden,                &errStatus);
	xml::util::store(p_node, "ignore_fog",                ignoreFog,             &errStatus);
	xml::util::store(p_node, "texture_top_left_u",        texTopLeftU,           &errStatus);
	xml::util::store(p_node, "texture_top_left_v",        texTopLeftV,           &errStatus);
	xml::util::store(p_node, "texture_top_right_u",       texTopRightU,          &errStatus);
	xml::util::store(p_node, "texture_top_right_v",       texTopRightV,          &errStatus);
	xml::util::store(p_node, "texture_bottom_right_u",    texBottomRightU,       &errStatus);
	xml::util::store(p_node, "texture_bottom_right_v",    texBottomRightV,       &errStatus);
	xml::util::store(p_node, "texture_bottom_left_u",     texBottomLeftU,        &errStatus);
	xml::util::store(p_node, "texture_bottom_left_v",     texBottomLeftV,        &errStatus);
	xml::util::store(p_node, "texture_anim_u",            texAnimU,              &errStatus);
	xml::util::store(p_node, "texture_anim_v",            texAnimV,              &errStatus);
	xml::util::store(p_node, "camera_uv_offset_scale",    texOffsetScale,        &errStatus);
	xml::util::store(p_node, "offset_anim_x_move",        offsetAnimXMove,       &errStatus);
	xml::util::store(p_node, "offset_anim_x_duration",    offsetAnimXDuration,   &errStatus);
	xml::util::store(p_node, "offset_anim_x_time_offset", offsetAnimXTimeOffset, &errStatus);
	xml::util::store(p_node, "offset_anim_y_move",        offsetAnimYMove,       &errStatus);
	xml::util::store(p_node, "offset_anim_y_duration",    offsetAnimYDuration,   &errStatus);
	xml::util::store(p_node, "offset_anim_y_time_offset", offsetAnimYTimeOffset, &errStatus);
	xml::util::store(p_node, "camera_space_u_scale",      cameraSpaceUScale,     &errStatus);
	xml::util::store(p_node, "camera_space_v_scale",      cameraSpaceVScale,     &errStatus);
	
	xml::util::store(p_node, "color_whole_quad",   colorWholeQuad,   &errStatus);
	xml::util::store(p_node, "color_top_left",     colorTopLeft,     &errStatus);
	xml::util::store(p_node, "color_top_right",    colorTopRight,    &errStatus);
	xml::util::store(p_node, "color_bottom_left",  colorBottomLeft,  &errStatus);
	xml::util::store(p_node, "color_bottom_right", colorBottomRight, &errStatus);
	
	xml::util::store(p_node, "priority", priority, &errStatus);
}


void PlaneData::loadPlane(const u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN_VOID("Loading Plane Data From Memory");
	loadRect(p_bufferOUT, p_sizeOUT, &errStatus);
	
	namespace bu = code::bufferutils;
	
	TT_ERR_ASSERTMSG(p_sizeOUT >= 2, "Buffer too small");
	id              = bu::be_get<std::string>(p_bufferOUT, p_sizeOUT);
	TT_ERR_ASSERTMSG(p_sizeOUT >= 2, "Buffer too small");
	textureFilename = bu::be_get<std::string>(p_bufferOUT, p_sizeOUT);
	
	TT_ERR_ASSERTMSG(p_sizeOUT >= s_size, "Buffer too small");
	
	blendMode   = static_cast<renderer::BlendMode >(bu::be_get<u8>(p_bufferOUT, p_sizeOUT));
	minFilter   = static_cast<renderer::FilterMode>(bu::be_get<u8>(p_bufferOUT, p_sizeOUT));
	magFilter   = static_cast<renderer::FilterMode>(bu::be_get<u8>(p_bufferOUT, p_sizeOUT));
	mipFilter   = static_cast<renderer::FilterMode>(bu::be_get<u8>(p_bufferOUT, p_sizeOUT));
	screenSpace = bu::be_get<bool>(p_bufferOUT, p_sizeOUT);
	hidden      = bu::be_get<bool>(p_bufferOUT, p_sizeOUT);
	ignoreFog   = bu::be_get<bool>(p_bufferOUT, p_sizeOUT);
	
	texTopLeftU     = bu::be_get<real>(p_bufferOUT, p_sizeOUT);
	texTopLeftV     = bu::be_get<real>(p_bufferOUT, p_sizeOUT);
	texTopRightU    = bu::be_get<real>(p_bufferOUT, p_sizeOUT);
	texTopRightV    = bu::be_get<real>(p_bufferOUT, p_sizeOUT);
	texBottomLeftU  = bu::be_get<real>(p_bufferOUT, p_sizeOUT);
	texBottomLeftV  = bu::be_get<real>(p_bufferOUT, p_sizeOUT);
	texBottomRightU = bu::be_get<real>(p_bufferOUT, p_sizeOUT);
	texBottomRightV = bu::be_get<real>(p_bufferOUT, p_sizeOUT);
	
	texAnimU              = bu::be_get<real>(p_bufferOUT, p_sizeOUT);
	texAnimV              = bu::be_get<real>(p_bufferOUT, p_sizeOUT);
	texOffsetScale        = bu::be_get<real>(p_bufferOUT, p_sizeOUT);
	offsetAnimXMove       = bu::be_get<real>(p_bufferOUT, p_sizeOUT);
	offsetAnimXDuration   = bu::be_get<real>(p_bufferOUT, p_sizeOUT);
	offsetAnimXTimeOffset = bu::be_get<real>(p_bufferOUT, p_sizeOUT);
	offsetAnimYMove       = bu::be_get<real>(p_bufferOUT, p_sizeOUT);
	offsetAnimYDuration   = bu::be_get<real>(p_bufferOUT, p_sizeOUT);
	offsetAnimYTimeOffset = bu::be_get<real>(p_bufferOUT, p_sizeOUT);
	cameraSpaceUScale     = bu::be_get<real>(p_bufferOUT, p_sizeOUT);
	cameraSpaceVScale     = bu::be_get<real>(p_bufferOUT, p_sizeOUT);
	
	if (offsetAnimXMove != 0)
	{
		TT_ASSERTMSG(offsetAnimXDuration != 0, "offsetAnimXDuration is set with duration 0.");
	}
	
	if (offsetAnimYMove != 0)
	{
		TT_ASSERTMSG(offsetAnimYDuration != 0, "offsetAnimYMove is set with duration 0.");
	}
	
	renderer::ColorRGBA color;
	color.r = bu::be_get<u8>(p_bufferOUT, p_sizeOUT);
	color.g = bu::be_get<u8>(p_bufferOUT, p_sizeOUT);
	color.b = bu::be_get<u8>(p_bufferOUT, p_sizeOUT);
	color.a = bu::be_get<u8>(p_bufferOUT, p_sizeOUT);
	colorTopLeft = color;
	
	color.r = bu::be_get<u8>(p_bufferOUT, p_sizeOUT);
	color.g = bu::be_get<u8>(p_bufferOUT, p_sizeOUT);
	color.b = bu::be_get<u8>(p_bufferOUT, p_sizeOUT);
	color.a = bu::be_get<u8>(p_bufferOUT, p_sizeOUT);
	colorTopRight = color;
	
	color.r = bu::be_get<u8>(p_bufferOUT, p_sizeOUT);
	color.g = bu::be_get<u8>(p_bufferOUT, p_sizeOUT);
	color.b = bu::be_get<u8>(p_bufferOUT, p_sizeOUT);
	color.a = bu::be_get<u8>(p_bufferOUT, p_sizeOUT);
	colorBottomLeft = color;
	
	color.r = bu::be_get<u8>(p_bufferOUT, p_sizeOUT);
	color.g = bu::be_get<u8>(p_bufferOUT, p_sizeOUT);
	color.b = bu::be_get<u8>(p_bufferOUT, p_sizeOUT);
	color.a = bu::be_get<u8>(p_bufferOUT, p_sizeOUT);
	colorBottomRight = color;
	
	priority = bu::be_get<s32>(p_bufferOUT, p_sizeOUT);
	
	TT_ERR_ASSERTMSG(p_sizeOUT >= 2, "Buffer too small");
	const u16 tagCount = bu::be_get<u16>(p_bufferOUT, p_sizeOUT);
	tags.resize(static_cast<Tags::size_type>(tagCount));
	for (Tags::iterator it = tags.begin(); it != tags.end(); ++it)
	{
		*it = bu::be_get<std::string>(p_bufferOUT, p_sizeOUT);
	}
	
	// NOTE: only for ingame loading, we need to inverse the Y axis on translations
	// this is not needed for the converter
	const bool hasAnimation = bu::be_get<bool>(p_bufferOUT, p_sizeOUT);
	if (hasAnimation)
	{
		animations.reset(new anim2d::AnimationStack2D);
		if (animations->load(p_bufferOUT, p_sizeOUT, true) == false)
		{
			TT_ERR_AND_RETURN("Animation loading failed");
		}
	}
	
	const bool hasTextureAnimation = bu::be_get<bool>(p_bufferOUT, p_sizeOUT);
	if (hasTextureAnimation)
	{
		textureanimations.reset(new anim2d::AnimationStack2D);
		if (textureanimations->load(p_bufferOUT, p_sizeOUT, false) == false)
		{
			TT_ERR_AND_RETURN("Texture Animation loading failed");
		}
	}
	
	const bool hasColorAnimation = bu::be_get<bool>(p_bufferOUT, p_sizeOUT);
	if (hasColorAnimation)
	{
		coloranimations.reset(new anim2d::ColorAnimationStack2D);
		if (coloranimations->load(p_bufferOUT, p_sizeOUT) == false)
		{
			TT_ERR_AND_RETURN("Color Animation loading failed");
		}
	}
}


void PlaneData::savePlane(u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus) const
{
	TT_ERR_CHAIN_VOID("Saving Plane Data To Memory");
	saveRect(p_bufferOUT, p_sizeOUT, &errStatus);
	
	namespace bu = code::bufferutils;
	
	TT_ERR_ASSERTMSG(p_sizeOUT >= (2 + id.length()), "Buffer too small");
	bu::be_put(id,              p_bufferOUT, p_sizeOUT);
	
	TT_ERR_ASSERTMSG(p_sizeOUT >= (2 + textureFilename.length()), "Buffer too small");
	bu::be_put(textureFilename, p_bufferOUT, p_sizeOUT);
	
	TT_ERR_ASSERTMSG(p_sizeOUT >= s_size, "Buffer too small");
	
	bu::be_put(static_cast<u8>(blendMode), p_bufferOUT, p_sizeOUT);
	bu::be_put(static_cast<u8>(minFilter), p_bufferOUT, p_sizeOUT);
	bu::be_put(static_cast<u8>(magFilter), p_bufferOUT, p_sizeOUT);
	bu::be_put(static_cast<u8>(mipFilter), p_bufferOUT, p_sizeOUT);
	bu::be_put(screenSpace.get(),          p_bufferOUT, p_sizeOUT);
	bu::be_put(hidden.get(),               p_bufferOUT, p_sizeOUT);
	bu::be_put(ignoreFog.get(),            p_bufferOUT, p_sizeOUT);
	
	bu::be_put(texTopLeftU.get(),     p_bufferOUT, p_sizeOUT);
	bu::be_put(texTopLeftV.get(),     p_bufferOUT, p_sizeOUT);
	bu::be_put(texTopRightU.get(),    p_bufferOUT, p_sizeOUT);
	bu::be_put(texTopRightV.get(),    p_bufferOUT, p_sizeOUT);
	bu::be_put(texBottomLeftU.get(),  p_bufferOUT, p_sizeOUT);
	bu::be_put(texBottomLeftV.get(),  p_bufferOUT, p_sizeOUT);
	bu::be_put(texBottomRightU.get(), p_bufferOUT, p_sizeOUT);
	bu::be_put(texBottomRightV.get(), p_bufferOUT, p_sizeOUT);
	
	bu::be_put(texAnimU.get(),              p_bufferOUT, p_sizeOUT);
	bu::be_put(texAnimV.get(),              p_bufferOUT, p_sizeOUT);
	bu::be_put(texOffsetScale.get(),        p_bufferOUT, p_sizeOUT);
	bu::be_put(offsetAnimXMove.get(),       p_bufferOUT, p_sizeOUT);
	bu::be_put(offsetAnimXDuration.get(),   p_bufferOUT, p_sizeOUT);
	bu::be_put(offsetAnimXTimeOffset.get(), p_bufferOUT, p_sizeOUT);
	bu::be_put(offsetAnimYMove.get(),       p_bufferOUT, p_sizeOUT);
	bu::be_put(offsetAnimYDuration.get(),   p_bufferOUT, p_sizeOUT);
	bu::be_put(offsetAnimYTimeOffset.get(), p_bufferOUT, p_sizeOUT);
	bu::be_put(cameraSpaceUScale.get(),     p_bufferOUT, p_sizeOUT);
	bu::be_put(cameraSpaceVScale.get(),     p_bufferOUT, p_sizeOUT);
	
	engine::renderer::ColorRGBA color = colorWholeQuad.isValid() ? colorWholeQuad : colorTopLeft;
	bu::be_put(color.r, p_bufferOUT, p_sizeOUT);
	bu::be_put(color.g, p_bufferOUT, p_sizeOUT);
	bu::be_put(color.b, p_bufferOUT, p_sizeOUT);
	bu::be_put(color.a, p_bufferOUT, p_sizeOUT);
	
	color = colorWholeQuad.isValid() ? colorWholeQuad : colorTopRight;
	bu::be_put(color.r, p_bufferOUT, p_sizeOUT);
	bu::be_put(color.g, p_bufferOUT, p_sizeOUT);
	bu::be_put(color.b, p_bufferOUT, p_sizeOUT);
	bu::be_put(color.a, p_bufferOUT, p_sizeOUT);
	
	color = colorWholeQuad.isValid() ? colorWholeQuad : colorBottomLeft;
	bu::be_put(color.r, p_bufferOUT, p_sizeOUT);
	bu::be_put(color.g, p_bufferOUT, p_sizeOUT);
	bu::be_put(color.b, p_bufferOUT, p_sizeOUT);
	bu::be_put(color.a, p_bufferOUT, p_sizeOUT);
	
	color = colorWholeQuad.isValid() ? colorWholeQuad : colorBottomRight;
	bu::be_put(color.r, p_bufferOUT, p_sizeOUT);
	bu::be_put(color.g, p_bufferOUT, p_sizeOUT);
	bu::be_put(color.b, p_bufferOUT, p_sizeOUT);
	bu::be_put(color.a, p_bufferOUT, p_sizeOUT);
	
	bu::be_put(priority.get(), p_bufferOUT, p_sizeOUT);
	
	TT_ERR_ASSERTMSG(p_sizeOUT >= 2, "Buffer too small");
	bu::be_put(static_cast<u16>(tags.size()), p_bufferOUT, p_sizeOUT);
	for (Tags::const_iterator it = tags.begin(); it != tags.end(); ++it)
	{
		bu::be_put(*it, p_bufferOUT, p_sizeOUT);
	}
	
	const bool haveAnimations = (animations != 0);
	bu::be_put(haveAnimations, p_bufferOUT, p_sizeOUT);
	if (haveAnimations)
	{
		if (animations->save(p_bufferOUT, p_sizeOUT, &errStatus) == false)
		{
			TT_ERR_AND_RETURN("Animation saving failed");
		}
	}
	
	const bool haveTextureAnimations = (textureanimations != 0);
	bu::be_put(haveTextureAnimations, p_bufferOUT, p_sizeOUT);
	if (haveTextureAnimations)
	{
		if (textureanimations->save(p_bufferOUT, p_sizeOUT, &errStatus) == false)
		{
			TT_ERR_AND_RETURN("Texture Animation saving failed");
		}
	}
	
	const bool haveColorAnimations = (coloranimations != 0);
	bu::be_put(haveColorAnimations, p_bufferOUT, p_sizeOUT);
	if (haveColorAnimations)
	{
		if (coloranimations->save(p_bufferOUT, p_sizeOUT, &errStatus) == false)
		{
			TT_ERR_AND_RETURN("Color Animation saving failed");
		}
	}
}


//--------------------------------------------------------------------------------------------------
// PresentationData

PresentationData PresentationData::parse(const xml::XmlNode* p_node, code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN(PresentationData, PresentationData(), "Parsing presentation data from XML node");
	PresentationData data;
	data.load(p_node, &errStatus);
	return data;
}


PresentationData PresentationData::parse(const u8*& p_bufferOUT, size_t& p_sizeOUT,
                                         code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN(PresentationData, PresentationData(), "Parsing presentation data from binary");
	PresentationData data;
	data.load(p_bufferOUT, p_sizeOUT, &errStatus);
	return data;
}


void PresentationData::load(const xml::XmlNode* p_node, code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN_VOID("Parsing presentation object data");
	
	TT_ERR_NULL_ASSERT(p_node);
	xml::util::checkName(p_node, "object", errStatus);
	
	presentationFileName = xml::util::parseStr    (p_node, "filename", &errStatus);
	delay                = xml::util::parseReal   (p_node, "delay",    &errStatus);
	position             = xml::util::parseVector3(p_node,             &errStatus);
}


void PresentationData::load(const u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN_VOID("Parsing presentation object data");
	TT_ERR_ASSERTMSG(p_sizeOUT >= s_size, "Buffer too small");
	TT_ERR_RETURN_ON_ERROR();
	
	namespace bu = code::bufferutils;
	presentationFileName = bu::be_get<std::string  >(p_bufferOUT, p_sizeOUT);
	delay                = bu::be_get<real         >(p_bufferOUT, p_sizeOUT);
	position             = bu::be_get<math::Vector3>(p_bufferOUT, p_sizeOUT);
}


void PresentationData::save(xml::XmlNode* p_node, code::ErrorStatus* p_errStatus) const
{
	TT_ERR_CHAIN_VOID("Storing presentation data");
	
	if (p_node != 0)
	{
		p_node->setName("object");
	}
	
	xml::util::store(p_node, "filename", presentationFileName, &errStatus);
	xml::util::store(p_node, "delay",    delay,                &errStatus);
	xml::util::store(p_node,             position,             &errStatus);
}


void PresentationData::save(u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus) const
{
	TT_ERR_CHAIN_VOID("Storing presentation data");
	TT_ERR_ASSERTMSG(p_sizeOUT >= s_size, "Buffer too small");
	TT_ERR_RETURN_ON_ERROR();
	
	namespace bu = code::bufferutils;
	
	bu::be_put(presentationFileName, p_bufferOUT, p_sizeOUT);
	bu::be_put(delay,                p_bufferOUT, p_sizeOUT);
	bu::be_put(position,             p_bufferOUT, p_sizeOUT);
	
	TT_ERR_RETURN_ON_ERROR();
}


//--------------------------------------------------------------------------------------------------
// IncludeData

IncludeData IncludeData::parse(const u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN(IncludeData, IncludeData(), "Parsing include data from binary");
	IncludeData data;
	data.load(p_bufferOUT, p_sizeOUT, &errStatus);
	return data;
}


void IncludeData::load(const u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN_VOID("Parsing include data");
	//TT_ERR_ASSERTMSG(p_sizeOUT >= s_size, "Buffer too small");
	//TT_ERR_RETURN_ON_ERROR();
	
	namespace bu = code::bufferutils;
	
	filename = bu::be_get<std::string  >(p_bufferOUT, p_sizeOUT);
	offset   = bu::be_get<math::Vector3>(p_bufferOUT, p_sizeOUT);
	priority = bu::be_get<s32          >(p_bufferOUT, p_sizeOUT);
	scale    = bu::be_get<real         >(p_bufferOUT, p_sizeOUT);
}


void IncludeData::save(xml::XmlNode* p_node, code::ErrorStatus* p_errStatus) const
{
	TT_ERR_CHAIN_VOID("Storing include data");
	TT_ERR_NULL_ASSERT(p_node);
	
	p_node->setName("shoebox");
	
	xml::util::store(p_node, "filename", filename, &errStatus);
	xml::util::store(p_node, "x_offset", offset.x, &errStatus);
	xml::util::store(p_node, "y_offset", offset.y, &errStatus);
	xml::util::store(p_node, "z_offset", offset.z, &errStatus);
	xml::util::store(p_node, "priority", priority, &errStatus);
	xml::util::store(p_node, "scale",    scale,    &errStatus);
}


//--------------------------------------------------------------------------------------------------
// ShoeboxData

ShoeboxDataPtr ShoeboxData::parse(const std::string& p_filename, code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN(ShoeboxDataPtr, ShoeboxDataPtr(), "Loading shoebox from file '" << p_filename << "'");
	TT_ERR_ASSERTMSG(fs::utils::getExtension(p_filename) == "shoebox",
	                 "Shoebox extension should be .shoebox. Only binary shoebox files are supported.");
	TT_ERR_ASSERTMSG(fs::fileExists(p_filename),
	                 "Shoebox file '" << p_filename << "' does not exist. Shoebox has not been loaded.\n"
	                 "If this file was included from another shoebox, that shoebox has not been loaded either.");
	
	code::BufferPtr content = fs::getFileContent(p_filename);
	TT_ERR_ASSERTMSG(content != 0, "Loading shoebox file contents failed.");
	
	const u8* data = reinterpret_cast<const u8*>(content->getData());
	size_t    size = static_cast<size_t>(content->getSize());
	return parse(data, size, &errStatus);
}


ShoeboxDataPtr ShoeboxData::parse(const u8*& p_buffer, size_t& p_size, code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN(ShoeboxDataPtr, ShoeboxDataPtr(), "Loading shoebox from binary file");
	
	namespace bu = code::bufferutils;
	
	const u16 version = bu::be_get<u16>(p_buffer, p_size);
	TT_ERR_ASSERTMSG(version == g_shoeboxDataVersion,
	                 "Shoebox version mismatch, got " << GET_MAJOR_VERSION(version) << "." << GET_MINOR_VERSION(version) <<
	                 " expected " << GET_MAJOR_VERSION(g_shoeboxDataVersion) << "." << GET_MINOR_VERSION(g_shoeboxDataVersion) <<
	                 ", please update your code/pack build and converters.");
	
	ShoeboxDataPtr returnData(new ShoeboxData);
	returnData->load(p_buffer, p_size, &errStatus);
	TT_ERR_RETURN_ON_ERROR();
	
	return returnData;
}


void ShoeboxData::saveAsXML(const std::string& p_filename, bool p_formatted, code::ErrorStatus* p_errStatus) const
{
	TT_ERR_CHAIN_VOID("Save shoebox as XML file '" << p_filename << "'");
	
	typedef tt_ptr<xml::XmlNode>::shared XmlNodePtr;
	XmlNodePtr root(new xml::XmlNode("shoebox"));
	
	// Set XML Schema reference
	root->setAttribute("xmlns",              "http://intranet.twotribes.com/schemas/common/shoebox.xsd");
	root->setAttribute("xmlns:xsi",          "http://www.w3.org/2001/XMLSchema-instance");
	root->setAttribute("xsi:schemaLocation", "http://intranet.twotribes.com/schemas/common/shoebox.xsd http://intranet.twotribes.com/schemas/common/shoebox.xsd");
	
	// Includes
	if (includes.empty() == false)
	{
		xml::XmlNode* includesNode = new xml::XmlNode("includes");
		
		for (Includes::const_iterator it = includes.begin(); it != includes.end(); ++it)
		{
			xml::XmlNode* includeNode = new xml::XmlNode("shoebox");
			(*it).save(includeNode, &errStatus);
			includesNode->addChild(includeNode);
			TT_ERR_RETURN_ON_ERROR();
		}
		
		root->addChild(includesNode);
	}
	
	// Planes
	for (Planes::const_iterator it = planes.begin(); it != planes.end(); ++it)
	{
		xml::XmlNode* planeNode = new xml::XmlNode("plane");
		(*it).savePlane(planeNode, &errStatus);
		root->addChild(planeNode);
		TT_ERR_RETURN_ON_ERROR();
	}
	
	// Particles
	for (Particles::const_iterator it = particles.begin(); it != particles.end(); ++it)
	{
		xml::XmlNode* particleNode = new xml::XmlNode("particle");
		(*it).saveParticle(particleNode, &errStatus);
		root->addChild(particleNode);
		TT_ERR_RETURN_ON_ERROR();
	}
	
	// Presentation Objects
	if (presentationObjects.empty() == false)
	{
		xml::XmlNode* presentationNode = new xml::XmlNode("presentation");
		
		for (PresentationObjects::const_iterator it = presentationObjects.begin(); it != presentationObjects.end(); ++it)
		{
			xml::XmlNode* objectNode = new xml::XmlNode("object");
			(*it).save(objectNode, &errStatus);
			presentationNode->addChild(objectNode);
			TT_ERR_RETURN_ON_ERROR();
		}
		
		root->addChild(presentationNode);
	}
	
	
	// Output everything to file
	xml::XmlFileWriter writer(root.get());
	if (writer.save(p_formatted == false, p_filename) == false)
	{
		TT_ERR_AND_RETURN("Writing shoebox XML tree to file failed.");
	}
}


void ShoeboxData::mergeWith(const ShoeboxData& p_other /*, p_positionOffset, p_scale, p_priorityOffset */)
{
	// Includes
	includes.reserve(includes.size() + p_other.includes.size());
	for (Includes::const_iterator it = p_other.includes.begin(); it != p_other.includes.end(); ++it)
	{
		IncludeData data(*it);
		// TODO: Apply position/scale/priority to copied data
		includes.push_back(data);
	}
	
	// Planes
	planes.reserve(planes.size() + p_other.planes.size());
	for (Planes::const_iterator it = p_other.planes.begin(); it != p_other.planes.end(); ++it)
	{
		PlaneData data(*it);
		// TODO: Apply position/scale/priority to copied data
		planes.push_back(data);
	}
	
	// Particles
	particles.reserve(particles.size() + p_other.particles.size());
	for (Particles::const_iterator it = p_other.particles.begin(); it != p_other.particles.end(); ++it)
	{
		ParticleData data(*it);
		// TODO: Apply position/scale/priority to copied data
		particles.push_back(data);
	}
	
	// Presentation objects
	presentationObjects.reserve(presentationObjects.size() + p_other.presentationObjects.size());
	for (PresentationObjects::const_iterator it = p_other.presentationObjects.begin();
	     it != p_other.presentationObjects.end(); ++it)
	{
		PresentationData data(*it);
		// TODO: Apply position/scale/priority to copied data
		presentationObjects.push_back(data);
	}
}


void ShoeboxData::load(const u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN_VOID("Loading shoebox binary data");
	
	namespace bu = code::bufferutils;
	
	// Include -> Other Shoebox. (optional)
	{
		const u16 includeCount = bu::be_get<u16>(p_bufferOUT, p_sizeOUT);
		includes.reserve(static_cast<Includes::size_type>(includeCount));
		for (u16 i = 0; i < includeCount; ++i)
		{
			IncludeData data(IncludeData::parse(p_bufferOUT, p_sizeOUT, &errStatus));
			TT_ERR_RETURN_ON_ERROR();
			
#if !defined(TT_BUILD_FINAL)
			for (Includes::iterator it = includes.begin(); it != includes.end(); ++it)
			{
				if (data == *it)
				{
					TT_PANIC("Found an identical (duplicate) include of shoebox '%s' "
					         "(offset (%f, %f, %f), priority %d, scale %f).",
					         data.filename.c_str(), data.offset.x, data.offset.y, data.offset.z,
					         data.priority, data.scale);
					break;
				}
			}
#endif
			
			includes.push_back(data);
		}
	}
	
	// Script. Script triggers. (no longer supported)
	{
		const u16 scriptCount = bu::be_get<u16>(p_bufferOUT, p_sizeOUT);
		TT_ASSERTMSG(scriptCount == 0,
		             "Shoebox contains %u script triggers. These are no longer supported.", scriptCount);
		for (u16 i = 0; i < scriptCount; ++i)
		{
			// NOTE: Script triggers are no longer supported. Load the data (so that the read pointer
			//       is moved appropriately), but discard it afterwards.
			ScriptTriggerData data(ScriptTriggerData::parse(p_bufferOUT, p_sizeOUT, &errStatus));
			TT_ERR_RETURN_ON_ERROR();
			(void)data;
		}
	}
	
	// Popup. Popup triggers. (no longer supported)
	{
		const u16 popupCount = bu::be_get<u16>(p_bufferOUT, p_sizeOUT);
		TT_ASSERTMSG(popupCount == 0,
		             "Shoebox contains %u popups. These are no longer supported.", popupCount);
		for (u16 i = 0; i < popupCount; ++i)
		{
			// NOTE: Popups are no longer supported. Load the data (so that the read pointer
			//       is moved appropriately), but discard it afterwards.
			PopupData data(PopupData::parse(p_bufferOUT, p_sizeOUT, &errStatus));
			TT_ERR_RETURN_ON_ERROR();
			(void)data;
		}
	}
	
	// Load planes
	{
		const u16 planeCount = bu::be_get<u16>(p_bufferOUT, p_sizeOUT);
		planes.reserve(static_cast<Planes::size_type>(planeCount));
		for (u16 i = 0; i < planeCount; ++i)
		{
			PlaneData data(PlaneData::parse(p_bufferOUT, p_sizeOUT, &errStatus));
			if (errStatus.hasError())
			{
				data.animations.reset();
				data.textureanimations.reset();
			}
			TT_ERR_RETURN_ON_ERROR();
			
			planes.push_back(data);
		}
	}
	
	// Load particles
	{
		const u16 particleCount = bu::be_get<u16>(p_bufferOUT, p_sizeOUT);
		particles.reserve(static_cast<Particles::size_type>(particleCount));
		for (u16 i = 0; i < particleCount; ++i)
		{
			ParticleData data(ParticleData::parse(p_bufferOUT, p_sizeOUT, &errStatus));
			TT_ERR_RETURN_ON_ERROR();
			
			particles.push_back(data);
		}
	}
	
	// Presentation objects. (optional)
	{
		const u16 objectCount = bu::be_get<u16>(p_bufferOUT, p_sizeOUT);
		presentationObjects.reserve(static_cast<PresentationObjects::size_type>(objectCount));
		for (u16 i = 0; i < objectCount; ++i)
		{
			PresentationData data(PresentationData::parse(p_bufferOUT, p_sizeOUT, &errStatus));
			TT_ERR_RETURN_ON_ERROR();
			
			presentationObjects.push_back(data);
		}
	}
}


/*
bool ShoeboxData::load(const xml::XmlNode* p_rootNode, code::ErrorStatus* p_errStatus)
{
	using xml::XmlNode;
	TT_ERR_CHAIN(bool, false, "Loading shoebox from XML");
	
	// Get the root of the file and verify it
	const XmlNode* root = p_rootNode;
	if (root == 0)
	{
		TT_ERR_AND_RETURN("XmlNode pointer is null!");
	}
	if (root->getName() != "shoebox")
	{
		TT_ERR_AND_RETURN("Expected XmlNode with the name: 'shoebox' found: '"
		                  << root->getName() << "'");
	}
	
	// Include -> Other Shoebox. (optional)
	{
		const XmlNode* node = root->getFirstChild("includes");
		if (node != 0)
		{
			parseIncludes(node, p_levelWidth, p_levelHeight, &errStatus);
		}
	}
	
	// Set clear color (optional)
	{
		const XmlNode* node = root->getFirstChild("clearcolor");
		if (node != 0)
		{
			m_clearColor = xml::util::parseColorRGB(node, &errStatus);
		}
	}
	
	// Background
	{
		const XmlNode* backgroundNode = root->getFirstChild("background");
		if (backgroundNode == 0)
		{
			TT_WARN("shoebox file '%s' is missing the background node.", root->getName().c_str());
		}
		else
		{
			parseXML(backgroundNode, false, p_levelWidth, p_levelHeight, &errStatus);
		}
	}
	
	// Foreground
	{
		const XmlNode* foregroundNode = root->getFirstChild("foreground");
		if (foregroundNode == 0)
		{
			TT_WARN("shoebox file '%s' is missing the foreground node.", root->getName().c_str());
		}
		else
		{
			parseXML(foregroundNode, true, p_levelWidth, p_levelHeight, &errStatus);
		}
	}
	
	// Presentation. (optional)
	{
		const XmlNode* node = root->getFirstChild("presentation");
		if (node != 0)
		{
			parsePresentationObject(node, &errStatus, p_levelWidth, p_levelHeight);
		}
	}
	
	return true;
}


void ShoeboxData::parseXML(const xml::XmlNode* p_node, code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN_VOID("Parsing shoebox XML " << (p_foreground ? "foreground" : "background"));
	
	if (p_node == 0 || p_node->getChildCount() == 0)
	{
		return;
	}
	
	for (const xml::XmlNode* child = p_node->getChild(); child != 0; child = child->getSibling())
	{
		const std::string& name = child->getName();
		
		if (name == "plane")
		{
			PlaneData data(PlaneData::parse(child, &errStatus));
			if (errStatus.hasError())
			{
				code::helpers::safeDelete(data.animations);
				code::helpers::safeDelete(data.textureanimations);
			}
			TT_ERR_RETURN_ON_ERROR();
			
			// Scale the plane
			data.position *= m_scale;
			data.position += m_positionOffset;
			data.width     = data.width  * m_scale;
			data.height    = data.height * m_scale;
			
			// Set priority
			data.priority  = data.priority.get() + m_priority;
			
			if (data.offsetAnimXMove.isValid())
			{
				data.offsetAnimXMove = data.offsetAnimXMove * m_scale;
			}
			
			if (data.offsetAnimYMove.isValid())
			{
				data.offsetAnimYMove = data.offsetAnimYMove * m_scale;
			}
			
			addPlane(data, p_levelWidth, p_levelHeight);
		}
		else if (name == "particle")
		{
			ParticleData data(ParticleData::parse(child, &errStatus));
			TT_ERR_RETURN_ON_ERROR();
			
			// scale the Position
			data.position *= m_scale;
			data.position += m_positionOffset;
			
			addParticle(data, p_foreground, p_levelWidth, p_levelHeight);
		}
		else
		{
			TT_WARN("Found tag with unknown name: '%s' (expected name 'plane' or 'particle')",
			        name.c_str());
		}
	}
}


void ShoeboxData::parseIncludes(const xml::XmlNode* p_node, code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN_VOID("Parsing 'includes'");
	
	TT_ERR_NULL_ASSERT(p_node);
	xml::util::checkName(p_node, "includes", errStatus);
	TT_ERR_RETURN_ON_ERROR();
	
	for (const xml::XmlNode* child = p_node->getChild(); child != 0; child = child->getSibling())
	{
		if (child->getName() == "shoebox")
		{
			std::string filename = xml::util::parseStr(child, "filename", 0);
			TT_Printf("Found include for shoebox file: '%s'\n", filename.c_str());
			std::string filePath(ms_shoeboxesPath + filename);
			TT_ASSERTMSG(fs::fileExists(filePath),
			             "Couldn't find include shoebox file: '%s'", filePath.c_str());
			load(filePath, p_levelWidth, p_levelHeight, m_scale, m_positionOffset);
		}
		else
		{
			TT_ERR_AND_RETURN("Found unknown child for 'include'. "
			                  "Found: '" << child->getName() << "'");
		}
	}
}


void ShoeboxData::parsePresentationObject(const xml::XmlNode* p_node, code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN_VOID("Parsing 'presentation'");
	
	TT_ERR_NULL_ASSERT(p_node);
	xml::util::checkName(p_node, "presentation", errStatus);
	TT_ERR_RETURN_ON_ERROR();
	
	for (const xml::XmlNode* child = p_node->getChild(); child != 0; child = child->getSibling())
	{
		if (child->getName() == "object")
		{
			PresentationData data(PresentationData::parse(child, &errStatus));
			TT_ERR_RETURN_ON_ERROR();
			TT_ERR_ASSERTMSG(m_presMgr != 0, "Creating shoebox presentation object without manager");
			TT_ERR_RETURN_ON_ERROR();
			
			// Bastiaan: Add width and height scale here.
			data.position *= m_scale;
			data.position += m_positionOffset;
			
			addPresentationObject(data, p_levelWidth, p_levelHeight);
		}
		else
		{
			TT_ERR_AND_RETURN("Found unknown child for 'presentation'. "
			                  "Found: '" << child->getName() << "'");
		}
	}
}
*/


void getAllUsedTextures(const ShoeboxDataPtr& p_data, renderer::EngineIDToTextures& p_usedTextures,
                        bool p_loadIncludeFiles)
{
	if (p_data == 0)
	{
		return;
	}
	
	// Get textures from this shoebox
	for (ShoeboxData::Planes::const_iterator it = p_data->planes.begin(); it != p_data->planes.end(); ++it)
	{
		EngineID engineID(getEngineID(it->textureFilename));
		
		// If not yet in needed list.
		if (p_usedTextures.find(engineID.getValue()) == p_usedTextures.end())
		{
			p_usedTextures.insert(createTextureCacheEntry(it->textureFilename));
		}
	}
	
	if (p_loadIncludeFiles)
	{
		// Load all includes mentioned in the data
		for (ShoeboxData::Includes::const_iterator it = p_data->includes.begin();
		     it != p_data->includes.end(); ++it)
		{
			const IncludeData& include(*it);
			
			const std::string filename(/*makeShoeboxFilename(*/include.filename + ".shoebox");
			//if (isAllowedToLoadForDeviceType(filename)) // FIXME: what to do with this Shoebox functions?
			{
				TT_ERR_CREATE("Load shoebox include '" << filename << "'.");
				ShoeboxDataPtr includedData = ShoeboxData::parse(filename, &errStatus);
				TT_ERR_ASSERT_ON_ERROR();
				if (errStatus.hasError())
				{
					continue;
				}
				
				// Recursion step
				getAllUsedTextures(includedData, p_usedTextures, true);
			}
		}
	}
}


void getAllUsedEngineIDs(const ShoeboxDataPtr& p_data, EngineIDValues& p_engineIDs, bool p_loadIncludeFiles)
{
	if (p_data == 0)
	{
		return;
	}
	
	// Get textures from this shoebox
	for (ShoeboxData::Planes::const_iterator it = p_data->planes.begin(); it != p_data->planes.end(); ++it)
	{
		EngineID engineID(getEngineID(it->textureFilename));
		u64 idValue = engineID.getValue();
		// If not yet in needed list.
		if (p_engineIDs.find(idValue) == p_engineIDs.end())
		{
			p_engineIDs.insert(idValue);
		}
	}
	
	if (p_loadIncludeFiles)
	{
		// Load all includes mentioned in the data
		for (ShoeboxData::Includes::const_iterator it = p_data->includes.begin(); it != p_data->includes.end(); ++it)
		{
			const IncludeData& include(*it);
			
			const std::string filename(/*makeShoeboxFilename(*/include.filename + ".shoebox");
			//if (isAllowedToLoadForDeviceType(filename)) // FIXME: what to do with this Shoebox functions?
			{
				TT_ERR_CREATE("Load shoebox include '" << filename << "'.");
				ShoeboxDataPtr includedData = ShoeboxData::parse(filename, &errStatus);
				TT_ERR_ASSERT_ON_ERROR();
				if (errStatus.hasError())
				{
					continue;
				}
				
				TT_NULL_ASSERT(includedData);
				
				// Recursion step
				getAllUsedEngineIDs(includedData, p_engineIDs, true);
			}
		}
	}
}


tt::engine::renderer::EngineIDToTextures getTexturesWithID(const tt::engine::renderer::EngineIDToTextures& p_textures,
                                                           const EngineIDValues& p_engineIDs)
{
	tt::engine::renderer::EngineIDToTextures::const_iterator textIt = p_textures.begin();
	EngineIDValues::const_iterator idIt = p_engineIDs.begin();
	
	tt::engine::renderer::EngineIDToTextures result;
	while (textIt != p_textures.end() && idIt != p_engineIDs.end())
	{
		u64 textValue = textIt->first;
		u64 idValue     = (*idIt);
		if (textValue < idValue)
		{
			++textIt;
		}
		else if (idValue < textValue)
		{
			++idIt;
		}
		else
		{
			result.insert(*textIt);
			++textIt;
			++idIt;
		}
	}
	return result;
}


EngineID getEngineID(const std::string& p_filename)
{
	std::string filename(p_filename);
	std::string file(filename);
	std::string nameSpace;
	
	// Convert PNG filename to asset ID
	if (str::endsWith(filename, ".png"))
	{
		filename = fs::utils::compactPath(Shoebox::setTexturesPath() + filename, "\\/");
		if (filename.empty())
		{
			TT_WARN("Invalid shoebox texture filename (empty string).");
			return EngineID(0,0);
		}
		
		// nameSpace output example: "textures.menu"
		nameSpace = filename.substr(0, filename.rfind(fs::getDirSeparator()));
		std::replace(nameSpace.begin(), nameSpace.end(), fs::getDirSeparator(), '.');
		
		std::string::size_type min = filename.rfind(fs::getDirSeparator());
		min += 1; // min will never be std::string::npos
		
		std::string::size_type max    = filename.rfind('.');
		std::string::size_type length = std::string::npos;
		if (max >= min)
		{
			length = max - min;
		}
		
		// File output example: "menucursor"
		file = filename.substr(min, length);
	}
	
	return EngineID(file, nameSpace);
}


renderer::EngineIDToTextures::value_type createTextureCacheEntry(const std::string& p_filename)
{
	EngineID engineID(getEngineID(p_filename));
	u64 idValue = engineID.getValue();
	
	if (idValue == 0)
	{
		return renderer::EngineIDToTextures::value_type();
	}
	
	// Load the texture
	// FIXME: Add namespaces to shoebox data
	renderer::TexturePtr texture = renderer::TextureCache::get(engineID, false);
	
	if (texture == 0)
	{
		TT_PANIC("Shoebox: Failed to load texture '%s'", p_filename.c_str());
		return renderer::EngineIDToTextures::value_type();
	}
	
	return std::make_pair(idValue, texture);
}



// Namespace end
}
}
}
}
