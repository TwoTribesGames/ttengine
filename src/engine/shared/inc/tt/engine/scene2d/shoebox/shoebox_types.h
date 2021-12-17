#if !defined(INC_TT_ENGINE_SCENE2D_SHOEBOX_SHOEBOX_TYPES_H)
#define INC_TT_ENGINE_SCENE2D_SHOEBOX_SHOEBOX_TYPES_H


#include <string>
#include <vector>
#include <set>

#include <tt/code/DefaultValue.h>
#include <tt/code/ErrorStatus.h>
#include <tt/code/OptionalValue.h>
#include <tt/engine/anim2d/fwd.h>
#include <tt/engine/anim2d/AnimationStack2D.h>
#include <tt/engine/anim2d/ColorAnimationStack2D.h>
#include <tt/engine/particles/fwd.h>
#include <tt/engine/renderer/ColorRGBA.h>
#include <tt/engine/scene2d/shoebox/fwd.h>
#include <tt/engine/scene2d/fwd.h>
#include <tt/engine/fwd.h>
#include <tt/fs/types.h>
#include <tt/math/Rect.h>
#include <tt/math/Vector3.h>
#include <tt/xml/fwd.h>
#include <tt/xml/XmlDocument.h>


namespace assettool { namespace conversion { namespace toki { class SourceItemTokiShoebox; } } }

namespace tt {
namespace engine {
namespace scene2d {
namespace shoebox {

struct PositionData
{
public:
	inline PositionData()
	:
	position(math::Vector3::zero)
	{
	}
	
	math::Vector3 position;
	
protected:
	void loadPosition(const xml::XmlNode* p_node, code::ErrorStatus* p_errStatus);
	void savePosition(      xml::XmlNode* p_node, code::ErrorStatus* p_errStatus) const;
	
	void loadPosition(const u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus);
	void savePosition(      u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus) const;
	
	static const size_t s_size = 4 + 4 + 4; // x + y + z
	
	friend class assettool::conversion::toki::SourceItemTokiShoebox;
};


struct ParticleData : public PositionData
{
	typedef std::vector<std::string> Tags;
	
	std::string                     particleFilename;
	code::DefaultValue<real>        scale;
	code::DefaultValue<std::string> parentID;
	code::DefaultValue<bool>        hidden; // Start 'hidden'. (same as Plane's hidden.) Needs to get started external code.
	Tags                            tags;
	
	inline ParticleData()
	:
	PositionData(),
	particleFilename(),
	scale(1.0f),
	parentID(""),
	hidden(false)
	{
	}
	
	static ParticleData parse(const xml::XmlNode* p_node, code::ErrorStatus* p_errStatus);
	static ParticleData parse(const u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus);
	
	void loadParticle(const xml::XmlNode* p_node, code::ErrorStatus* p_errStatus);
	void saveParticle(      xml::XmlNode* p_node, code::ErrorStatus* p_errStatus) const;
	
	void loadParticle(const u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus);
	void saveParticle(      u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus) const;
};



struct RectData : public PositionData
{
	inline RectData()
	:
	rotation(0.0f),
	width(),
	height()
	{
	}
	
	code::DefaultValue <real> rotation;
	code::OptionalValue<real> width;
	code::OptionalValue<real> height;
	
protected:
	void loadRect(const xml::XmlNode* p_node, code::ErrorStatus* p_errStatus);
	void saveRect(      xml::XmlNode* p_node, code::ErrorStatus* p_errStatus) const;
	
	void loadRect(const u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus);
	void saveRect(      u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus) const;
	
	static const size_t s_size = 4 + 4 + 4; // rotation + width + height
	
	friend class assettool::conversion::toki::SourceItemTokiShoebox;
};


struct ScriptTriggerData
{
	inline ScriptTriggerData()
	:
	rect(),
	triggerName(),
	scriptFilename(),
	tagName("")
	{
	}
	
	math::PointRect                 rect;
	std::string                     triggerName;
	std::string                     scriptFilename;
	code::DefaultValue<std::string> tagName;
	
	static ScriptTriggerData parse(const xml::XmlNode* p_node,                code::ErrorStatus* p_errStatus);
	static ScriptTriggerData parse(const u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus);
	void print() const;
	
protected:
	void loadTrigger(const xml::XmlNode* p_node, code::ErrorStatus* p_errStatus);
	void saveTrigger(      xml::XmlNode* p_node, code::ErrorStatus* p_errStatus) const;
	
	void loadTrigger(const u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus);
	void saveTrigger(      u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus) const;
	
	static const size_t s_size = 4 + 4 + 4 + 4; // top + left + width + height
	
	friend class assettool::conversion::toki::SourceItemTokiShoebox;
};


struct PopupData
{
	inline PopupData()
	:
	rect(),
	popupName(""),
	scriptFilename(""),
	locID(),
	tags()
	{
	}
	
	math::PointRect                 rect;
	code::DefaultValue<std::string> popupName;
	code::DefaultValue<std::string> scriptFilename;
	std::string                     locID;
	
	typedef std::vector<std::string> Tags;
	Tags tags;
	
	static PopupData parse(const xml::XmlNode* p_node,                code::ErrorStatus* p_errStatus);
	static PopupData parse(const u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus);
	void print() const;
	
protected:
	void loadPopup(const xml::XmlNode* p_node, code::ErrorStatus* p_errStatus);
	void savePopup(      xml::XmlNode* p_node, code::ErrorStatus* p_errStatus) const;
	
	void loadPopup(const u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus);
	void savePopup(      u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus) const;
	
	static const size_t s_size = 4 + 4 + 4 + 4; // top + left + width + height
	
	friend class assettool::conversion::toki::SourceItemTokiShoebox;
};


struct PlaneData : public RectData
{
	inline PlaneData()
	:
	id(),
	textureFilename(),
	blendMode(renderer::BlendMode_Blend),
	minFilter(renderer::FilterMode_Invalid),
	magFilter(renderer::FilterMode_Invalid),
	mipFilter(renderer::FilterMode_Invalid),
	screenSpace(false),
	hidden(false),
	ignoreFog(false),
	texTopLeftU(0.0f),
	texTopLeftV(0.0f),
	texTopRightU(1.0f),
	texTopRightV(0.0f),
	texBottomLeftU(0.0f),
	texBottomLeftV(1.0f),
	texBottomRightU(1.0f),
	texBottomRightV(1.0f),
	texAnimU(0.0f),
	texAnimV(0.0f),
	texOffsetScale(0.0f),
	offsetAnimXMove(0.0f),
	offsetAnimXDuration(0.0f),
	offsetAnimXTimeOffset(0.0f),
	offsetAnimYMove(0.0f),
	offsetAnimYDuration(0.0f),
	offsetAnimYTimeOffset(0.0f),
	cameraSpaceUScale(0.0f),
	cameraSpaceVScale(0.0f),
	colorWholeQuad(  renderer::ColorRGB::white),
	colorTopLeft(    renderer::ColorRGB::white),
	colorTopRight(   renderer::ColorRGB::white),
	colorBottomLeft( renderer::ColorRGB::white),
	colorBottomRight(renderer::ColorRGB::white),
	priority(0),
	animations(),
	textureanimations(),
	coloranimations(),
	tags()
	{
	}
	
	
	std::string                              id;
	std::string                              textureFilename;
	code::DefaultValue<renderer::BlendMode>  blendMode;
	code::DefaultValue<renderer::FilterMode> minFilter;
	code::DefaultValue<renderer::FilterMode> magFilter;
	code::DefaultValue<renderer::FilterMode> mipFilter;
	code::DefaultValue<bool>                 screenSpace;
	code::DefaultValue<bool>                 hidden;
	code::DefaultValue<bool>                 ignoreFog;
	
	code::DefaultValue<real> texTopLeftU;
	code::DefaultValue<real> texTopLeftV;
	code::DefaultValue<real> texTopRightU;
	code::DefaultValue<real> texTopRightV;
	code::DefaultValue<real> texBottomLeftU;
	code::DefaultValue<real> texBottomLeftV;
	code::DefaultValue<real> texBottomRightU;
	code::DefaultValue<real> texBottomRightV;
	
	code::DefaultValue<real> texAnimU;
	code::DefaultValue<real> texAnimV;
	code::DefaultValue<real> texOffsetScale;
	code::DefaultValue<real> offsetAnimXMove;
	code::DefaultValue<real> offsetAnimXDuration;
	code::DefaultValue<real> offsetAnimXTimeOffset;
	code::DefaultValue<real> offsetAnimYMove;
	code::DefaultValue<real> offsetAnimYDuration;
	code::DefaultValue<real> offsetAnimYTimeOffset;
	code::DefaultValue<real> cameraSpaceUScale;
	code::DefaultValue<real> cameraSpaceVScale;
	
	code::DefaultValue<renderer::ColorRGBA> colorWholeQuad;
	code::DefaultValue<renderer::ColorRGBA> colorTopLeft;
	code::DefaultValue<renderer::ColorRGBA> colorTopRight;
	code::DefaultValue<renderer::ColorRGBA> colorBottomLeft;
	code::DefaultValue<renderer::ColorRGBA> colorBottomRight;
	
	code::DefaultValue<s32> priority;
	
	anim2d::AnimationStack2DPtr      animations;
	anim2d::AnimationStack2DPtr      textureanimations;
	anim2d::ColorAnimationStack2DPtr coloranimations;
	
	typedef std::vector<std::string> Tags;
	Tags tags;
	
	
	static PlaneData parse(const xml::XmlNode* p_node,                code::ErrorStatus* p_errStatus);
	static PlaneData parse(const u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus);
	
	void loadPlane(const xml::XmlNode* p_node, code::ErrorStatus* p_errStatus);
	void savePlane(      xml::XmlNode* p_node, code::ErrorStatus* p_errStatus) const;
	
	void loadPlane(const u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus);
	void savePlane(      u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus) const;
	
private:
	static const size_t s_size = 1 + 1 + 3 + (8 * 4) + (11 * 4) + (4 * 4) + 4; // all members except animations, tags and colorwholequad
};


struct PresentationData
{
	inline PresentationData()
	:
	presentationFileName(),
	delay(0.0f),
	position(math::Vector3::zero)
	{ }
	
	
	std::string   presentationFileName;
	real          delay;
	math::Vector3 position;
	
	
	static PresentationData parse(const xml::XmlNode* p_node,                code::ErrorStatus* p_errStatus);
	static PresentationData parse(const u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus);
	
	void load(const xml::XmlNode* p_node,                code::ErrorStatus* p_errStatus);
	void load(const u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus);
	
	void save(xml::XmlNode* p_node,                code::ErrorStatus* p_errStatus) const;
	void save(u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus) const;
	
private:
	static const size_t s_size = 4 + 4 + 4 + 4; // delay + x + y + z
};


struct IncludeData
{
	inline IncludeData(const std::string& p_filename = std::string(),
	                   math::Vector3      p_offset   = math::Vector3::zero,
	                   s32                p_priority = 0,
	                   real               p_scale    = 1.0f)
	:
	filename(p_filename),
	offset(  p_offset),
	priority(p_priority),
	scale(   p_scale)
	{ }
	
	
	std::string   filename;
	math::Vector3 offset;
	s32           priority;
	real          scale;
	
	
	//static IncludeData parse(const xml::XmlNode* p_node,                code::ErrorStatus* p_errStatus);
	static IncludeData parse(const u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus);
	
	void save(xml::XmlNode* p_node, code::ErrorStatus* p_errStatus) const;
	
	inline bool operator==(const IncludeData& p_rhs) const
	{
		return filename == p_rhs.filename &&
		       offset   == p_rhs.offset   &&
		       priority == p_rhs.priority &&
		       scale    == p_rhs.scale;
	}
	inline bool operator!=(const IncludeData& p_rhs) const { return operator==(p_rhs) == false; }
	
private:
	void load(const u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus);
};


/*! \brief Data representing a shoebox as a whole (a Shoebox can be constructed from this). */
struct ShoeboxData
{
	typedef std::vector<IncludeData>       Includes;
	typedef std::vector<PlaneData>         Planes;
	typedef std::vector<ParticleData>      Particles;
	typedef std::vector<PresentationData>  PresentationObjects;
	
	
	Includes            includes;
	Planes              planes;
	Particles           particles;
	PresentationObjects presentationObjects;
	
	inline void clear()
	{
		includes.clear(); planes.clear(); particles.clear(); presentationObjects.clear();
	}
	
	inline ShoeboxData()
	:
	includes(),
	planes(),
	particles(),
	presentationObjects()
	{ }
	
	static ShoeboxDataPtr parse(const std::string& p_filename,             code::ErrorStatus* p_errStatus);
	static ShoeboxDataPtr parse(const u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus);
	void saveAsXML(const std::string& p_filename, bool p_formatted, code::ErrorStatus* p_errStatus) const;
	
	void mergeWith(const ShoeboxData& p_other /*, offset, scale, priority */);
	
private:
	void load(const u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus);
	/*
	bool load(const xml::XmlNode* p_rootNode, code::ErrorStatus* p_errStatus);
	
	void parseXML               (const xml::XmlNode* p_node, code::ErrorStatus* p_errStatus);
	void parseIncludes          (const xml::XmlNode* p_node, code::ErrorStatus* p_errStatus);
	void parsePresentationObject(const xml::XmlNode* p_node, code::ErrorStatus* p_errStatus);
	*/
};


typedef std::set<u64> EngineIDValues;

void getAllUsedTextures(const ShoeboxDataPtr& p_data, renderer::EngineIDToTextures& p_usedTextures,
                        bool p_loadIncludeFiles);
void getAllUsedEngineIDs(const ShoeboxDataPtr& p_data, EngineIDValues& p_engineIDs, bool p_loadIncludeFiles);

tt::engine::renderer::EngineIDToTextures getTexturesWithID(const tt::engine::renderer::EngineIDToTextures& p_textures,
                                                           const EngineIDValues& p_engineIDs);

EngineID getEngineID(const std::string& p_filename);
renderer::EngineIDToTextures::value_type createTextureCacheEntry(const std::string& p_filename);


// Namespace end
}
}
}
}


#endif  // !defined(INC_TT_ENGINE_SCENE2D_SHOEBOX_SHOEBOX_TYPES_H)
