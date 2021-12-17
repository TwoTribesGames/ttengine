SuperStrict

Import "Selection.bmx"
Import "ShoeTool.bmx"
Import "MoveTool.bmx"
Import "SelectTool.bmx"
Import "GlobalSelectTool.bmx"
Import "RotateTool.bmx"
Import "ScaleTool.bmx"
Import "SpecialScaleTool.bmx"

Import "SliceTool.bmx"
Import "FogTool.bmx"
Import "ScrollTool.bmx"
Import "LookTool.bmx"
Import "ZoomTool.bmx"
Import "SaveTool.bmx"
Import "FileTool.bmx"
Import "TagTool.bmx"
Import "OpenTool.bmx"
Import "OpenXMLTool.bmx"
Import "ReloadTool.bmx"
Import "CloneTool.bmx"
Import "BrushTool.bmx"
Import "DeleteTool.bmx"
Import "HideTool.bmx"
Import "HelpTool.bmx"
Import "AutoTileTool.bmx"

Import "UndoTool.bmx"
Import "LevelTool.bmx"
Import "ColorTool.bmx"
Import "CameraBookmarkTool.bmx"

Import "TextArea.bmx"

Type ShoeToolManager

	Field ActivatableTools:TList = New TList
	Field ActiveTool:ShoeTool
	Field shoebox:ShoeBox
	Field win:Window
	Field selected:Selection
	Field ResidentTools:TList = New TList' Tools that are always active

	Global instance:ShoeToolManager
	
		
	'----------------------------------------------------------------------------
	Method New()
	
		If instance RuntimeError( "No more than one instance allowed!" )
		instance = Self
		
	EndMethod
	
	'----------------------------------------------------------------------------
	Method Init:ShoeToolManager( aWindow:Window, aselected:Selection, ashoebox:ShoeBox )
		
		win = aWindow
		selected = aselected
		shoebox = ashoebox

		Activate()
		
		' create tools
		' DISABLE UNDO (TOO MANY ISSUES)
		'ResidentTools.addLast( New UndoTool.InitUndo(          KEY_Z, MODIFIER_CONTROL, aselected, aWindow ) )
		ResidentTools.addLast( New CloneTool.InitClone(        KEY_C, MODIFIER_CONTROL, aselected, aWindow, ashoebox ) )
		ResidentTools.addLast( New DeleteTool.InitDelete( KEY_DELETE, MODIFIER_CONTROL, aselected, aWindow, ashoebox ) )
		ResidentTools.addLast( New HideTool.InitHide(          KEY_H, MODIFIER_CONTROL, aselected, aWindow, ashoebox ) )
		ResidentTools.addLast( New FileTool.InitFile(    KEY_T,                0, aselected, aWindow, ashoebox ) )
		ResidentTools.addLast( New AutoTiletool.InitAutotile(  KEY_G,                0, aselected, aWindow, ashoebox ) )
		ResidentTools.addLast( New ColorTool.InitColor(        KEY_K,                0, aselected, aWindow, ashoebox ) )

		ResidentTools.addLast( New LevelTool.InitLevel(        KEY_L, 0, aselected, aWindow, ashoebox ) )

		ResidentTools.addLast( New HelpTool.InitHelp(          KEY_F1,  0, aselected, aWindow, ashoebox ) )


		ResidentTools.addLast( New SaveTool.InitSave(       KEY_S, MODIFIER_CONTROL, aselected, aWindow, ashoebox ) )
		ResidentTools.addLast( New ReloadTool.InitReload(   KEY_F5, 0, aselected, aWindow, ashoebox ) )
		ResidentTools.addLast( New OpenTool.InitOpen(       KEY_O, MODIFIER_CONTROL, aselected, aWindow, ashoebox ) )
		ResidentTools.addLast( New OpenXMLTool.InitOpenXML( KEY_O, MODIFIER_OPTION, aselected, aWindow, ashoebox ) )
		
		ResidentTools.addLast( New ScrollTool.InitScroll (         KEY_SPACE, 0, aselected, aWindow ) )
		ResidentTools.addLast( New LookTool.InitLook (             KEY_V, 0, aselected, aWindow ) )

		ResidentTools.addLast( New ZoomTool.InitZoom (                     0, 0, aselected, aWindow ) )
		ResidentTools.addLast( New SliceTool.InitSlice(                KEY_A, 0, aselected, aWindow ) )
		ResidentTools.addLast( New GlobalSelectTool.InitGlobalSelect(      0, 0, aselected, aWindow, ashoebox ) )
		ResidentTools.addLast( New CameraBookmarkTool.InitCameraBookmark(KEY_TAB, 0, aselected, aWindow) )
		
		ResidentTools.addLast( New ZoomTool.InitZoom (                     0, 0, aselected, aWindow ) )
		
		ActivatableTools.addLast( New SelectTool.InitSelect(        KEY_Q, 0, aselected, aWindow) )
		ActivatableTools.addLast( New MoveTool.InitMove(            KEY_W, 0, aselected, aWindow ) )
		ActivatableTools.addLast( New RotateTool.InitRotate(        KEY_E, 0, aselected, aWindow ) )
		ActivatableTools.addLast( New ScaleTool.InitScale(          KEY_R, 0, aselected, aWindow ) )
		'ActivatableTools.addLast( New SpecialScaleTool.InitScale(   KEY_D, 0, aselected, aWindow ) )
		ActivatableTools.addLast( New BrushTool.InitBrush(          KEY_B, 0, aselected, aWindow, ashoebox ) )
		
		ActivatableTools.addLast( New FogTool.InitFog(       KEY_F, MODIFIER_CONTROL, aselected, aWindow, ashoebox ) )
		
		ActivatableTools.addLast( New TagTool.InitTag(       KEY_N, 0, aselected, aWindow) )
		
		ActivateTool( ShoeTool(ActivatableTools.First()) )
		
		Return Self
			
	EndMethod
	
	'----------------------------------------------------------------------------
	Method Update()
	
		If ActiveTool
			ActiveTool.Update()
		EndIf
		
	EndMethod
	
	'----------------------------------------------------------------------------
	Method ActivateTool( aTool:ShoeTool )
		
		If ActiveTool
			ActiveTool.OnDeactivate()
		EndIf
		
		ActiveTool = aTool
		ActiveTool.OnActivate()
		
	EndMethod
	
	Field IsActive:Int = False
	
	'----------------------------------------------------------------------------
	Method Deactivate()
		
		If IsActive
			IsActive = False
			RemoveHook( EmitEventHook, ShoeToolManager.KeyHook )
		EndIf
		
	EndMethod
	
	'----------------------------------------------------------------------------
	Method Activate()
	
		If Not IsActive
			AddHook( EmitEventHook, ShoeToolManager.KeyHook )
			IsActive = True
		EndIf
		
	EndMethod
	
	
	
	'----------------------------------------------------------------------------
	Method MouseMove( x:Int, y:Int, speedx:Float, speedy:Float, scaledSpeedx:Float, scaledSpeedy:Float )
		
		For Local t:ShoeTool = EachIn ResidentTools
			If t.MouseMove( x, y, speedx, speedy, scaledSpeedx, scaledSpeedy ) Return
		Next

		If ActiveTool
			ActiveTool.MouseMove( x, y, speedx, speedy, scaledSpeedx, scaledSpeedy )
		EndIf
		
	EndMethod
	
	'----------------------------------------------------------------------------
	Method MouseUp( but:Int, x:Int, y:Int )
		
		For Local t:ShoeTool = EachIn ResidentTools
			If t.MouseUp( but, x, y ) Return
		Next
				
		If ActiveTool
			ActiveTool.MouseUp( but, x, y )
		EndIf

	EndMethod
	
	'----------------------------------------------------------------------------
	Method MouseDown( but:Int, x:Int, y:Int )

		For Local t:ShoeTool = EachIn ResidentTools
			If t.MouseDown( but, x, y ) Return
		Next
		
		If ActiveTool
			selected.StoreStates()
			ActiveTool.MouseDown( but, x, y )
		EndIf
		
	EndMethod
	
	'----------------------------------------------------------------------------
	Method KeyDown( key:Int, modifiers:Int )
	
		For Local t:ShoeTool = EachIn ResidentTools
			If t.KeyDown( key, modifiers ) Return
		Next
		
		If ActiveTool
			'selection.StoreStates()
			ActiveTool.KeyDown( key, modifiers )
		EndIf
		
	EndMethod
	
	'----------------------------------------------------------------------------
	Method KeyUp( key:Int, modifiers:Int )
		
		For Local t:ShoeTool = EachIn ResidentTools
			If t.KeyUp( key, modifiers) Return
		Next
	
		If ActiveTool
			If ActiveTool.KeyUp( key, modifiers ) Return
		EndIf
	
		For Local s:ShoeTool = EachIn ActivatableTools
			
			If s.CheckHotkey( key, modifiers )
				
				ActivateTool( s )
	
				Exit
			EndIf
			
		Next
		
	EndMethod
	
	'----------------------------------------------------------------------------
	Method MouseWheel( speed:Int, x:Int, y:Int )
	
		If ActiveTool
			ActiveTool.MouseWheel( speed, x, y)
		EndIf
	
		For Local t:ShoeTool = EachIn ResidentTools
			t.MouseWheel( speed, x, y)
		Next

	EndMethod
	
	'----------------------------------------------------------------------------
	Method Tweak( dx:Float, dy:Float, dz:Float, dp:Int, big:Int )
		
		If ActiveTool
			If ActiveTool.Tweak( dx, dy, dz, dp, big ) Return
		EndIf
	Rem
		For Local t:ShoeTool = EachIn ResidentTools
			if t.Tweak( dx, dy, dz ) return
		Next
	EndRem
	
	EndMethod
	
	'----------------------------------------------------------------------------
	Method Random(big:Int)
		If ActiveTool
			If ActiveTool.Random(big) Return
		EndIf
	EndMethod
	
	'----------------------------------------------------------------------------
	Function KeyHook:Object( id:Int, data:Object, context:Object )
		If Not instance Return Null
		
		Global oldmousex:Float, oldmousey:Float
		
		Local ev:TEvent = TEvent(data)
		If Not ev Return data
	
		Select ev.id
			Case EVENT_KEYDOWN, EVENT_KEYREPEAT
				instance.KeyDown( ev.Data, ev.Mods )
				
				If ev.Data = KEY_UP Or ev.Data = KEY_DOWN Or ev.Data = KEY_LEFT Or ev.Data = KEY_RIGHT Or ev.Data = KEY_PAGEUP Or ev.Data = KEY_PAGEDOWN Or ev.Data = KEY_OPENBRACKET Or ev.Data = KEY_CLOSEBRACKET
					Local stepMode:Int = 0
					If (ev.mods & MODIFIER_SHIFT And ev.mods & MODIFIER_CONTROL)
						stepMode = 2
					ElseIf (ev.mods & MODIFIER_SHIFT)
						stepMode = 1
					EndIf
					
					instance.Tweak( (ev.Data = KEY_RIGHT) - (ev.Data = KEY_LEFT), ..
					                (ev.Data = KEY_UP) - (ev.Data = KEY_DOWN), ..
					                (ev.Data = KEY_PAGEDOWN) - (ev.Data = KEY_PAGEUP), ..
					                (ev.Data = KEY_CLOSEBRACKET) - (ev.Data = KEY_OPENBRACKET), ..
					                stepMode )
				EndIf
				
				If ev.Data = KEY_SLASH
					instance.Random( (ev.mods & MODIFIER_SHIFT) )
				EndIf
				
			Case EVENT_KEYUP
				instance.KeyUp( ev.Data, ev.Mods )
				
			Case EVENT_MOUSEMOVE
				Local speedx:Float = (ev.x - oldmousex)
				Local speedy:Float = (ev.y - oldmousey)
				Local worldX:Float = 0.0
				Local worldY:Float = 0.0
				
				instance.win.ScreenToWorld(speedx, speedy, worldX, worldY)
				instance.MouseMove( ev.x, ev.y, speedx, speedy, worldX, worldY )

				oldmousex = ev.x
				oldmousey = ev.y
				
			Case EVENT_MOUSEDOWN
				instance.MouseDown( ev.Data, ev.x, ev.y )
				
			Case EVENT_MOUSEUP
				instance.MouseUp( ev.Data, ev.x, ev.y )

			Case EVENT_MOUSEWHEEL
				instance.MouseWheel( ev.Data, ev.x, ev.y )
			Case EVENT_WINDOWACCEPT
?debug			
Print oldmousex + ", " + oldmousey
Print "~t..." + ev.ToString()
?
' todo maybe check if the ev.extra is a png file... (A)

				Local x:Float = instance.win.Camera.px
				Local y:Float = -instance.win.Camera.py
				Local z:Float = 0
				
				Local p:ShoePlane = instance.selected.GetFirst()
				If p <> Null 
					x = p.GetX()
					y = p.GetY()
					z = p.GetZ()
				EndIf
				
				Local plane:ShoePlane = instance.shoebox.NewPlane(String(ev.extra), x, y, z, p)
				If plane
					instance.selected.Clear()
					instance.selected.Add(plane)
				EndIf
			Case EVENT_APPSUSPEND
				'Print "SUSPENDED!"
			Case EVENT_APPRESUME
				If instance.shoebox.IsShoeboxOnDiskNewer() Then instance.selected.Clear()
				instance.shoebox.ReloadIfNewer()

			Default
				'Print ev.ToString()
		EndSelect
	EndFunction
EndType