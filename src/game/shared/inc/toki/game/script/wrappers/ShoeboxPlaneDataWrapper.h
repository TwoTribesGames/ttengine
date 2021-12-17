#if !defined(INC_TOKI_GAME_SCRIPT_WRAPPERS_SHOEBOXPLANEDATAWRAPPER_H)
#define INC_TOKI_GAME_SCRIPT_WRAPPERS_SHOEBOXPLANEDATAWRAPPER_H


#include <tt/engine/scene2d/shoebox/shoebox_types.h>
#include <tt/math/Vector2.h>
#include <tt/script/VirtualMachine.h>


namespace toki   /*! */ {
namespace game   /*! */ {
namespace script /*! */ {
namespace wrappers {


/*! \brief The PlaneData for Shoebox. !THIS CLASS IS NOT SERIALIZED! */
class ShoeboxPlaneDataWrapper
{
public:
	ShoeboxPlaneDataWrapper()
	:
	m_data()
	{}
	
	ShoeboxPlaneDataWrapper(const tt::engine::scene2d::shoebox::PlaneData& p_plane)
	:
	m_data(p_plane)
	{}
	
	/*! \brief Sets position xy. */
	inline void setPositionXY(const tt::math::Vector2& p_pos) { m_data.position.xy(p_pos); }
	
	/*! \brief Gets position xy. */
	inline tt::math::Vector2 getPositionXY() const { return m_data.position.xy(); }
	
	/*! \brief Sets position z. */
	inline void setPositionZ(real p_z) { m_data.position.z = p_z; }
	
	/*! \brief Gets position z. */
	inline real getPositionZ() const { return m_data.position.z; }

	/*! \brief Sets rotation. */
	inline void setRotation(real p_rotation) { m_data.rotation = p_rotation; }
	
	/*! \brief Gets rotation. */
	inline real getRotation() const { return m_data.rotation; }
	
	/*! \brief Sets width. */
	inline void setWidth(real p_width) { m_data.width = p_width; }
	
	/*! \brief Gets width. */
	inline real getWidth() const { return m_data.width; }
	
	/*! \brief Sets height. */
	inline void setHeight(real p_height) { m_data.height = p_height; }
	
	/*! \brief Gets height. */
	inline real getHeight() const { return m_data.height; }
	
	/*! \brief Sets id. */
	inline void setId(const std::string& p_id) { m_data.id = p_id; }
	
	/*! \brief Gets id. */
	inline const std::string& getId() const { return m_data.id; }
	
	/*! \brief Sets textureFilename. */
	inline void setTextureFilename(const std::string& p_textureFilename) { m_data.textureFilename = p_textureFilename; }
	
	/*! \brief Gets textureFilename. */
	inline const std::string& getTextureFilename() const { return m_data.textureFilename; }
	
	/*
	code::DefaultValue<renderer::BlendMode>  blendMode;
	code::DefaultValue<renderer::FilterMode> minFilter;
	code::DefaultValue<renderer::FilterMode> magFilter;
	code::DefaultValue<renderer::FilterMode> mipFilter;
	code::DefaultValue<bool>                 screenSpace;
	*/
	
	/*! \brief Sets hidden. */
	inline void setHidden(bool p_hidden) { m_data.hidden = p_hidden; }
	
	/*! \brief Gets hidden. */
	inline bool getHidden() const { return m_data.hidden; }
	
	/*! \brief Sets ignoreFog. */
	inline void setIgnoreFog(bool p_ignoreFog) { m_data.ignoreFog = p_ignoreFog; }
	
	/*! \brief Gets ignoreFog. */
	inline bool getIgnoreFog() const { return m_data.ignoreFog; }
	
	/*! \brief Sets texture coordinate for Top Left U. */
	inline void setTexTopLeftU(real p_textureCoord) { m_data.texTopLeftU = p_textureCoord; }
	
	/*! \brief Gets texture coordinate for Top Left U. */
	inline real getTexTopLeftU() const { return m_data.texTopLeftU; }
	
	/*! \brief Sets texture coordinate for Top Left V. */
	inline void setTexTopLeftV(real p_textureCoord) { m_data.texTopLeftV = p_textureCoord; }
	
	/*! \brief Gets texture coordinate for Top Left V. */
	inline real getTexTopLeftV() const { return m_data.texTopLeftV; }
	
	/*! \brief Sets texture coordinate for Top Right U. */
	inline void setTexTopRightU(real p_textureCoord) { m_data.texTopRightU = p_textureCoord; }
	
	/*! \brief Gets texture coordinate for Top Right U. */
	inline real getTexTopRightU() const { return m_data.texTopRightU; }
	
	/*! \brief Sets texture coordinate for Top Right V. */
	inline void setTexTopRightV(real p_textureCoord) { m_data.texTopRightV = p_textureCoord; }
	
	/*! \brief Gets texture coordinate for Top Right V. */
	inline real getTexTopRightV() const { return m_data.texTopRightV; }
	
	/*! \brief Sets texture coordinate for Bottom Left U. */
	inline void setTexBottomLeftU(real p_textureCoord) { m_data.texBottomLeftU = p_textureCoord; }
	
	/*! \brief Gets texture coordinate for Bottom Left U. */
	inline real getTexBottomLeftU() const { return m_data.texBottomLeftU; }
	
	/*! \brief Sets texture coordinate for Bottom Left V. */
	inline void setTexBottomLeftV(real p_textureCoord) { m_data.texBottomLeftV = p_textureCoord; }
	
	/*! \brief Gets texture coordinate for Bottom Left V. */
	inline real getTexBottomLeftV() const { return m_data.texBottomLeftV; }
	
	/*! \brief Sets texture coordinate for Bottom Right U. */
	inline void setTexBottomRightU(real p_textureCoord) { m_data.texBottomRightU = p_textureCoord; }
	
	/*! \brief Gets texture coordinate for Bottom Right U. */
	inline real getTexBottomRightU() const { return m_data.texBottomRightU; }
	
	/*! \brief Sets texture coordinate for Bottom Right V. */
	inline void setTexBottomRightV(real p_textureCoord) { m_data.texBottomRightV = p_textureCoord; }
	
	/*! \brief Gets texture coordinate for Bottom Right V. */
	inline real getTexBottomRightV() const { return m_data.texBottomRightV; }
	
	/*
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
	*/
	
	/*! \brief Sets priority. */
	inline void setPriority(s32 p_priority) { m_data.priority = p_priority; }
	
	/*! \brief Gets priority. */
	inline s32 getPriority() const { return m_data.priority; }

	/*
	anim2d::AnimationStack2DPtr      animations;
	anim2d::AnimationStack2DPtr      textureanimations;
	anim2d::ColorAnimationStack2DPtr coloranimations;
	tt::engine::scene2d::shoebox::PlaneData::Tags tags;
	*/
	
	inline const tt::engine::scene2d::shoebox::PlaneData& getData() const { return m_data; }
	
	static void bind(const tt::script::VirtualMachinePtr& p_vm);
	
private:
	tt::engine::scene2d::shoebox::PlaneData m_data;
};

// Constructor with values in squirrel
ShoeboxPlaneDataWrapper* ShoeboxPlaneDataWrapper_constructor(HSQUIRRELVM v);

// Namespace end
}
}
}
}

#endif  // !defined(INC_TOKI_GAME_SCRIPT_WRAPPERS_SHOEBOXPLANEDATAWRAPPER_H)
