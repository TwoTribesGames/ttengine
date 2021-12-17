#if !defined(INC_TOKI_LEVEL_ENTITY_EDITOR_ENTITYINSTANCEEDITORREPRESENTATION_H)
#define INC_TOKI_LEVEL_ENTITY_EDITOR_ENTITYINSTANCEEDITORREPRESENTATION_H


#include <map>
#include <string>

#include <tt/engine/renderer/ColorRGBA.h>

#include <tt/engine/renderer/fwd.h>
#include <tt/math/Point2.h>
#include <tt/math/Vector2.h>

#include <toki/game/fwd.h>
#include <toki/level/entity/editor/fwd.h>
#include <toki/level/entity/fwd.h>
#include <toki/level/fwd.h>


namespace toki {
namespace level {
namespace entity {
namespace editor {

/*! \brief Visual representation of an entity instance, for use in the level editor. */
class EntityInstanceEditorRepresentation
{
public:
	enum ImageState
	{
		ImageState_Normal,
		ImageState_Hidden,
		ImageState_Selected,
		ImageState_SelectedHidden,
		
		ImageState_Count,
		ImageState_Invalid
	};
	
	explicit EntityInstanceEditorRepresentation(const EntityInstance* p_entity);
	~EntityInstanceEditorRepresentation();
	
	void update(real p_deltaTime, const LevelDataPtr& p_levelData);
	void renderBack (const game::Camera& p_camera, const RenderFlags& p_renderFlags);
	void renderFront(const game::Camera& p_camera, const RenderFlags& p_renderFlags);
	void renderEditorWarnings();
	void renderEditorWarningsIcon();
	
	inline const tt::math::Vector2& getImageSize()     const { return m_imageSize;     }
	inline real                     getImageRotation() const { return m_imageRotation; }
	
	inline bool                     hasSizeShape() const { return (m_sizeShape != 0); }
	tt::math::Vector2               getSizeShapeSize() const;
	
	inline const tt::math::Vector2& getPosOffset() const        { return m_posOffset;     }
	inline void setPosOffset(const tt::math::Vector2& p_offset) { m_posOffset = p_offset; }
	
	// EntityInstance 'callbacks':
	void onPropertiesChanged();
	
	inline bool hasEditorWarnings() { return m_editorWarnings.empty() == false; }
	void addEditorWarning(const std::wstring& p_warningStr);
	void clearEditorWarnings();
	
	inline void setImageState(ImageState p_state) { m_imageState = p_state; }
	
	void resetLineBuffers();
	
private:
	enum SizeShapeType
	{
		SizeShapeType_Rectangle,
		SizeShapeType_Circle,
		
		SizeShapeType_Count,
		SizeShapeType_Invalid
	};
	
	struct EntityReference
	{
		enum Status
		{
			Status_Valid,
			Status_TargetDoesNotExist,
			Status_TargetIsIncorrectType  // not actually used right now
		};
		
		
		s32                                          targetID;
		Status                                       status;
		tt::math::Vector2                            targetPos;
		bool                                         targetIsSelected;
		tt::engine::renderer::ColorRGBA              color;
		tt::engine::renderer::TrianglestripBufferPtr lineBuffer;
		
		inline EntityReference()
		:
		targetID(-1),
		status(Status_TargetDoesNotExist),
		targetPos(tt::math::Vector2::zero),
		targetIsSelected(false),
		color(tt::engine::renderer::ColorRGB::white),
		lineBuffer()
		{ }
	};
	
	typedef std::vector<EntityReference>            EntityReferences;
	typedef std::map<std::string, EntityReferences> EntityReferencesPerProperty;
	
	
	void updatePositionOfEditorWarnings();
	void rescanProperties();
	
	// No copying
	EntityInstanceEditorRepresentation(const EntityInstanceEditorRepresentation&);
	EntityInstanceEditorRepresentation& operator=(const EntityInstanceEditorRepresentation&);
	
	
	const EntityInstance* m_entity;  // the entity instance that this object represents
	bool                  m_shouldRescanProperties;
	tt::math::Vector2     m_posOffset;
	
	EntityReferencesPerProperty m_entityReferences;  // property references to other entities
	
	//bool                              m_isSelected;  // perhaps leave as current LevelData container? Or add flag to EntityInstance instead?
	ImageState                          m_imageState;
	tt::engine::renderer::QuadSpritePtr m_imageQuad;
	tt::math::Vector2                   m_imageSize;
	real                                m_imageRotation;
	real                                m_imageScale;
	
	level::Notes                        m_editorWarnings; // Editor warnings, use Note for rendering.
	tt::engine::renderer::QuadSpritePtr m_editorWarningIconQuad;
	
	SizeShapeType                       m_sizeShapeType;
	tt::math::Vector2                   m_sizeShapeRect;    // Size of entity as dictated by entity width/height properties
	real                                m_sizeShapeRadius;  // Size of entity as dictated by entity radius property
	tt::engine::renderer::QuadSpritePtr m_sizeShape;
	bool                                m_sizeShapeFromCenter;
};

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_LEVEL_ENTITY_EDITOR_ENTITYINSTANCEEDITORREPRESENTATION_H)
