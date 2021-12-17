#if !defined(INC_TOKI_GAME_HUD_RESOLUTIONPICKER_H)
#define INC_TOKI_GAME_HUD_RESOLUTIONPICKER_H


#include <tt/platform/tt_types.h>
#include <tt/engine/renderer/fwd.h>

#include <toki/game/hud/fwd.h>
#include <toki/game/entity/fwd.h>


namespace toki {
namespace game {
namespace hud {

class ResolutionPicker
{
public:
	static ResolutionPickerPtr create();
	~ResolutionPicker();
	
	void        open();
	void        close();
	inline bool isOpen() const { return m_open; }
	
	bool update(real p_deltaTime);
	void render();
	
	void selectPrevious();
	void selectNext();
	
	void applySelectedResolution();
	
	void setCallbackEntity(const entity::EntityHandle& p_entity);
	
	void setFollowEntity(const entity::EntityHandle& p_entity);
	void setSize        (real p_width, real p_height);
	
	void                           setColorScheme(const hud::ListBoxColorScheme& p_colorScheme);
	const hud::ListBoxColorScheme& getColorScheme() const;
	
private:
	typedef std::vector<tt::math::Point2> Point2s;
	ResolutionPicker();
	
	void setupUi();
	void populateLevelList();
	
	void notifyCallbackEntity(const std::string& p_notificatonFunction);
	
	static void callbackResolutionSelected(ListItem* p_item, void* p_userData);
	void onResolutionSelected();
	
	// No copying
	ResolutionPicker(const ResolutionPicker&);
	ResolutionPicker& operator=(const ResolutionPicker&);
	
	tt_ptr<ResolutionPicker>::weak m_this;
	
	ListBoxPtr           m_list;
	entity::EntityHandle m_followEntity;
	real                 m_width;
	real                 m_height;
	
	bool                 m_open;  // whether the picker is open/visible/active
	
	Point2s              m_supportedResolutions;
	Point2s              m_upscaleResolutions;
	
	entity::EntityHandle m_callbackEntity;
};

// Namespace end
}
}
}

#endif  // !defined(INC_TOKI_GAME_HUD_RESOLUTIONPICKER_H)
