SuperStrict

Import "ShoeTool.bmx"
Import "ShoeBox.bmx"

Type DeleteTool Extends ShoeTool

	Field sb:ShoeBox

	'----------------------------------------------------------------------------
	Method InitDelete:DeleteTool( akey:Int, amodifier:Int, aSelection:Selection, awin:Window, aShoeBox:ShoeBox )
		
		sb = aShoebox
		
		Init( akey, amodifier, "Delete", aSelection, awin )
		
		Return Self
	EndMethod
	
	'----------------------------------------------------------------------------
	Method KeyDown:Int( akey:Int, amodifiers:Int )
				
		If akey = key And amodifiers & modifier
			
			sb.RemoveSelection( selected )
			
			Return True
		EndIf
		
		Return Super.KeyDown( akey, amodifiers )
		
	EndMethod
		
	
	'----------------------------------------------------------------------------
	Method StoreUndoInfo()
			
		
	EndMethod	
	
EndType



