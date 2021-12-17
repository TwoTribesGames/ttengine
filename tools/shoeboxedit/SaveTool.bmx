SuperStrict

Import "ShoeBox.bmx"
Import "ShoeTool.bmx"

Type SaveTool Extends ShoeTool

	Field sb:ShoeBox
	
	'----------------------------------------------------------------------------
	Method InitSave:SaveTool( akey:Int, amodifier:Int, aSelection:Selection, awin:Window, aShoeBox:ShoeBox )
		
		Init( akey, amodifier, "Save", aSelection, awin )
		
		sb = aShoeBox
		
		Return Self
		
	EndMethod
	
	
	'----------------------------------------------------------------------------
	Method KeyDown:Int( akey:Int, amodifiers:Int )
				
		If akey = key And amodifiers & modifier
			If amodifiers & MODIFIER_SHIFT
				sb.SaveAS()
			Else
				sb.Save()
			EndIf
			
			Return True
		EndIf
		
		Return Super.KeyDown( akey, amodifiers )
		
	EndMethod
	
EndType


