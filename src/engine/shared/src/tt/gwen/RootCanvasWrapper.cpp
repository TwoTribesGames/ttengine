#include <tt/app/Application.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/ViewPort.h>
#include <tt/gwen/RootCanvasWrapper.h>
#include <tt/input/KeyList.h>



namespace tt {
namespace gwen {


RootCanvasWrapper::RootCanvasWrapper(const std::string&           p_canvasName,
                                     const std::string&           p_skin,
                                     engine::renderer::ViewPortID p_renderViewPort)
:
m_renderViewPort(p_renderViewPort),
m_gwenRenderer(),
m_gwenSkin(0),
m_rootCanvas(0)
{
	// Create the basic GWEN support classes
	m_gwenSkin.SetRender(&m_gwenRenderer);
	m_gwenSkin.Init(p_skin);
	
	m_rootCanvas = new Gwen::Controls::Canvas(&m_gwenSkin);
	m_rootCanvas->SetName(p_canvasName);
	
	updateCanvasSize();
	m_rootCanvas->SetDrawBackground(false);
	
	// Register self for device reset callbacks.
	tt::app::getApplication()->registerPlatformCallbackInterface(this);
}


RootCanvasWrapper::~RootCanvasWrapper()
{
	m_rootCanvas->DoThink(); // Flush delayed deletions
	delete m_rootCanvas;
	
	tt::app::getApplication()->unregisterPlatformCallbackInterface(this);
}


bool RootCanvasWrapper::handleInput(const tt::input::Pointer& p_currentPointer,
                                    const tt::input::Pointer& p_previousPointer,
                                    const tt::input::Button&  p_action,
                                    const tt::input::Button&  p_cancel,
                                    s32 p_wheelNotches, const Gwen::Controls::Base* p_ignoreControl) const
{
	/* // TODO:
	m_rootCanvas->InputKey(int key, bool down);
	m_rootCanvas->InputCharacter(wchar char);
	*/
	
	bool inputHandledByGwen = false;
	
	//if (inputState.pointer != previousInputState.pointer)
	if (p_currentPointer.valid)
	{
		//TT_Printf("Editor::update - pointer x: %d, y: %d\n", inputState.pointer.x, inputState.pointer.y);
		
		inputHandledByGwen = inputHandledByGwen ||
		                     m_rootCanvas->InputMouseMoved(p_currentPointer.x, p_currentPointer.y,
		                                                   p_currentPointer.x - p_previousPointer.x, 
		                                                   p_currentPointer.y - p_previousPointer.y);
	}
	else if (p_previousPointer.valid)
	{
		// Moved outside the screen, release action and cancel
		m_rootCanvas->InputMouseButton(0, false);
		m_rootCanvas->InputMouseButton(1, false);
		return inputHandledByGwen;
	}
	
	// HACK: Allow editor input to be handled if the control we're hovering over is the editor dock
	// (which is the full-screen root of the editor GUI)
	Gwen::Controls::Base* hoverControl = m_rootCanvas->GetControlAt(p_currentPointer.x, p_currentPointer.y);
	
	if (hoverControl == 0 || hoverControl == p_ignoreControl)
	{
		//TT_Printf("Editor::update: Hovering over '%s' (0x%08X)\n", hoverControl->GetName().c_str(), hoverControl);
		inputHandledByGwen = false;
	}
	
	if (p_wheelNotches != 0)
	{
		m_rootCanvas->InputMouseWheel(p_wheelNotches);
	}
	
	if (p_action.pressed)
	{
		m_rootCanvas->InputMouseButton(0, true);
		
		if (Gwen::HoveredControl == p_ignoreControl)
		{
			Gwen::Input::Blur();
		}
	}
	else if (p_action.released)
	{
		m_rootCanvas->InputMouseButton(0, false);
	}
	
	if (p_cancel.pressed)
	{
		m_rootCanvas->InputMouseButton(1, true);
		
		if (Gwen::HoveredControl == p_ignoreControl)
		{
			Gwen::Input::Blur();
		}
	}
	else if (p_cancel.released)
	{
		m_rootCanvas->InputMouseButton(1, false);
	}
	
	return inputHandledByGwen;
}


bool RootCanvasWrapper::handleKeyInput(const tt::input::Button* p_keys, const std::wstring& p_chars)
{
	bool inputHandledByGwen(false);

	const bool ctrlDown(p_keys[tt::input::Key_Control].down);

	for(u32 i = 0; i < tt::input::Key_Count; ++i)
	{
		if(p_keys[i].pressed || p_keys[i].released)
		{
			bool down(p_keys[i].pressed);

			if(down && ctrlDown && i >= tt::input::Key_A && i <= tt::input::Key_Z)
			{
				inputHandledByGwen = inputHandledByGwen || 
					m_rootCanvas->InputCharacter(static_cast<Gwen::UnicodeChar>(i));
			}

			int keyCode(-1);

			switch(i)
			{
			case tt::input::Key_Escape   : keyCode = Gwen::Key::Escape;    break;
			case tt::input::Key_Enter    : keyCode = Gwen::Key::Return;    break;
			case tt::input::Key_Shift    : keyCode = Gwen::Key::Shift;     break;
			case tt::input::Key_Backspace: keyCode = Gwen::Key::Backspace; break;
			case tt::input::Key_Delete   : keyCode = Gwen::Key::Delete;    break;
			case tt::input::Key_Left     : keyCode = Gwen::Key::Left;      break;
			case tt::input::Key_Right    : keyCode = Gwen::Key::Right;     break;
			case tt::input::Key_Tab      : keyCode = Gwen::Key::Tab;       break;
			case tt::input::Key_Space    : keyCode = Gwen::Key::Space;     break;
			case tt::input::Key_Home     : keyCode = Gwen::Key::Home;      break;
			case tt::input::Key_End      : keyCode = Gwen::Key::End;       break;
			case tt::input::Key_Control  : keyCode = Gwen::Key::Control;   break;
			case tt::input::Key_Up       : keyCode = Gwen::Key::Up;        break;
			case tt::input::Key_Down     : keyCode = Gwen::Key::Down;      break;
			default: break;
			}

			if(keyCode != -1)
			{
				inputHandledByGwen = inputHandledByGwen || m_rootCanvas->InputKey(keyCode, down);
			}

			if(down && i == tt::input::Key_Escape)
			{
				if(Gwen::KeyboardFocus != 0)
				{
					Gwen::Input::Blur();
					inputHandledByGwen = true;
				}
			}
		}
	}

	// Handle character input
	for(std::wstring::const_iterator it = p_chars.begin(); it != p_chars.end(); ++it)
	{
		inputHandledByGwen = inputHandledByGwen || m_rootCanvas->InputCharacter(*it);
	}

	return inputHandledByGwen;
}


void RootCanvasWrapper::render() const
{
	m_rootCanvas->RenderCanvas();
}


void RootCanvasWrapper::show()
{
	Gwen::Controls::Base::List& rootChildren(m_rootCanvas->Children);
	for (Gwen::Controls::Base::List::iterator it = rootChildren.begin(); it != rootChildren.end(); ++it)
	{
		(*it)->Show();
	}
}


void RootCanvasWrapper::hide()
{
	Gwen::Controls::Base::List& rootChildren(m_rootCanvas->Children);
	for (Gwen::Controls::Base::List::iterator it = rootChildren.begin(); it != rootChildren.end(); ++it)
	{
		(*it)->Hide();
	}
}


void RootCanvasWrapper::onResetDevice()
{
	updateCanvasSize();
}


//--------------------------------------------------------------------------------------------------
// Private member functions

void RootCanvasWrapper::updateCanvasSize()
{
	const engine::renderer::ViewPortContainer& viewPorts(engine::renderer::ViewPort::getViewPorts());
	engine::renderer::ViewPortContainer::size_type viewPortIndex =
			static_cast<engine::renderer::ViewPortContainer::size_type>(m_renderViewPort);
	if (m_renderViewPort >= 0 && viewPortIndex < viewPorts.size())
	{
		m_rootCanvas->SetSize(viewPorts[viewPortIndex].getSettings().width,
		                      viewPorts[viewPortIndex].getSettings().height);
	}
	else
	{
		TT_PANIC("Renderer does not have viewport %d. Using screen size for the GWEN root canvas.",
		         m_renderViewPort);
		engine::renderer::Renderer* renderer = engine::renderer::Renderer::getInstance();
		m_rootCanvas->SetSize(renderer->getScreenWidth(), renderer->getScreenHeight());
	}
}

// Namespace end
}
}
