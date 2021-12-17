SuperStrict

Import sidesign.minib3d
Import "ShoeBox.bmx"
Import "ShoeTool.bmx"
Import "TextureLoader.bmx"

Type ReloadTool Extends ShoeTool

	Field sb:ShoeBox
	
	'----------------------------------------------------------------------------
	Method InitReload:ReloadTool( akey:Int, amodifier:Int, aSelection:Selection, awin:Window, aShoeBox:ShoeBox )
		
		Init( akey, amodifier, "Reload", aSelection, awin )
		
		sb = aShoeBox
		
		Return Self
		
	EndMethod
	
	
	'----------------------------------------------------------------------------
	Method KeyDown:Int( akey:Int, amodifiers:Int )
				
		If akey = key 'And amodifiers & modifier


			selected.Clean()
			sb.ReloadShoebox(amodifiers & MODIFIER_CONTROL)
			sb.ReloadLevel()
			
			' catch all input
			While PollEvent()
			Wend
			
			Return True
		EndIf
		
		Return Super.KeyDown( akey, amodifiers )
		
	EndMethod
	
EndType




