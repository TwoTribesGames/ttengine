#if !defined(INC_TOKI_GAME_HUD_LEVELPICKER_H)
#define INC_TOKI_GAME_HUD_LEVELPICKER_H

#if defined(TT_STEAM_BUILD)  // Steam Workshop functionality is only available in Steam builds


#include <steam/steam_api.h>

#include <tt/engine/renderer/fwd.h>

#include <toki/game/hud/fwd.h>
#include <toki/game/entity/fwd.h>
#include <toki/game/entity/graphics/types.h>
#include <toki/steam/WorkshopObserver.h>
#include <toki/utils/GlyphSetMgr.h>


namespace toki {
namespace game {
namespace hud {

class WorkshopLevelPicker : public steam::WorkshopObserver
{
public:
	enum Element
	{
		Element_LevelList,
		Element_Title,
		Element_Description,
		Element_PreviewImage,
		Element_AuthorAvatar,
		Element_AuthorName,
		
		Element_Count
	};
	static inline bool isValidElement(Element p_element)
	{ return p_element >= 0 && p_element < Element_Count; }
	
	
	static WorkshopLevelPickerPtr create();
	~WorkshopLevelPicker();
	
	void        open();
	void        close();
	inline bool isOpen() const { return m_open; }
	
	bool update(real p_deltaTime);
	void render();
	
	float getSelectedLevelScore() const;
	bool isSelectedLevelDownloaded() const;
	bool isAnyLevelSelected() const;
	
	void showSelectedLevelWorkshopPage();
	
	void selectPreviousLevel();
	void selectNextLevel();
	
	void playSelectedLevel();
	
	void setShowTextBorders(bool p_show);
	
	void setCallbackEntity(const entity::EntityHandle& p_entity);
	
	// UI element position and size control:
	void setElementFollowEntity(Element p_element, const entity::EntityHandle& p_entity);
	void setElementSize        (Element p_element, real p_width, real p_height);
	void setElementColor       (Element p_element, const tt::engine::renderer::ColorRGBA& p_color);
	void setElementVerticalAlignment(  Element p_element, entity::graphics::VerticalAlignment   p_verticalAlignment);
	void setElementHorizontalAlignment(Element p_element, entity::graphics::HorizontalAlignment p_horizontalAlignment);
	
	void                           setLevelListColorScheme(const hud::ListBoxColorScheme& p_colorScheme);
	const hud::ListBoxColorScheme& getLevelListColorScheme() const;
	
	virtual void onWorkshopFileChange(PublishedFileId_t p_id, steam::WorkshopObserver::FileAction p_action);
	
private:
	struct ElementInfo
	{
		entity::EntityHandle                followEntity;
		real                                width;
		real                                height;
		ListBoxPtr                          list;
		tt::engine::renderer::QuadSpritePtr quad;
		bool                                isTextQuad;
		std::wstring                        renderedText;
		bool                                textNeedsRepaint;  // whether text needs to be re-rendered onto the texture
		utils::GlyphSetID                   glyphSet;
		entity::graphics::VerticalAlignment   verticalAlignment;
		entity::graphics::HorizontalAlignment horizontalAlignment;
		
		inline ElementInfo()
		:
		followEntity(),
		width(0.0f),
		height(0.0f),
		list(),
		quad(),
		isTextQuad(false),
		renderedText(),
		textNeedsRepaint(false),
		glyphSet(utils::GlyphSetID_Text),
		verticalAlignment  (entity::graphics::VerticalAlignment_Top),
		horizontalAlignment(entity::graphics::HorizontalAlignment_Left)
		{ }
	};
	
	
	WorkshopLevelPicker();
	
	void setupUi();
	void populateLevelList();
	ListItem* addListItemForId(PublishedFileId_t p_id);
	
	void setAuthorAvatarToEmpty();
	void setPreviewImageToEmpty();
	
	void setAvatarFromUser(uint64 p_steamID);
	void setNameFromUser  (uint64 p_steamID);
	
	void setText(Element p_element, const std::wstring& p_text);
	void refreshTextTexture(ElementInfo& p_element);
	
	static void callbackLevelSelected(ListItem* p_item, void* p_userData);
	void onLevelSelected();
	
	// No copying
	WorkshopLevelPicker(const WorkshopLevelPicker&);
	WorkshopLevelPicker& operator=(const WorkshopLevelPicker&);
	
	
	tt_ptr<WorkshopLevelPicker>::weak m_this;
	
	bool        m_open;  // whether the level picker is open/visible/active
	ElementInfo m_elements[Element_Count];
	
	uint64 m_currentOwnerID;  // Steam ID of the level owner currently being displayed
	
	bool m_renderTextBorders;  // debug feature (does nothing in final builds)
	s32  m_textPixelsPerWorldUnit;
	
	entity::EntityHandle m_callbackEntity;
	bool                 m_notifyCallbackEntityThisFrame;          // call "onWorkshopLevelSelected" this frame?
	bool                 m_notifyCallbackEntityListEmptyThisFrame; // call "onWorkshopLevelListEmpty" this frame?
	bool                 m_notifyCallbackEntityListEmptyValue;     // value for "onWorkshopLevelListEmpty" call
	
	static PublishedFileId_t ms_lastPlayedLevel;
	
	STEAM_CALLBACK_MANUAL(WorkshopLevelPicker, onPersonaStateChange, PersonaStateChange_t, m_callbackPersonaStateChange);
	STEAM_CALLBACK_MANUAL(WorkshopLevelPicker, onAvatarImageLoaded,  AvatarImageLoaded_t,  m_callbackAvatarImageLoaded);
};

// Namespace end
}
}
}


#endif  // defined(TT_STEAM_BUILD)

#endif  // !defined(INC_TOKI_GAME_HUD_LEVELPICKER_H)
