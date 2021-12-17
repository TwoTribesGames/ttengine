#if !defined(INC_TT_GWEN_ROOTCANVASWRAPPER_H)
#define INC_TT_GWEN_ROOTCANVASWRAPPER_H

#include <Gwen/Controls/Canvas.h>
#include <Gwen/Renderers/TTRenderer.h>
#include <Gwen/Skins/TexturedBase.h>


#include <tt/app/PlatformCallbackInterface.h>
#include <tt/input/Pointer.h>
#include <tt/input/Button.h>


namespace tt {
namespace gwen {


class RootCanvasWrapper : public tt::app::PlatformCallbackInterface
{
public:
	RootCanvasWrapper(const std::string&           p_canvasName,
	                  const std::string&           p_skin,
	                  engine::renderer::ViewPortID p_renderViewPort = engine::renderer::ViewPortID_Main);
	~RootCanvasWrapper();
	
	bool handleInput(const tt::input::Pointer& p_currentPointer, const tt::input::Pointer& p_previousPointer,
	                 const tt::input::Button&  p_action,         const tt::input::Button&  p_cancel,
	                 s32 p_wheelNotches, const Gwen::Controls::Base* p_ignoreControl = 0) const;
	
	bool handleKeyInput(const tt::input::Button* p_keys, const std::wstring& p_chars);
	
	void render() const;
	
	void show();
	void hide();
	
	inline Gwen::Controls::Canvas* getCanvas() { return m_rootCanvas; }
	virtual void onResetDevice();
	
private:
	void updateCanvasSize();
	
	RootCanvasWrapper(const RootCanvasWrapper&);                 // Disable copy.
	const RootCanvasWrapper operator=(const RootCanvasWrapper&); // Disable assigment.
	
	
	engine::renderer::ViewPortID m_renderViewPort;
	Gwen::Renderer::TTRenderer   m_gwenRenderer;
	Gwen::Skin::TexturedBase     m_gwenSkin;
	Gwen::Controls::Canvas*      m_rootCanvas;
};


// Namespace end
}
}


#endif  // !defined(INC_TT_GWEN_ROOTCANVASWRAPPER_H)
