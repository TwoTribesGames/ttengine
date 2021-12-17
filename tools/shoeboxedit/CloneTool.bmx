SuperStrict

Import sidesign.minib3d
Import "ShoeTool.bmx"
Import "ShoeBox.bmx"

Type CloneTool Extends ShoeTool

	Field sb:ShoeBox

	'----------------------------------------------------------------------------
	Method InitClone:CloneTool( akey:Int, amodifier:Int, aSelection:Selection, awin:Window, aShoeBox:ShoeBox )
		
		sb = aShoebox
		
		Init( akey, amodifier, "Clone", aSelection, awin )
		
		Return Self
	EndMethod
	
	'----------------------------------------------------------------------------
	Method KeyDown:Int( akey:Int, amodifiers:Int )
				
		If akey = key 'And (amodifiers & modifier Or modifier = 0)
			sb.DuplicateSelection( selected, amodifiers & modifier, amodifiers & MODIFIER_SHIFT )
			
			Return True
		EndIf
		
		Return Super.KeyDown( akey, amodifiers )
		
	EndMethod
		


EndType


