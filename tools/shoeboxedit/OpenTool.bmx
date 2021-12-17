SuperStrict

Import "ShoeBox.bmx"
Import "ShoeTool.bmx"

Type OpenTool Extends ShoeTool

	Field sb:ShoeBox
	
	'----------------------------------------------------------------------------
	Method InitOpen:OpenTool( akey:Int, amodifier:Int, aSelection:Selection, awin:Window, aShoeBox:ShoeBox )
		
		Init( akey, amodifier, "Open", aSelection, awin )
		
		sb = aShoeBox
		
		Return Self
		
	EndMethod
	
	
	'----------------------------------------------------------------------------
	Method KeyDown:Int( akey:Int, amodifiers:Int )
				
		If akey = key And amodifiers & modifier
			' clear selection
			selected.Clean()
			sb.Open()
			win.ResetCamera(True)
			' catch all input
			While PollEvent()
			Wend
			' FIXME: Hack to release the modifier
			EmitEvent CreateEvent(EVENT_KEYUP, Null, modifier)
			
			Return True
		EndIf
		
		Return Super.KeyDown( akey, amodifiers )
		
	EndMethod
	
EndType



