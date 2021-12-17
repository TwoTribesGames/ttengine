#include <iomanip>
#include <sstream>

#include <Gwen/Controls/Layout/Position.h>
#include <Gwen/Controls/CheckBox.h>
#include <Gwen/Controls/ComboBox.h>
#include <Gwen/Controls/DockBase.h>
#include <Gwen/Controls/HorizontalSlider.h>
#include <Gwen/Controls/MenuItem.h>
#include <Gwen/Controls/TabControl.h>
#include <Gwen/ToolTip.h>

#include <tt/app/Application.h>
#include <tt/engine/debug/DebugRenderer.h>
#include <tt/engine/glyph/GlyphSet.h>
#include <tt/engine/renderer/ColorRGBA.h>
#include <tt/engine/renderer/QuadSprite.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/Texture.h> 
#include <tt/engine/renderer/TextureHardware.h>
#include <tt/engine/renderer/TexturePainter.h>
#include <tt/engine/renderer/TrianglestripBuffer.h>
#include <tt/engine/renderer/UpScaler.h>
#include <tt/fs/utils/utils.h>
#include <tt/gwen/EqualSizes.h>
#include <tt/gwen/ListBoxEx.h>
#include <tt/http/HttpConnectMgr.h>
#if defined(TT_PLATFORM_WIN) || defined(TT_PLATFORM_OSX_MAC) || defined(TT_PLATFORM_LNX)
#include <tt/input/MouseController.h>
#endif
#include <tt/math/math.h>
#include <tt/mem/util.h>
#include <tt/platform/tt_printf.h>
#include <tt/steam/helpers.h>
#include <tt/str/str.h>
#include <tt/str/StringFormatter.h>
#include <tt/system/Calendar.h>
#include <tt/system/utils.h>
#include <tt/undo/UndoStack.h>

#include <toki/audio/AudioPlayer.h>
#include <toki/game/editor/commands/CommandAddRemoveEntities.h>
#include <toki/game/editor/commands/CommandAddRemoveNotes.h>
#include <toki/game/editor/commands/CommandApplyLevelSection.h>
#include <toki/game/editor/commands/CommandPaintTiles.h>
#include <toki/game/editor/commands/CommandReplaceLevel.h>
#include <toki/game/editor/commands/CommandSetDefaultMission.h>
#include <toki/game/editor/commands/CommandSetLevelBackground.h>
#include <toki/game/editor/commands/CommandSetLevelTheme.h>
#include <toki/game/editor/tools/BoxSelectTool.h>
#include <toki/game/editor/tools/EntityMoveTool.h>
#include <toki/game/editor/tools/EntityPaintTool.h>
#include <toki/game/editor/tools/FloodFillTool.h>
#include <toki/game/editor/tools/HandTool.h>
#include <toki/game/editor/tools/NotesTool.h>
#include <toki/game/editor/tools/PaintTool.h>
#include <toki/game/editor/tools/ResizeTool.h>
#include <toki/game/editor/ui/DebugSteamCloudFileManager.h>
#include <toki/game/editor/ui/EntityPropertyList.h>
#include <toki/game/editor/ui/GenericDialogBox.h>
#include <toki/game/editor/ui/helpers.h>
#include <toki/game/editor/ui/NewLevelDialog.h>
#include <toki/game/editor/ui/PublishedLevelBrowser.h>
#include <toki/game/editor/ui/PublishToWorkshopDialog.h>
#include <toki/game/editor/ui/SaveAsDialog.h>
#include <toki/game/editor/ui/SaveChangesDialog.h>
#include <toki/game/editor/ui/SetThemeColorUI.h>
#include <toki/game/editor/ui/SteamWorkshopTest.h>
#include <toki/game/editor/ui/SvnCommands.h>
#include <toki/game/editor/ui/Testbed.h>
#include <toki/game/editor/Editor.h>
#include <toki/game/editor/EditorSettings.h>
#include <toki/game/editor/helpers.h>
#include <toki/game/entity/EntityMgr.h>
#include <toki/game/script/EntityScriptMgr.h>
#include <toki/game/AttributeDebugView.h>
#include <toki/game/Border.h>
#include <toki/game/Game.h>
#include <toki/input/Recorder.h>
#include <toki/level/entity/editor/EntityInstanceEditorRepresentation.h>
#include <toki/level/entity/EntityProperty.h>
#include <toki/level/entity/helpers.h>
#include <toki/level/AttributeLayerSection.h>
#include <toki/level/helpers.h>
#include <toki/level/LevelData.h>
#include <toki/level/LevelSection.h>
#include <toki/level/Note.h>
#include <toki/loc/Loc.h>
#include <toki/steam/Workshop.h>
#include <toki/utils/GlyphSetMgr.h>
#include <toki/AppGlobal.h>
#include <toki/AppOptions.h>
#include <toki/cfg.h>
#include <toki/constants.h>

#if defined(TT_PLATFORM_WIN)
#include "../../../../../windows/src/resource.h"
#endif

#if defined(TT_PLATFORM_SDL)
#include <SDL2/SDL_mouse.h>
#endif

namespace toki {
namespace game {
namespace editor {

static const char* const g_editorSettingsFile = "editorsettings.json";

static const char* const g_attributeViewTextureID = "attributeview_editor";


static tt::engine::renderer::QuadSpritePtr createQuadWithText(
		const std::wstring&                    p_text,
		utils::GlyphSetID                      p_glyphSet,
		const tt::engine::renderer::ColorRGBA& p_textColor)
{
	using namespace tt::engine::glyph;
	using namespace tt::engine::renderer;
	
	GlyphSetPtr glyphSet(utils::GlyphSetMgr::get(p_glyphSet));
	if (glyphSet == 0)
	{
		TT_PANIC("Glyph set %d is not loaded. Cannot use it for rendering text.", p_glyphSet);
		return QuadSpritePtr();
	}
	
	const tt::math::Point2 textTopMargin(0, /*glyphSet->get*/ 2);  // FIXME: This doesn't work for all fonts
	const tt::math::Point2 textSize(glyphSet->getStringPixelDimensions(p_text) + textTopMargin);
	const tt::math::Point2 textureSize(TextureHardware::getRequirements().correctDimension(textSize));
	TexturePtr tex = Texture::createForText(textureSize, true);
	TT_NULL_ASSERT(tex);
	
	{
		TexturePainter painter(tex->lock());
		painter.clear();
		glyphSet->drawTruncatedString(p_text, painter, ColorRGB::white,
				GlyphSet::ALIGN_LEFT, GlyphSet::ALIGN_TOP, 0, 0, textTopMargin.y, 0, 0);
	}
	
	QuadSpritePtr quad(QuadSprite::createQuad(tex, p_textColor));
	TT_NULL_ASSERT(quad);
	quad->setFrame(textSize.x, textSize.y);
	return quad;
}


static void renderBottomLeft(const tt::engine::renderer::QuadSpritePtr& p_quad,
                             s32                                        p_marginLeft,
                             s32                                        p_marginBottom)
{
#if defined(TT_PLATFORM_WIN)
	static const real pixelPerfectOffset = 0.5f;
#else
	static const real pixelPerfectOffset = 0.0f;
#endif
	
	const s32 screenHeight = tt::engine::renderer::Renderer::getInstance()->getScreenHeight();
	const s32 x            = static_cast<s32>(               (p_quad->getWidth()  * 0.5f) + p_marginLeft);
	const s32 y            = static_cast<s32>(screenHeight - (p_quad->getHeight() * 0.5f) - p_marginBottom);
	p_quad->setPosition(x + pixelPerfectOffset, y + pixelPerfectOffset, 0.0f);
	p_quad->update();
	p_quad->render();
}



//--------------------------------------------------------------------------------------------------
// Public member functions

EditorPtr Editor::create(const level::LevelDataPtr& p_levelData)
{
	TT_NULL_ASSERT(p_levelData);
	if (p_levelData == 0)
	{
		return EditorPtr();
	}
	
	EditorPtr instance(new Editor(p_levelData));
	instance->m_this = instance;
	instance->setupNotifications();
	return instance;
}


Editor::~Editor()
{
#if EDITOR_SUPPORTS_SAVING
	// If there were unsaved changes, save a backup copy before exiting
	if (hasUnsavedChanges())
	{
		const std::string levelName(getCurrentLevelName());
		const tt::system::Calendar datetime(tt::system::Calendar::getCurrentDate());
		std::ostringstream filename;
		
		filename
#if EDITOR_SUPPORTS_ASSETS_SOURCE
		         << "../../source/shared/levels_unsavedchanges/"
#else
		         << tt::app::getApplication()->getAssetRootDir() << "levels_unsavedchanges/"
#endif
		         << levelName
		         << "_"
		         << std::setw(4) << std::setfill('0') << datetime.getYear()   << "-"
		         << std::setw(2) << std::setfill('0') << datetime.getMonth()  << "-"
		         << std::setw(2) << std::setfill('0') << datetime.getDay()    << "_"
		         << std::setw(2) << std::setfill('0') << datetime.getHour()   << "-"
		         << std::setw(2) << std::setfill('0') << datetime.getMinute() << "-"
		         << std::setw(2) << std::setfill('0') << datetime.getSecond()
		         << ".ttlvl";
		
		// Save agentradii to level data
		m_levelData->setAgentRadii(AppGlobal::getGame()->getPathMgr().getUniqueAgentRadiiForLevel(m_levelData));
		
		// Save leveldata
		m_levelData->saveAsText(filename.str());
	}
#endif
	
	saveEditorSettings();
	
	AppGlobal::getInputRecorder()->onEditorClosed();
}


void Editor::update(real p_deltaTime)
{
	if (isActive() == false)
	{
		return;
	}
	
	// For responding to keyboard focus changes:
	Gwen::Controls::Base* const prevKeyboardFocus = Gwen::KeyboardFocus;
	
	const input::Controller::State& inputState(        AppGlobal::getController(tt::input::ControllerIndex_One).cur);
	const input::Controller::State& previousInputState(AppGlobal::getController(tt::input::ControllerIndex_One).prev);
	const input::Controller::State::EditorState& editorState(inputState.editor);
	
	const tt::input::Button& spaceBar(editorState.keys[tt::input::Key_Space]);
	
	if (m_editorMode == EditorMode_Normal &&
	    Gwen::KeyboardFocus == 0 &&
	    m_openDialogBoxes.empty()) // no hotkeys when a dialog box is open
	{
		// Handle the registered hotkeys
		m_hotKeyMgr.update(editorState);
		
		//* FIXME: The code below should also be handled via HotKeys
		const bool pointerDown = editorState.pointerLeft.down   ||
		                         editorState.pointerMiddle.down ||
		                         editorState.pointerRight.down;
		const bool noModifiersDown =
			editorState.keys[tt::input::Key_Control].down == false &&
			editorState.keys[tt::input::Key_Alt    ].down == false &&
			editorState.keys[tt::input::Key_Shift  ].down == false;
		
		if (noModifiersDown && pointerDown == false)
		{
			// Handle tile selection shortcut keys
			// (keys 1 - 9 for the corresponding tile in the tile list)
			for (s32 keyIdx = tt::input::Key_1; keyIdx <= tt::input::Key_9; ++keyIdx)
			{
				if (editorState.keys[keyIdx].pressed)
				{
					const unsigned int listIdx = static_cast<int>(keyIdx - tt::input::Key_1);
					if (listIdx < m_ui.listTiles->RowCount())
					{
						m_ui.listTiles->SetSelectedRow(m_ui.listTiles->GetRow(listIdx));
						break;
					}
				}
			}
			
			// Special-case key 0 for the tenth tile (index 9) in the tile list
			if (editorState.keys[tt::input::Key_0].pressed)
			{
				const unsigned int listIdx = 9;
				if (listIdx < m_ui.listTiles->RowCount())
				{
					m_ui.listTiles->SetSelectedRow(m_ui.listTiles->GetRow(listIdx));
				}
			}
			
			// Nudge the selection using the arrow keys
			if (hasSelectionRect())
			{
				const tt::math::Point2 oldPos(m_selectionRect.getPosition());
				tt::math::Point2 newPos(oldPos);
				
				if (editorState.keys[tt::input::Key_Left].pressed)
				{
					--newPos.x;
				}
				if (editorState.keys[tt::input::Key_Right].pressed)
				{
					++newPos.x;
				}
				if (editorState.keys[tt::input::Key_Up].pressed)
				{
					++newPos.y;
				}
				if (editorState.keys[tt::input::Key_Down].pressed)
				{
					--newPos.y;
				}
				
				if (newPos != oldPos)
				{
					m_selectionRect.setPosition(newPos);
					updateSelectionIndicator();
					if (hasFloatingSection())
					{
						setFloatingSectionPosition(newPos);
					}
				}
			}
		}
		// */
	}
	
	if (m_showLoadLevelThisFrame)
	{
		m_showLoadLevelThisFrame = false;
		doShowLoadLevelDialog();
	}
	
	if (m_showEntitiesUpdatedDialog)
	{
		m_showEntitiesUpdatedDialog = false;
		
		TT_ASSERT(m_entitiesUpdatedByScript.empty() == false);
		
		std::wstring promptText;
		const s32 updateCount = static_cast<s32>(m_entitiesUpdatedByScript.size());
		if (updateCount == 1)
		{
			promptText = translateString("WINDOW_PROPERTIES_UPDATED_TEXT_SINGLE");
		}
		else
		{
			promptText = translateString("WINDOW_PROPERTIES_UPDATED_TEXT_MULTIPLE", updateCount);
		}
		
		showGenericDialog(translateString("WINDOW_PROPERTIES_UPDATED_TITLE"), promptText);
	}
	
	// GWEN Input
	
	bool inputHandledByGwen         = false;
	bool keyboardInputHandledByGwen = false;
	bool haveClickLockedTool        = false;
	for (s32 i = 0; i < PointerButton_Count; ++i)
	{
		if (tools::isValidToolID(m_clickLockedTool[i]))
		{
			haveClickLockedTool = true;
			break;
		}
	}
	
	if (m_spacebarScrollMode.down == false &&
	    haveClickLockedTool       == false &&
	    m_editorMode              == EditorMode_Normal)
	{
		inputHandledByGwen = m_gwenRoot.handleInput(editorState.pointer, previousInputState.editor.pointer,
		                                            editorState.pointerLeft, editorState.pointerRight,
		                                            inputState.wheelNotches * 120, m_ui.editorDock);
		inputHandledByGwen = (Gwen::HoveredControl != m_ui.editorDock);
		
		if (m_gwenRoot.handleKeyInput(editorState.keys, editorState.chars))
		{
			inputHandledByGwen         = true;
			keyboardInputHandledByGwen = true;
		}
	}
	
	// Block editor input handling while a dialog box is open
	if (keyboardInputHandledByGwen == false && m_openDialogBoxes.empty() == false)
	{
		inputHandledByGwen = true;
		m_openDialogBoxes.back()->handleKeyInput(editorState.keys);
	}
	
	// Middle clicking outside the GUI should also reset keyboard focus
	if (editorState.pointerMiddle.pressed && Gwen::HoveredControl == m_ui.editorDock)
	{
		Gwen::Input::Blur();
	}
	
	// Do not allow ComboBox to claim focus (since it doesn't indicate focus very well either)
	if (gwen_cast<Gwen::Controls::ComboBox>(Gwen::KeyboardFocus) != 0)
	{
		//Gwen::Input::Blur();
	}
	
	// Update the info panel
	if (m_spacebarScrollMode.down == false)
	{
		const tt::math::Vector2 worldPos(getEditorCamera().screenToWorld(editorState.pointer));
		
		/* Show world position:
		m_ui.labelInfoPosX->SetText(Gwen::Utility::Format(L"%.3f", worldPos.x));
		m_ui.labelInfoPosY->SetText(Gwen::Utility::Format(L"%.3f", worldPos.y));
		// */
		//* Show tile position (probably more useful for now):
		m_ui.labelInfoPosX->SetText(Gwen::Utility::Format(L"%d", level::worldToTile(worldPos.x)));
		m_ui.labelInfoPosY->SetText(Gwen::Utility::Format(L"%d", level::worldToTile(worldPos.y)));
		// */
		if (hasSelectionRect())
		{
			m_ui.labelInfoSelectionWidth ->SetText(Gwen::Utility::Format(L"%d", m_selectionRect.getWidth()));
			m_ui.labelInfoSelectionHeight->SetText(Gwen::Utility::Format(L"%d", m_selectionRect.getHeight()));
		}
		else
		{
			m_ui.labelInfoSelectionWidth ->SetText("");
			m_ui.labelInfoSelectionHeight->SetText("");
		}
		
		const bool haveChanges = m_undoStack->isClean() == false || m_entitiesUpdatedByScript.empty() == false;
		m_ui.labelInfoHasChanges->SetText     (haveChanges ? m_statusTextUnsaved : m_statusTextSaved);
		m_ui.labelInfoHasChanges->SetTextColor(haveChanges ? Gwen::Colors::Red   : Gwen::Colors::Black);
		
		m_ui.labelInfoEntityCount->SetText(Gwen::Utility::Format(L"%d", m_levelData->getEntityCount()));
		
		const std::string missionID(AppGlobal::getGame()->getMissionID());
		m_ui.labelInfoLevelName->SetText("LEVEL: " + getCurrentLevelName() + " | MISSION: " + (missionID.empty() ? "*" : missionID));
	}
	
	
	if (inputHandledByGwen == false)
	{
		// Handle "spacebar scrolling"
		if ((editorState.pointerLeft.pressed && spaceBar.down) ||
		    editorState.pointerMiddle.pressed)
		{
			m_spacebarScrollMode.update(true);
			switchToTool(tools::ToolID_Hand, true);
		}
		else if (m_spacebarScrollMode.down &&
		         (editorState.pointerLeft.down == false && editorState.pointerMiddle.down == false))
		{
			m_spacebarScrollMode.update(false);
		}
		else
		{
			m_spacebarScrollMode.update(m_spacebarScrollMode.down);
		}
		
		if (m_pointerWasOnCanvas == false && editorState.pointer.valid && m_editorMode == EditorMode_Normal)
		{
			// Pointer moved onto the drawing canvas
			m_pointerWasOnCanvas = true;
			if (m_tools[m_activePaintTool] != 0)
			{
				m_tools[m_activePaintTool]->onPointerEnter();
			}
		}
		handleToolInput();
		
		if (inputState.wheelNotches != 0)
		{
			changeCameraFovBy(static_cast<real>(-inputState.wheelNotches * 5));
		}
	}
	
	if (m_pointerWasOnCanvas                                       &&
	    (inputHandledByGwen || editorState.pointer.valid == false) &&
	    m_editorMode == EditorMode_Normal)
	{
		// Pointer moved off the drawing canvas
		m_pointerWasOnCanvas = false;
		if (m_tools[m_activePaintTool] != 0)
		{
			m_tools[m_activePaintTool]->onPointerLeave();
		}
	}
	
	
	if (m_spacebarScrollMode.released)
	{
		// No longer in special "spacebar scrolling" mode: restore previously active tool
		switchToTool(m_previousPaintTool);
	}
	
	if (m_floatingSectionVisual != 0)
	{
		m_floatingSectionVisual->update();
		TT_NULL_ASSERT(m_floatingSectionThemeVisual); // both visuals must exist at the same time
		m_floatingSectionThemeVisual->update();
	}
	m_levelSavedIndicator->update();
	m_levelSaveFailedIndicator->update();
	
	if (inputHandledByGwen == false)
	{
		if (m_spacebarScrollMode.down == false &&
		    haveClickLockedTool       == false &&
		    spaceBar.down)
		{
			// Priming for spacebar scrolling: already display hand cursor
			setCursor(EditCursor_OpenHand);
		}
		else
		{
			setCursor(m_activeCursor);
		}
	}
	
	m_collisionTileView->update();
	m_themeTileView->update();
	
	// Update the entity visuals
	{
		const level::entity::EntityInstances& entities(m_levelData->getAllEntities());
		for (level::entity::EntityInstances::const_iterator it = entities.begin(); it != entities.end(); ++it)
		{
			level::entity::editor::EntityInstanceEditorRepresentation* editorRep =
					(*it)->getOrCreateEditorRepresentation();
			TT_NULL_ASSERT(editorRep);
			editorRep->setPosOffset(tt::math::Vector2::zero);
			editorRep->update(p_deltaTime, m_levelData);
		}
	}
	
	if (m_floatingSection != 0)
	{
		const tt::math::Vector2 floatingSectionPos(level::tileToWorld(m_floatingSection->getPosition()));
		const level::entity::EntityInstances& entities(m_floatingSection->getEntities());
		for (level::entity::EntityInstances::const_iterator it = entities.begin(); it != entities.end(); ++it)
		{
			level::entity::editor::EntityInstanceEditorRepresentation* editorRep =
					(*it)->getOrCreateEditorRepresentation();
			TT_NULL_ASSERT(editorRep);
			editorRep->setPosOffset(floatingSectionPos);
			editorRep->update(p_deltaTime, m_levelData);
		}
	}
	
	// Update the note graphics
	{
		const level::Notes& notes(m_levelData->getAllNotes());
		for (level::Notes::const_iterator it = notes.begin(); it != notes.end(); ++it)
		{
			if (*it != m_noteForTextEdit && *it != m_focusNote)
			{
				(*it)->update(p_deltaTime);
			}
		}
		
		if (m_noteForTextEdit != 0)
		{
			m_noteForTextEdit->update(p_deltaTime);
		}
		else if (m_focusNote != 0)
		{
			m_focusNote->update(p_deltaTime);
		}
	}
	
	m_cameraMgr.update(p_deltaTime);
	
	// If GWEN received or lost keyboard focus, disable or enable app framework debug input
	if (m_editorActive &&  // editor could have been closed during this update
	    ((Gwen::KeyboardFocus != 0 && prevKeyboardFocus == 0) ||
	     (Gwen::KeyboardFocus == 0 && prevKeyboardFocus != 0)))
	{
		tt::app::getApplication()->setHandleDebugKeys(
				(Gwen::KeyboardFocus == 0) ? tt::app::DebugKeys_All : tt::app::DebugKeys_None);
	}
	
	if (Gwen::KeyboardFocus != 0)
	{
		m_ui.labelFocusStatus->SetPos(
				getEditorCamera().getViewPortSize().x - m_ui.labelFocusStatus->Width() - m_ui.labelFocusStatus->GetMargin().right,
				0);
		m_ui.labelFocusStatus->BringToFront();
		m_ui.labelFocusStatus->SetHidden(false);
	}
	else
	{
		m_ui.labelFocusStatus->SetHidden(true);
	}
	
	// Force-disable the tooltip if the hovered control (or its parent) doesn't have a tooltip
	if (Gwen::HoveredControl != 0)
	{
		// NOTE: This logic is copied from Gwen::Controls::Base::OnMouseEnter
		if (Gwen::HoveredControl->GetToolTip() == 0 &&
		    (Gwen::HoveredControl->GetParent() == 0 ||
		     Gwen::HoveredControl->GetParent()->GetToolTip() == 0))
		{
			ToolTip::ForceDisable();
		}
	}
	
	/*
	if (Gwen::KeyboardFocus != prevKeyboardFocus)
	{
		TT_Printf("[%06d] Editor::update: Keyboard focus changed from %p to %p\n",
		          AppGlobal::getUpdateFrameCount(), prevKeyboardFocus, Gwen::KeyboardFocus);
	}
	// */
	
	/*
	static bool showSteamWorkshopTest = true;
	if (showSteamWorkshopTest)
	{
		showSteamWorkshopTest = false;
		onOpenSteamWorkshopTest();
	}
	// */
}


void Editor::updateForRender()
{
	if (isActive() == false)
	{
		return;
	}
	
	//m_cameraMgr.updateForRender();  // NOTE: This function is now called by Game
	tt::engine::renderer::Renderer::getInstance()->setClearColor(
			tt::engine::renderer::ColorRGBA(127, 127, 127, 255));
}


void Editor::render()
{
	if (isActive() == false)
	{
		return;
	}
	
	//m_cameraMgr.onRenderBegin();  // NOTE: This function is now called by Game
	
	if (m_persistentSettings.gameLayersVisible.checkFlag(GameLayer_Attributes))
	{
		m_collisionTileView->render();
	}
	
	if (m_tileLayerMode == TileLayerMode_ThemeType)
	{
		m_themeTileView->render();
	}
	
	m_levelBorder->render();
	
	renderEntities();
	
	renderFloatingSection();
	
	if (hasSelectionRect())
	{
		m_selectionQuadTop->render();
		m_selectionQuadBottom->render();
		m_selectionQuadLeft->render();
		m_selectionQuadRight->render();
		m_selectionOutline->render();
	}
	
	renderNotes();
	renderEditorWarnings();
	
	if (m_editorMode == EditorMode_Normal)
	{
		m_gwenRoot.render();
	}
	
	tt::engine::renderer::Renderer* renderer = tt::engine::renderer::Renderer::getInstance();
	renderer->beginHud();
	
	if (m_editorMode == EditorMode_PickEntity)
	{
		// Render indicator to show that picking mode is active
		m_entityPickModeIndicator->render();
		renderBottomLeft(m_helpTextPickEntity, 10, 0);
	}
	else if (m_editorMode == EditorMode_EditNote)
	{
		// Render indicator for note edit mode, along with some instructions
		m_entityPickModeIndicator->render();
		renderBottomLeft(m_helpTextEditNote, 10, 0);
	}
	else if (m_editorMode == EditorMode_Normal)
	{
#if defined(TT_STEAM_BUILD)
		if (m_ui.windowPublishToWorkshop != 0)
		{
			m_ui.windowPublishToWorkshop->renderPostGwen();
		}
#endif
	}
	
	m_levelSavedIndicator     ->render();
	m_levelSaveFailedIndicator->render();
	
	renderer->endHud();
}


void Editor::show(bool p_userLevelCompleted)
{
	if (isActive())
	{
		// Already active
		return;
	}
	
	// Disable upscaler to prevent black bars in editor
	tt::engine::renderer::UpScalerPtr upScaler = tt::engine::renderer::Renderer::getInstance()->getUpScaler();
	if (upScaler->isActive())
	{
		upScaler->setEnabled(false);
	}
	
	// Notify game that editor is about to open
	if (AppGlobal::hasGame())
	{
		AppGlobal::getGame()->onEditorWillOpen();
	}
	
	m_editorActive = true;
	
	m_spacebarScrollMode.reset();
	
	if (m_tools[m_activePaintTool] != 0)
	{
		m_tools[m_activePaintTool]->onActivate();
	}
	
	// FIXME: Is this really necessary?
	resetClickLockedToolIDs();
	
	// Clear GWEN keyboard focus, so that editor shortcuts work by default
	// FIXME: Add hotkey flag to allow shortcut when GWEN has keyboard focus?
	Gwen::Input::Blur();
	
	// Disable 30 FPS mode while in editor
	m_restoreOnClose.targetFps = tt::app::getApplication()->getTargetFPS();
	if (m_restoreOnClose.targetFps < 60)
	{
		AppGlobal::setDoubleUpdateMode(false);
		tt::app::getApplication()->setTargetFPS(0);
	}
	
	toki::input::Controller& controller(AppGlobal::getController(tt::input::ControllerIndex_One));
	m_pointerAutoVisibility = controller.getPointerAutoVisibility();
	m_pointerVisible        = controller.isPointerVisible();
	controller.setPointerAutoVisibility(false);
	controller.setPointerVisible(true);
	
	// Pause all audio
	audio::AudioPlayer::getInstance()->pauseAllAudio();
	
	AppGlobal::getInputRecorder()->onEditorOpened();
	
	bool cancelScreenshot = true;
	
	// Editor is opened during play test. Check if this is want the user wanted.
	// (Don't open dialog when the editor is opened because it has warnings.)
	//TT_Printf("Editor::show: m_doingPlayTest: %s | hasEditorWarnings(): %s | p_userLevelCompleted: %s\n",
	//          m_doingPlayTest ? "yes" : "no", hasEditorWarnings() ? "yes" : "no",
	//          p_userLevelCompleted ? "yes" : "no");
	if (m_doingPlayTest)
	{
		if (hasEditorWarnings())
		{
			// There were warnings when starting the level: playtest didn't succeed
#if defined(TT_STEAM_BUILD)
			TT_NULL_ASSERT(m_ui.windowPublishToWorkshop);
			if (m_ui.windowPublishToWorkshop != 0)
			{
				m_ui.windowPublishToWorkshop->levelPlayTestFailed();
			}
#endif
		}
		else if (p_userLevelCompleted == false)
		{
			// User opened the editor without completing the level: ask if the playtest should be cancelled
			ui::GenericDialogBox* dialog = showGenericDialog(
				translateString("WINDOW_STOP_PLAYTEST_AND_EDIT_TITLE"),
				translateString("WINDOW_STOP_PLAYTEST_AND_EDIT_PROMPT"),
				ui::DialogButtons_OKCancel, true);
			dialog->onWindowClosed.Add(this, &Editor::onOpenEditorDuringPlayTestDialogClosed);
			cancelScreenshot = false;
		}
		else
		{
			// Userlevel was completed: continue publishing the level
			m_doingPlayTest = false;
			
#if defined(TT_STEAM_BUILD)
			TT_NULL_ASSERT(m_ui.windowPublishToWorkshop);
			if (m_ui.windowPublishToWorkshop != 0)
			{
				m_ui.windowPublishToWorkshop->levelPlayTestSucceeded();
			}
#endif
		}
	}
	
	if (cancelScreenshot)
	{
		AppGlobal::getGame()->unscheduleScreenshot();
	}
}


void Editor::hide(bool p_restartLevel)
{
	if (isActive() == false)
	{
		// Already hidden
		return;
	}
	
	tt::engine::renderer::Renderer::getInstance()->getUpScaler()->setEnabled(true);
	
	cancelEntityPickMode();
	
	if (m_pointerWasOnCanvas)
	{
		m_pointerWasOnCanvas = false;
		if (m_tools[m_activePaintTool] != 0)
		{
			m_tools[m_activePaintTool]->onPointerLeave();
		}
	}
	
	if (m_tools[m_activePaintTool] != 0)
	{
		m_tools[m_activePaintTool]->onDeactivate();
	}
	
#if defined(TT_PLATFORM_WIN) || defined(TT_PLATFORM_OSX_MAC) || defined(TT_PLATFORM_LNX)
	tt::input::MouseController::resetToDefaultCursor();
#endif
	
	//m_levelData->deselectAllEntities();
	hotKeyApplyFloatingSection();
	
	TT_ASSERT(m_noteForTextEdit       == 0);
	TT_ASSERT(m_noteTextChangeCommand == 0);
	m_focusNote.reset();
	m_noteForTextEdit.reset();
	m_noteTextChangeCommand.reset();
	m_addTextEditNoteToLevelOnAccept = false;
	
	// Reset the keyboard focus, so that the control with focus gets a chance to handle focus loss
	// (which might make changes to the editor state, which need to be flushed before the editor closes)
	Gwen::Input::Blur();
	
	// Close the various dialogs and hide the GUI
	//closeLoadLevelDialog();
	for (DialogBoxes::iterator it = m_openDialogBoxes.begin(); it != m_openDialogBoxes.end(); )
	{
		ui::DialogBoxBase* dialog = *it;
		if (dialog->UserData.Get<bool>("closeOnEditorClose"))
		{
			dialog->onWindowClosed.RemoveHandler(this);
			dialog->CloseButtonPressed();
			it = m_openDialogBoxes.erase(it);
		}
		else
		{
			++it;
		}
	}
	
	hideEditorGUI();
	
	// Restore app framework debug key handling
	tt::app::getApplication()->setHandleDebugKeys(tt::app::DebugKeys_All);
	
	// Restore target FPS
	if (m_restoreOnClose.targetFps < 60)
	{
		tt::app::getApplication()->setTargetFPS(m_restoreOnClose.targetFps);
		AppGlobal::setDoubleUpdateMode(AppOptions::getInstance().in30FpsMode);
	}
	
	toki::input::Controller& controller(AppGlobal::getController(tt::input::ControllerIndex_One));
	controller.setPointerAutoVisibility(m_pointerAutoVisibility);
	controller.setPointerVisible(m_pointerVisible);
	
	// Resume all audio
	audio::AudioPlayer::getInstance()->resumeAllAudio();
	
	AppGlobal::getInputRecorder()->onEditorClosed();
	
	// Notify game that editor has been closed
	if (AppGlobal::hasGame())
	{
		AppGlobal::getGame()->onEditorClosed(p_restartLevel);
	}
}


void Editor::setLevelData(const level::LevelDataPtr& p_levelData)
{
	TT_NULL_ASSERT(p_levelData);
	if (p_levelData == 0)
	{
		return;
	}
	
	TT_NULL_ASSERT(m_levelData);
	m_levelData->unregisterObserver(m_this);
	
	m_entitiesUpdatedByScript.clear();
	
	m_doingPlayTest = false;
	
	m_levelData = p_levelData;
	m_levelRect = tt::math::PointRect(
		tt::math::Point2(0, 0),
		m_levelData->getLevelWidth(),
		m_levelData->getLevelHeight());
	
	m_levelData->registerObserver(m_this);
	
	if (m_collisionTileView != 0)
	{
		m_collisionTileView->setAttributeLayer(m_levelData->getAttributeLayer());
	}
	if (m_themeTileView != 0)
	{
		m_themeTileView->setAttributeLayer(m_levelData->getAttributeLayer());
	}
	m_levelBorder->fitAroundRectangle(level::tileToWorld(m_levelRect));
	
	updateSelectionIndicator();
	
	m_ui.listEntityProperties->setEntities(m_levelData->getSelectedEntities());
	
	// Refresh the "Set Theme Color" window
	for (s32 config = 0; config < level::skin::SkinConfigType_Count; ++config)
	{
		for (s32 theme = 0; theme < level::ThemeType_Count; ++theme)
		{
			onThemeColorChanged(static_cast<level::skin::SkinConfigType>(config),
			                    static_cast<level::ThemeType           >(theme));
		}
	}
	
	// Make the menu reflect the current level settings
	updateLevelBackgroundMenuChecks();
	updateLevelThemeMenuChecks();
	updateDefaultMissionMenuChecks();
	
	// If level data changed, the undo stack has been invalidated as well, so clear it.
	// Martijn: disabled for now as it seems to work fine
	//m_undoStack->clear();
	
	scanLevelForEntitiesUpdatedByScript();
}


const std::string& Editor::getCurrentLevelName() const
{
	return getCurrentLevelInfo().getLevelName();
}


const StartInfo& Editor::getCurrentLevelInfo() const
{
	static const StartInfo dummy;
	return AppGlobal::hasGame() ? AppGlobal::getGame()->getStartInfo() : dummy;
}


bool Editor::isUnnamedLevel() const
{
	return getCurrentLevelName().empty();
}


bool Editor::hasEditorWarnings() const
{
	if (m_levelData == 0)
	{
		TT_PANIC("No levelData");
		return false;
	}
	
	// Render the editor warnings of entities
	const level::entity::EntityInstances entities = getMissionSpecificEntities();
	for (level::entity::EntityInstances::const_iterator it = entities.begin(); it != entities.end(); ++it)
	{
		using level::entity::editor::EntityInstanceEditorRepresentation;
		EntityInstanceEditorRepresentation* editorRep = (*it)->getEditorRepresentation();
		if (editorRep != 0)
		{
			if (editorRep->hasEditorWarnings())
			{
				return true;
			}
		}
	}
	return false;
}


void Editor::editorWarning(s32 p_id, const std::string& p_warningStr)
{
	if (m_levelData == 0)
	{
		TT_PANIC("No levelData");
		return;
	}
	
	level::entity::EntityInstancePtr entityInstance = m_levelData->getEntityByID(p_id);
	if (entityInstance != 0)
	{
		level::entity::editor::EntityInstanceEditorRepresentation* editorRep = entityInstance->getOrCreateEditorRepresentation();
		TT_NULL_ASSERT(editorRep);
		if (editorRep != 0)
		{
			const std::wstring warningWStr(tt::str::widen(p_warningStr));
			editorRep->addEditorWarning(warningWStr);
		}
		
		// Focus the camera on the warning.
		getEditorCamera().setPosition(entityInstance->getPosition(), true);
	}
	else
	{
		TT_NULL_ASSERT(entityInstance);
		// TODO:: Maybe store these warnings in the Editor?
	}
}


void Editor::clearEditorWarnings()
{
	if (m_levelData == 0)
	{
		TT_PANIC("No levelData");
		return;
	}
	
	const level::entity::EntityInstances& allEntities = m_levelData->getAllEntities();
	
	for (level::entity::EntityInstances::const_iterator it = allEntities.begin(); it != allEntities.end(); ++it)
	{
		const level::entity::EntityInstancePtr& instancePtr = (*it);
		level::entity::editor::EntityInstanceEditorRepresentation* editorRep = instancePtr->getEditorRepresentation();
		
		if (editorRep != 0)
		{
			editorRep->clearEditorWarnings();
		}
	}
}


void Editor::handleUnsavedChangesBeforeLevelLoadFromGame(const std::string& p_levelName)
{
	show();
	TT_ASSERT(hasUnsavedChanges());
	
	m_loadLevelFromGameName = p_levelName;
	
	ui::GenericDialogBox* dialog = showGenericDialog(
		translateString("WINDOW_UNSAVED_CHANGES_TITLE"),
		translateString("WINDOW_UNSAVED_CHANGES_PROMPT", getCurrentLevelName()),
		ui::DialogButtons_YesNo, true);
	dialog->onWindowClosed.Add(this, &Editor::onSaveBeforeLevelLoadFromGameDialogClosed);
}


void Editor::showLoadLevelDialog()
{
	m_showLoadLevelThisFrame = true;
}


ui::GenericDialogBox* Editor::showGenericDialog(const std::wstring& p_title,
                                                const std::wstring& p_promptText,
                                                ui::DialogButtons   p_buttons,
                                                bool                p_autoCloseWhenEditorClosed)
{
	ui::GenericDialogBox* dlg = ui::GenericDialogBox::create(p_title, p_promptText,
			p_buttons, m_gwenRoot.getCanvas());
	openModalDialogBox(dlg, p_autoCloseWhenEditorClosed);
	return dlg;
}


void Editor::setSelectionRect(const tt::math::PointRect& p_rect, bool p_updateEntitySelection)
{
	if (p_rect != m_selectionRect)
	{
		m_selectionRect = p_rect;
		
		// Also update entity selection
		if (hasFloatingSection() == false && p_updateEntitySelection)
		{
			const level::entity::EntityInstances entities(getMissionSpecificEntities());
			level::entity::EntityInstanceSet newSelection;
			
			tt::math::VectorRect selectionWorldRect(getSelectionWorldRect());
			for (level::entity::EntityInstances::const_iterator it = entities.begin(); it != entities.end(); ++it)
			{
				if (selectionWorldRect.contains((*it)->getPosition()))
				{
					newSelection.insert(*it);
				}
			}
			
			m_levelData->setSelectedEntities(newSelection);
		}
		
		updateSelectionIndicator();
	}
}


void Editor::clearSelectionRect(bool p_alsoDeselectEntities)
{
	setSelectionRect(tt::math::PointRect(tt::math::Point2(0, 0), 0, 0), p_alsoDeselectEntities);
	
	// Clearing the selection also applies the floating section
	if (hasFloatingSection())
	{
		applyFloatingSection();
	}
	
	if (p_alsoDeselectEntities)
	{
		m_levelData->deselectAllEntities();
	}
}


tt::math::PointRect Editor::getEditableRect() const
{
	// If no selection: return a rect for the entire level size
	return hasSelectionRect() ? getLevelIntersectedSelectionRect() : m_levelRect;
}


tt::math::VectorRect Editor::getSelectionWorldRect() const
{
	return level::makeEntitySelectionWorldRect(m_selectionRect);
}


void Editor::createFloatingSectionFromSelection(level::LevelSection::Type p_sectionType)
{
	if (hasSelectionRect() == false)
	{
		return;
	}
	
	if (hasFloatingSection())
	{
		applyFloatingSection();
	}
	
	// Creating a floating section also deselects all entities
	m_levelData->deselectAllEntities();
	
	m_floatingSection = level::LevelSection::createFromLevelRect(m_levelData, m_selectionRect, p_sectionType);
	
	if (m_floatingSection != 0)
	{
		//m_floatingSection->debugPrint();
		createFloatingSectionVisual();
		setFloatingSectionPosition(m_floatingSection->getPosition());
	}
}


void Editor::cloneFloatingSection()
{
	if (hasFloatingSection() == false)
	{
		return;
	}
	
	pushUndoCommand(commands::CommandApplyLevelSection::create(m_levelData, m_floatingSection));
	m_floatingSection = m_floatingSection->clone();
}


void Editor::applyFloatingSection()
{
	if (m_floatingSection == 0)
	{
		TT_ASSERT(m_floatingSectionVisual == 0);
		TT_ASSERT(m_floatingSectionThemeVisual == 0);
		TT_ASSERT(m_floatingSectionBackground == 0);
		m_floatingSectionVisual.reset();
		m_floatingSectionThemeVisual.reset();
		m_floatingSectionBackground.reset();
		return;
	}
	
	pushUndoCommand(commands::CommandApplyLevelSection::create(m_levelData, m_floatingSection));
	
	m_floatingSection.reset();
	m_floatingSectionVisual.reset();
	m_floatingSectionThemeVisual.reset();
	m_floatingSectionBackground.reset();
}


bool Editor::hasFloatingSection() const
{
	return m_floatingSection != 0;
}


void Editor::setFloatingSectionPosition(const tt::math::Point2& p_pos)
{
	if (m_floatingSection != 0)
	{
		m_floatingSection->setPosition(p_pos);
		TT_NULL_ASSERT(m_floatingSectionVisual);
		TT_NULL_ASSERT(m_floatingSectionThemeVisual);
		TT_NULL_ASSERT(m_floatingSectionBackground);
		
		const tt::math::Vector3 visualPos(level::tileToWorld(p_pos.x), level::tileToWorld(p_pos.y), 0.0f);
		m_floatingSectionVisual     ->setPosition(visualPos);
		m_floatingSectionThemeVisual->setPosition(visualPos);
		
		const tt::math::Vector2 sectionWorldCenter(level::tileToWorld(m_floatingSection->getRect()).getCenterPosition());
		m_floatingSectionBackground->setPosition(sectionWorldCenter.x, -sectionWorldCenter.y, 0.0f);
		m_floatingSectionBackground->update();
	}
}


void Editor::onResetDevice()
{
	const real scrW = static_cast<real>(tt::engine::renderer::Renderer::getInstance()->getScreenWidth());
	const real scrH = static_cast<real>(tt::engine::renderer::Renderer::getInstance()->getScreenHeight());
	
	m_levelSavedIndicator->setPosition(scrW * 0.5f, scrH * 0.5f, 0.0f);
	m_levelSaveFailedIndicator->setPosition(scrW * 0.5f, scrH * 0.5f, 0.0f);
	
	m_entityPickModeIndicator->fitInsideRectangle(
		tt::math::VectorRect(tt::math::Vector2(0.0f, -scrH), scrW, scrH));
}


void Editor::releaseResourcesForReload()
{
	m_collisionTileView.reset();
	m_themeTileView.reset();
	// FIXME: If we have "floating section" views, also recreate those!
}


void Editor::onRequestReloadAssets()
{
	createAttributeViews();
	
	// Get the new level data pointer
	setLevelData(AppGlobal::getGame()->getLevelData());
	
	// Refresh the pickers
	refreshEntityLibrary(0);
	refreshTilesList();
	refreshLevelBackgroundMenu();
	m_gwenRoot.getCanvas()->DoThink(); // Flush delayed deletions
	
	// Reset all entity references
	const level::entity::EntityInstances entities = m_levelData->getAllEntities();
	for (level::entity::EntityInstances::const_iterator it = entities.begin(); it != entities.end(); ++it)
	{
		const level::entity::EntityInstancePtr& instance(*it);
		
		level::entity::editor::EntityInstanceEditorRepresentation* editorRep = instance->getEditorRepresentation();
		if (editorRep != 0)
		{
			editorRep->resetLineBuffers();
		}
	}
}


void Editor::overrideLevelBorderRect(const tt::math::PointRect& p_rect)
{
	m_levelBorder->fitAroundRectangle(level::tileToWorld(p_rect));
}


void Editor::resetLevelBorderRect()
{
	m_levelRect = m_levelData->getLevelRect();
	m_levelBorder->fitAroundRectangle(level::tileToWorld(m_levelRect));
}


tt::math::PointRect Editor::getLevelBorderRect() const
{
	return level::worldToTile(m_levelBorder->getInnerRect());
}


real Editor::getLevelBorderThickness() const
{
	return m_levelBorder->getThickness();
}


void Editor::handleLevelResized()
{
	resetLevelBorderRect();
	updateSelectionIndicator();
	AppGlobal::getGame()->handleLevelResized();
}


void Editor::setCursor(EditCursor p_cursor)
{
	m_activeCursor = p_cursor;
	
#if defined(TT_PLATFORM_WIN) || defined(TT_PLATFORM_OSX_MAC) || defined(TT_PLATFORM_LNX)
	tt::input::MouseController::setCustomCursor(m_editCursors[p_cursor]);
#endif
}


void Editor::restoreDefaultCursor()
{
	setCursor(EditCursor_NormalArrow);
}


level::entity::EntityInstancePtr Editor::createEntity(const std::string& p_typeName)
{
	const level::entity::EntityInfo* info = AppGlobal::getEntityLibrary().getEntityInfo(p_typeName);
	TT_ASSERTMSG(info != 0,
	             "Entity type '%s' does not exist in the entity library. "
	             "Do not know what editor graphic to display.", p_typeName.c_str());
	
	level::entity::EntityInstancePtr instance(level::entity::EntityInstance::create(
		p_typeName, m_levelData->createEntityID()));
	
	return instance;
}


level::entity::EntityInstances Editor::findEntitiesAtWorldPos(const tt::math::Vector2& p_worldPos) const
{
	using level::entity::EntityInstances;
	EntityInstances entities;
	entities.reserve(2);
	
	const level::entity::EntityInstances levelEntities = getMissionSpecificEntities();
	for (EntityInstances::const_iterator it = levelEntities.begin(); it != levelEntities.end(); ++it)
	{
		const level::entity::EntityInstancePtr& instance(*it);
		level::entity::editor::EntityInstanceEditorRepresentation* editorRep = instance->getEditorRepresentation();
		if (editorRep == 0)
		{
			continue;
		}
		
		tt::math::Vector2 size(editorRep->getImageSize());
		// FIXME: Do true OBB test in case of rotated image. Now I simply convert the rectangle to a square
		if (editorRep->getImageRotation() != 0.0f)
		{
			size.x = std::max(size.x, size.y);
			size.y = std::max(size.x, size.y);
		}
		
		// FIXME: This code contains hard-coded "placement offset" assumptions
		const real halfW = size.x * 0.5f;
		if (p_worldPos.x >= instance->getPosition().x - halfW &&
		    p_worldPos.x <= instance->getPosition().x + halfW &&
		    p_worldPos.y >= instance->getPosition().y         &&
		    p_worldPos.y <= instance->getPosition().y + size.y)
		{
			entities.push_back(instance);
		}
	}
	
	return entities;
}


bool Editor::hasEntityAtWorldPos(const tt::math::Vector2& p_worldPos) const
{
	using level::entity::EntityInstances;
	const level::entity::EntityInstances entities = getMissionSpecificEntities();
	for (EntityInstances::const_iterator it = entities.begin(); it != entities.end(); ++it)
	{
		const level::entity::EntityInstancePtr& instance(*it);
		level::entity::editor::EntityInstanceEditorRepresentation* editorRep = instance->getEditorRepresentation();
		if (editorRep == 0)
		{
			continue;
		}
		
		tt::math::Vector2 size(editorRep->getImageSize());
		// FIXME: Do true OBB test in case of rotated image. Now I simply convert the rectangle to a square
		if (editorRep->getImageRotation() != 0.0f)
		{
			size.x = std::max(size.x, size.y);
			size.y = std::max(size.x, size.y);
		}
		
		// FIXME: This code contains hard-coded "placement offset" assumptions
		const real halfW = size.x * 0.5f;
		if (p_worldPos.x >= instance->getPosition().x - halfW &&
		    p_worldPos.x <= instance->getPosition().x + halfW &&
		    p_worldPos.y >= instance->getPosition().y         &&
		    p_worldPos.y <= instance->getPosition().y + size.y)
		{
			return true;
		}
	}
	
	return false;
}


tt::math::Vector2 Editor::getEntitySize(const level::entity::EntityInstancePtr& p_entity) const
{
	TT_NULL_ASSERT(p_entity);
	if (p_entity == 0 || p_entity->getEditorRepresentation() == 0)
	{
		return tt::math::Vector2::zero;
	}
	
	const level::entity::editor::EntityInstanceEditorRepresentation* editorRep = p_entity->getEditorRepresentation();
	
	if (editorRep->hasSizeShape())
	{
		// Entity has a scalable size shape, return that size
		return editorRep->getSizeShapeSize();
	}
	
	// Return size based on graphic
	return editorRep->getImageSize();
}


void Editor::snapEntityToTile(const level::entity::EntityInstancePtr& p_entity)
{
	// FIXME: This code contains hard-coded "placement offset" assumptions
	const tt::math::Vector2 entitySize(getEntitySize(p_entity));
	p_entity->setPosition(level::snapToTilePos(
			p_entity->getPosition(),
			tt::math::Vector2(-entitySize.x * 0.5f, 0.0f)));
}


void Editor::switchToToolUntilPointerReleased(tools::ToolID p_tool)
{
	const input::Controller::State::EditorState& editorState(AppGlobal::getController(tt::input::ControllerIndex_One).cur.editor);
	if (editorState.pointerLeft.down   == false &&
	    editorState.pointerMiddle.down == false &&
	    editorState.pointerRight.down  == false)
	{
		// No pointer button pressed to start with, so refuse to switch
		return;
	}
	
	switchToTool(p_tool, true);
	m_switchToPreviousToolOnPointerRelease = true;
	
	// Ensure the new tool gets pressed events
	handleToolInput();
}


void Editor::setPaintTile(level::CollisionType p_tile)
{
	TT_ASSERTMSG(m_tileLayerMode == TileLayerMode_CollisionType,
	             "Should not call setPaintTile when tile layer mode is not CollisionType.");
	
	if (p_tile == m_paintTile)
	{
		return;
	}
	
	if (level::isValidCollisionType(p_tile) == false)
	{
		TT_PANIC("Cannot select invalid paint tile %d.", p_tile);
		return;
	}
	
	m_paintTile = p_tile;
	
	if (ui::selectItemByName(m_ui.listTiles, level::getCollisionTypeName(m_paintTile)) == false)
	{
		TT_PANIC("Selecting tile '%s', which does not appear in the tiles list!",
		         level::getCollisionTypeName(m_paintTile));
	}
}


void Editor::setPaintTheme(level::ThemeType p_theme)
{
	TT_ASSERTMSG(m_tileLayerMode == TileLayerMode_ThemeType,
	             "Should not call setPaintTheme when tile layer mode is not ThemeType.");
	
	if (p_theme == m_paintTheme)
	{
		return;
	}
	
	if (level::isValidThemeType(p_theme) == false)
	{
		TT_PANIC("Cannot select invalid paint theme %d.", p_theme);
		return;
	}
	
	m_paintTheme = p_theme;
	
	if (ui::selectItemByName(m_ui.listTiles, level::getThemeTypeName(m_paintTheme)) == false)
	{
		TT_PANIC("Selecting theme '%s', which does not appear in the tiles list!",
		         level::getThemeTypeName(m_paintTheme));
	}
}


void Editor::enterEntityPickMode(const level::entity::EntityProperty& p_property,
                                 const level::entity::EntityIDSet&    p_disallowedIDs)
{
	if (m_editorMode == EditorMode_PickEntity)
	{
		TT_PANIC("Entering entity pick mode when already in entity pick mode.");
		cancelEntityPickMode();
	}
	
	m_editorMode              = EditorMode_PickEntity;
	m_entityPickTypeFilter    = p_property.getFilter();
	m_entityPickPropertyName  = p_property.getName();
	m_entityPickDisallowedIDs = p_disallowedIDs;
	m_entityPickCount         = 0;
	// Set max pick count based on type of property. -1 for infinite multipicking
	using namespace level::entity;
	EntityProperty::Type type = p_property.getType();
	const bool isEntity = (type == EntityProperty::Type_Entity ||
	                       type == EntityProperty::Type_EntityID ||
	                       type == EntityProperty::Type_DelayedEntityID);
	m_entityPickMaxCount = isEntity ? 1 : -1;
}


void Editor::cancelEntityPickMode()
{
	if (m_editorMode == EditorMode_PickEntity)
	{
		m_editorMode = EditorMode_Normal;
		m_entityPickTypeFilter.clear();
		m_entityPickPropertyName.clear();
		m_entityPickDisallowedIDs.clear();
		m_ui.listEntityProperties->entityPickCancelled();
	}
}


void Editor::selectFirstEntityOfType(const std::string& p_type)
{
	cancelEntityPickMode();
	clearSelectionRect(true);
	
	// Find entity type
	const level::entity::EntityInstances entities(getMissionSpecificEntities());
	for (level::entity::EntityInstances::const_iterator it = entities.begin(); it != entities.end(); ++it)
	{
		const level::entity::EntityInstancePtr& instance(*it);
		if (instance->getType() == p_type)
		{
			// Focus the camera on the entity
			getEditorCamera().setPosition(instance->getPosition(), true);
			m_levelData->addEntityToSelection(instance);
			return;
		}
	}
}


void Editor::enterNoteTextEditMode(const level::NotePtr& p_noteToEdit, bool p_addToLevelOnAccept)
{
#if defined(TT_PLATFORM_WIN) || defined(TT_PLATFORM_OSX_MAC) || defined(TT_PLATFORM_LNX)
	if (m_editorMode != EditorMode_Normal)
	{
		TT_PANIC("Cannot start editing note: editor is already in an edit mode.");
		return;
	}
	
	TT_NULL_ASSERT(p_noteToEdit);
	if (p_noteToEdit == 0)
	{
		return;
	}
	
	setCursor(EditCursor_TextEdit);
	
	m_noteForTextEdit       = p_noteToEdit;
	m_addTextEditNoteToLevelOnAccept = p_addToLevelOnAccept;
	if (m_addTextEditNoteToLevelOnAccept == false)
	{
		m_noteTextChangeCommand = commands::CommandChangeNoteText::create(m_noteForTextEdit);
	}
	else
	{
		TT_ASSERT(m_noteTextChangeCommand == 0);
		m_noteTextChangeCommand.reset();
	}
	m_editorMode            = EditorMode_EditNote;
	m_noteForTextEdit->setEditCaretVisible(true);
	
	if (tt::input::KeyboardController::getWideString(
			keyboardStringCallback, this, p_noteToEdit->getText()) == false)
	{
		resetNoteEditVariables();
		return;
	}
#else
	(void)p_noteToEdit;
	(void)p_addToLevelOnAccept;
#endif
}


void Editor::cancelNoteTextEditMode()
{
#if defined(TT_PLATFORM_WIN) || defined(TT_PLATFORM_OSX_MAC) || defined(TT_PLATFORM_LNX)
	tt::input::KeyboardController::cancelGetString();
	resetNoteEditVariables();
#endif
}


void Editor::onLevelBackgroundChanged()
{
	// Tell the game to deal with the new selection
	if (AppGlobal::hasGame())
	{
		AppGlobal::getGame()->handleLevelBackgroundPicked();
	}
	
	// Update the UI
	updateLevelBackgroundMenuChecks();
}


void Editor::onLevelThemeChanged()
{
	// Update the UI
	updateLevelThemeMenuChecks();
}


void Editor::onLevelDefaultMissionChanged()
{
	// Update the UI
	updateDefaultMissionMenuChecks();
}


void Editor::onThemeColorChanged(level::skin::SkinConfigType p_skinConfig, level::ThemeType p_theme)
{
	if (m_ui.windowSetThemeColor != 0)
	{
		m_ui.windowSetThemeColor->onThemeColorChanged(p_skinConfig, p_theme);
	}
}


bool Editor::saveLevel()
{
	TT_ASSERTMSG(isUnnamedLevel() == false, "saveLevel called for a level with no name.");
	
	const game::StartInfo& startInfo(getCurrentLevelInfo());
	
	const std::string filename(startInfo.getLevelFilePath());
	std::string       internalSourceFilename;
	
#if EDITOR_SUPPORTS_ASSETS_SOURCE
	// HACK: Also save the source asset (hard-coded relative path to the asset source files)
	if (startInfo.isUserLevel() == false)
	{
		internalSourceFilename = getLevelsSourceDir() + startInfo.getLevelName() + ".ttlvl";
	}
#endif
	
	return saveLevel(filename, internalSourceFilename);
}


bool Editor::hasUnsavedChanges() const
{
	return m_undoStack != 0 && m_undoStack->isClean() == false;
}


void Editor::startPlayTest()
{
	// Make sure level is saved before starting level in play test mode.
	if (hasUnsavedChanges() || isUnnamedLevel())
	{
		ui::GenericDialogBox* dialog = showGenericDialog(
			translateString("WINDOW_PLAYTEST_NEEDS_SAVE_TITLE"),
			translateString("WINDOW_PLAYTEST_NEEDS_SAVE_PROMPT"),
			ui::DialogButtons_OKCancel, true);
		dialog->onWindowClosed.Add(this, &Editor::onSavePlayTestAsDialogClosed);
	}
	else
	{
		m_doingPlayTest = true;
		hide(true);
	}
}


void Editor::pushUndoCommand(const tt::undo::UndoCommandPtr& p_command)
{
	m_undoStack->push(p_command);
	
	/* DEBUG: Show new undo stack contents
	const s32 commandCount = m_undoStack->getCommandCount();
	TT_Printf("Editor::pushUndoCommand: Current stack (%d commands):\n", commandCount);
	for (s32 i = 0; i < commandCount; ++i)
	{
		TT_Printf("Editor::pushUndoCommand: - '%s'\n",
		          tt::str::utf16ToUtf8(m_undoStack->getCommand(i)->getDisplayName()).c_str());
	}
	// END DEBUG */
}


void Editor::undo(s32 p_depth)
{
	for (s32 i = 0; i < p_depth; ++i)
	{
		m_undoStack->undo();
	}
}


s32 Editor::getUndoStackSize() const
{
	return m_undoStack->getCurrentIndex();
}


void Editor::centerInScreen(Gwen::Controls::Base* p_control)
{
	const tt::math::Point2 scrSize(getEditorCamera().getViewPortSize());
	p_control->SetPos(
			(scrSize.x - p_control->Width())  / 2,
			(scrSize.y - p_control->Height()) / 2);
}


//--------------------------------------------------------------------------------------------------
// Private member functions

Editor::Editor(const level::LevelDataPtr& p_levelData)
:
m_this(),
m_editorActive(false),
m_pointerAutoVisibility(true),
m_pointerVisible(true),
m_doingPlayTest(false),
m_restoreOnClose(),
m_persistentSettings(),
m_levelData(p_levelData),
m_levelBorder(Border::create(level::tileToWorld(4),
                             tt::engine::renderer::ColorRGBA(tt::engine::renderer::ColorRGB::blue, 63))),
m_gwenRoot("gwenRootCanvas", "gwen_skin", tt::engine::renderer::ViewPortID_Main),
m_ui(),
m_hotKeyMgr(),
m_svnCommands(),
m_openDialogBoxes(),
m_showLoadLevelThisFrame(false),
m_loadLevelDialogHasBeenOpen(false),
m_entitiesUpdatedByScript(),
m_showEntitiesUpdatedDialog(false),
m_propertiesUpdatedOverlay(tt::engine::renderer::QuadSprite::createQuad(
		tt::engine::renderer::TextureCache::get("entity_properties_updated", "textures.editor.ui"))),
m_activePaintTool(tools::ToolID_Draw),
m_previousPaintTool(tools::ToolID_Draw),
m_switchToPreviousToolOnPointerRelease(false),
m_axisLockStartScreenPos(-1, -1),
m_lockAxis(LockAxis_None),
m_axisLockTrigger(),
m_paintEntityType(),
m_tileLayerMode(TileLayerMode_CollisionType),
m_paintTile(level::CollisionType_Solid),
m_paintTheme(level::ThemeType_UseLevelDefault),
m_selectionRect(tt::math::Point2(0, 0), 0, 0),
m_levelRect(p_levelData->getLevelRect()),
m_collisionTileView(),
m_themeTileView(),
m_floatingSection(),
m_floatingSectionVisual(),
m_floatingSectionThemeVisual(),
m_floatingSectionBackground(),
m_selectionQuadTop(),
m_selectionQuadBottom(),
m_selectionQuadLeft(),
m_selectionQuadRight(),
m_selectionOutline(),
m_levelSavedIndicator(tt::engine::renderer::QuadSprite::createQuad(
	tt::engine::renderer::TextureCache::get("level_saved", "textures.editor.ui"),
	tt::engine::renderer::ColorRGB::white)),
m_levelSaveFailedIndicator(tt::engine::renderer::QuadSprite::createQuad(
	tt::engine::renderer::TextureCache::get("level_save_failed", "textures.editor.ui"),
	tt::engine::renderer::ColorRGB::white)),
m_helpTextPickEntity(),
m_helpTextEditNote(),
m_statusTextSaved(),
m_statusTextUnsaved(),
m_pointerWasOnCanvas(false),
m_spacebarScrollMode(),
m_editorMode(EditorMode_Normal),
m_entityPickTypeFilter(),
m_entityPickPropertyName(),
m_entityPickDisallowedIDs(),
m_entityPickModeIndicator(Border::create(25,
                                         tt::engine::renderer::ColorRGBA(tt::engine::renderer::ColorRGB::red, 127))),
m_entityPickCount(0),
m_entityPickMaxCount(0),
m_focusNote(),
m_noteForTextEdit(),
m_addTextEditNoteToLevelOnAccept(false),
m_noteTextChangeCommand(),
m_undoStack(new tt::undo::UndoStack(0)),
m_loadLevelFromGameName(),
m_cameraMgr("toki.camera.editor"),
m_activeCursor(EditCursor_NormalArrow)
{
	if (AppGlobal::getLoc().hasLocStr(loc::SheetID_Editor) == false)
	{
		AppGlobal::getLoc().createLocStr(loc::SheetID_Editor);
	}
	
	m_statusTextSaved   = translateString("STATUSBAR_SAVED");
	m_statusTextUnsaved = translateString("STATUSBAR_UNSAVED_CHANGES");
	
	static const utils::GlyphSetID helpTextFont = utils::GlyphSetID_EditorHelpText;
	m_helpTextPickEntity = createQuadWithText(translateString("HELPTEXT_PICK_ENTITY"),
			helpTextFont, tt::engine::renderer::ColorRGB::white);
	m_helpTextEditNote = createQuadWithText(translateString("HELPTEXT_EDIT_NOTE"),
			helpTextFont, tt::engine::renderer::ColorRGB::white);
	
	m_cameraMgr.setDrcCameraEnabled(true);
	m_cameraMgr.setEmulateDRC(true);
	m_cameraMgr.getCamera()   .setFreeScrollMode(true);  // used to make camera follow pointer while zooming
	m_cameraMgr.getDrcCamera().setFreeScrollMode(true);  // used to make camera follow pointer while zooming
	m_cameraMgr.getDrcCamera().selectCameraFromViewPort();
	setupPreviewCamera();
	
	loadCursors();
	
	createAttributeViews();
	
	// Create the various edit tools
	m_tools[tools::ToolID_Draw       ].reset(new tools::PaintTool(this));
	m_tools[tools::ToolID_FloodFill  ].reset(new tools::FloodFillTool(this));
	m_tools[tools::ToolID_Hand       ].reset(new tools::HandTool(this));
	m_tools[tools::ToolID_BoxSelect  ].reset(new tools::BoxSelectTool(this));
	m_tools[tools::ToolID_Resize     ].reset(new tools::ResizeTool(this));
	m_tools[tools::ToolID_EntityPaint].reset(new tools::EntityPaintTool(this));
	m_tools[tools::ToolID_EntityMove ].reset(new tools::EntityMoveTool(this));
	m_tools[tools::ToolID_Notes      ].reset(new tools::NotesTool(this));
	
	resetClickLockedToolIDs();
	
	// Create the editor GUI
	setupUi();
	createSelectionIndicator();
	m_levelBorder->fitAroundRectangle(level::tileToWorld(m_levelRect));
	
	const real scrW = static_cast<real>(tt::engine::renderer::Renderer::getInstance()->getScreenWidth() );
	const real scrH = static_cast<real>(tt::engine::renderer::Renderer::getInstance()->getScreenHeight());
	m_entityPickModeIndicator->fitInsideRectangle(
		tt::math::VectorRect(tt::math::Vector2(0.0f, -scrH), scrW, scrH));
	
	m_levelSavedIndicator->setPosition(scrW * 0.5f, scrH * 0.5f, 0.0f);
	m_levelSavedIndicator->resetFlag(tt::engine::renderer::QuadSprite::Flag_Visible);
	
	m_levelSaveFailedIndicator->setPosition(scrW * 0.5f, scrH * 0.5f, 0.0f);
	m_levelSaveFailedIndicator->resetFlag(tt::engine::renderer::QuadSprite::Flag_Visible);
	
	setupHotKeys();
	
	// Hide the editor by default
	hideEditorGUI();
	
	// Scan the level data for any entity properties that were updated by script
	m_propertiesUpdatedOverlay->setWidth (1.0f);
	m_propertiesUpdatedOverlay->setHeight(1.0f);
	scanLevelForEntitiesUpdatedByScript();
	
	//m_hotKeyMgr.debugPrint();
}


void Editor::scanLevelForEntitiesUpdatedByScript()
{
	// Scan the level data for any entity properties that were updated by script
	m_entitiesUpdatedByScript.clear();
	const level::entity::EntityInstances& entities(m_levelData->getAllEntities());
	for (level::entity::EntityInstances::const_iterator it = entities.begin(); it != entities.end(); ++it)
	{
		const level::entity::EntityInstancePtr& instance(*it);
		if (instance->getPropertiesUpdatedByScript())
		{
			m_entitiesUpdatedByScript.insert(instance);
			
			// Clear the "updated" flag, now that the editor is aware of this change
			instance->setPropertiesUpdatedByScript(false);
		}
	}
	
	if (m_entitiesUpdatedByScript.empty() == false)
	{
		m_showEntitiesUpdatedDialog = true;
	}
}


void Editor::loadCursors()
{
	// Load the platform-specific edit cursors
#if defined(TT_PLATFORM_SDL)
	using tt::input::SDLMouseCursor;
	
	m_editCursors[EditCursor_NormalArrow             ] = SDLMouseCursor::create(SDL_SYSTEM_CURSOR_ARROW);
	m_editCursors[EditCursor_OpenHand                ] = SDLMouseCursor::create("cursors/editor_hand_open.cur");
	m_editCursors[EditCursor_ClosedHand              ] = SDLMouseCursor::create("cursors/editor_hand_closed.cur");
	m_editCursors[EditCursor_SelectionRectCreate     ] = SDLMouseCursor::create("cursors/editor_selection_rect_create.cur");
	m_editCursors[EditCursor_SelectionRectMove       ] = SDLMouseCursor::create("cursors/editor_selection_rect_move.cur");
	m_editCursors[EditCursor_SelectionContentCut     ] = SDLMouseCursor::create("cursors/editor_selection_content_cut.cur");
	m_editCursors[EditCursor_SelectionContentClone   ] = SDLMouseCursor::create("cursors/editor_selection_content_clone.cur");
	m_editCursors[EditCursor_SelectionContentMove    ] = SDLMouseCursor::create("cursors/editor_selection_content_move.cur");
	m_editCursors[EditCursor_ResizeTopBottom         ] = SDLMouseCursor::create(SDL_SYSTEM_CURSOR_SIZENS);
	m_editCursors[EditCursor_ResizeLeftRight         ] = SDLMouseCursor::create(SDL_SYSTEM_CURSOR_SIZEWE);
	m_editCursors[EditCursor_ResizeTopLeftBottomRight] = SDLMouseCursor::create("cursors/editor_resize_diagonal_backward.cur");
	m_editCursors[EditCursor_ResizeTopRightBottomLeft] = SDLMouseCursor::create("cursors/editor_resize_diagonal_forward.cur");
	m_editCursors[EditCursor_NotAllowed              ] = SDLMouseCursor::create(SDL_SYSTEM_CURSOR_NO);
	m_editCursors[EditCursor_Draw                    ] = SDLMouseCursor::create("cursors/editor_draw.cur");
	m_editCursors[EditCursor_DrawEntity              ] = SDLMouseCursor::create("cursors/editor_drawentity.cur");
	m_editCursors[EditCursor_FloodFill               ] = SDLMouseCursor::create("cursors/editor_floodfill.cur");
	m_editCursors[EditCursor_GenericMove             ] = SDLMouseCursor::create("cursors/editor_generic_move.cur");
	m_editCursors[EditCursor_Erase                   ] = SDLMouseCursor::create("cursors/editor_erase.cur");
	m_editCursors[EditCursor_TilePicker              ] = SDLMouseCursor::create("cursors/editor_tilepicker.cur");
	m_editCursors[EditCursor_EntityPickerInvalid     ] = SDLMouseCursor::create("cursors/editor_entitypicker_invalid.cur");
	m_editCursors[EditCursor_EntityPickerValid       ] = SDLMouseCursor::create("cursors/editor_entitypicker_valid.cur");
	m_editCursors[EditCursor_AddNote                 ] = SDLMouseCursor::create("cursors/editor_addnote.cur");
	m_editCursors[EditCursor_TextEdit                ] = SDLMouseCursor::create(SDL_SYSTEM_CURSOR_IBEAM);
	
	// HACK: For all unsupported cursors, use the default arrow cursor
	for (s32 i = 0; i < EditCursor_Count; ++i)
	{
		if (m_editCursors[i] == 0)
		{
			m_editCursors[i] = SDLMouseCursor::create(SDL_SYSTEM_CURSOR_ARROW);
		}
	}
#elif defined(TT_PLATFORM_WIN)
	
	for (s32 i = 0; i < EditCursor_Count; ++i)
	{
		m_editCursors[i] = 0;
	}
	
	HINSTANCE inst = ::GetModuleHandle(0);
	m_editCursors[EditCursor_NormalArrow             ] = ::LoadCursor(0, IDC_ARROW);
	m_editCursors[EditCursor_OpenHand                ] = ::LoadCursor(inst, MAKEINTRESOURCE(IDC_CURSOR_EDITOR_HAND_OPEN));
	m_editCursors[EditCursor_ClosedHand              ] = ::LoadCursor(inst, MAKEINTRESOURCE(IDC_CURSOR_EDITOR_HAND_CLOSED));
	m_editCursors[EditCursor_SelectionRectCreate     ] = ::LoadCursor(inst, MAKEINTRESOURCE(IDC_CURSOR_EDITOR_SELECTION_RECT_CREATE));
	m_editCursors[EditCursor_SelectionRectMove       ] = ::LoadCursor(inst, MAKEINTRESOURCE(IDC_CURSOR_EDITOR_SELECTION_RECT_MOVE));
	m_editCursors[EditCursor_SelectionContentCut     ] = ::LoadCursor(inst, MAKEINTRESOURCE(IDC_CURSOR_EDITOR_SELECTION_CONTENT_CUT));
	m_editCursors[EditCursor_SelectionContentClone   ] = ::LoadCursor(inst, MAKEINTRESOURCE(IDC_CURSOR_EDITOR_SELECTION_CONTENT_CLONE));
	m_editCursors[EditCursor_SelectionContentMove    ] = ::LoadCursor(inst, MAKEINTRESOURCE(IDC_CURSOR_EDITOR_SELECTION_CONTENT_MOVE));
	
	// FIXME: VS2015 upgrade: fix these shitty windows macros
#pragma warning (push)
#pragma warning(disable : 4302)
	
	m_editCursors[EditCursor_ResizeTopBottom         ] = ::LoadCursor(0, MAKEINTRESOURCE(IDC_SIZENS));
	m_editCursors[EditCursor_ResizeLeftRight         ] = ::LoadCursor(0, MAKEINTRESOURCE(IDC_SIZEWE));
	m_editCursors[EditCursor_ResizeTopLeftBottomRight] = ::LoadCursor(0, MAKEINTRESOURCE(IDC_SIZENWSE));
	m_editCursors[EditCursor_ResizeTopRightBottomLeft] = ::LoadCursor(0, MAKEINTRESOURCE(IDC_SIZENESW));
	m_editCursors[EditCursor_NotAllowed              ] = ::LoadCursor(0, MAKEINTRESOURCE(IDC_NO));
	
#pragma warning (pop)
	
	m_editCursors[EditCursor_Draw                    ] = ::LoadCursor(inst, MAKEINTRESOURCE(IDC_CURSOR_EDITOR_DRAW));
	m_editCursors[EditCursor_DrawEntity              ] = ::LoadCursor(inst, MAKEINTRESOURCE(IDC_CURSOR_EDITOR_DRAWENTITY));
	m_editCursors[EditCursor_FloodFill               ] = ::LoadCursor(inst, MAKEINTRESOURCE(IDC_CURSOR_EDITOR_FLOODFILL));
	m_editCursors[EditCursor_GenericMove             ] = ::LoadCursor(0, IDC_SIZEALL);
	m_editCursors[EditCursor_Erase                   ] = ::LoadCursor(inst, MAKEINTRESOURCE(IDC_CURSOR_EDITOR_ERASE));
	m_editCursors[EditCursor_TilePicker              ] = ::LoadCursor(inst, MAKEINTRESOURCE(IDC_CURSOR_EDITOR_TILEPICKER));
	m_editCursors[EditCursor_EntityPickerInvalid     ] = ::LoadCursor(inst, MAKEINTRESOURCE(IDC_CURSOR_EDITOR_ENTITYPICKER_INVALID));
	m_editCursors[EditCursor_EntityPickerValid       ] = ::LoadCursor(inst, MAKEINTRESOURCE(IDC_CURSOR_EDITOR_ENTITYPICKER_VALID));
	m_editCursors[EditCursor_AddNote                 ] = ::LoadCursor(inst, MAKEINTRESOURCE(IDC_CURSOR_EDITOR_ADDNOTE));
	m_editCursors[EditCursor_TextEdit                ] = ::LoadCursor(0, IDC_IBEAM);
	
#elif defined(TT_PLATFORM_OSX_MAC)
	
	using tt::input::MouseCursor;
	
	m_editCursors[EditCursor_NormalArrow             ] = MouseCursor::create(MouseCursor::SystemCursor_Arrow);
	m_editCursors[EditCursor_OpenHand                ] = MouseCursor::create("cursors/editor_hand_open.cur");   // or, using system cursor: MouseCursor::create(MouseCursor::SystemCursor_OpenHand);
	m_editCursors[EditCursor_ClosedHand              ] = MouseCursor::create("cursors/editor_hand_closed.cur"); // or, using system cursor: MouseCursor::create(MouseCursor::SystemCursor_ClosedHand);
	m_editCursors[EditCursor_SelectionRectCreate     ] = MouseCursor::create("cursors/editor_selection_rect_create.cur");
	m_editCursors[EditCursor_SelectionRectMove       ] = MouseCursor::create("cursors/editor_selection_rect_move.cur");
	m_editCursors[EditCursor_SelectionContentCut     ] = MouseCursor::create("cursors/editor_selection_content_cut.cur");
	m_editCursors[EditCursor_SelectionContentClone   ] = MouseCursor::create("cursors/editor_selection_content_clone.cur");
	m_editCursors[EditCursor_SelectionContentMove    ] = MouseCursor::create("cursors/editor_selection_content_move.cur");
	m_editCursors[EditCursor_ResizeTopBottom         ] = MouseCursor::create(MouseCursor::SystemCursor_ResizeUpDown);
	m_editCursors[EditCursor_ResizeLeftRight         ] = MouseCursor::create(MouseCursor::SystemCursor_ResizeLeftRight);
	m_editCursors[EditCursor_ResizeTopLeftBottomRight] = MouseCursor::create("cursors/editor_resize_diagonal_backward.cur");
	m_editCursors[EditCursor_ResizeTopRightBottomLeft] = MouseCursor::create("cursors/editor_resize_diagonal_forward.cur");
	//m_editCursors[EditCursor_NotAllowed              ] = MouseCursor::create("cursors/editor_.cur");  //::LoadCursor(0, MAKEINTRESOURCE(IDC_NO));
	m_editCursors[EditCursor_Draw                    ] = MouseCursor::create("cursors/editor_draw.cur");
	m_editCursors[EditCursor_DrawEntity              ] = MouseCursor::create("cursors/editor_drawentity.cur");
	m_editCursors[EditCursor_FloodFill               ] = MouseCursor::create("cursors/editor_floodfill.cur");
	m_editCursors[EditCursor_GenericMove             ] = MouseCursor::create("cursors/editor_generic_move.cur");
	m_editCursors[EditCursor_Erase                   ] = MouseCursor::create("cursors/editor_erase.cur");
	m_editCursors[EditCursor_TilePicker              ] = MouseCursor::create("cursors/editor_tilepicker.cur");
	m_editCursors[EditCursor_EntityPickerInvalid     ] = MouseCursor::create("cursors/editor_entitypicker_invalid.cur");
	m_editCursors[EditCursor_EntityPickerValid       ] = MouseCursor::create("cursors/editor_entitypicker_valid.cur");
	m_editCursors[EditCursor_AddNote                 ] = MouseCursor::create("cursors/editor_addnote.cur");
	m_editCursors[EditCursor_TextEdit                ] = MouseCursor::create(MouseCursor::SystemCursor_IBeam);
	
	// HACK: For all unsupported cursors, use the default arrow cursor
	for (s32 i = 0; i < EditCursor_Count; ++i)
	{
		if (m_editCursors[i] == 0)
		{
			m_editCursors[i] = MouseCursor::create(MouseCursor::SystemCursor_Arrow);
		}
	}
#endif
}


void Editor::setupPreviewCamera()
{
}


void Editor::setupHotKeys()
{
	// Set up the hot keys that aren't already associated with a menu item
	// (those are set up along with the menu items themselves)
	const hotkey::Modifiers noModifier;
	
	// Tool selection
	addHotKey(tt::input::Key_M, noModifier, &Editor::hotKeySelectToolBoxSelect);
	addHotKey(tt::input::Key_B, noModifier, &Editor::hotKeySelectToolDraw);
	addHotKey(tt::input::Key_G, noModifier, &Editor::hotKeySelectToolFloodFill);
	addHotKey(tt::input::Key_H, noModifier, &Editor::hotKeySelectToolHand);
	addHotKey(tt::input::Key_R, noModifier, &Editor::hotKeySelectToolResize);
	addHotKey(tt::input::Key_E, noModifier, &Editor::hotKeySelectToolEntityDraw);
	addHotKey(tt::input::Key_V, noModifier, &Editor::hotKeySelectToolEntityMove);
	addHotKey(tt::input::Key_N, noModifier, &Editor::hotKeySelectToolNotes);
	
	// Generic hotkeys
	addHotKey(tt::input::Key_Escape, noModifier, &Editor::hotKeyCancelSelection);
	addHotKey(tt::input::Key_Enter,  noModifier, &Editor::hotKeyEnterPressed);
}


void Editor::setupNotifications()
{
	TT_NULL_ASSERT(m_levelData);
	m_levelData->registerObserver(m_this);
}


void Editor::setupUi()
{
 	m_persistentSettings.load(g_editorSettingsFile);
	if (AppGlobal::isInLevelEditorMode() && AppGlobal::isInDeveloperMode())
	{
		// Default to "user levels" filter when running in editor mode
		m_persistentSettings.loadLevelFilter = LoadLevelFilter_UserLevels;
	}
	
	// Restore the editor camera's field of view
	getEditorCamera().setFOV(m_persistentSettings.cameraFov, true);
	
	createMenu     (m_persistentSettings, m_gwenRoot.getCanvas());
	createStatusBar(m_persistentSettings, m_gwenRoot.getCanvas());
	
	// Create the root dock control to attach all other docks to.
	// This control fills the entire screen (but is not actually visible)
	m_ui.editorDock = new Gwen::Controls::DockBase(m_gwenRoot.getCanvas());
	// Cursor 255 = special Two Tribes hack added to GWEN to indicate "no cursor":
	// stop GWEN and the editor fighting over which system cursor should be active
	m_ui.editorDock->SetCursor(255);
	m_ui.editorDock->Dock(Gwen::Pos::Fill);
	
	Gwen::Controls::DockBase* dockLeft  = m_ui.editorDock->GetLeft();
	Gwen::Controls::DockBase* dockRight = m_ui.editorDock->GetRight();
	
	// Edit tools
	createToolsWindow(m_persistentSettings, m_gwenRoot.getCanvas());
	
	// Select missions
	createMissionsWindow(m_persistentSettings, m_gwenRoot.getCanvas());
	
	// Load Level dialog
	createLoadLevelWindow(m_persistentSettings, m_gwenRoot.getCanvas());
	
	// Entity library (docked top right)
	dockRight->GetTabControl()->AddPage(
			translateString("TITLE_ENTITY_LIBRARY"),
			createEntityLibraryWindow(m_persistentSettings, m_ui.editorDock));
	refreshEntityLibrary(&m_persistentSettings.entityLibraryExpansionState);
	
	// Tiles picker (docked bottom right)
	m_ui.tabTiles = dockRight->GetBottom()->GetTabControl()->AddPage(
			getTilesTabTitle(), createTilesWindow(m_persistentSettings, dockRight));
	
	// Entity Properties (docked left)
	dockLeft->GetTabControl()->AddPage(
			translateString("TITLE_ENTITY_PROPERTIES"),
			createEntityPropertiesWindow(m_persistentSettings, m_ui.editorDock));
	
	// If the Set Theme Colors window was visible, show it again
	if (m_persistentSettings.toolWindow[EditorSettings::ToolWindow_SetThemeColors].visible)
	{
		onOpenThemeColorPicker();
	}
	
	// Create a label to indicate whether GWEN has keyboard focus
	// (because editor hotkeys are disabled in that case, and GWEN
	// doesn't always indicate focus properly, which can be confusing)
	m_ui.labelFocusStatus = new Gwen::Controls::Label(m_gwenRoot.getCanvas());
	m_ui.labelFocusStatus->SetAlignment(Gwen::Pos::Right | Gwen::Pos::CenterV);
	m_ui.labelFocusStatus->SetTextColorOverride(Gwen::Colors::Red);
	m_ui.labelFocusStatus->SetText("GWEN has keyboard focus");  // FIXME: Localize this
	m_ui.labelFocusStatus->SizeToContents();
	m_ui.labelFocusStatus->SetHeight(m_ui.menuBar->Height());
	m_ui.labelFocusStatus->SetMargin(Gwen::Margin(0, 0, 15, 0));
	m_ui.labelFocusStatus->SetHidden(true);
	
	// Tweak the default sizes of the editor docks (bit of a hack)
	{
		s32 remainingHeight = getEditorCamera().getViewPortSize().y;
		
		static const int dockHeightTiles = 125;
		
		dockRight->GetBottom()->SetHeight(dockHeightTiles);
		remainingHeight -= dockRight->GetBottom()->Height();
		
		dockRight->SetHeight(remainingHeight);
		
		dockLeft ->SetWidth(m_persistentSettings.dockWidthLeft);
		dockRight->SetWidth(m_persistentSettings.dockWidthRight);
	}
	
#if !defined(TT_BUILD_FINAL)
	/* DEBUG: Test some GWEN controls/layouts/etc
	ui::Testbed* testbed = new ui::Testbed(m_gwenRoot.getCanvas());
	testbed->SetPos(400, 400);
	testbed->Show();
	// END DEBUG */
#endif
	
	// Set the default edit tool
	switchToTool(tools::ToolID_Draw);
}


void Editor::createMenu(const EditorSettings& p_settings, Gwen::Controls::Base* p_parent)
{
	m_ui.menuBar = new Gwen::Controls::MenuStrip(p_parent);
	
	const hotkey::Modifiers ctrl     (hotkey::Modifier_Ctrl);
	const hotkey::Modifiers ctrlShift(hotkey::M(hotkey::Modifier_Ctrl, hotkey::Modifier_Shift));
	const hotkey::Modifiers ctrlAlt  (hotkey::M(hotkey::Modifier_Ctrl, hotkey::Modifier_Alt));
	const hotkey::Modifiers alt      (hotkey::Modifier_Alt);
	const hotkey::Modifiers shift    (hotkey::Modifier_Shift);
	const hotkey::Modifiers noMod;
	
	// File
	{
		Gwen::Controls::MenuItem* menu = m_ui.menuBar->AddItem(translateString("MENU_FILE"));
		
#if EDITOR_SUPPORTS_SAVING
		addMenuItem(menu, translateString("MENU_FILE_NEW"), &Editor::onNewLevel, tt::input::Key_N, ctrl, "editor.ui.new");
#endif
		
		addMenuItem(menu, translateString("MENU_FILE_OPEN"), &Editor::showLoadLevelDialog, tt::input::Key_O, ctrl);
		
#if EDITOR_SUPPORTS_SAVING
		addMenuItem(menu, translateString("MENU_FILE_SAVE"),    &Editor::onSaveLevel,   tt::input::Key_S, ctrl, "editor.ui.save");
		addMenuItem(menu, translateString("MENU_FILE_SAVE_AS"), &Editor::onSaveLevelAs, tt::input::Key_S, ctrlShift);
#endif
		
#if defined(TT_STEAM_BUILD)
		addMenuItem(menu, translateString("MENU_FILE_PUBLISH_TO_WORKSHOP"), &Editor::onPublishToWorkshop, tt::input::Key_P, ctrlShift, "editor.ui.publish_steam");
		
		// FIXME: Localize this!
		//addMenuItem(menu, L"Browse Published Levels...", &Editor::onOpenPublishedLevelBrowser, tt::input::Key_B, ctrlShift);
#endif
		
#if EDITOR_SUPPORTS_ASSETS_SOURCE
		if (AppGlobal::isInDeveloperMode())
		{
			addMenuItem(menu, translateString("MENU_FILE_SAVE_SKIN"), &Editor::hotKeySaveLevelSkinShoebox, tt::input::Key_S, ctrlAlt);
		}
#endif
		
		addMenuItem(menu, translateString("MENU_FILE_OPEN_FOLDER"), &Editor::hotKeyShowLevelDir, tt::input::Key_O, ctrlShift, "editor.ui.folder_open");
		
		m_svnCommands = ui::SvnCommands::create(this, menu);
		
		menu->GetMenu()->AddDivider();
		
		//* FIXME: Doesn't work with this usage... editor should show hotkey, but Game handles the actual key
		Gwen::Controls::MenuItem* item = 0;
		// FIXME: Localize the hotkey string!
		item = menu->GetMenu()->AddItem(translateString("MENU_FILE_EXIT_EDITOR"), "", "Tab");
		item->onMenuItemSelected.Add(this, &Editor::onCloseEditor);
		// */ addMenuItem(menu, "Return To Game", &Editor::onCloseEditor, tt::input::Key_Tab, noMod);
		
		addMenuItem(menu, translateString("MENU_FILE_RELOADALL"), &Editor::onReloadAll, tt::input::Key_F5, ctrl);
		
		if (AppGlobal::isInLevelEditorMode())
		{
			addMenuItem(menu, translateString("MENU_FILE_EXIT_APP"), &Editor::onExitApp);
		}
	}
	
	// Edit
	{
		Gwen::Controls::MenuItem* menu = m_ui.menuBar->AddItem(translateString("MENU_EDIT"));
		
		addMenuItem(menu, translateString("MENU_EDIT_UNDO"), &Editor::hotKeyUndo, tt::input::Key_Z, ctrl, "editor.ui.undo");
		addMenuItem(menu, translateString("MENU_EDIT_REDO"), &Editor::hotKeyRedo, tt::input::Key_Y, ctrl, "editor.ui.redo");
		addHotKey(tt::input::Key_Z, ctrlShift, &Editor::hotKeyRedo); // alternative shortcut for Redo (Ctrl+Shift+Z)
		
		menu->GetMenu()->AddDivider();
		
		addMenuItem(menu, translateString("MENU_EDIT_CUT"),   &Editor::hotKeyCut,   tt::input::Key_X, ctrl, "editor.ui.clipboard_cut");
		addMenuItem(menu, translateString("MENU_EDIT_COPY"),  &Editor::hotKeyCopy,  tt::input::Key_C, ctrl, "editor.ui.clipboard_copy");
		addMenuItem(menu, translateString("MENU_EDIT_PASTE"), &Editor::hotKeyPaste, tt::input::Key_V, ctrl, "editor.ui.clipboard_paste");
	}
	
	// Select
	{
		Gwen::Controls::MenuItem* menu = m_ui.menuBar->AddItem(translateString("MENU_SELECT"));
		
		addMenuItem(menu, translateString("MENU_SELECT_ALL"),  &Editor::hotKeySelectAll,   tt::input::Key_A, ctrl, "editor.ui.select_all");
		addMenuItem(menu, translateString("MENU_SELECT_NONE"), &Editor::hotKeyDeselectAll, tt::input::Key_D, ctrl, "editor.ui.select_none");
		
		menu->GetMenu()->AddDivider();
		
		addMenuItem(menu, translateString("MENU_SELECT_FILL_WITH_TILES"),     &Editor::hotKeyFillTilesInSelection,        tt::input::Key_Backspace, alt,   "editor.ui.fill");
		addMenuItem(menu, translateString("MENU_SELECT_FILL_WITH_AIR_TILES"), &Editor::hotKeyFillTilesWithAirInSelection, tt::input::Key_Backspace, ctrl,  "editor.ui.fill");
		addMenuItem(menu, translateString("MENU_SELECT_ERASE_CONTENTS"),      &Editor::hotKeyEraseTilesInSelection,       tt::input::Key_Delete,    noMod, "editor.ui.delete");
		
		// Alternative shortcut for Erase Contents (Backspace)
		addHotKey(tt::input::Key_Backspace, noMod, &Editor::hotKeyEraseTilesInSelection); 
		
		menu->GetMenu()->AddDivider();
		
		addMenuItem(menu, translateString("MENU_SELECT_FLIP_HORIZONTAL"), &Editor::hotKeyFlipSelectionContentsHorizontally, "editor.ui.flip_horizontal");
		addMenuItem(menu, translateString("MENU_SELECT_FLIP_VERTICAL"),   &Editor::hotKeyFlipSelectionContentsVertically,   "editor.ui.flip_vertical");
	}
	
	// Level
	{
		Gwen::Controls::MenuItem* menu = m_ui.menuBar->AddItem(translateString("MENU_LEVEL"));
		
		addMenuCheckItem(menu, translateString("MENU_LEVEL_EDIT_THEME_TILES"),
		                 m_tileLayerMode == TileLayerMode_ThemeType,
		                 &Editor::onShowThemeTilesChanged, tt::input::Key_T, noMod);
		
		addMenuItem(menu, translateString("MENU_LEVEL_SET_THEME_COLORS"), &Editor::onOpenThemeColorPicker);
		
		m_ui.subMenuLevelBackground = menu->GetMenu()->AddItem(translateString("MENU_LEVEL_BACKGROUND"));
		refreshLevelBackgroundMenu();
		
		// Level theme
		{
			Gwen::Controls::MenuItem* subMenu = menu->GetMenu()->AddItem(translateString("MENU_LEVEL_THEME"));
			
			const level::ThemeType levelTheme = m_levelData->getLevelTheme();
			
			for (s32 i = 0; i < level::ThemeType_Count; ++i)
			{
				const level::ThemeType theme = static_cast<level::ThemeType>(i);
				if (theme == level::ThemeType_UseLevelDefault)
				{
					// Level default theme cannot be "use level default"
					// (so do not create a menu item for it)
					continue;
				}
			
				Gwen::Controls::MenuItem* item = subMenu->GetMenu()->AddItem(getThemeDisplayName(theme), "", "");
				m_ui.menuItemLevelTheme[theme] = item;
				item->SetName(level::getThemeTypeName(theme));
				item->SetCheckable(true);
				item->SetChecked(levelTheme == theme);
				item->onMenuItemSelected.Add(this, &Editor::onSelectLevelTheme);
			}
		}
		
		// Default Mission
		{
			Gwen::Controls::MenuItem* subMenu = menu->GetMenu()->AddItem(translateString("MENU_LEVEL_DEFAULT_MISSION"));
			
			const std::string defaultMission = m_levelData->getDefaultMission();
			
			tt::code::helpers::freePointerContainer(m_ui.menuItemsLevelDefaultMission);
			// FIXME: Remove hardcoded dependency on folder
			tt::str::StringSet filenames(tt::fs::utils::getFilesInDir("missions", "*.json"));
			filenames.insert("*");
			m_ui.menuItemsLevelDefaultMission.reserve(filenames.size());
			s32 i = 0;
			for (tt::str::StringSet::iterator it = filenames.begin(); it != filenames.end(); ++it)
			{
				const std::string& filename(*it);
				
				Gwen::Controls::MenuItem* item = subMenu->GetMenu()->AddItem(filename, "", "");
				m_ui.menuItemsLevelDefaultMission.push_back(item);
				item->SetName(filename);
				item->SetCheckable(true);
				item->SetChecked(defaultMission == filename);
				item->onMenuItemSelected.Add(this, &Editor::onSelectDefaultMission);
				++i;
			}
		}
	}
	
	// View
	{
		Gwen::Controls::MenuItem* menu = m_ui.menuBar->AddItem(translateString("MENU_VIEW"));
		Gwen::Controls::MenuItem* item = 0;
		
		GameLayer layer;
		
		layer = GameLayer_ShoeboxBackground;
		item  = addMenuCheckItem(menu, translateString("MENU_VIEW_SHOEBOX_BACKGROUND"),
		                         m_persistentSettings.gameLayersVisible.checkFlag(layer),
		                         &Editor::onGameLayerVisibilityChanged, tt::input::Key_B, shift);
		item->UserData.Set("gameLayer", layer);
		
		layer = GameLayer_Attributes;
		item  = addMenuCheckItem(menu, translateString("MENU_VIEW_ATTRIBUTES"),
		                         m_persistentSettings.gameLayersVisible.checkFlag(layer),
		                         &Editor::onGameLayerVisibilityChanged, tt::input::Key_L, noMod);
		item->UserData.Set("gameLayer", layer);
		
		layer = GameLayer_ShoeboxZero;
		item  = addMenuCheckItem(menu, translateString("MENU_VIEW_SHOEBOX_ZERO"),
		                         m_persistentSettings.gameLayersVisible.checkFlag(layer),
		                         &Editor::onGameLayerVisibilityChanged, tt::input::Key_L, shift);
		item->UserData.Set("gameLayer", layer);
		
		layer = GameLayer_ShoeboxForeground;
		item  = addMenuCheckItem(menu, translateString("MENU_VIEW_SHOEBOX_FOREGROUND"),
		                         m_persistentSettings.gameLayersVisible.checkFlag(layer),
		                         &Editor::onGameLayerVisibilityChanged);
		item->UserData.Set("gameLayer", layer);
		
		layer = GameLayer_Notes;
		item  = addMenuCheckItem(menu, translateString("MENU_VIEW_NOTES"),
		                         m_persistentSettings.gameLayersVisible.checkFlag(layer),
		                         &Editor::onGameLayerVisibilityChanged, tt::input::Key_N, shift);
		item->UserData.Set("gameLayer", layer);
		
		{
			// Entity render flags
			menu->GetMenu()->AddDivider();
			
			level::entity::editor::RenderFlag flag;
			
			flag = level::entity::editor::RenderFlag_SizeShapes;
			item = addMenuCheckItem(menu, translateString("MENU_VIEW_ENTITY_SIZE_SHAPES"),
					m_persistentSettings.entityRenderFlags.checkFlag(flag),
					&Editor::onEntityRenderFlagChanged, tt::input::Key_S, shift);
			item->UserData.Set("flag", flag);
			
			flag = level::entity::editor::RenderFlag_AllEntityIDs;
			item = addMenuCheckItem(menu, translateString("MENU_VIEW_ENTITY_IDS"),
					m_persistentSettings.entityRenderFlags.checkFlag(flag),
					&Editor::onEntityRenderFlagChanged, tt::input::Key_I, shift);
			item->UserData.Set("flag", flag);
			
			flag = level::entity::editor::RenderFlag_AllEntityReferences;
			item = addMenuCheckItem(menu, translateString("MENU_VIEW_ALL_ENTITY_REFERENCES"),
					m_persistentSettings.entityRenderFlags.checkFlag(flag),
					&Editor::onEntityRenderFlagChanged, tt::input::Key_R, shift);
			item->UserData.Set("flag", flag);
			
#if !defined(TT_BUILD_FINAL)  // reference labels use debug font for text rendering, which isn't available in final builds
			flag = level::entity::editor::RenderFlag_EntityReferencesLabels;
			item = addMenuCheckItem(menu, translateString("MENU_VIEW_ENTITY_REFERENCE_LABELS"),
					m_persistentSettings.entityRenderFlags.checkFlag(flag),
					&Editor::onEntityRenderFlagChanged, tt::input::Key_R, ctrlShift);
			item->UserData.Set("flag", flag);
#endif
		}
		
		menu->GetMenu()->AddDivider();
		
		addMenuItem(menu, translateString("MENU_VIEW_ZOOM_IN"),         &Editor::hotKeyZoomIn,              tt::input::Key_Plus,  ctrl, "editor.ui.zoom_in");
		addMenuItem(menu, translateString("MENU_VIEW_ZOOM_OUT"),        &Editor::hotKeyZoomOut,             tt::input::Key_Minus, ctrl, "editor.ui.zoom_out");
		addMenuItem(menu, translateString("MENU_VIEW_CENTER_ON_LEVEL"), &Editor::hotKeyCenterCameraOnLevel, tt::input::Key_0,     ctrl);
		
		// Alternative hotkeys for zoom in/out (and an undocumented "fit level in screen" hotkey)
		addHotKey(tt::input::Key_PageUp,   noMod,   &Editor::hotKeyZoomIn);
		addHotKey(tt::input::Key_PageDown, noMod,   &Editor::hotKeyZoomOut);
		addHotKey(tt::input::Key_0,        ctrlAlt, &Editor::hotKeyFitLevelInScreen);
		
		menu->GetMenu()->AddDivider();
		
		addMenuCheckItem(menu, translateString("MENU_VIEW_STATUSBAR"),
		                 p_settings.statusBarVisible, &Editor::onShowStatusBarChanged);
	}
	
	// Window
	{
		Gwen::Controls::MenuItem* menu = m_ui.menuBar->AddItem(translateString("MENU_WINDOW"));
		
		addMenuCheckItem(menu, translateString("MENU_WINDOW_TOOLS"),
		                 p_settings.toolWindow[EditorSettings::ToolWindow_EditTools].visible,
		                 &Editor::onShowEditToolsChanged);
		
		addMenuCheckItem(menu, translateString("MENU_WINDOW_MISSIONS"),
		                 p_settings.toolWindow[EditorSettings::ToolWindow_SelectMission].visible,
		                 &Editor::onShowSelectMissionChanged, tt::input::Key_M, ctrl);
		
		/*
#if defined(TT_STEAM_BUILD) && !defined(TT_BUILD_FINAL)
		addMenuItem(menu, L"Steam Workshop Test", &Editor::onOpenSteamWorkshopTest);
#endif
		*/
	}
	
	// Utilities
	{
		Gwen::Controls::MenuItem* menu = m_ui.menuBar->AddItem(translateString("MENU_UTILITIES"));
		
		addMenuItem(menu, translateString("MENU_UTILITIES_ERASE_ALL_TILES"),     &Editor::onClearTiles);
		addMenuItem(menu, translateString("MENU_UTILITIES_REMOVE_ALL_ENTITIES"), &Editor::onClearEntities);
		//addMenuItem(menu, translateString("MENU_UTILITIES_ERASE_ALL_CONTENTS"),  &Editor::onClearWholeLevel);
		
		menu->GetMenu()->AddDivider();
		
		addMenuCheckItem(menu, translateString("MENU_UTILITIES_AUTO_SYNC_CHANGES"),
		                 p_settings.autoSyncEntityChanges, &Editor::onAutoSyncEntityChangesChanged);
		
#if defined(TT_STEAM_BUILD) && !defined(TT_BUILD_FINAL)
		if (AppGlobal::isInDeveloperMode())
		{
			addMenuItem(menu, L"[DEBUG] Steam Cloud File Manager", &Editor::onOpenDebugSteamCloudFileManager);
		}
#endif
		
#if EDITOR_SUPPORTS_HELP_PAGE
		menu->GetMenu()->AddDivider();
		addMenuItem(menu, translateString("MENU_UTILITIES_SHOW_HELP"), &Editor::hotKeyShowHelp, tt::input::Key_F1, noMod, "editor.ui.help");
#endif
	}
	
	// Quick selection of often used entities (e.g., LevelSettings and PlayerBot)
	// FIXME: Ideally this should be handled by script but that requires the Editor to be scriptable
	{
		Gwen::Controls::MenuItem* menu = m_ui.menuBar->AddItem(translateString("MENU_QUICKSELECT"));
		
		addMenuItem(menu, translateString("MENU_QUICKSELECT_LEVELSETTINGS"), 
			&Editor::hotKeySelectLevelSettings, tt::input::Key_L, ctrl);
		addMenuItem(menu, translateString("MENU_QUICKSELECT_PLAYERBOT"),
			&Editor::hotKeySelectPlayerBot, tt::input::Key_P, ctrl);
	}
}


void Editor::createStatusBar(const EditorSettings& p_settings, Gwen::Controls::Base* p_parent)
{
	m_ui.statusBar = new Gwen::Controls::StatusBar(p_parent);
	m_ui.statusBar->SetText("");
	m_ui.statusBar->SetHidden(p_settings.statusBarVisible == false);
	
	Gwen::Controls::Label* label = 0;
	
	label = new Gwen::Controls::Label(m_ui.statusBar);
	m_ui.labelInfoLevelName = label;
	label->SetWidth(400);
	m_ui.statusBar->AddControl(label, false);
	
	label = new Gwen::Controls::Label(m_ui.statusBar);
	m_ui.labelInfoHasChanges = label;
	label->SetWidth(130);
	m_ui.statusBar->AddControl(label, false);
	
	// NOTE: These labels are added in reverse order, because they're right-aligned in the status bar
	
	// Entity count
	label = new Gwen::Controls::Label(m_ui.statusBar);
	m_ui.labelInfoEntityCount = label;
	label->SetText("");
	label->SetWidth(30);
	m_ui.statusBar->AddControl(label, true);
	
	label = new Gwen::Controls::Label(m_ui.statusBar);
	label->SetText(translateString("STATUSBAR_ENTITIES"));
	label->SetWidth(50);
	m_ui.statusBar->AddControl(label, true);
	
	// Selection size
	label = new Gwen::Controls::Label(m_ui.statusBar);
	m_ui.labelInfoSelectionHeight = label;
	label->SetText("");
	label->SetWidth(30);
	m_ui.statusBar->AddControl(label, true);
	
	label = new Gwen::Controls::Label(m_ui.statusBar);
	label->SetText(translateString("STATUSBAR_SELECTION_HEIGHT"));
	label->SetWidth(15);
	m_ui.statusBar->AddControl(label, true);
	
	label = new Gwen::Controls::Label(m_ui.statusBar);
	m_ui.labelInfoSelectionWidth = label;
	label->SetText("");
	label->SetWidth(30);
	m_ui.statusBar->AddControl(label, true);
	
	label = new Gwen::Controls::Label(m_ui.statusBar);
	label->SetText(translateString("STATUSBAR_SELECTION_WIDTH"));
	label->SetWidth(15);
	m_ui.statusBar->AddControl(label, true);
	
	// Cursor position
	label = new Gwen::Controls::Label(m_ui.statusBar);
	m_ui.labelInfoPosY = label;
	label->SetText("");
	label->SetWidth(30);
	m_ui.statusBar->AddControl(label, true);
	
	label = new Gwen::Controls::Label(m_ui.statusBar);
	label->SetText(translateString("STATUSBAR_CURSOR_Y"));
	label->SetWidth(15);
	m_ui.statusBar->AddControl(label, true);
	
	label = new Gwen::Controls::Label(m_ui.statusBar);
	m_ui.labelInfoPosX = label;
	label->SetText("");
	label->SetWidth(30);
	m_ui.statusBar->AddControl(label, true);
	
	label = new Gwen::Controls::Label(m_ui.statusBar);
	label->SetText(translateString("STATUSBAR_CURSOR_X"));
	label->SetWidth(15);
	m_ui.statusBar->AddControl(label, true);
}


Gwen::Controls::Base* Editor::createToolsWindow(const EditorSettings& p_settings,
                                                Gwen::Controls::Base* p_parent)
{
	const EditorSettings::ToolWindowSettings& windowSettings(
			p_settings.toolWindow[EditorSettings::ToolWindow_EditTools]);
	
	m_ui.windowTools = new Gwen::Controls::WindowControl(p_parent);
	m_ui.windowTools->SetTitle(translateString("WINDOW_TOOLS_TITLE"));
	m_ui.windowTools->SetMinimumSize(Gwen::Point(50, 50));
	m_ui.windowTools->SetPos (windowSettings.windowRect.getPosition().x, windowSettings.windowRect.getPosition().y);
	m_ui.windowTools->SetSize(windowSettings.windowRect.getWidth(), windowSettings.windowRect.getHeight());
	m_ui.windowTools->SetDeleteOnClose(false);
	m_ui.windowTools->SetClosable(false);
	m_ui.windowTools->SetHidden(windowSettings.visible == false);
	
	Gwen::Controls::Base* toolsParent = m_ui.windowTools;
	
	m_ui.listTools = new tt::gwen::ButtonList(toolsParent);
	
	typedef std::pair<tools::ToolID, std::string> ToolItem;
	typedef std::vector<ToolItem> ToolItems;
	
	ToolItems toolItems;
	toolItems.push_back(ToolItem(tools::ToolID_Draw,        "TOOL_DRAW"));
	toolItems.push_back(ToolItem(tools::ToolID_FloodFill,   "TOOL_FLOODFILL"));
	toolItems.push_back(ToolItem(tools::ToolID_Hand,        "TOOL_HAND"));
	toolItems.push_back(ToolItem(tools::ToolID_BoxSelect,   "TOOL_BOXSELECT"));
	toolItems.push_back(ToolItem(tools::ToolID_Resize,      "TOOL_RESIZE"));
	toolItems.push_back(ToolItem(tools::ToolID_EntityPaint, "TOOL_ENTITYDRAW"));
	toolItems.push_back(ToolItem(tools::ToolID_EntityMove,  "TOOL_ENTITYMOVE"));
	toolItems.push_back(ToolItem(tools::ToolID_Notes,       "TOOL_NOTES"));
	
	Gwen::Controls::Button* firstItem = 0;
	Gwen::Controls::Button* item      = 0;
	for (ToolItems::iterator it = toolItems.begin(); it != toolItems.end(); ++it)
	{
		const std::string toolName(tools::getToolIDName((*it).first));
		item = m_ui.listTools->AddItem("", "editor.tools.tool_" + toolName, toolName);
		item->SetToolTip(translateString((*it).second));
		
		if (firstItem == 0) firstItem = item;
	}
	
	m_ui.listTools->SetSelectedRow(firstItem);
	m_ui.listTools->onRowSelected.Add(this, &Editor::selectedTool);
	
	m_ui.listTools->Dock(Gwen::Pos::Fill);
	
	return m_ui.windowTools;
}


Gwen::Controls::Base* Editor::createMissionsWindow(const EditorSettings& p_settings,
                                                   Gwen::Controls::Base* p_parent)
{
	const EditorSettings::ToolWindowSettings& windowSettings(
			p_settings.toolWindow[EditorSettings::ToolWindow_SelectMission]);
	
	m_ui.windowMissions = new Gwen::Controls::WindowControl(p_parent);
	m_ui.windowMissions->SetTitle(translateString("WINDOW_MISSIONS_TITLE"));
	m_ui.windowMissions->SetMinimumSize(Gwen::Point(150, 150));
	m_ui.windowMissions->SetPos (windowSettings.windowRect.getPosition().x, windowSettings.windowRect.getPosition().y);
	m_ui.windowMissions->SetSize(windowSettings.windowRect.getWidth(), windowSettings.windowRect.getHeight());
	m_ui.windowMissions->SetDeleteOnClose(false);
	m_ui.windowMissions->SetClosable(true);
	m_ui.windowMissions->SetHidden(windowSettings.visible == false);
	
	// The list of missions
	tt::gwen::ListBoxEx* listEx = new tt::gwen::ListBoxEx(m_ui.windowMissions);
	m_ui.listMissions = listEx;
	m_ui.listMissions->SetAllowMultiSelect(false);
	m_ui.listMissions->SetTabbable(true);
	m_ui.listMissions->SetKeyboardInputEnabled(true);
	m_ui.listMissions->Dock(Gwen::Pos::Fill);
	
	listEx->onRowSelected.Add(this, &Editor::selectedMission);
	
	refreshMissionList();
	
	return m_ui.windowMissions;
}


Gwen::Controls::Base* Editor::createLoadLevelWindow(const EditorSettings& p_settings,
                                                    Gwen::Controls::Base* p_parent)
{
	const EditorSettings::ToolWindowSettings& windowSettings(
			p_settings.toolWindow[EditorSettings::ToolWindow_LoadLevel]);
	
	const tt::math::Point2 scrSize(getEditorCamera().getViewPortSize());
	const tt::math::PointRect scrRect(tt::math::Point2(0, 0), scrSize.x, scrSize.y);
	
	tt::math::PointRect windowRect(windowSettings.windowRect);
	if (windowRect.getPosition() != tt::math::Point2(-1, -1) &&
	    scrRect.intersects(windowSettings.windowRect, &windowRect) == false)
	{
		// Window rect isn't even within the current screen area: use some defaults
		windowRect = tt::math::PointRect(tt::math::Point2(-1, -1), 300, 400);
	}
	
	// The window itself
	m_ui.windowLoadLevel = new Gwen::Controls::WindowControl(p_parent);
	m_ui.windowLoadLevel->SetTitle(translateString("WINDOW_LOADLEVEL_TITLE"));
	m_ui.windowLoadLevel->SetMinimumSize(Gwen::Point(50, 50));
	m_ui.windowLoadLevel->SetSize(windowRect.getWidth(), windowRect.getHeight());
	m_ui.windowLoadLevelNeedsCentering = (windowRect.getPosition() == tt::math::Point2(-1, -1));
	m_ui.windowLoadLevel->SetPos(windowRect.getPosition().x, windowRect.getPosition().y);
	m_ui.windowLoadLevel->SetDeleteOnClose(false);
	m_ui.windowLoadLevel->SetClosable(true);
	m_ui.windowLoadLevel->SetHidden(true);
	
	// Buttons at the top to select the levels to show
	if (AppGlobal::isInDeveloperMode())
	{
		tt::gwen::EqualSizes* buttonPanel = new tt::gwen::EqualSizes(m_ui.windowLoadLevel);
		buttonPanel->SetHorizontal(true);
		buttonPanel->SetInnerSpacing(7);
		buttonPanel->SetPadding(Gwen::Padding(0, 0, 0, 3));
		buttonPanel->SetHeight(25);
		buttonPanel->Dock(Gwen::Pos::Top);
		
		typedef std::pair<LoadLevelFilter, const char*> Filter;  // filter value, loc ID
		static const Filter filters[] =
		{
			Filter(LoadLevelFilter_AllLevels,  "WINDOW_LOADLEVEL_FILTER_ALL"),
			Filter(LoadLevelFilter_UserLevels, "WINDOW_LOADLEVEL_FILTER_USERLEVELS"),
			Filter(LoadLevelFilter_Slices,     "WINDOW_LOADLEVEL_FILTER_SLICES")
		};
		static const size_t filterCount = sizeof(filters) / sizeof(filters[0]);
		
		for (size_t i = 0; i < filterCount; ++i)
		{
			Gwen::Controls::Button* button = new Gwen::Controls::Button(buttonPanel);
			m_ui.windowLoadLevelFilterButton[filters[i].first] = button;
			button->SetText(translateString(filters[i].second));
			button->SetIsToggle(true);
			button->UserData.Set("filter", filters[i].first);
			button->SetToggleState(p_settings.loadLevelFilter == filters[i].first);
			button->onToggleOn .Add(this, &Editor::onLoadLevelDialog_FilterSelected);
			button->onToggleOff.Add(this, &Editor::onLoadLevelDialog_FilterDeselected);
		}
	}
	
	// Buttons at the bottom for closing the window or opening the selected level
	Gwen::Controls::Base* buttonPanel = new Gwen::Controls::Base(m_ui.windowLoadLevel);
	buttonPanel->SetPadding(Gwen::Padding(0, 3, 0, 0));
	
	Gwen::Controls::Button* buttonLoad = new Gwen::Controls::Button(buttonPanel);
	buttonLoad->SetText(translateString("BUTTON_OPEN"));
	buttonLoad->SetSize(100, 25);
	buttonLoad->Dock(Gwen::Pos::Right);
	buttonLoad->onPress.Add(this, &Editor::onLoadLevelDialog_OpenClicked);
	
	Gwen::Controls::Button* buttonCancel = new Gwen::Controls::Button(buttonPanel);
	buttonCancel->SetText(translateString("BUTTON_CANCEL"));
	buttonCancel->SetSize(100, 25);
	buttonCancel->Dock(Gwen::Pos::Left);
	buttonCancel->onPress.Add(m_ui.windowLoadLevel, &Gwen::Controls::WindowControl::CloseButtonPressed);
	
	buttonPanel->Dock(Gwen::Pos::Bottom);
	buttonPanel->SetHeight(25);
	
	// The list of levels
	tt::gwen::ListBoxEx* listEx = new tt::gwen::ListBoxEx(m_ui.windowLoadLevel);
	m_ui.listLevels = listEx;
	m_ui.listLevels->SetAllowMultiSelect(false);
	m_ui.listLevels->SetTabbable(true);
	m_ui.listLevels->SetKeyboardInputEnabled(true);
	m_ui.listLevels->Dock(Gwen::Pos::Fill);
	listEx->onKeyEscape.Add(m_ui.windowLoadLevel, &Gwen::Controls::WindowControl::CloseButtonPressed);
	listEx->onKeyReturn       .Add(this, &Editor::onLoadLevelDialog_OpenClicked);
	listEx->onRowDoubleClicked.Add(this, &Editor::onLoadLevelDialog_OpenClicked);
	
	m_showLoadLevelThisFrame = windowSettings.visible;
	
	return m_ui.windowLoadLevel;
}


Gwen::Controls::Base* Editor::createTilesWindow(const EditorSettings& /*p_settings*/,
                                                Gwen::Controls::Base* p_parent)
{
	// Create a picker for the paint tiles
	m_ui.listTiles = new tt::gwen::ButtonList(p_parent);
	
	Gwen::Controls::Button* firstItem = ui::addCollisionTypesToPicker(m_ui.listTiles, "");
	
	m_ui.listTiles->SetSelectedRow(firstItem);
	m_ui.listTiles->onRowSelected.Add(this, &Editor::selectedPaintTile);
	
	return m_ui.listTiles;
}


Gwen::Controls::Base* Editor::createEntityLibraryWindow(const EditorSettings& /*p_settings*/,
                                                        Gwen::Controls::Base* p_parent)
{
	m_ui.listEntities = new tt::gwen::GroupedButtonList(p_parent);
	m_ui.listEntities->onRowSelected.Add(this, &Editor::selectedEntityInLibrary);
	return m_ui.listEntities;
}


Gwen::Controls::Base* Editor::createEntityPropertiesWindow(const EditorSettings& /*p_settings*/,
                                                           Gwen::Controls::Base* p_parent)
{
	m_ui.listEntityProperties = new ui::EntityPropertyList(p_parent);
	m_ui.listEntityProperties->setEditor(this);
	return m_ui.listEntityProperties;
}


Gwen::Controls::MenuItem* Editor::addMenuItem(Gwen::Controls::MenuItem* p_targetMenu,
                                              const std::wstring&       p_displayName,
                                              ActionFunc                p_actionFunc,
                                              const std::string&        p_iconName)
{
	Gwen::Controls::MenuItem* item = p_targetMenu->GetMenu()->AddItem(p_displayName, p_iconName, "");
	item->onMenuItemSelected.Add(this, p_actionFunc);
	return item;
}


Gwen::Controls::MenuItem* Editor::addMenuItem(Gwen::Controls::MenuItem* p_targetMenu,
                                              const std::wstring&       p_displayName,
                                              ActionFunc                p_actionFunc,
                                              tt::input::Key            p_actionKey,
                                              const hotkey::Modifiers&  p_modifiers,
                                              const std::string&        p_iconName)
{
	const std::string hotkeyDisplayName(hotkey::getDisplayName(p_actionKey, p_modifiers));
	Gwen::Controls::MenuItem* item = p_targetMenu->GetMenu()->AddItem(p_displayName, p_iconName, hotkeyDisplayName);
	item->onMenuItemSelected.Add(this, p_actionFunc);
	addHotKey(p_actionKey, p_modifiers, p_actionFunc);
	return item;
}


Gwen::Controls::MenuItem* Editor::addMenuCheckItem(Gwen::Controls::MenuItem* p_targetMenu,
                                                   const std::wstring&       p_displayName,
                                                   bool                      p_checked,
                                                   ActionFunc                p_actionFunc)
{
	Gwen::Controls::MenuItem* item = p_targetMenu->GetMenu()->AddItem(p_displayName, "", "");
	item->SetCheckable(true);
	item->SetChecked(p_checked);
	item->onCheckChange.Add(this, p_actionFunc);
	return item;
}


Gwen::Controls::MenuItem* Editor::addMenuCheckItem(Gwen::Controls::MenuItem* p_targetMenu,
                                                   const std::wstring&       p_displayName,
                                                   bool                      p_checked,
                                                   ActionFunc                p_actionFunc,
                                                   tt::input::Key            p_actionKey,
                                                   const hotkey::Modifiers&  p_modifiers)
{
	const std::string hotkeyDisplayName(hotkey::getDisplayName(p_actionKey, p_modifiers));
	Gwen::Controls::MenuItem* item = p_targetMenu->GetMenu()->AddItem(p_displayName, "", hotkeyDisplayName);
	item->SetCheckable(true);
	item->SetChecked(p_checked);
	item->onCheckChange.Add(this, p_actionFunc);
	//addHotKey(p_actionKey, p_modifiers, p_actionFunc);
	addToggleMenuItemHotKey(p_actionKey, p_modifiers, item);
	return item;
}


Gwen::Controls::MenuItem* Editor::addMenuCheckItem(Gwen::Controls::MenuItem* p_targetMenu,
                                                   const std::wstring&       p_displayName,
                                                   bool                      p_checked,
                                                   ActionFuncWithSource      p_actionFunc)
{
	Gwen::Controls::MenuItem* item = p_targetMenu->GetMenu()->AddItem(p_displayName, "", "");
	item->SetCheckable(true);
	item->SetChecked(p_checked);
	item->onCheckChange.Add(this, p_actionFunc);
	return item;
}


Gwen::Controls::MenuItem* Editor::addMenuCheckItem(Gwen::Controls::MenuItem* p_targetMenu,
                                                   const std::wstring&       p_displayName,
                                                   bool                      p_checked,
                                                   ActionFuncWithSource      p_actionFunc,
                                                   tt::input::Key            p_actionKey,
                                                   const hotkey::Modifiers&  p_modifiers)
{
	const std::string hotkeyDisplayName(hotkey::getDisplayName(p_actionKey, p_modifiers));
	Gwen::Controls::MenuItem* item = p_targetMenu->GetMenu()->AddItem(p_displayName, "", hotkeyDisplayName);
	item->SetCheckable(true);
	item->SetChecked(p_checked);
	item->onCheckChange.Add(this, p_actionFunc);
	addToggleMenuItemHotKey(p_actionKey, p_modifiers, item);
	return item;
}


void Editor::addHotKey(tt::input::Key            p_actionKey,
                       const hotkey::Modifiers&  p_modifiers,
                       ActionFunc                p_actionFunc)
{
	getHotKeyMgr().addHotKey(p_actionKey, p_modifiers,
			hotkey::Handler<Editor>::create(this, p_actionFunc));
}


void Editor::addToggleMenuItemHotKey(tt::input::Key            p_actionKey,
                                     const hotkey::Modifiers&  p_modifiers,
                                     Gwen::Controls::MenuItem* p_menuItem)
{
	getHotKeyMgr().addHotKey(p_actionKey, p_modifiers,
			hotkey::Handler<Editor>::create(this, &Editor::hotKeyToggleMenuItem, p_menuItem));
}


void Editor::saveEditorSettings()
{
	m_persistentSettings.cameraFov             = getEditorCamera().getTargetFOV();
	m_persistentSettings.statusBarVisible      = (m_ui.statusBar->Hidden() == false);
	if (m_ui.editorDock->HasLeft())
	{
		m_persistentSettings.dockWidthLeft = m_ui.editorDock->GetLeft()->Width();
	}
	if (m_ui.editorDock->HasRight())
	{
		m_persistentSettings.dockWidthRight = m_ui.editorDock->GetRight()->Width();
	}
	
	m_persistentSettings.entityLibraryExpansionState = m_ui.listEntities->GetAllGroupExpansionState();
	
	
	{
		EditorSettings::ToolWindowSettings& settingsEditTools(
				m_persistentSettings.toolWindow[EditorSettings::ToolWindow_EditTools]);
		settingsEditTools.visible    = (m_ui.windowTools->Hidden() == false);
		settingsEditTools.windowRect = tt::math::PointRect(
				tt::math::Point2(m_ui.windowTools->X(), m_ui.windowTools->Y()),
				m_ui.windowTools->Width(),
				m_ui.windowTools->Height());
	}
	
	{
		EditorSettings::ToolWindowSettings& settingsSelectMission(
				m_persistentSettings.toolWindow[EditorSettings::ToolWindow_SelectMission]);
		settingsSelectMission.visible    = (m_ui.windowMissions->Hidden() == false);
		settingsSelectMission.windowRect = tt::math::PointRect(
				tt::math::Point2(m_ui.windowMissions->X(), m_ui.windowMissions->Y()),
				m_ui.windowMissions->Width(),
				m_ui.windowMissions->Height());
	}
	
	{
		EditorSettings::ToolWindowSettings& settingsLoadLevel(
				m_persistentSettings.toolWindow[EditorSettings::ToolWindow_LoadLevel]);
		settingsLoadLevel.visible    = (m_ui.windowLoadLevel->Hidden() == false);
		// If dialog wasn't opened this run, the window position won't be accurate:
		// centering is done manually, so the window will still have the default position
		if (m_loadLevelDialogHasBeenOpen)
		{
			settingsLoadLevel.windowRect = tt::math::PointRect(
					tt::math::Point2(m_ui.windowLoadLevel->X(), m_ui.windowLoadLevel->Y()),
					m_ui.windowLoadLevel->Width(),
					m_ui.windowLoadLevel->Height());
		}
	}
	
	{
		EditorSettings::ToolWindowSettings& windowSettings(
				m_persistentSettings.toolWindow[EditorSettings::ToolWindow_SetThemeColors]);
		
		windowSettings.visible = (m_ui.windowSetThemeColor != 0 &&
		                          m_ui.windowSetThemeColor->Hidden() == false);
		
		if (m_ui.windowSetThemeColor != 0)
		{
			windowSettings.windowRect = tt::math::PointRect(
					tt::math::Point2(m_ui.windowSetThemeColor->X(), m_ui.windowSetThemeColor->Y()),
					m_ui.windowSetThemeColor->Width(),
					m_ui.windowSetThemeColor->Height());
		}
	}
	
	m_persistentSettings.save(g_editorSettingsFile);
}


void Editor::createSelectionIndicator()
{
	using tt::engine::renderer::QuadSprite;
	static const tt::engine::renderer::ColorRGBA areaColor(0, 0, 0, 127);
	m_selectionQuadTop    = QuadSprite::createQuad(1.0f, 1.0f, areaColor);
	m_selectionQuadBottom = QuadSprite::createQuad(1.0f, 1.0f, areaColor);
	m_selectionQuadLeft   = QuadSprite::createQuad(1.0f, 1.0f, areaColor);
	m_selectionQuadRight  = QuadSprite::createQuad(1.0f, 1.0f, areaColor);
	
	m_selectionOutline.reset(new tt::engine::renderer::TrianglestripBuffer(
		5,
		1,
		tt::engine::renderer::TexturePtr(),
		tt::engine::renderer::BatchFlagTrianglestrip_UseVertexColor,
		tt::engine::renderer::TrianglestripBuffer::PrimitiveType_LineStrip));
	tt::engine::renderer::BufferVtxUV<1> defaultVertex;
	defaultVertex.setColor(tt::engine::renderer::ColorRGB::white);
	m_selectionOutline->resize<1>(5, defaultVertex);
	
	updateSelectionIndicator();
}


void Editor::updateSelectionIndicator()
{
	const tt::math::VectorRect selectRect(level::tileToWorld(getLevelIntersectedSelectionRect()));
	const tt::math::VectorRect levelRect(level::tileToWorld(m_levelRect));
	
	m_selectionQuadTop->setWidth(levelRect.getWidth());
	m_selectionQuadTop->setHeight(levelRect.getMaxInside().y - selectRect.getMaxInside().y);
	m_selectionQuadTop->setPosition(levelRect.getCenterPosition().x,
		-(selectRect.getMaxEdge().y + (m_selectionQuadTop->getHeight() * 0.5f)), 0.0f);
	
	m_selectionQuadBottom->setWidth(levelRect.getWidth());
	m_selectionQuadBottom->setHeight(selectRect.getMin().y);
	m_selectionQuadBottom->setPosition(levelRect.getCenterPosition().x, -selectRect.getMin().y * 0.5f, 0.0f);
	
	m_selectionQuadLeft->setWidth(selectRect.getMin().x);
	m_selectionQuadLeft->setHeight(selectRect.getHeight());
	m_selectionQuadLeft->setPosition(selectRect.getLeft() * 0.5f, -selectRect.getCenterPosition().y, 0.0f);
	
	m_selectionQuadRight->setWidth(levelRect.getMaxInside().x - selectRect.getMaxInside().x);
	m_selectionQuadRight->setHeight(selectRect.getHeight());
	m_selectionQuadRight->setPosition(
		selectRect.getMaxEdge().x + (m_selectionQuadRight->getWidth() * 0.5f),
		-selectRect.getCenterPosition().y, 0.0f);
	
	m_selectionQuadTop->update();
	m_selectionQuadBottom->update();
	m_selectionQuadLeft->update();
	m_selectionQuadRight->update();
	
	// Update the selection lines
	const tt::math::VectorRect unclippedSelectRect(level::tileToWorld(m_selectionRect));
	const real z = 0.0f;
	const tt::math::Vector3 topLeft    (unclippedSelectRect.getMin().x,     unclippedSelectRect.getMin().y,     z);
	const tt::math::Vector3 topRight   (unclippedSelectRect.getMaxEdge().x, unclippedSelectRect.getMin().y,     z);
	const tt::math::Vector3 bottomRight(unclippedSelectRect.getMaxEdge().x, unclippedSelectRect.getMaxEdge().y, z);
	const tt::math::Vector3 bottomLeft (unclippedSelectRect.getMin().x,     unclippedSelectRect.getMaxEdge().y, z);
	
	m_selectionOutline->modifyVtx<1>(0).setPosition(topLeft);
	m_selectionOutline->modifyVtx<1>(1).setPosition(topRight);
	m_selectionOutline->modifyVtx<1>(2).setPosition(bottomRight);
	m_selectionOutline->modifyVtx<1>(3).setPosition(bottomLeft);
	m_selectionOutline->modifyVtx<1>(4).setPosition(topLeft);
	
	m_selectionOutline->applyChanges();
}


void Editor::hideEditorGUI()
{
	m_ui.menuBar->CloseAll();
	m_editorActive = false;
}


void Editor::handleToolInput()
{
	if (m_tools[m_activePaintTool] == 0 && m_editorMode == EditorMode_Normal)
	{
		return;
	}
	
	// No tool input when editing a note
	if (m_editorMode == EditorMode_EditNote)
	{
		return;
	}
	
	const input::Controller::State::EditorState& editorState    (AppGlobal::getController(tt::input::ControllerIndex_One).cur.editor );
	const input::Controller::State::EditorState& prevEditorState(AppGlobal::getController(tt::input::ControllerIndex_One).prev.editor);
	
	if (m_editorMode == EditorMode_PickEntity)
	{
		const tt::math::Vector2 worldPos(getEditorCamera().screenToWorld(editorState.pointer));
		level::entity::EntityInstances entitiesUnderPointer(findEntitiesAtWorldPos(worldPos));
		
		level::entity::EntityInstancePtr pickingEntity;
		for (level::entity::EntityInstances::reverse_iterator it = entitiesUnderPointer.rbegin();
		     it != entitiesUnderPointer.rend(); ++it)
		{
			if (isAllowedEntityInstance(*it))
			{
				pickingEntity = *it;
				break;
			}
		}
		
		// Pick entity
		if (m_spacebarScrollMode.down == false && editorState.pointerLeft.pressed && pickingEntity != 0)
		{
			m_ui.listEntityProperties->entityPicked(pickingEntity, m_entityPickPropertyName);
			++m_entityPickCount;
			// Make sure we can't pick this entity again
			m_entityPickDisallowedIDs.insert(pickingEntity->getID());
			pickingEntity.reset();
		}
		
		// Cancel picking
		if (editorState.keys[tt::input::Key_Escape].pressed)
		{
			cancelEntityPickMode();
			pickingEntity.reset();
			
			// Cancelled; undo all picked entities
			undo(m_entityPickCount);
		}
		
		// Accept picking
		if (editorState.keys[tt::input::Key_Enter].pressed || editorState.pointerRight.pressed ||
		    (m_entityPickCount >= m_entityPickMaxCount && m_entityPickMaxCount > 0))
		{
			cancelEntityPickMode();
			pickingEntity.reset();
		}
		
		if (m_activePaintTool != tools::ToolID_Hand)
		{
			setCursor((pickingEntity != 0) ? EditCursor_EntityPickerValid : EditCursor_EntityPickerInvalid);
			return;
		}
	}
	
	const tt::input::Button& keyCtrl (editorState.keys[tt::input::Key_Control]);
	const tt::input::Button& keyAlt  (editorState.keys[tt::input::Key_Alt    ]);
	const tt::input::Button& keyShift(editorState.keys[tt::input::Key_Shift  ]);
	
	const bool anyPointerButtonDown =
		editorState.pointerLeft.down   ||
		editorState.pointerMiddle.down ||
		editorState.pointerRight.down;
	tt::input::Pointer pointerCur(editorState.pointer);
	
	// Check whether the pointer should be locked to a specific axis
	m_axisLockTrigger.update(keyShift.down && anyPointerButtonDown);
	
	if (m_axisLockTrigger.pressed)
	{
		m_axisLockStartScreenPos = pointerCur;
		m_lockAxis               = LockAxis_None;
	}
	else if (m_axisLockTrigger.down || m_axisLockTrigger.released)
	{
		if (m_lockAxis == LockAxis_None)
		{
			static const s32 pixelMovementRequiredForLock = 15;
			
			const tt::math::Point2 pointerMovement(std::abs(pointerCur.x - m_axisLockStartScreenPos.x),
			                                       std::abs(pointerCur.y - m_axisLockStartScreenPos.y));
			if (pointerMovement.x > pixelMovementRequiredForLock ||
			    pointerMovement.y > pixelMovementRequiredForLock)
			{
				m_lockAxis = (pointerMovement.x > pointerMovement.y) ? LockAxis_X : LockAxis_Y;
			}
		}
		
		if (m_lockAxis == LockAxis_X)
		{
			// Restrict movement to X axis
			pointerCur.y = m_axisLockStartScreenPos.y;
		}
		else if (m_lockAxis == LockAxis_Y)
		{
			// Restrict movement to Y axis
			pointerCur.x = m_axisLockStartScreenPos.x;
		}
	}
	
	tools::Tool::PointerState pointerState;
	pointerState.screenPosCur  = pointerCur;
	pointerState.screenPosPrev = prevEditorState.pointer;  // FIXME: This is technically incorrect. Needs to be the previous possibly axis-locked pointer.
	pointerState.worldPos      = getEditorCamera().screenToWorld(pointerState.screenPosCur);
	pointerState.ctrl          = keyCtrl;
	pointerState.alt           = keyAlt;
	pointerState.shift         = keyShift;
	pointerState.capsLockOn    = editorState.capsLockOn;
	pointerState.scrollLockOn  = editorState.scrollLockOn;
	pointerState.numLockOn     = editorState.numLockOn;
	
	// If no pointer buttons pressing, send the tool "pointer hover" events
	if (anyPointerButtonDown == false)
	{
		m_tools[m_activePaintTool]->onPointerHover(pointerState);
	}
	
	// Handle left pointer button clicks
	if (editorState.pointerLeft.pressed)
	{
		m_clickLockedTool[PointerButton_Left] = m_activePaintTool;
		
		// Painting will apply floating sections
		if (m_activePaintTool != tools::ToolID_BoxSelect &&
		    m_activePaintTool != tools::ToolID_Hand      &&
		    hasFloatingSection())
		{
			applyFloatingSection();
		}
		
		m_tools[m_activePaintTool]->onPointerLeftPressed(pointerState);
	}
	else if (editorState.pointerLeft.down && m_activePaintTool == m_clickLockedTool[PointerButton_Left])
	{
		m_tools[m_activePaintTool]->onPointerLeftDown(pointerState);
	}
	else if (editorState.pointerLeft.released && m_activePaintTool == m_clickLockedTool[PointerButton_Left])
	{
		m_clickLockedTool[PointerButton_Left] = tools::ToolID_Invalid;
		m_tools[m_activePaintTool]->onPointerLeftReleased(pointerState);
	}
	
	// Handle middle pointer button clicks
	if (editorState.pointerMiddle.pressed)
	{
		m_clickLockedTool[PointerButton_Middle] = m_activePaintTool;
		
		// Painting will apply floating sections
		if (m_activePaintTool != tools::ToolID_BoxSelect &&
		    m_activePaintTool != tools::ToolID_Hand      &&
		    hasFloatingSection())
		{
			applyFloatingSection();
		}
		
		m_tools[m_activePaintTool]->onPointerMiddlePressed(pointerState);
	}
	else if (editorState.pointerMiddle.down && m_activePaintTool == m_clickLockedTool[PointerButton_Middle])
	{
		m_tools[m_activePaintTool]->onPointerMiddleDown(pointerState);
	}
	else if (editorState.pointerMiddle.released && m_activePaintTool == m_clickLockedTool[PointerButton_Middle])
	{
		m_clickLockedTool[PointerButton_Middle] = tools::ToolID_Invalid;
		m_tools[m_activePaintTool]->onPointerMiddleReleased(pointerState);
	}
	
	// Handle right pointer button clicks
	if (editorState.pointerRight.pressed)
	{
		m_clickLockedTool[PointerButton_Right] = m_activePaintTool;
		
		// Painting will apply floating sections
		if (m_activePaintTool != tools::ToolID_BoxSelect &&
		    m_activePaintTool != tools::ToolID_Hand      &&
		    hasFloatingSection())
		{
			applyFloatingSection();
		}
		
		m_tools[m_activePaintTool]->onPointerRightPressed(pointerState);
	}
	else if (editorState.pointerRight.down && m_activePaintTool == m_clickLockedTool[PointerButton_Right])
	{
		m_tools[m_activePaintTool]->onPointerRightDown(pointerState);
	}
	else if (editorState.pointerRight.released && m_activePaintTool == m_clickLockedTool[PointerButton_Right])
	{
		m_clickLockedTool[PointerButton_Right] = tools::ToolID_Invalid;
		m_tools[m_activePaintTool]->onPointerRightReleased(pointerState);
	}
	
	
	if (anyPointerButtonDown == false && m_switchToPreviousToolOnPointerRelease)
	{
		if (tools::isValidToolID(m_previousPaintTool))
		{
			switchToTool(m_previousPaintTool);
		}
		m_switchToPreviousToolOnPointerRelease = false;
	}
	
	TT_NULL_ASSERT(m_tools[tools::ToolID_Resize]);
	if (m_activePaintTool != tools::ToolID_Resize &&
	    m_activePaintTool != tools::ToolID_Hand &&
	    m_activePaintTool != tools::ToolID_BoxSelect &&
	    m_tools[tools::ToolID_Resize]->canActivate(pointerState))
	{
		m_tools[tools::ToolID_Resize]->onPointerHover(pointerState);
		switchToToolUntilPointerReleased(tools::ToolID_Resize);
	}
}


bool Editor::isAllowedEntityInstance(const level::entity::EntityInstancePtr& p_instance) const
{
	// Check for disallowed entities first
	if (m_entityPickDisallowedIDs.find(p_instance->getID()) != m_entityPickDisallowedIDs.end())
	{
		return false;
	}
	
	if (m_entityPickTypeFilter.empty())
	{
		// No filter; allow ALL entities
		return true;
	}
	
	return p_instance->matchesFilter(m_entityPickTypeFilter);
}


tt::math::PointRect Editor::getLevelIntersectedSelectionRect() const
{
	// Get the part of the selection rectangle that is within the level bounds and return that
	tt::math::PointRect selectRect(tt::math::Point2(0, 0), 0, 0);
	m_levelRect.intersects(m_selectionRect, &selectRect);
	return selectRect;
}


void Editor::fillSelectionWithTile(level::CollisionType p_tile)
{
	if (hasSelectionRect() == false)
	{
		// No selection: nothing to fill
		return;
	}
	
	const tt::math::Point2 fillMin(getSelectionRect().getMin());
	const tt::math::Point2 fillMax(getSelectionRect().getMaxEdge());
	
	level::AttributeLayerPtr attribs(m_levelData->getAttributeLayer());
	
	commands::CommandPaintTilesPtr paintCmd(commands::CommandPaintTiles::create(attribs, p_tile));
	
	tt::math::Point2 pos(fillMin);
	for (pos.y = fillMin.y; pos.y < fillMax.y; ++pos.y)
	{
		for (pos.x = fillMin.x; pos.x < fillMax.x; ++pos.x)
		{
			paintCmd->addTile(pos);
		}
	}
	
	if (paintCmd->hasTiles())
	{
		pushUndoCommand(paintCmd);
	}
}


void Editor::fillSelectionWithTile(level::ThemeType p_tile)
{
	if (hasSelectionRect() == false)
	{
		// No selection: nothing to fill
		return;
	}
	
	const tt::math::Point2 fillMin(getSelectionRect().getMin());
	const tt::math::Point2 fillMax(getSelectionRect().getMaxEdge());
	
	level::AttributeLayerPtr attribs(m_levelData->getAttributeLayer());
	
	commands::CommandPaintTilesPtr paintCmd(commands::CommandPaintTiles::create(attribs, p_tile));
	
	tt::math::Point2 pos(fillMin);
	for (pos.y = fillMin.y; pos.y < fillMax.y; ++pos.y)
	{
		for (pos.x = fillMin.x; pos.x < fillMax.x; ++pos.x)
		{
			paintCmd->addTile(pos);
		}
	}
	
	if (paintCmd->hasTiles())
	{
		pushUndoCommand(paintCmd);
	}
}


bool Editor::copySelectionToClipboard()
{
	const tt::math::PointRect selection(getLevelIntersectedSelectionRect());
	if (selection.hasArea() == false)
	{
		// No selection rect: try to copy the selected entities instead
		return copySelectedEntitiesToClipboard();
	}
	
	// A copy (or cut) operation always applies the floating section
	if (hasFloatingSection())
	{
		applyFloatingSection();
	}
	
	// Create a plain-text copyable representation of the selected tiles
	const tt::math::Point2 posMin(selection.getMin());
	const tt::math::Point2 posMax(selection.getMaxInside());
	level::AttributeLayerPtr attribs(m_levelData->getAttributeLayer());
	
	tt::str::Strings tilesAsPlainText;
	
	// - CollisionType of each tile
	tilesAsPlainText.push_back(g_clipboardTilesBeginMarker);
	for (tt::math::Point2 pos(posMin.x, posMax.y); pos.y >= posMin.y; --pos.y)
	{
		std::string line;
		line.reserve(static_cast<std::string::size_type>(selection.getWidth()));
		for (pos.x = posMin.x; pos.x <= posMax.x; ++pos.x)
		{
			const level::CollisionType type = attribs->getCollisionType(pos);
			line += level::getCollisionTypeAsChar(type);
		}
		
		tilesAsPlainText.push_back(line);
	}
	tilesAsPlainText.push_back(g_clipboardTilesEndMarker);
	
	// - ThemeType of each tile
	tilesAsPlainText.push_back(g_clipboardThemeTilesBeginMarker);
	for (tt::math::Point2 pos(posMin.x,  posMax.y); pos.y >= posMin.y; --pos.y)
	{
		std::string line;
		line.reserve(static_cast<std::string::size_type>(selection.getWidth()));
		for (pos.x = posMin.x; pos.x <= posMax.x; ++pos.x)
		{
			const level::ThemeType type = attribs->getThemeType(pos);
			line += level::getThemeTypeAsChar(type);
		}
		
		tilesAsPlainText.push_back(line);
	}
	tilesAsPlainText.push_back(g_clipboardThemeTilesEndMarker);
	
	
	// Create JSON for all the entities inside the selection
	tt::math::VectorRect selectionWorldRect(level::tileToWorld(m_selectionRect));
	// HACK: Tweak the height slightly, so that entities just above the selection rect aren't selected
	//       (entity positions are for the entity bottom-center)
	selectionWorldRect.setHeight(selectionWorldRect.getHeight() - 0.01f);
	const tt::math::Vector2    entityOffset(-selectionWorldRect.getMin());
	Json::Value entityRoot;
	bool        haveEntities = false;
	
	const level::entity::EntityInstances entities(getMissionSpecificEntities());
	for (level::entity::EntityInstances::const_iterator it = entities.begin(); it != entities.end(); ++it)
	{
		if (selectionWorldRect.contains((*it)->getPosition()) == false)
		{
			continue;
		}
		
		entityRoot.append(createEntityInstanceJson(*it, entityOffset));
		
		haveEntities = true;
	}
	
	if (haveEntities)
	{
		tilesAsPlainText.push_back(g_clipboardEntityMarker + Json::FastWriter().write(entityRoot));
	}
	
	tt::system::setSystemClipboardText(tilesAsPlainText);
	
	return true;
}


bool Editor::copySelectedEntitiesToClipboard()
{
	const level::entity::EntityInstanceSet& selectedEntities(m_levelData->getSelectedEntities());
	if (selectedEntities.empty())
	{
		return false;
	}
	
	// Determine the minimum position in the entity set
	tt::math::Vector2 minEntityPos((*selectedEntities.begin())->getPosition());
	for (level::entity::EntityInstanceSet::const_iterator it = selectedEntities.begin();
	     it != selectedEntities.end(); ++it)
	{
		if ((*it)->getPosition().x < minEntityPos.x)
		{
			minEntityPos.x = (*it)->getPosition().x;
		}
		
		if ((*it)->getPosition().y < minEntityPos.y)
		{
			minEntityPos.y = (*it)->getPosition().y;
		}
	}
	
	// Create JSON for all selected entities
	const tt::math::Vector2 entityOffset(-minEntityPos);
	Json::Value             entityRoot;
	for (level::entity::EntityInstanceSet::const_iterator it = selectedEntities.begin();
	     it != selectedEntities.end(); ++it)
	{
		entityRoot.append(createEntityInstanceJson(*it, entityOffset));
	}
	
	tt::str::Strings text;
	text.push_back(g_clipboardEntityMarker + Json::FastWriter().write(entityRoot));
	tt::system::setSystemClipboardText(text);
	
	return true;
}


void Editor::createFloatingSectionVisual()
{
	TT_NULL_ASSERT(m_floatingSection);
	m_floatingSectionVisual      = AttributeDebugView::create(m_floatingSection->getAttributeLayer(),
	                                                          AttributeDebugView::ViewMode_CollisionType,
	                                                          g_attributeViewTextureID);
	m_floatingSectionThemeVisual = AttributeDebugView::create(m_floatingSection->getAttributeLayer(),
	                                                          AttributeDebugView::ViewMode_ThemeType,
	                                                          g_attributeViewTextureID);
	m_floatingSectionBackground  = tt::engine::renderer::QuadSprite::createQuad(
			level::tileToWorld(m_floatingSection->getRect().getWidth()),
			level::tileToWorld(m_floatingSection->getRect().getHeight()),
			tt::engine::renderer::ColorRGBA(200, 200, 200, 192));
	
	TT_NULL_ASSERT(m_floatingSectionVisual);
	TT_NULL_ASSERT(m_floatingSectionThemeVisual);
}


void Editor::changeCameraFovBy(real p_deltaDegrees)
{
	Camera& cam(getEditorCamera());
	cam.setFOV(cam.getTargetFOV() + p_deltaDegrees, true);
}


void Editor::resetClickLockedToolIDs()
{
	for (s32 i = 0; i < PointerButton_Count; ++i)
	{
		m_clickLockedTool[i] = tools::ToolID_Invalid;
	}
}


void Editor::refreshEntityLibrary(tt::gwen::GroupedButtonList::ExpansionState* p_savedExpansionState)
{
	// Remember the currently selected paint tool
	const tools::ToolID activePaintTool = m_activePaintTool;
	
	const tt::gwen::GroupedButtonList::ExpansionState prevExpansionState(
			(p_savedExpansionState != 0) ? *p_savedExpansionState : m_ui.listEntities->GetAllGroupExpansionState());
	
	const std::string prevSelectedEntity((m_ui.listEntities->GetSelectedRow() != 0) ?
		m_ui.listEntities->GetSelectedRow()->GetName() : std::string());
	
	m_ui.listEntities->Clear();
	
	Gwen::Controls::Button* firstItem = 0;
	
	// Sort entities by display name
	typedef std::map<std::string, const level::entity::EntityInfo*> EntityInfoByDisplayName;
	typedef std::map<std::string, EntityInfoByDisplayName> DisplayNamesPerGroup;
	DisplayNamesPerGroup groups;
	
	entity::EntityLibrary& entityLibrary(AppGlobal::getEntityLibrary());
	for (entity::EntityLibrary::const_iterator it = entityLibrary.begin(); it != entityLibrary.end(); ++it)
	{
		groups[(*it).second.getGroup()][(*it).second.getDisplayName()] = &(*it).second;
	}
	
	// Mask controlling which Placeable values make an entity show up in the entity library
	level::PlaceableMask showInLibrary(level::Placeable_Everyone);
	if (AppGlobal::isInDeveloperMode())
	{
		showInLibrary.setFlag(level::Placeable_Developer);
	}
	if (AppGlobal::isInLevelEditorMode())
	{
		showInLibrary.setFlag(level::Placeable_UserLevelEditor);
	}
	
	const std::wstring noGroupName(translateString("ENTITY_GROUP_NONE"));
	for (DisplayNamesPerGroup::iterator groupIt = groups.begin(); groupIt != groups.end(); ++groupIt)
	{
		const std::wstring groupName((*groupIt).first.empty() ? noGroupName : tt::str::widen((*groupIt).first));
		
		for (EntityInfoByDisplayName::iterator it = (*groupIt).second.begin();
		     it != (*groupIt).second.end(); ++it)
		{
			const level::entity::EntityInfo* entity = (*it).second;
			if (showInLibrary.checkFlag(entity->getPlaceable()))
			{
				Gwen::Controls::Button* item = m_ui.listEntities->AddItem(
						groupName, "", entity->getLibraryImage(), entity->getName());
				item->SetToolTip(entity->getDisplayName());
				
				if (firstItem == 0)
				{
					firstItem = item;
				}
			}
		}
	}
	
	if (prevSelectedEntity.empty() == false)
	{
		m_ui.listEntities->SelectByName(prevSelectedEntity);
	}
	
	if (m_ui.listEntities->GetSelectedRow() == 0)
	{
		m_ui.listEntities->SetSelectedRow(firstItem);
	}
	
	m_ui.listEntities->SetAllGroupExpansionState(prevExpansionState);
	
	// Restore the original paint tool (might have been changed by setting the selected entity)
	if (m_activePaintTool != activePaintTool)
	{
		switchToTool(activePaintTool);
	}
}


void Editor::refreshLevelBackgroundMenu()
{
	TT_NULL_ASSERT(m_ui.subMenuLevelBackground);
	TT_NULL_ASSERT(m_levelData);
	if (m_ui.subMenuLevelBackground == 0 || m_levelData == 0)
	{
		return;
	}
	
	// Remove all existing background menu items
	m_ui.subMenuLevelBackground->GetMenu()->ClearItems();
	m_ui.menuItemsLevelBackground.clear();
	
	// Add the current list
	tt::str::Strings backgrounds;
	backgrounds.push_back("");  // first item is always "no background"
	if (tt::fs::dirExists(level::getUserLevelShoeboxPath()))
	{
		tt::str::StringSet dirContents(tt::fs::utils::getFilesInDir(
				level::getUserLevelShoeboxPath(), "*.shoebox"));
		backgrounds.reserve(dirContents.size() + 1);
		backgrounds.insert(backgrounds.end(), dirContents.begin(), dirContents.end());
	}
	
	bool checkedAnything = false;
	const std::string& currentBackground(m_levelData->getLevelBackground());
	for (tt::str::Strings::iterator it = backgrounds.begin(); it != backgrounds.end(); ++it)
	{
		const std::string& name(*it);
		
		// Ignore light mask shoeboxes
		if (tt::str::endsWith(name, "_lightmask"))
		{
			continue;
		}
		
		std::wstring displayName;
		if (name.empty())
		{
			displayName = translateString("MENU_LEVEL_BACKGROUND_NONE");
		}
		else
		{
			displayName = tt::str::widen(name);
		}
		
		Gwen::Controls::MenuItem* item = m_ui.subMenuLevelBackground->GetMenu()->AddItem(displayName);
		item->SetName(name);
		item->SetCheckable(true);
		item->SetChecked(name == currentBackground);
		if (item->GetChecked()) checkedAnything = true;
		item->onMenuItemSelected.Add(this, &Editor::onSelectLevelBackground);
		m_ui.menuItemsLevelBackground.push_back(item);
	}
	
	if (checkedAnything == false)
	{
		// Selection could not be restored: set a default selection
		TT_ASSERT(m_ui.menuItemsLevelBackground.empty() == false);
		m_ui.menuItemsLevelBackground.front()->SetChecked(true);
	}
}


void Editor::updateLevelBackgroundMenuChecks()
{
	for (MenuItems::iterator it = m_ui.menuItemsLevelBackground.begin();
	     it != m_ui.menuItemsLevelBackground.end(); ++it)
	{
		TT_NULL_ASSERT(*it);
		(*it)->SetChecked((*it)->GetName() == m_levelData->getLevelBackground());
	}
}


void Editor::updateLevelThemeMenuChecks()
{
	for (s32 i = 0; i < level::ThemeType_Count; ++i)
	{
		if (m_ui.menuItemLevelTheme[i] != 0)
		{
			m_ui.menuItemLevelTheme[i]->SetChecked(i == m_levelData->getLevelTheme());
		}
	}
}


void Editor::updateDefaultMissionMenuChecks()
{
	const std::string& defaultMission(m_levelData->getDefaultMission());
	for (MenuItems::const_iterator it = m_ui.menuItemsLevelDefaultMission.begin();
	     it != m_ui.menuItemsLevelDefaultMission.end();++it)
	{
		if (*it != 0)
		{
			(*it)->SetChecked((*it)->GetName() == defaultMission);
		}
	}
}


void Editor::createAttributeViews()
{
	TT_NULL_ASSERT(m_levelData);
	m_collisionTileView = AttributeDebugView::create(m_levelData->getAttributeLayer(),
	                                                 AttributeDebugView::ViewMode_CollisionType,
	                                                 g_attributeViewTextureID);
	m_themeTileView = AttributeDebugView::create(m_levelData->getAttributeLayer(),
	                                             AttributeDebugView::ViewMode_ThemeType,
	                                             g_attributeViewTextureID);
}


std::wstring Editor::getTilesTabTitle() const
{
	switch (m_tileLayerMode)
	{
	default:
		TT_PANIC("Unsupported tile layer mode: %d. Cannot determine tiles tab title.", m_tileLayerMode);
		// intentional fall-through (default to CollisionType)
		
	case TileLayerMode_CollisionType: return translateString("TITLE_PICKER_COLLISIONTILES");
	case TileLayerMode_ThemeType:     return translateString("TITLE_PICKER_THEMETILES");
	}
}


void Editor::refreshTilesList()
{
	TT_NULL_ASSERT(m_ui.listTiles);
	if (m_ui.listTiles == 0)
	{
		return;
	}
	
	m_ui.listTiles->onRowSelected.RemoveHandler(this);
	
	Gwen::Controls::Button* itemToSelect = 0;
	
	m_ui.listTiles->Clear();
	
	if (m_tileLayerMode == TileLayerMode_CollisionType)
	{
		itemToSelect = ui::addCollisionTypesToPicker(m_ui.listTiles, level::getCollisionTypeName(m_paintTile));
	}
	else if (m_tileLayerMode == TileLayerMode_ThemeType)
	{
		const std::string nameToSelect(level::getThemeTypeName(m_paintTheme));
		
		typedef std::vector<level::ThemeType> Tiles;
		
		Tiles tiles;
		tiles.push_back(level::ThemeType_UseLevelDefault);
		tiles.push_back(level::ThemeType_DoNotTheme);
		tiles.push_back(level::ThemeType_Sand);
		tiles.push_back(level::ThemeType_Rocks);
		tiles.push_back(level::ThemeType_Beach);
		tiles.push_back(level::ThemeType_DarkRocks);
		
		s32 itemNumber = 1;
		for (Tiles::iterator it = tiles.begin(); it != tiles.end(); ++it, ++itemNumber)
		{
			const std::string typeName(level::getThemeTypeName(*it));
			Gwen::Controls::Button* item = m_ui.listTiles->AddItem("", "editor.tiles.theme_" + typeName, typeName);
			
			std::wostringstream tooltip;
			tooltip << itemNumber << L": " << getThemeDisplayName(*it);
			item->SetToolTip(tooltip.str());
			
			if (itemToSelect == 0 || typeName == nameToSelect)
			{
				itemToSelect = item;
			}
		}
	}
	else
	{
		TT_PANIC("Unsupported tile layer mode: %d. Cannot produce tiles list for it.",
		         m_tileLayerMode);
	}
	
	m_ui.listTiles->SetSelectedRow(itemToSelect);
	m_ui.listTiles->onRowSelected.Add(this, &Editor::selectedPaintTile);
}


void Editor::setTileLayerMode(TileLayerMode p_mode)
{
	m_tileLayerMode = p_mode;
	
	m_ui.tabTiles->SetText(getTilesTabTitle());
	m_ui.tabTiles->GetTabControl()->Invalidate();
	refreshTilesList();
}


void Editor::switchToTool(tools::ToolID p_tool, bool /*p_temporary*/)
{
	if (p_tool == m_activePaintTool)
	{
		// Tool did not change
		return;
	}
	
	m_previousPaintTool = m_activePaintTool;
	
	// Deactivate previous paint tool
	if (tools::isValidToolID(m_activePaintTool) && m_tools[m_activePaintTool] != 0)
	{
		m_tools[m_activePaintTool]->onDeactivate();
	}
	
	// If a floating section is active, apply it to the level as soon as we switch to an edit tool
	// (i.e., not the box select or hand tool)
	if (p_tool != tools::ToolID_BoxSelect &&
	    p_tool != tools::ToolID_Hand)
	{
		hotKeyApplyFloatingSection();
	}
	
	// FIXME: Is this really necessary?
	resetClickLockedToolIDs();
	
	m_activePaintTool = p_tool;
	
	if (ui::selectItemByName(m_ui.listTools, tools::getToolIDName(m_activePaintTool)) == false)
	{
		TT_PANIC("Switching to tool '%s', which does not appear in the tools list!",
		         tools::getToolIDName(m_activePaintTool));
	}
	
	// Activate the new one
	if (m_tools[m_activePaintTool] != 0)
	{
		m_tools[m_activePaintTool]->onActivate();
		m_ui.statusBar->SetText(m_tools[m_activePaintTool]->getHelpText());
	}
	else
	{
		m_ui.statusBar->SetText("");
	}
}


void Editor::doShowLoadLevelDialog()
{
	TT_ASSERTMSG(isActive(), "Cannot show Load Level dialog if editor is not active.");
	TT_NULL_ASSERT(m_ui.windowLoadLevel);
	
	if (m_ui.windowLoadLevel == 0)
	{
		return;
	}
	
	if (m_ui.windowLoadLevel->Hidden() == false)
	{
		// Window is already open: don't open it a second time
		return;
	}
	
	refreshLevelListForCurrentFilter();
	
	if (m_ui.windowLoadLevelNeedsCentering)
	{
		// HACK: Center the dialog manually, because the GWEN way appears to be broken
		const tt::math::Point2 scrSize(getEditorCamera().getViewPortSize());
		
		//m_ui.windowLoadLevel->Position(Gwen::Pos::Center);
		m_ui.windowLoadLevel->SetPos(
				(scrSize.x - m_ui.windowLoadLevel->Width())  / 2,
				(scrSize.y - m_ui.windowLoadLevel->Height()) / 2);
		m_ui.windowLoadLevelNeedsCentering = false;
	}
	m_ui.windowLoadLevel->SetHidden(false);
	m_ui.listLevels->Focus();
	m_loadLevelDialogHasBeenOpen = true;
}


void Editor::closeLoadLevelDialog()
{
	if (m_ui.windowLoadLevel != 0)
	{
		m_ui.windowLoadLevel->CloseButtonPressed();
	}
}


bool Editor::isLoadLevelDialogOpen() const
{
	return m_ui.windowLoadLevel != 0 && m_ui.windowLoadLevel->Hidden() == false;
}


void Editor::refreshLevelListForCurrentFilter()
{
	tt::str::Strings filters;
	StartInfo        pathInfo(getCurrentLevelInfo());
	
	if (AppGlobal::isInDeveloperMode())
	{
		switch (m_persistentSettings.loadLevelFilter)
		{
		case LoadLevelFilter_AllLevels:
			// Keep the empty container: no filtering
			if (pathInfo.getType() != StartInfo::Type_NormalLevel)
			{
				pathInfo.resetToDefaultLevel();  // pick built-in levels
			}
			break;
			
		case LoadLevelFilter_Slices:
			// Only show "menu_" and "slice_" levels
			if (pathInfo.getType() != StartInfo::Type_NormalLevel)
			{
				pathInfo.resetToDefaultLevel();  // pick built-in levels
			}
			filters.push_back("menu_*");
			filters.push_back("slice_*");
			break;
			
		case LoadLevelFilter_UserLevels:
			// No filtering, but get levels from a different location
			if (pathInfo.getType() != StartInfo::Type_UserLevel)
			{
				pathInfo.setToUserLevelMode();  // pick user levels
			}
			break;
			
		default:
			TT_PANIC("Unsupported level filter: %d", m_persistentSettings.loadLevelFilter);
			break;
		}
	}
	
	refreshLevelList(filters, pathInfo);
}


void Editor::refreshLevelList(const tt::str::Strings& p_filters, const StartInfo& p_pathInfo)
{
	TT_NULL_ASSERT(m_ui.listLevels);
	if (m_ui.listLevels == 0) return;
	
	// Save currently selected level name, so that the selection can be restored afterwards
	std::string selectedLevelName(m_ui.listLevels->GetSelectedRowName());
	if (selectedLevelName.empty())
	{
		// No level was selected: default to the current level name
		selectedLevelName = getCurrentLevelName();
	}
	
	// Repopulate the listbox
	m_ui.listLevels->Clear();
	
	m_ui.listLevels->UserData.Set("pathInfo", p_pathInfo);
	
#if defined(TT_STEAM_BUILD)
	steam::Workshop* workshop = steam::Workshop::getInstance();
	const bool displayPublishStatus = p_pathInfo.isUserLevel();
	if (displayPublishStatus)
	{
		m_ui.listLevels->SetColumnCount(2);
		m_ui.listLevels->SetColumnWidth(1, 65);
	}
	else
	{
		// NOTE: 'Hiding' second column by setting its width to 1, because setting column count
		//       to a lower number than it already has causes GWEN to crash. Hidden width is 1,
		//       because a column width of 0 has special meaning: it means 'auto-size this column'.
		m_ui.listLevels->SetColumnCount(2);
		m_ui.listLevels->SetColumnWidth(1, 1);
	}
#endif
	
	const std::string levelsDir(p_pathInfo.getLevelPath());
	
	Gwen::Controls::Layout::TableRow* itemToReselect = 0;
	tt::str::StringSet filenames(tt::fs::utils::getFilesInDir(levelsDir, "*.ttlvl"));
	for (tt::str::StringSet::iterator it = filenames.begin(); it != filenames.end(); ++it)
	{
		const std::string& filename(*it);
		
		bool matchesFilter = p_filters.empty();
		for (tt::str::Strings::const_iterator filterIt = p_filters.begin();
		     filterIt != p_filters.end(); ++filterIt)
		{
			if (tt::fs::utils::matchesFilter(filename, *filterIt))
			{
				matchesFilter = true;
				break;
			}
		}
		if (matchesFilter == false)
		{
			continue;
		}
		
		Gwen::Controls::Layout::TableRow* item = m_ui.listLevels->AddItem(filename, filename);
		if (filename == selectedLevelName)
		{
			itemToReselect = item;
		}
		
#if defined(TT_STEAM_BUILD)
		if (displayPublishStatus)
		{
			const std::string fullPath(levelsDir + filename + ".ttlvl");
			tt::fs::FilePtr file = tt::fs::open(fullPath, tt::fs::OpenMode_Read);
			if (file != 0)
			{
				level::LevelDataPtr levelData(level::LevelData::loadLevel(file, filename));
				if (levelData != 0                       &&
				    levelData->getPublishedFileId() != 0 &&
				    workshop->isPublishedFile(levelData->getPublishedFileId()))
				{
					bool wasModifiedAfterPublishing = false;
					
					const steam::Workshop::FileDetails* details = workshop->getCachedFileDetails(
							levelData->getPublishedFileId());
					if (details != 0)
					{
						const s64 modifiedTime = tt::fs::convertToUnixTime(tt::fs::getWriteTime(file));
						//TT_Printf("Level '%s' modification time: %lld | last publish time: %u\n",
						//          filename.c_str(), modifiedTime, details->details.m_rtimeUpdated);
						wasModifiedAfterPublishing =
								modifiedTime > static_cast<s64>(details->details.m_rtimeUpdated);
					}
					
					item->SetCellText(1, translateString(wasModifiedAfterPublishing ?
							"WINDOW_LOADLEVEL_STATUS_MODIFIED" : "WINDOW_LOADLEVEL_STATUS_PUBLISHED"));
				}
			}
		}
#endif
	}
	
	// And now reselect the previously selected level
	if (itemToReselect != 0)
	{
		m_ui.listLevels->SetSelectedRow(itemToReselect);
	}
}


level::entity::EntityInstances Editor::getMissionSpecificEntities() const
{
	const std::string& missionID(AppGlobal::getGame()->getMissionID());
	
	level::entity::EntityInstances entities;
	entity::EntityMgr::appendMissionSpecificEntities(m_levelData->getAllEntities(), missionID, entities);
	return entities;
}


void Editor::refreshMissionList()
{
	const std::string missionID(AppGlobal::getGame()->getMissionID());
	
	Gwen::Controls::Layout::TableRow* itemToSelect = 0;
	Gwen::Controls::Layout::TableRow* item = 0;
	
	m_ui.listMissions->Clear();
	item = m_ui.listMissions->AddItem("*", "");
	
	if (missionID.empty())
	{
		itemToSelect = item;
	}
	
	// FIXME: Remove hardcoded dependency on folder
	tt::str::StringSet filenames(tt::fs::utils::getFilesInDir("missions", "*.json"));
	for (tt::str::StringSet::iterator it = filenames.begin(); it != filenames.end(); ++it)
	{
		const std::string& filename(*it);
		
		item = m_ui.listMissions->AddItem(filename, filename);
		if (filename == missionID)
		{
			itemToSelect = item;
		}
	}
	
	// And now select the currently active mission
	if (itemToSelect != 0)
	{
		m_ui.listMissions->SetSelectedRow(itemToSelect);
	}
}


void Editor::doLoadSelectedLevel(bool p_promptForUnsavedChanges)
{
	if (m_ui.listLevels == 0)
	{
		return;
	}
	
	const std::string selectedLevelName(m_ui.listLevels->GetSelectedRowName());
	if (selectedLevelName.empty())
	{
		// TODO: Display message about no level being selected?
		return;
	}
	
	// Flush the "pressed" state of all keyboard keys, so that all pressed keys are now registered as "down"
	// (thereby not triggering further on-press handling)
	input::Controller::State::EditorState& editorState(AppGlobal::getController(tt::input::ControllerIndex_One).cur.editor);
	for (s32 i = 0; i < tt::input::Key_Count; ++i)
	{
		if (editorState.keys[i].pressed)
		{
			editorState.keys[i].update(true);
		}
	}
	
#if EDITOR_SUPPORTS_SAVING
	if (p_promptForUnsavedChanges && hasUnsavedChanges())
	{
		showSaveChangesDialog(SaveChangesContinueAction_LoadLevel);
		return;
	}
#else
	(void)p_promptForUnsavedChanges;
#endif
	
	const StartInfo pathInfo(m_ui.listLevels->UserData.Get<StartInfo>("pathInfo"));
	closeLoadLevelDialog();
	doLoadLevel(pathInfo, selectedLevelName);
	hide();
}


void Editor::doLoadLevel(const StartInfo& p_pathInfo, const std::string& p_levelName)
{
	StartInfo startInfo(p_pathInfo);
	if (startInfo.isUserLevel())
	{
		startInfo.setUserLevelName(p_levelName);
	}
	else
	{
		startInfo.setLevel(p_levelName);
	}
	AppGlobal::setGameStartInfo(startInfo);
	AppGlobal::getGame()->forceReload();
}


void Editor::setCurrentLevelName(const std::string& p_levelName, bool p_alsoSetForActiveGame)
{
	StartInfo startInfo(getCurrentLevelInfo());
	if (startInfo.isUserLevel() == false)
	{
		// Don't validate the level; as it might not yet exist at this point
		startInfo.setLevel(p_levelName, false);
	}
	else
	{
		startInfo.setUserLevel(startInfo.getLevelPath() + p_levelName + ".ttlvl");
	}
	AppGlobal::setGameStartInfo(startInfo);
	
	if (p_alsoSetForActiveGame && AppGlobal::hasGame())
	{
		AppGlobal::getGame()->setStartInfo(startInfo);
	}
}


bool Editor::saveLevel(const std::string& p_filename, const std::string& p_internalSourceFilename)
{
#if EDITOR_SUPPORTS_SAVING
	
	bool saveOk = true;
	
	m_doingPlayTest = false;
	
	// Clone level because we don't want the unreferences entities to be removed from the m_levelData as well
	// as that could interfere with the undo buffer
	level::LevelDataPtr level(m_levelData->clone());
	
	// Make sure no unreferences entities are saved
	level::entity::removeUnreferencedEntityReferences(level);
	
	// Save leveldata
	if (p_internalSourceFilename.empty() == false)
	{
		// Save agentradii to level data
		level->setAgentRadii(AppGlobal::getGame()->getPathMgr().getUniqueAgentRadiiForLevel(level));
		
		if (level->saveAsText(p_internalSourceFilename) == false)
		{
			saveOk = false;
		}
	}
	else
	{
		// Save tilecaches to level data
		AppGlobal::getGame()->getPathMgr().saveTileCachesToLevelData(level);
		
		if (level->save(p_filename) == false)
		{
			saveOk = false;
		}
	}
	
	if (saveOk)
	{
		m_undoStack->setClean();
		
		m_entitiesUpdatedByScript.clear();
		
		// Trigger the "level saved" indicator
		m_levelSavedIndicator->setColor(tt::engine::renderer::ColorRGB::white);
		m_levelSavedIndicator->setOpacity(255);
		m_levelSavedIndicator->fadeOut(0.5f);
	}
	else
	{
		// Show "save failed" notification
		m_levelSaveFailedIndicator->setOpacity(255);
		m_levelSaveFailedIndicator->fadeOut(1.5f);
	}
	
	return saveOk;
	
#else
	(void)p_filename;
	(void)p_internalSourceFilename;
	return false;
#endif
}


void Editor::showSaveChangesDialog(SaveChangesContinueAction p_continueAction)
{
	ui::SaveChangesDialog* dialog = ui::SaveChangesDialog::create(
			translateString("WINDOW_UNSAVED_CHANGES_PROMPT", getCurrentLevelName()),
			m_gwenRoot.getCanvas());
	dialog->UserData.Set("continueAction", p_continueAction);
	dialog->onWindowClosed.Add(this, &Editor::onSaveChangesDialogClosed);
	openModalDialogBox(dialog, true);
}


void Editor::showSaveAsDialog(SaveChangesContinueAction p_continueAction)
{
	ui::SaveAsDialog* dlg = ui::SaveAsDialog::create(this, getCurrentLevelName(), m_gwenRoot.getCanvas());
	dlg->UserData.Set("continueAction", p_continueAction);
	dlg->onWindowClosed.Add(this, &Editor::onSaveAsDialogClosed);
	openModalDialogBox(dlg, true);
}


void Editor::showNewLevelDialog()
{
	ui::NewLevelDialog* dlg = ui::NewLevelDialog::create(this, m_gwenRoot.getCanvas());
	dlg->onWindowClosed.Add(this, &Editor::onNewLevelDialogClosed);
	openModalDialogBox(dlg, true);
}


void Editor::openModalDialogBox(ui::DialogBoxBase* p_dialog, bool p_autoCloseWhenEditorClosed)
{
	TT_NULL_ASSERT(p_dialog);
	if (p_dialog == 0) return;
	
	p_dialog->UserData.Set("closeOnEditorClose", p_autoCloseWhenEditorClosed);
	p_dialog->onWindowClosed.Add(this, &Editor::onModalDialogClosed);
	m_openDialogBoxes.push_back(p_dialog);
	
	centerInScreen(p_dialog);
	p_dialog->MakeModal();
	p_dialog->SetHidden(false);
}


void Editor::renderEntities()
{
	tt::engine::debug::DebugRendererPtr debug(tt::engine::renderer::Renderer::getInstance()->getDebug());
	
	const Camera& editorCam(getEditorCamera());
	using level::entity::editor::EntityInstanceEditorRepresentation;
	
	// Render all entities
	const level::entity::EntityInstances entities = getMissionSpecificEntities();
	
	level::entity::editor::RenderFlags renderFlags(m_persistentSettings.entityRenderFlags);
	if (renderFlags.checkFlag(level::entity::editor::RenderFlag_AllEntityReferences))
	{
		// Only render all references if there isn't a single selected entity
		for (level::entity::EntityInstances::const_iterator it = entities.begin(); it != entities.end(); ++it)
		{
			if (m_levelData->isEntitySelected(*it))
			{
				renderFlags.resetFlag(level::entity::editor::RenderFlag_AllEntityReferences);
				break;
			}
		}
	}
	
	for (level::entity::EntityInstances::const_iterator it = entities.begin(); it != entities.end(); ++it)
	{
		const level::entity::EntityInstancePtr& instance(*it);
		
		EntityInstanceEditorRepresentation* editorRep = instance->getEditorRepresentation();
		if (editorRep != 0)
		{
			// Pass image state
			EntityInstanceEditorRepresentation::ImageState state(EntityInstanceEditorRepresentation::ImageState_Normal);
			const bool isHidden(m_editorMode == EditorMode_PickEntity && isAllowedEntityInstance(instance) == false);
			if (m_levelData->isEntitySelected(instance))
			{
				state = isHidden ? EntityInstanceEditorRepresentation::ImageState_SelectedHidden :
				                   EntityInstanceEditorRepresentation::ImageState_Selected;
			}
			else if (isHidden)
			{
				state = EntityInstanceEditorRepresentation::ImageState_Hidden;
			}
			editorRep->setImageState(state);
			
			editorRep->renderBack(editorCam, renderFlags);
		}
		
		// Render an overlay for all entities whose properties were changed by script
		if (m_entitiesUpdatedByScript.find(instance) != m_entitiesUpdatedByScript.end())
		{
			m_propertiesUpdatedOverlay->setPosition(instance->getPosition().x,
			                                        -instance->getPosition().y,
			                                        0.0f);
			m_propertiesUpdatedOverlay->update();
			m_propertiesUpdatedOverlay->render();
		}
	}
	
	// Render the "front layer" of entities (overlays, arrows, etc)
	for (level::entity::EntityInstances::const_iterator it = entities.begin(); it != entities.end(); ++it)
	{
		EntityInstanceEditorRepresentation* editorRep = (*it)->getEditorRepresentation();
		if (editorRep != 0)
		{
			editorRep->renderFront(editorCam, renderFlags);
		}
	}
}


void Editor::renderFloatingSection()
{
	if (m_floatingSectionVisual == 0)
	{
		return;
	}
	
	TT_NULL_ASSERT(m_floatingSectionThemeVisual);
	
	// Render the tiles
	TT_NULL_ASSERT(m_floatingSectionBackground);
	m_floatingSectionBackground->render();
	m_floatingSectionVisual->render();
	if (m_tileLayerMode == TileLayerMode_ThemeType)
	{
		m_floatingSectionThemeVisual->render();
	}
	
	// Render the entities
	const Camera& editorCam(getEditorCamera());
	const tt::math::Vector2 floatingSectionPos(level::tileToWorld(m_floatingSection->getPosition()));
	
	const level::entity::EntityInstances& entities(m_floatingSection->getEntities());
	for (level::entity::EntityInstances::const_iterator it = entities.begin();
	     it != entities.end(); ++it)
	{
		const level::entity::EntityInstancePtr& instance(*it);
		
		using level::entity::editor::EntityInstanceEditorRepresentation;
		EntityInstanceEditorRepresentation* editorRep = instance->getEditorRepresentation();
		if (editorRep != 0)
		{
			editorRep->setImageState(EntityInstanceEditorRepresentation::ImageState_Selected);
			editorRep->renderBack(editorCam, m_persistentSettings.entityRenderFlags);
		}
	}
}


void Editor::renderNotes()
{
	if (m_persistentSettings.gameLayersVisible.checkFlag(GameLayer_Notes) == false)
	{
		return;
	}
	
	const level::Notes& notes(m_levelData->getAllNotes());
	for (level::Notes::const_iterator it = notes.begin(); it != notes.end(); ++it)
	{
		if (*it != m_noteForTextEdit && *it != m_focusNote)
		{
			(*it)->render();
		}
	}
	
	if (m_noteForTextEdit != 0)
	{
		m_noteForTextEdit->render();
	}
	else if (m_focusNote != 0)
	{
		m_focusNote->render();
	}
}


void Editor::renderEditorWarnings()
{
	// Render the editor warnings of entities
	const level::entity::EntityInstances entities = getMissionSpecificEntities();
	for (level::entity::EntityInstances::const_iterator it = entities.begin(); it != entities.end(); ++it)
	{
		using level::entity::editor::EntityInstanceEditorRepresentation;
		EntityInstanceEditorRepresentation* editorRep = (*it)->getEditorRepresentation();
		if (editorRep != 0)
		{
			editorRep->renderEditorWarnings();
			
			// Always render the warning icon.
			editorRep->renderEditorWarningsIcon();
		}
	}
}


void Editor::resetNoteEditVariables()
{
	m_editorMode = EditorMode_Normal;
	if (m_noteForTextEdit != 0)
	{
		m_noteForTextEdit->setEditCaretVisible(false);
	}
	m_noteForTextEdit.reset();
	m_noteTextChangeCommand.reset();
	m_addTextEditNoteToLevelOnAccept = false;
	restoreDefaultCursor();
}


#if defined(TT_PLATFORM_WIN) || defined(TT_PLATFORM_OSX_MAC) || defined(TT_PLATFORM_LNX)

bool Editor::keyboardStringCallback(void*                                          p_userData,
                                    const std::wstring&                            p_currentString,
                                    tt::input::KeyboardController::GetStringStatus p_status)
{
	Editor* instance = reinterpret_cast<Editor*>(p_userData);
	return (instance != 0) ? instance->onKeyboardStringCallback(p_currentString, p_status) : false;
}


bool Editor::onKeyboardStringCallback(const std::wstring&                            p_currentString,
                                      tt::input::KeyboardController::GetStringStatus p_status)
{
	if (m_editorMode != EditorMode_EditNote)
	{
		TT_PANIC("Got keyboard string callback when not editing a note.");
		return true;
	}
	
	if (m_noteForTextEdit == 0 ||
	    (m_noteTextChangeCommand == 0 && m_addTextEditNoteToLevelOnAccept == false))
	{
		TT_PANIC("In note text edit mode, but do not have a note to edit.");
		return true;
	}
	
	// Set new note text
	if (m_addTextEditNoteToLevelOnAccept)
	{
		m_noteForTextEdit->setText(p_currentString);
	}
	else
	{
		m_noteTextChangeCommand->setText(p_currentString);
	}
	
	switch (p_status)
	{
	case tt::input::KeyboardController::GetStringStatus_Complete:
		// Accept the new text
		m_noteForTextEdit->setEditCaretVisible(false);
		if (m_addTextEditNoteToLevelOnAccept)
		{
			commands::CommandAddRemoveNotesPtr cmd = commands::CommandAddRemoveNotes::createForAdd(this);
			cmd->addNote(m_noteForTextEdit);
			pushUndoCommand(cmd);
		}
		else if (m_noteTextChangeCommand->isTextChanged())
		{
			pushUndoCommand(m_noteTextChangeCommand);
		}
		resetNoteEditVariables();
		break;
		
	case tt::input::KeyboardController::GetStringStatus_Cancelled:
		// Cancel editing: restore original note text
		if (m_addTextEditNoteToLevelOnAccept == false)
		{
			m_noteForTextEdit->setText(m_noteTextChangeCommand->getOriginalText());
		}
		resetNoteEditVariables();
		break;
		
	default: break;
	}
	
	// Accept new string
	return true;
}

#endif


void Editor::onLevelDataEntitySelectionChanged()
{
	m_ui.listEntityProperties->setEntities(m_levelData->getSelectedEntities());
}


void Editor::hotKeyShowHelp()
{
#if defined(TT_BUILD_FINAL)
	// Final build uses a different help URL (and opens it in the Steam overlay if possible)
	const std::string externalUrl("http://twotribes.com/message/toki-tori-2-editor-keys/");
#if defined(TT_STEAM_BUILD)
	tt::steam::openURL(externalUrl);
#else
	if (tt::http::HttpConnectMgr::hasInstance())
	{
		tt::http::HttpConnectMgr::getInstance()->openUrlExternally(externalUrl);
	}
#endif
#else
	if (tt::http::HttpConnectMgr::hasInstance())
	{
		tt::http::HttpConnectMgr::getInstance()->openUrlExternally(
			"http://wiki.twotribes.com/Projects/Toki_Tori_2/Technology/Level_Editor");
	}
#endif
}


void Editor::hotKeyShowLevelDir()
{
	const StartInfo& startInfo(AppGlobal::getGame()->getStartInfo());
	
	std::string levelsDir;
	
#if EDITOR_SUPPORTS_ASSETS_SOURCE
	if (startInfo.isUserLevel() == false)
	{
		// Internal Windows builds: show the levels source dir
		levelsDir = getLevelsSourceDir();
	}
	else
#endif
	{
		// Final builds (assumed to be public builds): show the levels output dir
		// Same goes for user levels: don't attempt to show the source dir either
		levelsDir = startInfo.getLevelPath();
	}
	
	levelsDir = tt::fs::utils::compactPath(levelsDir, "/\\");
	
	bool highlightLevelFile = (isUnnamedLevel() == false);
#if defined(TT_PLATFORM_OSX) || defined(TT_PLATFORM_LNX) // there is no working showFileInFileNavigator for Mac OS X yet: always open dir
	highlightLevelFile = false;
#endif
	
	// FIXME: Steam builds currently abort with STATUS_INVALID_CRUNTIME_PARAMETER (code 0xC0000417)
	//        when SHOpenFolderAndSelectItems is called. No idea why yet. As a workaround,
	//        only open the folder, do not highlight the file itself (avoiding the call to this function).
#if defined(TT_STEAM_BUILD)
	highlightLevelFile = false;
#endif
	
	if (highlightLevelFile)
	{
		levelsDir += startInfo.getLevelName() + ".ttlvl";  // highlight the level file
		tt::system::showFileInFileNavigator(levelsDir);
	}
	else
	{
		tt::system::openWithDefaultApplication(levelsDir);
	}
}


void Editor::hotKeyUndo()
{
	m_undoStack->undo();
}


void Editor::hotKeyRedo()
{
	m_undoStack->redo();
}


#if EDITOR_SUPPORTS_ASSETS_SOURCE
void Editor::hotKeySaveLevelSkinShoebox()
{
	if (AppGlobal::getGame()->saveLevelSkinAsXML(true))
	{
		// Reuse the "level saved" indicator to show that the XML file was saved
		m_levelSavedIndicator->setColor(tt::engine::renderer::ColorRGB::green);
		m_levelSavedIndicator->setOpacity(255);
		m_levelSavedIndicator->fadeOut(0.5f);
	}
	else
	{
		// Show "save failed" notification
		m_levelSaveFailedIndicator->setOpacity(255);
		m_levelSaveFailedIndicator->fadeOut(1.5f);
	}
}
#endif


void Editor::hotKeyCut()
{
	if (copySelectionToClipboard())
	{
		if (hasSelectionRect())
		{
			// Selection rect (tiles and entities) was cut
			pushUndoCommand(commands::CommandApplyLevelSection::create(
				m_levelData,
				level::LevelSection::createEmpty(m_selectionRect)));
		}
		else
		{
			// Only entities were cut
			level::entity::EntityInstanceSet selectedEntities(m_levelData->getSelectedEntities());
			m_levelData->deselectAllEntities();
			pushUndoCommand(commands::CommandAddRemoveEntities::createForRemove(
				this,
				level::entity::EntityInstances(selectedEntities.begin(), selectedEntities.end())));
		}
	}
}


void Editor::hotKeyCopy()
{
	copySelectionToClipboard();
}


void Editor::hotKeyPaste()
{
	// These variables will receive the pre-parsed data from the system clipboard
	// (the actual clipboard data gathering is scoped, to prevent accidental use of invalid variables)
	tt::str::Strings tilesAsPlainText;
	tt::str::Strings themeTilesAsPlainText;
	std::string      entityJson;
	
	{
		// Get the tile data from the system clipboard
		tt::str::Strings rawClipboardText;
		if (tt::system::getSystemClipboardText(&rawClipboardText) == false)
		{
			return;
		}
		
		// Strip all leading and trailing empty lines
		while (rawClipboardText.empty() == false && rawClipboardText.front().empty())
		{
			rawClipboardText.erase(rawClipboardText.begin());
		}
		while (rawClipboardText.empty() == false && rawClipboardText.back().empty())
		{
			rawClipboardText.pop_back();
		}
		
		if (rawClipboardText.empty())
		{
			// No data to paste
			return;
		}
		
		// Find an optional line describing the entities
		for (tt::str::Strings::iterator it = rawClipboardText.begin(); it != rawClipboardText.end(); ++it)
		{
			if (tt::str::startsWith(*it, g_clipboardEntityMarker))
			{
				entityJson = *it;
				entityJson.erase(0, g_clipboardEntityMarker.length());
				rawClipboardText.erase(it);
				break;
			}
		}
		
		// Find the markers for the collision tile data
		bool foundCollisionStartMarker = false;
		for (tt::str::Strings::iterator it = rawClipboardText.begin(); it != rawClipboardText.end(); ++it)
		{
			if (*it == g_clipboardTilesEndMarker)
			{
				TT_ASSERTMSG(foundCollisionStartMarker,
				             "Malformed clipboard data: found tile data end marker without (or before) start marker.");
				break;
			}
			else if (*it == g_clipboardTilesBeginMarker)
			{
				TT_ASSERTMSG(foundCollisionStartMarker == false,
				             "Malformed clipboard data: found more than one tile data start marker.");
				foundCollisionStartMarker = true;
			}
			else if (foundCollisionStartMarker)
			{
				tilesAsPlainText.push_back(*it);
			}
		}
		
		// Find the markers for the theme tile data
		bool foundThemeStartMarker = false;
		for (tt::str::Strings::iterator it = rawClipboardText.begin(); it != rawClipboardText.end(); ++it)
		{
			if (*it == g_clipboardThemeTilesEndMarker)
			{
				TT_ASSERTMSG(foundThemeStartMarker,
				             "Malformed clipboard data: found tile data end marker without (or before) start marker.");
				break;
			}
			else if (*it == g_clipboardThemeTilesBeginMarker)
			{
				TT_ASSERTMSG(foundThemeStartMarker == false,
				             "Malformed clipboard data: found more than one tile data start marker.");
				foundThemeStartMarker = true;
			}
			else if (foundThemeStartMarker)
			{
				themeTilesAsPlainText.push_back(*it);
			}
		}
	}
	
	if (tilesAsPlainText.empty() && themeTilesAsPlainText.empty())
	{
		// No tiles in clipboard, only entities: paste entities only
		
		// Create all instances that are in the JSON, but do not add them to the level yet
		tt::math::Vector2              entityMinPos;
		tt::math::Vector2              entityMaxPos;
		level::entity::EntityInstances instances(
				createEntitiesFromJson(entityJson, m_levelData, &entityMinPos, &entityMaxPos));
		
		if (instances.empty())
		{
			return;
		}
		
		// Determine an appropriate position for the new entities (and offset each entity's position)
		tt::math::Vector2 entityOffset;
		{
			tt::engine::renderer::Renderer* renderer = tt::engine::renderer::Renderer::getInstance();
			const tt::math::Point2 screenCenter(renderer->getScreenWidth() / 2, renderer->getScreenHeight() / 2);
			// Translate cursor position to world position, snap that to a tile position
			entityOffset = level::tileToWorld(level::worldToTile(
					getEditorCamera().screenToWorld(
							AppGlobal::getController(tt::input::ControllerIndex_One).cur.pointer.valid ?
							AppGlobal::getController(tt::input::ControllerIndex_One).cur.pointer : screenCenter)));
		}
		entityOffset -= (entityMaxPos - entityMinPos) * 0.5f;
		
		for (level::entity::EntityInstances::iterator it = instances.begin(); it != instances.end(); ++it)
		{
			(*it)->setPosition((*it)->getPosition() + entityOffset);
		}
		
		// If only a single entity is pasted, always snap it to a tile position
		if (instances.size() == 1)
		{
			snapEntityToTile(instances.front());
		}
		
		// Apply floating section and deselect all entities before adding the new entities to the level
		applyFloatingSection();
		m_levelData->deselectAllEntities();
		
		// Add the new entities to the level via an undo command
		commands::CommandAddRemoveEntitiesPtr addCmd =
				commands::CommandAddRemoveEntities::createForAdd(this, instances);
		pushUndoCommand(addCmd);
		
		// Select all new entities and switch to the entity move tool
		// FIXME: Have an overload for this function, to prevent all these container type switches?
		m_levelData->setSelectedEntities(level::entity::EntityInstanceSet(instances.begin(), instances.end()));
		if (m_activePaintTool != tools::ToolID_Draw        &&
		    m_activePaintTool != tools::ToolID_EntityPaint &&
		    m_activePaintTool != tools::ToolID_EntityMove)
		{
			switchToTool(tools::ToolID_EntityMove);
		}
		return;
	}
	
	// ---------- Clipboard text contains tile data ----------
	
	// Determine the dimensions of the clipboard tile data
	tt::math::PointRect sectionRect(tt::math::Point2(0, 0), 0, static_cast<s32>(tilesAsPlainText.size()));
	for (tt::str::Strings::reverse_iterator rowIt = tilesAsPlainText.rbegin();
	     rowIt != tilesAsPlainText.rend(); ++rowIt)
	{
		const s32 rowLen = static_cast<s32>((*rowIt).length());
		if (rowLen > sectionRect.getWidth())
		{
			sectionRect.setWidth(rowLen);
		}
	}
	
	if (sectionRect.hasArea() == false)
	{
		return;
	}
	
	// Determine an appropriate position for the new floating section (based on the pointer position)
	{
		tt::engine::renderer::Renderer* renderer = tt::engine::renderer::Renderer::getInstance();
		const tt::math::Point2 screenCenter(renderer->getScreenWidth() / 2, renderer->getScreenHeight() / 2);
		sectionRect.setCenterPosition(level::worldToTile(
			getEditorCamera().screenToWorld(
				AppGlobal::getController(tt::input::ControllerIndex_One).cur.pointer.valid ? 
				AppGlobal::getController(tt::input::ControllerIndex_One).cur.pointer : screenCenter)));
	}
	
	// If a floating section already existed, apply that one now
	if (hasFloatingSection())
	{
		applyFloatingSection();
	}
	
	// Create a new floating section
	m_floatingSection = level::LevelSection::createEmpty(sectionRect);
	createFloatingSectionVisual();
	
	// Parse the tiles using AttributeLayerSection (prevent code duplication)
	level::AttributeLayerSectionPtr attribSection = level::AttributeLayerSection::createFromText(
			tilesAsPlainText, sectionRect.getPosition());
	if (attribSection != 0)
	{
		if (themeTilesAsPlainText.empty() == false)
		{
			attribSection->copyThemeTilesFromText(themeTilesAsPlainText);
		}
		
		TT_ASSERT(attribSection->getAttributeLayer()->getWidth()  == m_floatingSection->getAttributeLayer()->getWidth() &&
		          attribSection->getAttributeLayer()->getHeight() == m_floatingSection->getAttributeLayer()->getHeight());
		m_floatingSection->getAttributeLayer()->swap(attribSection->getAttributeLayer());
	}
	
	// Parse the entities from JSON
	m_floatingSection->addEntities(createEntitiesFromJson(entityJson, m_levelData, 0, 0));
	
	m_levelData->deselectAllEntities();
	setFloatingSectionPosition(sectionRect.getPosition());
	setSelectionRect(sectionRect);
	switchToTool(tools::ToolID_BoxSelect);
}


void Editor::hotKeySelectAll()
{
	// Changing the selection also applies the floating section
	if (hasFloatingSection())
	{
		applyFloatingSection();
	}
	
	setSelectionRect(m_levelRect);
}


void Editor::hotKeyDeselectAll()
{
	clearSelectionRect(true);
}


void Editor::hotKeyFillTilesInSelection()
{
	if (hasSelectionRect())
	{
		// Fill selection with the selected paint tile
		switch (m_tileLayerMode)
		{
		case TileLayerMode_CollisionType:
			fillSelectionWithTile(m_paintTile);
			break;
			
		case TileLayerMode_ThemeType:
			fillSelectionWithTile(m_paintTheme);
			break;
			
		default:
			TT_PANIC("Unsupported tile layer mode: %d. Cannot fill selection with paint tile.");
			break;
		}
	}
}


void Editor::hotKeyFillTilesWithAirInSelection()
{
	if (hasSelectionRect())
	{
		// Fill selection with the selected paint tile
		switch (m_tileLayerMode)
		{
		case TileLayerMode_CollisionType:
			fillSelectionWithTile(level::CollisionType_Air);
			break;
			
		case TileLayerMode_ThemeType:
			fillSelectionWithTile(level::ThemeType_UseLevelDefault);
			break;
			
		default:
			TT_PANIC("Unsupported tile layer mode: %d. Cannot fill selection with air.");
			break;
		}
	}
}


void Editor::hotKeyEraseTilesInSelection()
{
	if (hasSelectionRect())
	{
		// Clear tiles and entities in selection by applying an empty level section to it
		pushUndoCommand(commands::CommandApplyLevelSection::create(
			m_levelData,
			level::LevelSection::createEmpty(m_selectionRect)));
	}
	else if (m_levelData->getSelectedEntities().empty() == false)
	{
		// Delete the selected entities
		const level::entity::EntityInstanceSet& selectedEntities(m_levelData->getSelectedEntities());
		pushUndoCommand(commands::CommandAddRemoveEntities::createForRemove(this,
			level::entity::EntityInstances(selectedEntities.begin(), selectedEntities.end())));
	}
}


void Editor::hotKeyFlipSelectionContentsHorizontally()
{
	if (hasSelectionRect() == false)
	{
		// No selection: cannot do anything
		return;
	}
	
	level::LevelSectionPtr section = level::LevelSection::createFromLevelRect(m_levelData, m_selectionRect);
	if (section == 0)
	{
		TT_PANIC("Could not create a level section for the flipped selection.");
		return;
	}
	
	// Flip the tiles in-place
	{
		level::AttributeLayerPtr attribs = section->getAttributeLayer();
		const s32                width   = attribs->getWidth();
		const s32                height  = attribs->getHeight();
		u8*                      rowPtr  = attribs->getRawData();
		
		for (s32 y = 0; y < height; ++y, rowPtr += width)
		{
			s32 dstX = width - 1;
			for (s32 srcX = 0; srcX < width / 2; ++srcX, --dstX)
			{
				std::swap(rowPtr[srcX], rowPtr[dstX]);
			}
		}
	}
	
	// Flip the entity positions in-place
	{
		using level::entity::EntityInstances;
		const EntityInstances& entities(section->getEntities());
		const real             width = level::tileToWorld(section->getRect()).getWidth();
		for (EntityInstances::const_iterator it = entities.begin(); it != entities.end(); ++it)
		{
			tt::math::Vector2 pos((*it)->getPosition());
			pos.x = width - pos.x;
			(*it)->setPosition(pos);
		}
	}
	
	pushUndoCommand(commands::CommandApplyLevelSection::create(m_levelData, section));
}


void Editor::hotKeyFlipSelectionContentsVertically()
{
	if (hasSelectionRect() == false)
	{
		// No selection: cannot do anything
		return;
	}
	
	level::LevelSectionPtr section = level::LevelSection::createFromLevelRect(m_levelData, m_selectionRect);
	if (section == 0)
	{
		TT_PANIC("Could not create a level section for the flipped selection.");
		return;
	}
	
	// Flip the tiles in-place
	section->getAttributeLayer()->flipRows();
	
	// Flip the entity positions in-place
	{
		using level::entity::EntityInstances;
		const EntityInstances& entities(section->getEntities());
		const real             height = level::tileToWorld(section->getRect()).getHeight();
		for (EntityInstances::const_iterator it = entities.begin(); it != entities.end(); ++it)
		{
			tt::math::Vector2 pos((*it)->getPosition());
			pos.y = height - pos.y;
			(*it)->setPosition(pos);
		}
	}
	
	pushUndoCommand(commands::CommandApplyLevelSection::create(m_levelData, section));
}


void Editor::hotKeyCancelSelection()
{
	// Special hack for pressing Esc while Load Level window is open: close the window
	if (isLoadLevelDialogOpen())
	{
		closeLoadLevelDialog();
		return;
	}
	
	m_floatingSection.reset();
	m_floatingSectionVisual.reset();
	m_floatingSectionThemeVisual.reset();
	m_floatingSectionBackground.reset();
	clearSelectionRect(true);
}


void Editor::hotKeyApplyFloatingSection()
{
	if (hasFloatingSection())
	{
		clearSelectionRect(true);
	}
}


void Editor::hotKeySelectToolBoxSelect()
{
	switchToTool(tools::ToolID_BoxSelect);
}


void Editor::hotKeySelectToolDraw()
{
	switchToTool(tools::ToolID_Draw);
}


void Editor::hotKeySelectToolFloodFill()
{
	switchToTool(tools::ToolID_FloodFill);
}


void Editor::hotKeySelectToolHand()
{
	switchToTool(tools::ToolID_Hand);
}


void Editor::hotKeySelectToolResize()
{
	switchToTool(tools::ToolID_Resize);
}


void Editor::hotKeySelectToolEntityDraw()
{
	switchToTool(tools::ToolID_EntityPaint);
}


void Editor::hotKeySelectToolEntityMove()
{
	switchToTool(tools::ToolID_EntityMove);
}


void Editor::hotKeySelectToolNotes()
{
	switchToTool(tools::ToolID_Notes);
}


void Editor::hotKeyZoomIn()
{
	changeCameraFovBy(-cfg()->getRealDirect("toki.camera.editor.fov_zoom_step"));
}


void Editor::hotKeyZoomOut()
{
	changeCameraFovBy(cfg()->getRealDirect("toki.camera.editor.fov_zoom_step"));
}


void Editor::hotKeyCenterCameraOnLevel()
{
	Camera& cam(getEditorCamera());
	cam.setPosition(tt::math::Vector2(
		level::tileToWorld(m_levelData->getLevelWidth())  * 0.5f,
		level::tileToWorld(m_levelData->getLevelHeight()) * 0.5f));
}


void Editor::hotKeyFitLevelInScreen()
{
	Camera& cam(getEditorCamera());
	const real targetFov = cam.getFOVForWorldHeight(level::tileToWorld(m_levelData->getLevelHeight() + 8));
	
	cam.setFOV(targetFov, true);  // FIXME: If not instant, it will follow the pointer pos while zooming... annoying
	cam.setPosition(tt::math::Vector2(
		level::tileToWorld(m_levelData->getLevelWidth())  * 0.5f,
		level::tileToWorld(m_levelData->getLevelHeight()) * 0.5f));
	// FIXME: Also change FOV so that the entire level fits on screen
}


void Editor::hotKeyEnterPressed()
{
	// The behavior of the Enter key is conditional:
	// - If the Load Level dialog is open, confirm the selection in that dialog
	// - Otherwise (the default behavior): apply the floating section
	
	if (isLoadLevelDialogOpen())
	{
		onLoadLevelDialog_OpenClicked(0);
	}
	else
	{
		hotKeyApplyFloatingSection();
	}
}


void Editor::hotKeySelectLevelSettings()
{
	selectFirstEntityOfType("LevelSettings");
}


void Editor::hotKeySelectPlayerBot()
{
	selectFirstEntityOfType("PlayerBot");
}


void Editor::hotKeyToggleMenuItem(Gwen::Controls::Base* p_sender)
{
	Gwen::Controls::MenuItem* item = gwen_cast<Gwen::Controls::MenuItem>(p_sender);
	TT_NULL_ASSERT(item);
	if (item != 0)
	{
		item->SetChecked(item->GetChecked() == false);
	}
}


void Editor::onNewLevel()
{
	if (hasUnsavedChanges())
	{
		showSaveChangesDialog(SaveChangesContinueAction_NewLevel);
	}
	else
	{
		showNewLevelDialog();
	}
}


void Editor::onSaveLevel()
{
	if (isUnnamedLevel())
	{
		onSaveLevelAs();
	}
	else
	{
		saveLevel();
	}
}


void Editor::onSaveLevelAs()
{
	showSaveAsDialog(SaveChangesContinueAction_None);
}


void Editor::onPublishToWorkshop()
{
#if defined(TT_STEAM_BUILD)
	
	// Do not allow publishing to Steam Workshop if the user has not enabled Steam Cloud
	// (because that is the mechanism Workshop operates on, so without it publishing won't work)
	if (SteamRemoteStorage()->IsCloudEnabledForAccount() == false ||
	    SteamRemoteStorage()->IsCloudEnabledForApp()     == false)
	{
		ui::GenericDialogBox* dlg = showGenericDialog(translateString("WINDOW_PUBLISH_NO_CLOUD_TITLE"),
		                                              translateString("WINDOW_PUBLISH_NO_CLOUD_PROMPT"));
		dlg->SetSize(415, 130);
		centerInScreen(dlg);
		return;
	}
	
	// Publishing to Workshop always auto-saves the level
	if (isUnnamedLevel())
	{
		showSaveAsDialog(SaveChangesContinueAction_PublishToWorkshop);
		return;
	}
	
	if (hasUnsavedChanges() || m_entitiesUpdatedByScript.empty() == false)
	{
		saveLevel();
	}
	
	// Now that the level is saved, show the Publish To Workshop UI
	if (m_ui.windowPublishToWorkshop == 0)
	{
		m_ui.windowPublishToWorkshop = ui::PublishToWorkshopDialog::create(this, m_gwenRoot.getCanvas());
	}
	
	TT_NULL_ASSERT(m_ui.windowPublishToWorkshop);
	if (m_ui.windowPublishToWorkshop->Hidden())  // don't show dialog if already open
	{
		openModalDialogBox(m_ui.windowPublishToWorkshop, false);
	}
#endif
}


void Editor::onCloseEditor()
{
	hide();
}


void Editor::onReloadAll()
{
	AppGlobal::getGame()->onEditorReloadAll();
}


void Editor::onExitApp()
{
	if (hasUnsavedChanges())
	{
		showSaveChangesDialog(SaveChangesContinueAction_ExitApp);
	}
	else
	{
		tt::app::getApplication()->terminate(true);
	}
}


// FIXME: This function and the one below it should be placed above onSaveLevel!
void Editor::onClearTiles()
{
	//TT_Printf("Editor::pressedClearTiles: Clear tiles!\n");
	
	// FIXME: Should this also clear all theme tiles?
	
	// Clear the attribute layer by painting all tiles to Air
	// FIXME: This is a very inefficient implementation...
	level::AttributeLayerPtr layer(m_levelData->getAttributeLayer());
	commands::CommandPaintTilesPtr paintCmd(commands::CommandPaintTiles::create(layer, level::CollisionType_Air));
	
	tt::math::Point2 pos;
	for (pos.y = 0; pos.y < layer->getHeight(); ++pos.y)
	{
		for (pos.x = 0; pos.x < layer->getWidth(); ++pos.x)
		{
			paintCmd->addTile(pos);
		}
	}
	
	if (paintCmd->hasTiles())
	{
		pushUndoCommand(paintCmd);
	}
}


void Editor::onClearEntities()
{
	const level::entity::EntityInstances entities(getMissionSpecificEntities());
	
	commands::CommandAddRemoveEntitiesPtr cmd =
		commands::CommandAddRemoveEntities::createForRemove(this, entities);
	if (cmd->hasEntities())
	{
		pushUndoCommand(cmd);
	}
}


void Editor::onClearWholeLevel()
{
	level::LevelDataPtr clearedLevel = level::LevelData::create(
			m_levelData->getLevelWidth(), m_levelData->getLevelHeight());
	if (clearedLevel != 0)
	{
		commands::CommandReplaceLevelPtr cmd = commands::CommandReplaceLevel::create(this, clearedLevel);
		if (cmd != 0)
		{
			pushUndoCommand(cmd);
		}
	}
}


void Editor::selectedTool(Gwen::Controls::Base* p_sender)
{
	tt::gwen::ButtonList* toolList = gwen_cast<tt::gwen::ButtonList>(p_sender);
	TT_NULL_ASSERT(toolList);
	if (toolList == 0)
	{
		return;
	}
	
	Gwen::Controls::Base* selectedRow = toolList->GetSelectedRow();
	if (selectedRow != 0)
	{
		const tools::ToolID newTool = tools::getToolIDFromName(selectedRow->GetName());
		if (tools::isValidToolID(newTool))
		{
			//TT_Printf("Editor::selectedTool: Switching active paint tool from '%s' to '%s'.\n",
			//          getPaintToolName(m_activePaintTool), getPaintToolName(selectedTool));
			switchToTool(newTool);
		}
		else
		{
			TT_PANIC("Unsupported paint tool: '%s'", selectedRow->GetName().c_str());
		}
	}
}


void Editor::selectedMission(Gwen::Controls::Base* p_sender)
{
	tt::gwen::ListBoxEx* listEx = gwen_cast<tt::gwen::ListBoxEx>(p_sender);
	if (listEx == 0)
	{
		return;
	}
	
	const std::string missionID(listEx->GetSelectedRowName());
	
	AppGlobal::getGame()->setMissionID(missionID);
}


void Editor::selectedPaintTile(Gwen::Controls::Base* p_sender)
{
	tt::gwen::ButtonList* tileList = gwen_cast<tt::gwen::ButtonList>(p_sender);
	if (tileList == 0)
	{
		return;
	}
	
	Gwen::Controls::Base* selectedRow = tileList->GetSelectedRow();
	if (selectedRow != 0)
	{
		const std::string typeName(selectedRow->GetName());
		bool updatedTile = false;
		
		switch (m_tileLayerMode)
		{
		case TileLayerMode_CollisionType:
			{
				const level::CollisionType selectedTile = level::getCollisionTypeFromName(typeName);
				if (level::isValidCollisionType(selectedTile))
				{
					//TT_Printf("Editor::selectedPaintTile: Switching paint tile from '%s' to '%s'.\n",
					//          level::getCollisionTypeName(m_paintTile), getCollisionTypeName(selectedTile));
					m_paintTile = selectedTile;
					updatedTile = true;
				}
			}
			break;
			
		case TileLayerMode_ThemeType:
			{
				const level::ThemeType selectedTile = level::getThemeTypeFromName(typeName);
				if (level::isValidThemeType(selectedTile))
				{
					//TT_Printf("Editor::selectedPaintTile: Switching paint theme tile from '%s' to '%s'.\n",
					//          level::getThemeTypeName(m_paintTheme), getThemeTypeName(selectedTile));
					m_paintTheme = selectedTile;
					updatedTile  = true;
				}
			}
			break;
			
		default:
			TT_PANIC("Unsupported tile layer mode: %d. Do not know how to handle tile selection.",
			         m_tileLayerMode);
			break;
		}
		
		if (updatedTile)
		{
			// Also switch to a tile painting tool, if one is not active
			if (m_activePaintTool != tools::ToolID_Draw &&
			    m_activePaintTool != tools::ToolID_FloodFill)
			{
				if (m_previousPaintTool == tools::ToolID_Draw ||
				    m_previousPaintTool == tools::ToolID_FloodFill)
				{
					switchToTool(m_previousPaintTool);
				}
				else
				{
					switchToTool(tools::ToolID_Draw);
				}
			}
		}
		else
		{
			TT_PANIC("Unsupported paint tile: '%s'", typeName.c_str());
		}
	}
}


void Editor::selectedEntityInLibrary(Gwen::Controls::Base* p_sender)
{
	tt::gwen::GroupedButtonList* list = gwen_cast<tt::gwen::GroupedButtonList>(p_sender);
	if (list == 0)
	{
		return;
	}
	
	Gwen::Controls::Button* selectedItem = list->GetSelectedRow();
	if (selectedItem == 0)
	{
		// "Nothing" was selected in the entity library
		m_paintEntityType.clear();
		return;
	}
	
	// Verify that the entity name that was selected actually (still) exists in the library
	const std::string typeName(selectedItem->GetName());
	if (AppGlobal::getEntityLibrary().getEntityInfo(typeName) == 0)
	{
		TT_PANIC("Selected an entity in the entity library that does not exist (name '%s').",
		         typeName.c_str());
		return;
	}
	
	m_paintEntityType = typeName;
	
	// Also switch to an entity painting tool, if one is not active
	if (m_activePaintTool != tools::ToolID_EntityPaint)
	{
		switchToTool(tools::ToolID_EntityPaint);
	}
}


void Editor::onShowThemeTilesChanged(Gwen::Controls::Base* p_sender)
{
	Gwen::Controls::MenuItem* menuItem = gwen_cast<Gwen::Controls::MenuItem>(p_sender);
	TT_NULL_ASSERT(menuItem);
	setTileLayerMode(menuItem->GetChecked() ? TileLayerMode_ThemeType : TileLayerMode_CollisionType);
}


void Editor::onShowStatusBarChanged(Gwen::Controls::Base* p_sender)
{
	Gwen::Controls::MenuItem* menuItem = gwen_cast<Gwen::Controls::MenuItem>(p_sender);
	TT_NULL_ASSERT(menuItem);
	m_ui.statusBar->SetHidden(menuItem->GetChecked() == false);
}


void Editor::onGameLayerVisibilityChanged(Gwen::Controls::Base* p_sender)
{
	Gwen::Controls::MenuItem* menuItem = gwen_cast<Gwen::Controls::MenuItem>(p_sender);
	TT_NULL_ASSERT(menuItem);
	m_persistentSettings.gameLayersVisible.setFlag(menuItem->UserData.Get<GameLayer>("gameLayer"), menuItem->GetChecked());
}


void Editor::onEntityRenderFlagChanged(Gwen::Controls::Base* p_sender)
{
	Gwen::Controls::MenuItem* menuItem = gwen_cast<Gwen::Controls::MenuItem>(p_sender);
	TT_NULL_ASSERT(menuItem);
	
	using level::entity::editor::RenderFlag;
	const RenderFlag flag = menuItem->UserData.Get<RenderFlag>("flag");
	m_persistentSettings.entityRenderFlags.setFlag(flag, menuItem->GetChecked());
}


void Editor::onShowEditToolsChanged(Gwen::Controls::Base* p_sender)
{
	Gwen::Controls::MenuItem* menuItem = gwen_cast<Gwen::Controls::MenuItem>(p_sender);
	TT_NULL_ASSERT(menuItem);
	m_ui.windowTools->SetHidden(menuItem->GetChecked() == false);
}


void Editor::onShowSelectMissionChanged(Gwen::Controls::Base* p_sender)
{
	Gwen::Controls::MenuItem* menuItem = gwen_cast<Gwen::Controls::MenuItem>(p_sender);
	TT_NULL_ASSERT(menuItem);
	const bool hidden(menuItem->GetChecked() == false);
	m_ui.windowMissions->SetHidden(hidden);
	
	if (hidden == false)
	{
		refreshMissionList();
	}
}


void Editor::onAutoSyncEntityChangesChanged(Gwen::Controls::Base* p_sender)
{
	Gwen::Controls::MenuItem* menuItem = gwen_cast<Gwen::Controls::MenuItem>(p_sender);
	TT_NULL_ASSERT(menuItem);
	m_persistentSettings.autoSyncEntityChanges = menuItem->GetChecked();
}


void Editor::onSelectLevelBackground(Gwen::Controls::Base* p_sender)
{
	Gwen::Controls::MenuItem* menuItem = gwen_cast<Gwen::Controls::MenuItem>(p_sender);
	TT_NULL_ASSERT(menuItem);
	if (menuItem == 0) return;
	
	commands::CommandSetLevelBackgroundPtr cmd = commands::CommandSetLevelBackground::create(
			this, menuItem->GetName());
	if (cmd->hasChanges())
	{
		pushUndoCommand(cmd);
	}
}


void Editor::onSelectLevelTheme(Gwen::Controls::Base* p_sender)
{
	Gwen::Controls::MenuItem* menuItem = gwen_cast<Gwen::Controls::MenuItem>(p_sender);
	TT_NULL_ASSERT(menuItem);
	if (menuItem == 0) return;
	
	const std::string      themeName(menuItem->GetName());
	const level::ThemeType theme = level::getThemeTypeFromName(themeName);
	if (level::isValidThemeType(theme) == false)
	{
		TT_PANIC("Internal error: invalid level theme selected from menu: '%s'", themeName.c_str());
		return;
	}
	
	commands::CommandSetLevelThemePtr cmd = commands::CommandSetLevelTheme::create(
			this, theme);
	if (cmd->hasChanges())
	{
		pushUndoCommand(cmd);
	}
}


void Editor::onSelectDefaultMission(Gwen::Controls::Base* p_sender)
{
	Gwen::Controls::MenuItem* menuItem = gwen_cast<Gwen::Controls::MenuItem>(p_sender);
	TT_NULL_ASSERT(menuItem);
	if (menuItem == 0) return;
	
	const std::string& missionName(menuItem->GetName());
	
	commands::CommandSetDefaultMissionPtr cmd = commands::CommandSetDefaultMission::create(
			this, missionName);
	if (cmd->hasChanges())
	{
		pushUndoCommand(cmd);
	}
}


void Editor::onOpenThemeColorPicker()
{
	if (m_ui.windowSetThemeColor == 0)
	{
		m_ui.windowSetThemeColor = ui::SetThemeColorUI::create(this, "Set Theme Colors", m_gwenRoot.getCanvas());
		
		// Restore the saved window size and position
		const tt::math::PointRect& savedRect(m_persistentSettings.toolWindow[
				EditorSettings::ToolWindow_SetThemeColors].windowRect);
		
		if (savedRect.hasArea())
		{
			m_ui.windowSetThemeColor->SetSize(savedRect.getWidth(), savedRect.getHeight());
		}
		
		if (savedRect.getPosition() != tt::math::Point2(-1, -1))
		{
			m_ui.windowSetThemeColor->SetPos(savedRect.getPosition().x, savedRect.getPosition().y);
		}
		else
		{
			const tt::math::Point2 scrSize(getEditorCamera().getViewPortSize());
			m_ui.windowSetThemeColor->SetPos(
					(scrSize.x - m_ui.windowSetThemeColor->Width())  / 2,
					(scrSize.y - m_ui.windowSetThemeColor->Height()) / 2);
		}
	}
	
	if (m_ui.windowSetThemeColor != 0)
	{
		m_ui.windowSetThemeColor->SetHidden(false);
	}
}


void Editor::onOpenPublishedLevelBrowser()
{
#if defined(TT_STEAM_BUILD)
	// FIXME: Only allow one instance to open (also make the window visibility and rect persistent?)
	ui::PublishedLevelBrowser* dlg = ui::PublishedLevelBrowser::create(this, m_gwenRoot.getCanvas());
	if (dlg != 0)
	{
		centerInScreen(dlg);
		dlg->SetHidden(false);
	}
#endif
}


void Editor::onOpenSteamWorkshopTest()
{
#if defined(TT_STEAM_BUILD) && !defined(TT_BUILD_FINAL)
	ui::SteamWorkshopTest* wnd = ui::SteamWorkshopTest::create(this, m_gwenRoot.getCanvas());
	if (wnd != 0)
	{
		const tt::math::Point2 scrSize(getEditorCamera().getViewPortSize());
		wnd->SetPos((scrSize.x - wnd->Width()) / 2, (scrSize.y - wnd->Height()) / 2);
		wnd->SetHidden(false);
	}
#endif
}


void Editor::onOpenDebugSteamCloudFileManager()
{
#if defined(TT_STEAM_BUILD) && !defined(TT_BUILD_FINAL)
	ui::DebugSteamCloudFileManager* wnd = ui::DebugSteamCloudFileManager::create(this, m_gwenRoot.getCanvas());
	if (wnd != 0)
	{
		centerInScreen(wnd);
		wnd->SetHidden(false);
	}
#endif
}


void Editor::onLoadLevelDialog_FilterSelected(Gwen::Controls::Base* p_sender)
{
	Gwen::Controls::Button* button = gwen_cast<Gwen::Controls::Button>(p_sender);
	TT_NULL_ASSERT(button);
	if (button == 0) return;
	
	const LoadLevelFilter selectedFilter = button->UserData.Get<LoadLevelFilter>("filter");
	if (isValidLoadLevelFilter(selectedFilter) == false)
	{
		TT_PANIC("Filter button does not have valid 'filter' user data.");
		return;
	}
	
	if (selectedFilter == m_persistentSettings.loadLevelFilter)
	{
		// Selected filter did not change
		return;
	}
	
	// Update the filter selection buttons
	m_persistentSettings.loadLevelFilter = selectedFilter;
	for (s32 i = 0; i < LoadLevelFilter_Count; ++i)
	{
		if (m_ui.windowLoadLevelFilterButton[i] != 0 &&
		    i != selectedFilter)
		{
			m_ui.windowLoadLevelFilterButton[i]->SetToggleState(false);
		}
	}
	
	// Refresh the level list for the new filter
	refreshLevelListForCurrentFilter();
	m_ui.listLevels->Focus();
}


void Editor::onLoadLevelDialog_FilterDeselected(Gwen::Controls::Base* p_sender)
{
	// Do not allow deselecting the selected filter
	Gwen::Controls::Button* button = gwen_cast<Gwen::Controls::Button>(p_sender);
	TT_NULL_ASSERT(button);
	if (button == 0) return;
	
	const LoadLevelFilter filter = button->UserData.Get<LoadLevelFilter>("filter");
	if (isValidLoadLevelFilter(filter) == false)
	{
		TT_PANIC("Filter button does not have valid 'filter' user data.");
		return;
	}
	
	if (filter == m_persistentSettings.loadLevelFilter)
	{
		button->SetToggleState(true);
	}
}


void Editor::onLoadLevelDialog_OpenClicked(Gwen::Controls::Base* /*p_sender*/)
{
	if (isLoadLevelDialogOpen() == false || m_ui.listLevels == 0)
	{
		return;
	}
	
	const std::string selectedLevelName(m_ui.listLevels->GetSelectedRowName());
	if (selectedLevelName.empty())
	{
		// TODO: Display message box?
		return;
	}
	
	doLoadSelectedLevel(true);
}


void Editor::onNewLevelDialogClosed(Gwen::Controls::Base* p_sender)
{
	ui::NewLevelDialog* dialog = gwen_cast<ui::NewLevelDialog>(p_sender);
	TT_NULL_ASSERT(dialog);
	if (dialog == 0 || dialog->getResult() != ui::SaveAsDialog::Result_Save)
	{
		return;
	}
	
	// Load the template level
	const std::string templateLevelFilename("levels/template_userlevel.ttlvl");
	level::LevelDataPtr templateLevel;
	if (tt::fs::fileExists(templateLevelFilename))
	{
		templateLevel = level::LevelData::loadLevel(templateLevelFilename);
	}
	if (templateLevel == 0)
	{
		showGenericDialog(translateString("WINDOW_NEWLEVEL_FAILED_TITLE"),
		                  translateString("WINDOW_NEWLEVEL_FAILED_TEMPLATE_PROMPT"));
		return;
	}
	
	// Save the level under the new name
	const StartInfo&  currentStartInfo(getCurrentLevelInfo());
	const std::string newLevelName(dialog->getFilename());
	const std::string filename(currentStartInfo.getLevelPath() + newLevelName + ".ttlvl");
	std::string       internalSourceFilename;
	
#if EDITOR_SUPPORTS_ASSETS_SOURCE
	// HACK: Also save the source asset (hard-coded relative path to the asset source files)
	if (currentStartInfo.isUserLevel() == false)
	{
		internalSourceFilename = getLevelsSourceDir() + newLevelName + ".ttlvl";
	}
#endif
	
	bool saveOk = true;
	if (templateLevel->save(filename) == false)
	{
		saveOk = false;
	}
	
	if (internalSourceFilename.empty() == false &&
	    templateLevel->save(internalSourceFilename) == false)
	{
		saveOk = false;
	}
	
	if (saveOk == false)
	{
		showGenericDialog(translateString("WINDOW_NEWLEVEL_FAILED_TITLE"),
		                  translateString("WINDOW_NEWLEVEL_FAILED_SAVE_PROMPT"));
		return;
	}
	
	// Update the levels list if this isn't a user level
	if (currentStartInfo.isUserLevel() == false)
	{
		AppGlobal::addLevelToLevelNames(newLevelName);
		TT_ASSERT(AppGlobal::findInLevelNames(newLevelName));
	}
	
	// Load the newly created level
	onModalDialogClosed(p_sender);
	doLoadLevel(currentStartInfo, newLevelName);
}


void Editor::onSaveChangesDialogClosed(Gwen::Controls::Base* p_sender)
{
	ui::SaveChangesDialog* dialog = gwen_cast<ui::SaveChangesDialog>(p_sender);
	TT_NULL_ASSERT(dialog);
	if (dialog == 0) return;
	
	const ui::SaveChangesDialog::Result result = dialog->getResult();
	const SaveChangesContinueAction continueAction = dialog->UserData.Get<SaveChangesContinueAction>("continueAction");
	
	switch (result)
	{
	case ui::SaveChangesDialog::Result_Save:
		// Save the level and continue
		if (isUnnamedLevel())
		{
			// Level has no name: request a name from the user before saving
			showSaveAsDialog(continueAction);
			return;
		}
		else if (saveLevel() == false)
		{
			// Could not save the level! Don't continue with loading, because that would mean losing data.
			return;
		}
		
		// NOTE: Intentional fall-through: after saving, we still need to load the selected level
		
	case ui::SaveChangesDialog::Result_Discard:
		// Before closing the editor, remove the Save Changes dialog from the list of open dialogs
		// (so that hide() won't attempt to auto-close the dialog for which we're still handling callbacks,
		//  causing the GWEN callback calling code to use an invalid iterator)
		onModalDialogClosed(p_sender);
		
		switch (continueAction)
		{
		case SaveChangesContinueAction_LoadLevel:
			// Don't save the level and continue loading the new level
			doLoadSelectedLevel(false);
			break;
			
		case SaveChangesContinueAction_ExitApp:
			tt::app::getApplication()->terminate(true);
			break;
			
		case SaveChangesContinueAction_NewLevel:
			showNewLevelDialog();
			break;
			
		default:
			TT_PANIC("Unsupported action to perform after closing Save Changes dialog: %d", continueAction);
			break;
		}
		break;
		
	case ui::SaveChangesDialog::Result_Cancel:
		// Do not continue with the post-close action (in fact, do nothing at all)
		break;
		
	default:
		TT_PANIC("Save Changes dialog was closed with an unsupported result: %d", result);
		break;
	}
}


void Editor::onSaveAsDialogClosed(Gwen::Controls::Base* p_sender)
{
	ui::SaveAsDialog* dialog = gwen_cast<ui::SaveAsDialog>(p_sender);
	TT_NULL_ASSERT(dialog);
	if (dialog == 0)
	{
		return;
	}
	
	const SaveChangesContinueAction continueAction = dialog->UserData.Get<SaveChangesContinueAction>("continueAction");
	
	if (dialog->getResult() != ui::SaveAsDialog::Result_Save)
	{
		// Saving was cancelled (or we got this notification from something we didn't expect)
		if (continueAction != SaveChangesContinueAction_None     &&
		    continueAction != SaveChangesContinueAction_PlayTest &&
		    continueAction != SaveChangesContinueAction_PublishToWorkshop)
		{
			// If this dialog was opened in the "level load" / "save changes" use case,
			// reopen the "Save Changes" dialog
			showSaveChangesDialog(continueAction);
		}
		return;
	}
	
	// When saving a level under a new name, that new level is no longer published to Steam Workshop
	m_levelData->setPublishedFileId(0);
	
#if defined(TT_STEAM_BUILD)
	// Also force the Publish To Workshop UI to reset itself when reopened
	// (the publish UI is modal, so Save As can't be called while the publish UI is open: safe to simply destroy it)
	if (m_ui.windowPublishToWorkshop != 0)
	{
		m_ui.windowPublishToWorkshop->DelayedDelete();
		m_ui.windowPublishToWorkshop = 0;
	}
#endif
	
	// Save the level under the new name
	const StartInfo&  currentStartInfo(getCurrentLevelInfo());
	const std::string newLevelName(dialog->getFilename());
	const std::string filename(currentStartInfo.getLevelPath() + newLevelName + ".ttlvl");
	std::string       internalSourceFilename;
	
#if EDITOR_SUPPORTS_ASSETS_SOURCE
	// HACK: Also save the source asset (hard-coded relative path to the asset source files)
	if (currentStartInfo.isUserLevel() == false)
	{
		internalSourceFilename = getLevelsSourceDir() + newLevelName + ".ttlvl";
	}
#endif
	
	if (saveLevel(filename, internalSourceFilename) == false)
	{
		// Saving the level failed
		if (continueAction != SaveChangesContinueAction_None)
		{
			// If this dialog was opened in the "level load" / "save changes" use case,
			// reopen the "Save Changes" dialog
			showSaveChangesDialog(continueAction);
		}
		return;
	}
	
	// Update the levels list if this isn't a user level
	if (currentStartInfo.isUserLevel() == false)
	{
		AppGlobal::addLevelToLevelNames(newLevelName);
		TT_ASSERT(AppGlobal::findInLevelNames(newLevelName));
	}
	
	// Update the game's start info for the new level name
	// (the game is able to handle level name changes by itself)
	setCurrentLevelName(newLevelName, true);
	
	switch (continueAction)
	{
	case SaveChangesContinueAction_None:
		// Do nothing
		break;
		
	case SaveChangesContinueAction_LoadLevel:
		// If this dialog was opened in the "level load" / "save changes" use case,
		// continue loading the selected level now that this level has been saved
		onModalDialogClosed(p_sender);
		doLoadSelectedLevel(false);
		break;
		
	case SaveChangesContinueAction_ExitApp:
		tt::app::getApplication()->terminate(true);
		break;
		
	case SaveChangesContinueAction_PlayTest:
		onModalDialogClosed(p_sender);
		startPlayTest();
		break;
		
	case SaveChangesContinueAction_NewLevel:
		showNewLevelDialog();
		break;
		
	case SaveChangesContinueAction_PublishToWorkshop:
		onPublishToWorkshop();
		break;
		
	default:
		TT_PANIC("Unsupported action to perform after closing Save As dialog: %d", continueAction);
		break;
	}
}


void Editor::onSavePlayTestAsDialogClosed(Gwen::Controls::Base* p_sender)
{
	ui::GenericDialogBox* dialog = gwen_cast<ui::GenericDialogBox>(p_sender);
	TT_NULL_ASSERT(dialog);
	if (dialog == 0)
	{
		return;
	}
	
	if (dialog->getResult() != ui::SaveAsDialog::Result_OK)
	{
		return;
	}
	
	// Save the level and continue
	if (isUnnamedLevel())
	{
		// Level has no name: request a name from the user before saving
		showSaveAsDialog(SaveChangesContinueAction_PlayTest);
		return;
	}
	else if (saveLevel() == false)
	{
		// Could not save the level! Don't continue with loading, because that would mean losing data.
		return;
	}
	
	onModalDialogClosed(p_sender);
	startPlayTest();
}


void Editor::onOpenEditorDuringPlayTestDialogClosed(Gwen::Controls::Base* p_sender)
{
	ui::GenericDialogBox* dialog = gwen_cast<ui::GenericDialogBox>(p_sender);
	TT_NULL_ASSERT(dialog);
	if (dialog == 0)
	{
		return;
	}
	
	if (dialog->getResult() == ui::SaveAsDialog::Result_OK)
	{
		// User wanted to edit level (stop play test)
		m_doingPlayTest = false;
		AppGlobal::getGame()->unscheduleScreenshot();
		return;
	}
	else
	{
		// User didn't want to edit the level. (Wanted to continue play test.)
		onModalDialogClosed(p_sender);
		hide();
	}
}


void Editor::onModalDialogClosed(Gwen::Controls::Base* p_sender)
{
	ui::DialogBoxBase* dialog = gwen_cast<ui::DialogBoxBase>(p_sender);
	TT_NULL_ASSERT(dialog);
	if (dialog == 0) return;
	
	DialogBoxes::iterator it = std::find(m_openDialogBoxes.begin(), m_openDialogBoxes.end(), dialog);
	if (it != m_openDialogBoxes.end())
	{
		m_openDialogBoxes.erase(it);
	}
}


void Editor::onSaveBeforeLevelLoadFromGameDialogClosed(Gwen::Controls::Base* p_sender)
{
	ui::GenericDialogBox* dialog = gwen_cast<ui::GenericDialogBox>(p_sender);
	TT_NULL_ASSERT(dialog);
	if (dialog == 0)
	{
		return;
	}
	
	if (dialog->getResult() == ui::SaveAsDialog::Result_Yes)
	{
		saveLevel();
	}
	StartInfo info(AppGlobal::getGame()->getStartInfo());
	info.setLevel(m_loadLevelFromGameName);
	AppGlobal::setGameStartInfo(info);
	AppGlobal::getGame()->forceReload();
	hide();
}

// Namespace end
}
}
}
