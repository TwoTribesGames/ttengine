SuperStrict

Import "TConfig.bmx"
Import "ShoeToolManager.bmx"
Import "ShoeBox.bmx"
Import "Selection.bmx"
Import "Window.bmx"
Import "Action.bmx"

Type Editor

	Field selected:Selection
	
	Field ToolManager:ShoeToolManager
	
	Field win:Window
	Field box:ShoeBox
	
	'----------------------------------------------------------------------------
	Method Init:Editor( aWindow:Window )
		
		win = aWindow
		
		' create selection object
		selected = New Selection
		
		' create a shoebox
		box = New ShoeBox.Init( win )
		
		'If aLevel = ""
			box.Open()
		'Else
		'	box.OpenFromXml(aLevel)
		'EndIf
		
		' add tools etc.
		ToolManager = New ShoeToolManager.Init( win, selected, box )
			
		Return Self
		
	EndMethod

	'----------------------------------------------------------------------------
	Method Run()
		
		Local quit:Int = False
		
		While Not quit
		
			Update()
			win.render()
			
			While AppSuspended()
				sleep( 500 )
				win.render()
			Wend
			
			If AppTerminate()
				?debug
				Print Action.IsAtFirstAction()
				?
				quit = box.CheckUnsavedChanges()
			EndIf
			?debug
			If ( KeyDown( KEY_ESCAPE ) Or  AppTerminate() ) quit = True
			?
		Wend
		
	EndMethod

	
	'----------------------------------------------------------------------------
	Method Update()
	
		' update Tools
		ToolManager.Update()
		
		win.CamPos.SetText( "(" + Int(win.Camera.px) + ", " + Int(-win.Camera.py) + ") (" + (win.cameraSpeedx) + ", " + (win.cameraSpeedy) + ") ~n" + ..
		                    "[" + Int((win.Camera.px) + (box.LevelWidth * 0.5)) + ", " + Int(-(win.Camera.py - (box.LevelHeight * 0.5))) + "]" )
	EndMethod
	
EndType
