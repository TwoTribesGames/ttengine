#include <Gwen/Controls/RGBColorPicker.h>
#include <Gwen/Controls/Menu.h>
#include <Gwen/Controls/ScrollControl.h>
#include <Gwen/Controls/TabControl.h>

#include <tt/code/helpers.h>
#include <tt/code/NoDeleter.h>
#include <tt/engine/debug/DebugRenderer.h>
#include <tt/engine/file/FileUtils.h>
#include <tt/engine/particles/ParticleMgr.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/TrianglestripBuffer.h>
#include <tt/fs/utils/utils.h>
#include <tt/fs/fs.h>
#include <tt/pres/PresentationCache.h>
#include <tt/pres/PresentationMgr.h>
#include <tt/str/common.h>

#include <toki/audio/AudioPlayer.h>
#include <toki/game/entity/types.h>
#include <toki/pres/TriggerFactory.h>
#include <toki/viewer/StatePresentationViewer.h>
#include <toki/AppGlobal.h>


namespace toki {
namespace viewer {

//--------------------------------------------------------------------------------------------------
// Public member functions

StatePresentationViewer::StatePresentationViewer(tt::code::StateMachine* p_stateMachine)
:
Gwen::Event::Handler(),
tt::code::State(p_stateMachine),
m_camera(0),
m_presentationMgr(),
m_presentationObject(),
m_presentationFilename(),
m_presentationFileTimestamp(0),
m_nameToStart(),
m_renderPass(),
m_tagsToStart(),
m_spacebarScrollMode(),
m_grid(),
m_fileList(),
m_gwenRoot(0),
m_ui(),
m_callbackView(10, tt::math::Point2(10, 10), 10.0f),
m_mipBias(0.0f)
{
}


StatePresentationViewer::~StatePresentationViewer()
{
}


void StatePresentationViewer::enter()
{
	// Set up the presentation system
	m_presentationMgr = tt::pres::PresentationMgr::create(
		tt::pres::Tags(),
		tt::pres::TriggerFactoryInterfacePtr(new pres::TriggerFactory)/*,
		particles::ParticleCategory_Game,
		tt::pres::GroupFactoryInterfacePtr(new tt::pres::GroupFactory<pres::CustomPresObjLess>)*/);
	
	m_camera = new game::Camera("toki.camera.game");
	
	m_spacebarScrollMode.reset();
	
	createGrid();
	setupUi();
	
	tt::engine::renderer::Renderer::getInstance()->setZBufferEnabled(false);
	tt::engine::renderer::Renderer::getInstance()->setClearColor(
			tt::engine::renderer::ColorRGBA(127, 127, 127, 255));
}


void StatePresentationViewer::exit()
{
	m_presentationObject.reset();
	m_presentationFilename.clear();
	m_presentationFileTimestamp = 0;
	m_presentationMgr.reset();
	tt::code::helpers::safeDelete(m_camera);
	m_grid.reset();
	
	tt::code::helpers::safeDelete(m_gwenRoot);
	m_ui = UI();
	
	m_fileList.clear();
	
	tt::engine::renderer::Renderer::getInstance()->setClearColor(tt::engine::renderer::ColorRGB::black);
}


void StatePresentationViewer::update(real p_deltaTime)
{
	const input::Controller::State&              inputState(        AppGlobal::getController(tt::input::ControllerIndex_One).cur);
	const input::Controller::State&              previousInputState(AppGlobal::getController(tt::input::ControllerIndex_One).prev);
	const input::Controller::State::EditorState& editorState(       inputState.editor);
	
	bool inputHandledByGwen = m_gwenRoot->handleInput(editorState.pointer, previousInputState.editor.pointer,
	                                                  editorState.pointerLeft, editorState.pointerRight,
	                                                  inputState.wheelNotches * 120, m_ui.rootDock);
	
	
	const tt::math::Vector2 pointerWorldPos(m_camera->screenToWorld(inputState.pointer));
	
	// Handle "spacebar scrolling"
	const tt::input::Button& spaceBar(editorState.keys[tt::input::Key_Space]);
	
	if ((editorState.pointerLeft.pressed && spaceBar.down) ||
	    editorState.pointerMiddle.pressed ||
	    inputState.panCamera.pressed)
	{
		m_spacebarScrollMode.update(true);
	}
	else if (m_spacebarScrollMode.down &&
	         (editorState.pointerLeft.down   == false &&
	          editorState.pointerMiddle.down == false &&
	          inputState.panCamera.down      == false))
	{
		m_spacebarScrollMode.update(false);
	}
	else
	{
		m_spacebarScrollMode.update(m_spacebarScrollMode.down);
	}
	
	static tt::math::Vector2 levelDragStartInputPos;
	if (m_spacebarScrollMode.pressed)
	{
		levelDragStartInputPos = pointerWorldPos;
	}
	else if (m_spacebarScrollMode.down)
	{
		const tt::math::Vector2 distanceWorld(levelDragStartInputPos - pointerWorldPos);
		m_camera->setPosition(m_camera->getCurrentPosition() + distanceWorld, true);
	}
	else if (m_spacebarScrollMode.released)
	{
		tt::math::Vector2 diff(AppGlobal::getController(tt::input::ControllerIndex_One).cur.pointer -
		                       AppGlobal::getController(tt::input::ControllerIndex_One).prev.pointer);
		if (diff.lengthSquared() > 25) // 5 pixels deadzone. (Check against 5^2). FIXME: Add to cfg.
		{
			const tt::math::Vector2 distanceWorld(levelDragStartInputPos - pointerWorldPos);
			m_camera->initSpeedFromPos(m_camera->getCurrentPosition() + distanceWorld);
		}
	}
	
	if (editorState.keys[tt::input::Key_Control].down &&
	    editorState.keys[tt::input::Key_0].pressed)
	{
		m_camera->setPosition(tt::math::Vector2::zero);
	}
	
	// Update camera FOV
	s32 fovChange = inputHandledByGwen ? 0 : (-inputState.wheelNotches * 5);
	
	if (editorState.keys[tt::input::Key_Alt  ].down == false &&
	    editorState.keys[tt::input::Key_Shift].down == false)
	{
		if (editorState.keys[tt::input::Key_Control].down)
		{
			if (editorState.keys[tt::input::Key_Plus].pressed)
			{
				fovChange -= 10;
			}
			if (editorState.keys[tt::input::Key_Minus].pressed)
			{
				fovChange += 10;
			}
		}
		else
		{
			if (editorState.keys[tt::input::Key_PageUp].pressed)
			{
				fovChange -= 10;
			}
			if (editorState.keys[tt::input::Key_PageDown].pressed)
			{
				fovChange += 10;
			}
		}
	}
	
	if (fovChange != 0)
	{
		m_camera->setFOV(m_camera->getTargetFOV() + fovChange);
	}
	
	m_camera->update(p_deltaTime);
	
	audio::AudioPlayer::getInstance()->setListenerPosition(tt::math::Vector3(
		m_camera->getCurrentPositionWithEffects().x,
		m_camera->getCurrentPositionWithEffects().y,
		10.0f));
	
	
	if (editorState.keys[tt::input::Key_Enter].pressed &&
	    m_presentationObject != 0)
	{
		m_presentationObject->stop();
		m_presentationObject->start(m_tagsToStart, false, m_nameToStart);
	}
	
	if (editorState.keys[tt::input::Key_LeftSquareBracket ].pressed ||
	    editorState.keys[tt::input::Key_RightSquareBracket].pressed)
	{
		m_ui.flipHorizontal->Checkbox()->Toggle();
	}
	
	if (editorState.keys[tt::input::Key_G].pressed)
	{
		m_ui.showGrid->Checkbox()->Toggle();
	}

	if (editorState.keys[tt::input::Key_M].pressed)
	{
		if(editorState.keys[tt::input::Key_Shift].down)
		{
			tt::engine::renderer::Texture::restoreMipmapLodBias();
			m_mipBias = 0.0f;
		}
		else
		{
			m_mipBias += 1.0f;
			if(m_mipBias > 16.0f) m_mipBias = 0.0f;

			tt::engine::renderer::Texture::forceMipmapLodBias(m_mipBias);
		}
	}
	
	if (m_presentationObject != 0)
	{
		std::string playStatusStr = (m_presentationObject->isLooping()) ? "Looping - " : "";
		playStatusStr += m_presentationObject->isActive() ? "Playing." : "Idle.";
		
		m_ui.playStatus->SetText(playStatusStr);
	}
	else
	{
		m_ui.playStatus->SetText("Nothing loaded.");
	}
	
	m_presentationMgr->update(p_deltaTime);
	
	tt::engine::particles::ParticleMgr::getInstance()->update(p_deltaTime);

	m_callbackView.update(p_deltaTime);
}


void StatePresentationViewer::updateForRender(real p_deltaTime)
{
	tt::engine::particles::ParticleMgr::getInstance()->updateForRender(0);
	m_presentationMgr->updateForRender(p_deltaTime);
}


void StatePresentationViewer::render()
{
	tt::engine::renderer::Renderer* renderer = tt::engine::renderer::Renderer::getInstance();
	renderer->setZBufferEnabled(false);
	
	if (m_ui.showGrid->Checkbox()->IsChecked())
	{
		m_grid->render();
	}
	
	renderer->setCullingEnabled(false);
	
	if (m_renderPass.empty())
	{
		m_presentationMgr->render();
	}
	else
	{
		m_presentationMgr->renderPass(m_renderPass);
	}
	
	renderer->setCullingEnabled(true);
	
	tt::engine::particles::ParticleMgr::getInstance()->renderAllGroups();
	
	if (m_presentationObject != 0 && m_presentationObject->isMissingFrame())
	{
		tt::engine::debug::DebugRendererPtr debug(renderer->getDebug());
		
		using tt::math::Vector2;
		using tt::math::Vector3;
		using tt::engine::renderer::ColorRGB;
		using tt::engine::renderer::ColorRGBA;
		
		static const ColorRGBA circleColor(255, 150, 150, 128);
		static const ColorRGBA lineColor(ColorRGB::yellow);
		
		debug->renderSolidCircle(circleColor, circleColor, Vector2::zero, 1.0f);
		debug->renderLine(lineColor, Vector3(-1.0f, -1.0f, 0.0f), Vector3( 1.0f,  1.0f, 0.0f));
		debug->renderLine(lineColor, Vector3(-1.0f,  1.0f, 0.0f), Vector3( 1.0f, -1.0f, 0.0f));
	}

	if(m_ui.showCallbacks->Checkbox()->IsChecked())
	{
		m_callbackView.render();
	}
	
	m_gwenRoot->render();
}


void StatePresentationViewer::handleVBlankInterrupt()
{
}


tt::code::StateID StatePresentationViewer::getPathToState(tt::code::StateID p_targetState) const
{
	return p_targetState;
}


void StatePresentationViewer::onAppActive()
{
	// Reload the namespace mapping
	tt::engine::file::FileUtils::getInstance()->generateNamespaceMapping();
	
	refreshFileList();
	
	// Check if the presentation file was changed (reload if so)
	if (m_presentationFilename.empty() == false &&
	    tt::fs::fileExists(m_presentationFilename + ".pres"))
	{
		tt::fs::FilePtr file = tt::fs::open(m_presentationFilename + ".pres", tt::fs::OpenMode_Read);
		if (file != 0 && file->getWriteTime() != m_presentationFileTimestamp)
		{
			file.reset();
			loadPresentationFile(m_presentationFilename, true);
		}
	}
}


void StatePresentationViewer::onResetDevice()
{
	tt::engine::renderer::Renderer::getInstance()->setZBufferEnabled(false);
}


void StatePresentationViewer::onRequestReloadAssets()
{
	// Force removal of permanent cache objects; otherwise they won't get reloaded
	tt::pres::PresentationCache::setRemovePermanentObjectsEnabled(true);
	
	// Reload the namespace mapping
	tt::engine::file::FileUtils::getInstance()->generateNamespaceMapping();
	
	refreshFileList();
	
	// Reload the current presentation file unconditionally
	if (m_presentationFilename.empty() == false)
	{
		if (tt::fs::fileExists(m_presentationFilename + ".pres"))
		{
			loadPresentationFile(m_presentationFilename, true);
		}
		else
		{
			TT_PANIC("Cannot reload presentation file '%s', because it no longer exists.",
			         m_presentationFilename.c_str());
		}
	}
	
	// Disable removal of permanent cache objects again
	tt::pres::PresentationCache::setRemovePermanentObjectsEnabled(false);
}


void StatePresentationViewer::callback(const std::string& p_data,
                                       const tt::pres::PresentationObjectPtr& /*p_object*/)
{
	m_callbackView.addLine(game::TextLine(p_data, tt::engine::renderer::ColorRGB::green));
}


//--------------------------------------------------------------------------------------------------
// Private member functions

void StatePresentationViewer::setupUi()
{
	TT_ASSERT(m_gwenRoot == 0);
	m_gwenRoot = new tt::gwen::RootCanvasWrapper("rootCanvas", "gwen_skin");
	
	m_ui.rootDock = new Gwen::Controls::DockBase(m_gwenRoot->getCanvas());
	m_ui.rootDock->Dock(Gwen::Pos::Fill);
	
	
	Gwen::Controls::DockBase* dock = m_ui.rootDock->GetRight();
	
	{
		Gwen::Controls::ScrollControl* panel = new Gwen::Controls::ScrollControl(dock);
		m_ui.contentPanel = panel;
		panel->SetAutoHideBars(true);
	}
	
	
	const s32 dockWidth = 163;
	const s32 spacingY  = 10;
	const s32 x         = 2;
	s32 y = 0;
	
	Gwen::Controls::Label* label = new Gwen::Controls::Label(m_ui.contentPanel);
	label->SetText("Presentation Files:");
	label->SizeToContents();
	label->SetWidth(dockWidth);
	label->SetPos(x, y);
	y += label->Height();
	
	m_ui.fileList = new Gwen::Controls::ListBox(m_ui.contentPanel);
	m_ui.fileList->SetSize(dockWidth, 250);
	m_ui.fileList->SetPos(x, y);
	m_ui.fileList->onRowSelected.Add(this, &StatePresentationViewer::onFileSelected);
	y += m_ui.fileList->Height() + spacingY;
	
	m_ui.showGrid = new Gwen::Controls::CheckBoxWithLabel(m_ui.contentPanel);
	m_ui.showGrid->SetPos(x, y);
	m_ui.showGrid->Label()->SetText("Show Grid");
	m_ui.showGrid->Checkbox()->SetChecked(true);
	m_ui.showGrid->SizeToChildren();
	m_ui.showGrid->SetWidth(75);
	//y += m_ui.showGrid->Height() + 5;
	
	m_ui.flipHorizontal = new Gwen::Controls::CheckBoxWithLabel(m_ui.contentPanel);
	m_ui.flipHorizontal->SetPos(x + (dockWidth / 2), y);
	m_ui.flipHorizontal->Label()->SetText("Flip");
	m_ui.flipHorizontal->Checkbox()->onCheckChanged.Add(this, &StatePresentationViewer::onFlipHorizontalChanged);
	m_ui.flipHorizontal->SizeToChildren();
	m_ui.flipHorizontal->SetWidth(dockWidth / 2);
	y += m_ui.flipHorizontal->Height() + 5;

	m_ui.showCallbacks = new Gwen::Controls::CheckBoxWithLabel(m_ui.contentPanel);
	m_ui.showCallbacks->SetPos(x, y);
	m_ui.showCallbacks->Label()->SetText("Show Callbacks");
	m_ui.showCallbacks->Checkbox()->SetChecked(true);
	m_ui.showCallbacks->SizeToChildren();
	y += m_ui.showCallbacks->Height() + 5;
	
	// Floor direction combo box
	label = new Gwen::Controls::Label(m_ui.contentPanel);
	label->SetText("Floor Dir:");
	label->SizeToContents();
	label->SetWidth(55);
	label->SetPos(x, y);
	
	{
		m_ui.floorDirection = new Gwen::Controls::ComboBox(m_ui.contentPanel);
		
		Gwen::Controls::MenuItem* itemToSelect = 0;
		
		for (s32 i = 0; i < game::movement::Direction_Count; ++i)
		{
			const game::movement::Direction dir = static_cast<game::movement::Direction>(i);
			const std::string dirName(game::movement::getDirectionName(dir));
			Gwen::Controls::MenuItem* item =
					m_ui.floorDirection->AddItem(tt::str::widen(dirName), dirName);
			
			if (dir == game::movement::Direction_Down)
			{
				itemToSelect = item;
			}
		}
		
		if (itemToSelect != 0)
		{
			m_ui.floorDirection->OnItemSelected(itemToSelect);
		}
		
		m_ui.floorDirection->onSelection.Add(this, &StatePresentationViewer::onFloorDirectionSelected);
		m_ui.floorDirection->SizeToContents();
		m_ui.floorDirection->SetWidth(dockWidth - 55);
		m_ui.floorDirection->SetPos(x + 55, y);
		y += m_ui.floorDirection->Height() + spacingY;
	}
	
	Gwen::Controls::Button* button = new Gwen::Controls::Button(m_ui.contentPanel);
	button->SetPos(x, y);
	button->SetSize(dockWidth, 20);
	button->SetText("Change Background Color");
	button->onPress.Add(this, &StatePresentationViewer::onPickBgColor);
	y += button->Height() + spacingY;
	
	{
		Gwen::Controls::GroupBox* playControls = new Gwen::Controls::GroupBox(m_ui.contentPanel);
		playControls->SetPos(x, y);
		playControls->SetSize(dockWidth, 50);
		playControls->SetText("Playback");
		
		m_ui.playStatus = new Gwen::Controls::Label(playControls);
		m_ui.playStatus->SetPos(5, 5);
		m_ui.playStatus->SetText("Nothing loaded.");
		m_ui.playStatus->SizeToContents();
		m_ui.playStatus->SetWidth(150);
		
		y += playControls->Height() + spacingY;
	}
	
	label = new Gwen::Controls::Label(m_ui.contentPanel);
	label->SetText("Animation Name:");
	label->SizeToContents();
	label->SetWidth(dockWidth);
	label->SetPos(x, y);
	y += label->Height();
	
	m_ui.namesList = new Gwen::Controls::ListBox(m_ui.contentPanel);
	m_ui.namesList->SetPos(x, y);
	m_ui.namesList->SetSize(dockWidth, 300);
	m_ui.namesList->onRowSelected.Add(this, &StatePresentationViewer::onNameSelected);
	y += m_ui.namesList->Height() + spacingY;
	
	label = new Gwen::Controls::Label(m_ui.contentPanel);
	label->SetText("Render Pass:");
	label->SizeToContents();
	label->SetWidth(dockWidth);
	label->SetPos(x, y);
	y += label->Height();
	
	m_ui.renderPassList = new Gwen::Controls::ListBox(m_ui.contentPanel);
	m_ui.renderPassList->SetPos(x, y);
	m_ui.renderPassList->SetSize(dockWidth, 50);
	m_ui.renderPassList->onRowSelected.Add(this, &StatePresentationViewer::onRenderPassSelected);
	y += m_ui.renderPassList->Height() + spacingY;
	
	m_ui.tagsGroup = new Gwen::Controls::GroupBox(m_ui.contentPanel);
	m_ui.tagsGroup->SetPos(x, y);
	m_ui.tagsGroup->SetSize(dockWidth, 50);
	m_ui.tagsGroup->SetText("Tags To Add");
	
	
	dock->GetTabControl()->AddPage("Presentation Viewer", m_ui.contentPanel);
	
	
	refreshFileList();
}


void StatePresentationViewer::createGrid()
{
	enum
	{
		Grid_LinesFromCenter = 15,
		Grid_LinesPerAxis    = (2 * Grid_LinesFromCenter) + 1,
		Grid_TotalLineCount  = 2 * Grid_LinesPerAxis,
		Grid_VertexCount     = 2 * Grid_TotalLineCount // (2 vertices per line)
	};
	static const real gridSpacing = 1.0f;
	
	m_grid.reset(new tt::engine::renderer::TrianglestripBuffer(
		Grid_VertexCount,
		1,
		tt::engine::renderer::TexturePtr(),
		tt::engine::renderer::BatchFlagTrianglestrip_UseVertexColor,
		tt::engine::renderer::TrianglestripBuffer::PrimitiveType_Lines));
	
	tt::engine::renderer::BufferVtxUV<1> defaultVertex;
	defaultVertex.setColor(100, 100, 100, 127);
	m_grid->resize<1>(Grid_VertexCount, defaultVertex);
	
	const real minPos = -Grid_LinesFromCenter * gridSpacing;
	const real maxPos =  Grid_LinesFromCenter * gridSpacing;
	
	s32 vertexIndex = 0;
	
	// Center horizontal line
	m_grid->modifyVtx<1>(vertexIndex).setPosition(minPos, 0.0f, 0.0f);
	m_grid->modifyVtx<1>(vertexIndex).setColor(150, 150, 150, 255);
	++vertexIndex;
	
	m_grid->modifyVtx<1>(vertexIndex).setPosition(maxPos, 0.0f, 0.0f);
	m_grid->modifyVtx<1>(vertexIndex).setColor(150, 150, 150, 255);
	++vertexIndex;
	
	// Center vertical line
	m_grid->modifyVtx<1>(vertexIndex).setPosition(0.0f, minPos, 0.0f);
	m_grid->modifyVtx<1>(vertexIndex).setColor(150, 150, 150, 255);
	++vertexIndex;
	
	m_grid->modifyVtx<1>(vertexIndex).setPosition(0.0f, maxPos, 0.0f);
	m_grid->modifyVtx<1>(vertexIndex).setColor(150, 150, 150, 255);
	++vertexIndex;
	
	// All lines outward from center
	for (s32 i = 0; i < Grid_LinesFromCenter; ++i)
	{
		const real posOffset = (i + 1) * gridSpacing;
		
		// Horizontal line, left
		m_grid->modifyVtx<1>(vertexIndex).setPosition(minPos, -posOffset, 0.0f);  ++vertexIndex;
		m_grid->modifyVtx<1>(vertexIndex).setPosition(maxPos, -posOffset, 0.0f);  ++vertexIndex;
		
		// Horizontal line, right
		m_grid->modifyVtx<1>(vertexIndex).setPosition(minPos, posOffset, 0.0f);  ++vertexIndex;
		m_grid->modifyVtx<1>(vertexIndex).setPosition(maxPos, posOffset, 0.0f);  ++vertexIndex;
		
		// Vertical line, top
		m_grid->modifyVtx<1>(vertexIndex).setPosition(posOffset, minPos, 0.0f);  ++vertexIndex;
		m_grid->modifyVtx<1>(vertexIndex).setPosition(posOffset, maxPos, 0.0f);  ++vertexIndex;
		
		// Vertical line, bottom
		m_grid->modifyVtx<1>(vertexIndex).setPosition(-posOffset, minPos, 0.0f);  ++vertexIndex;
		m_grid->modifyVtx<1>(vertexIndex).setPosition(-posOffset, maxPos, 0.0f);  ++vertexIndex;
	}
	
	m_grid->applyChanges();
}


void StatePresentationViewer::refreshFileList()
{
	// Get the current list of files and early out if this list didn't change from the previous call
	tt::str::Strings fullList(tt::fs::utils::getRecursiveFileList("presentation/", "*.pres"));
	if (fullList == m_fileList)
	{
		// List didn't change; do not refresh
		return;
	}
	
	m_fileList = fullList;
	
	// Get the name of the currently selected entry, so that it can be restored after repopulating the list
	std::string selectedEntryName;
	if (m_ui.fileList->GetSelectedRow() != 0)
	{
		selectedEntryName = m_ui.fileList->GetSelectedRow()->GetName();
	}
	
	// Repopulate the list of files
	m_ui.fileList->Clear();
	
	Gwen::Controls::Layout::TableRow* rowToReselect = 0;
	for (tt::str::Strings::iterator it = fullList.begin(); it != fullList.end(); ++it)
	{
		const std::string rowName("presentation/" + *it);
		Gwen::Controls::Layout::TableRow* row = m_ui.fileList->AddItem(*it, rowName);
		if (rowName == selectedEntryName)
		{
			rowToReselect = row;
		}
	}
	
	// If an entry was selected before, restore that selection (if it is still a valid entry)
	if (rowToReselect != 0)
	{
		m_ui.fileList->onRowSelected.RemoveHandler(this);
		m_ui.fileList->SetSelectedRow(rowToReselect);
		m_ui.fileList->onRowSelected.Add(this, &StatePresentationViewer::onFileSelected);
	}
}


void StatePresentationViewer::loadPresentationFile(const std::string& p_filename,
                                                   bool               p_keepSelectedTags)
{
	const std::string fullFilename(p_filename + ".pres");
	if (tt::fs::fileExists(fullFilename) == false)
	{
		TT_PANIC("File '%s' does not exist. Cannot load presentation data from it.", fullFilename.c_str());
		return;
	}
	
	{
		tt::fs::FilePtr file = tt::fs::open(fullFilename, tt::fs::OpenMode_Read);
		if (file == 0)
		{
			TT_PANIC("Opening file '%s' for reading failed. Cannot load presentation data from it.",
			         fullFilename.c_str());
			return;
		}
		
		m_presentationFileTimestamp = file->getWriteTime();
	}
	
	m_presentationFilename = p_filename;
	
	m_presentationObject.reset(); // reset separately to negate any possible caching effects
	m_presentationObject = m_presentationMgr->createPresentationObject(p_filename);
	
	// Remove all previous tag checkboxes
	for (CheckBoxes::iterator it = m_ui.tags.begin(); it != m_ui.tags.end(); ++it)
	{
		(*it)->DelayedDelete();
	}
	m_ui.tags.clear();
	
	// If presentation file failed to load, clean up the UI and skip the rest
	if (m_presentationObject == 0)
	{
		m_tagsToStart.clear();
		m_nameToStart.clear();
		m_renderPass.clear();
		m_ui.tagsGroup->SetHeight(50);
		return;
	}
	
	m_presentationObject->setCallbackInterface(tt::pres::CallbackTriggerInterfacePtr(
			this, 
			tt::code::NoDeleter<tt::pres::CallbackTriggerInterface>()));
	
	// Update the UI for the new presentation file
	
	tt::pres::Tags allTags(m_presentationObject->getTags().getAllUsedTags());
	
	// Update the set of tags to play
	if (p_keepSelectedTags == false)
	{
		m_tagsToStart.clear();
	}
	else
	{
		// Get rid of tags that are no longer present in the file
		for (tt::pres::Tags::iterator it = m_tagsToStart.begin(); it != m_tagsToStart.end(); )
		{
			if (allTags.find(*it) == allTags.end())
			{
				it = m_tagsToStart.erase(it);
			}
			else
			{
				++it;
			}
		}
	}
	
	// Apply GUI settings to newly loaded object
	setScaleFromUI   (m_presentationObject, m_ui.flipHorizontal->Checkbox());
	setRotationFromUI(m_presentationObject, m_ui.floorDirection);
	
	// Add names to the name list
	tt::str::StringSet names;
#if !defined TT_BUILD_FINAL
	names = m_presentationObject->getTags().getAllUsedNames();
#endif
	m_ui.namesList->Clear();
	m_ui.namesList->AddItem("<no name>", "");
	
	for (tt::str::StringSet::iterator it = names.begin(); it != names.end(); ++it)
	{
		if (it->empty() == false)
		{
			m_ui.namesList->AddItem(*it, *it);
		}
	}
	
	// Add names to the render pass list
	tt::str::StringSet passNames(m_presentationObject->getRenderPassNames());
	
	m_ui.renderPassList->Clear();
	m_ui.renderPassList->AddItem("<no render pass>", "");
	
	for (tt::str::StringSet::iterator it = passNames.begin(); it != passNames.end(); ++it)
	{
		if (it->empty() == false)
		{
			m_ui.renderPassList->AddItem(*it, *it);
		}
	}
	
	// Add checkboxes for the new tags
	tt::str::StringSet tagNames;
	for (tt::pres::Tags::iterator it = allTags.begin(); it != allTags.end(); ++it)
	{
		tagNames.insert((*it).getName());
	}
	
	static const s32 spacingY = 23;
	m_ui.tagsGroup->SetHeight(tagNames.empty() ? 50 : static_cast<int>((tagNames.size() * spacingY) + 35));
	
	s32 y = 5;
	for (tt::str::StringSet::iterator it = tagNames.begin(); it != tagNames.end(); ++it)
	{
		Gwen::Controls::CheckBoxWithLabel* tag = new Gwen::Controls::CheckBoxWithLabel(m_ui.tagsGroup);
		tag->Label()->SetText(*it);
		tag->Checkbox()->SetName(*it);
		tag->Checkbox()->SetChecked(m_tagsToStart.find(tt::pres::Tag(*it)) != m_tagsToStart.end());
		tag->Checkbox()->onCheckChanged.Add(this, &StatePresentationViewer::onTagCheckBoxChanged);
		tag->SizeToChildren();
		tag->SetPos(5, y);
		y += spacingY;
		
		m_ui.tags.push_back(tag);
	}
}


void StatePresentationViewer::setScaleFromUI(const tt::pres::PresentationObjectPtr& p_presentation,
                                             Gwen::Controls::CheckBox*              p_checkBox)
{
	if (p_presentation == 0 || p_checkBox == 0)
	{
		return;
	}
	
	m_presentationObject->setFlipMask(p_checkBox->IsChecked() ?
		tt::pres::PresentationObject::FlipMask_Horizontal : tt::pres::PresentationObject::FlipMask_None);
}


void StatePresentationViewer::setRotationFromUI(const tt::pres::PresentationObjectPtr& p_presentation,
                                                Gwen::Controls::ComboBox*              p_comboBox)
{
	if (p_presentation == 0 || p_comboBox == 0)
	{
		return;
	}
	
	Gwen::Controls::Label* selectedItem = p_comboBox->GetSelectedItem();
	if (selectedItem == 0)
	{
		return;
	}
	
	const std::string directionName(selectedItem->GetName());
	const game::movement::Direction direction = game::movement::getDirectionFromName(directionName);
	if (game::movement::isValidDirection(direction) == false)
	{
		TT_PANIC("Internal error: combo box item '%s' is not a valid direction.",
		         directionName.c_str());
		return;
	}
	
	m_presentationObject->setRotation(game::entity::getOrientationQuaternion(direction));
}


void StatePresentationViewer::onFileSelected(Gwen::Controls::Base* p_sender)
{
	Gwen::Controls::ListBox* fileList = gwen_cast<Gwen::Controls::ListBox>(p_sender);
	if (fileList == 0)
	{
		return;
	}
	
	Gwen::Controls::Layout::TableRow* selectedRow = fileList->GetSelectedRow();
	TT_NULL_ASSERT(selectedRow);
	if (selectedRow != 0)
	{
		loadPresentationFile(selectedRow->GetName(), false);
	}
}


void StatePresentationViewer::onFlipHorizontalChanged(Gwen::Controls::Base* p_sender)
{
	setScaleFromUI(m_presentationObject, gwen_cast<Gwen::Controls::CheckBox>(p_sender));
}


void StatePresentationViewer::onFloorDirectionSelected(Gwen::Controls::Base* p_sender)
{
	setRotationFromUI(m_presentationObject, gwen_cast<Gwen::Controls::ComboBox>(p_sender));
}


void StatePresentationViewer::onPickBgColor(Gwen::Controls::Base* p_sender)
{
	Gwen::Controls::Menu* menu = new Gwen::Controls::Menu(p_sender->GetCanvas());
	menu->SetSize(256, 180);
	menu->SetDeleteOnClose(true);
	menu->SetDisableIconMargin(true);
	
	Gwen::Controls::RGBColorPicker* picker = new Gwen::Controls::RGBColorPicker(menu);
	picker->Dock(Gwen::Pos::Fill);
	picker->SetSize(256, 128);
	
	const tt::engine::renderer::ColorRGBA bgColor(
		tt::engine::renderer::Renderer::getInstance()->getClearColor());
	
	picker->SetColor(Gwen::Color(bgColor.r, bgColor.g, bgColor.b, 255), false, true);
	picker->onColorChanged.Add(this, &StatePresentationViewer::onBgColorChanged);
	
	// FIXME: Open to the left if no room remains on the right (same for the opposite situation)
	menu->Open(Gwen::Pos::Right | Gwen::Pos::Top);
	//menu->Open(Gwen::Pos::Left | Gwen::Pos::Top);
}


void StatePresentationViewer::onBgColorChanged(Gwen::Controls::Base* p_sender)
{
	Gwen::Controls::RGBColorPicker* picker = gwen_cast<Gwen::Controls::RGBColorPicker>(p_sender);
	if (picker != 0)
	{
		const Gwen::Color pickedColor(picker->GetColor());
		
		tt::engine::renderer::Renderer::getInstance()->setClearColor(
			tt::engine::renderer::ColorRGB(pickedColor.r, pickedColor.g, pickedColor.b));
	}
}


void StatePresentationViewer::onTagCheckBoxChanged(Gwen::Controls::Base* p_sender)
{
	Gwen::Controls::CheckBox* checkBox = gwen_cast<Gwen::Controls::CheckBox>(p_sender);
	if (checkBox == 0)
	{
		return;
	}
	
	if (checkBox->IsChecked())
	{
		// Add tag to set
		m_tagsToStart.insert(tt::pres::Tag(checkBox->GetName()));
	}
	else
	{
		// Remove tag from set
		m_tagsToStart.erase(tt::pres::Tag(checkBox->GetName()));
	}
}


void StatePresentationViewer::onNameSelected(Gwen::Controls::Base* p_sender)
{
	Gwen::Controls::ListBox* nameList = gwen_cast<Gwen::Controls::ListBox>(p_sender);
	if (nameList == 0)
	{
		return;
	}
	
	Gwen::Controls::Layout::TableRow* selectedRow = nameList->GetSelectedRow();
	TT_NULL_ASSERT(selectedRow);
	if (selectedRow != 0)
	{
		m_nameToStart = selectedRow->GetName();
	}
}


void StatePresentationViewer::onRenderPassSelected(Gwen::Controls::Base* p_sender)
{
	Gwen::Controls::ListBox* nameList = gwen_cast<Gwen::Controls::ListBox>(p_sender);
	if (nameList == 0)
	{
		return;
	}
	
	Gwen::Controls::Layout::TableRow* selectedRow = nameList->GetSelectedRow();
	TT_NULL_ASSERT(selectedRow);
	if (selectedRow != 0)
	{
		m_renderPass = selectedRow->GetName();
	}
}

// Namespace end
}
}
