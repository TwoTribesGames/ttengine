SuperStrict

Import "ShoeBox.bmx"
Import "ShoeTool.bmx"
Import "Action.bmx"

Type UndoTool Extends ShoeTool


	'----------------------------------------------------------------------------
	Method InitUndo:UndoTool( akey:Int, amodifier:Int, aSelection:Selection, awin:Window )
		
		Init( akey, amodifier, "Undo", aSelection, awin )
	

		Return Self
		
	EndMethod
	
	
	'----------------------------------------------------------------------------
	Method KeyUp:Int( akey:Int, amodifiers:Int )
				
		If akey = key And amodifiers & modifier
			
			If amodifiers & MODIFIER_SHIFT	
				Action.Redo()
			Else
				Action.Undo()
			EndIf
			
			Return True
		EndIf
		
		Return Super.KeyUp( akey, amodifiers )
		
	EndMethod
	
EndType
