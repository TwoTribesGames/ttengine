SuperStrict

Import "ShoeTool.bmx"
Import "ShoeBox.bmx"

Type LevelTool Extends ShoeTool

	Field sb:ShoeBox
	Field showlevelToggle:Int
	Field showSkinToggle:Int
	
	'----------------------------------------------------------------------------
	Method InitLevel:LevelTool( akey:Int, amodifier:Int, aSelection:Selection, awin:Window, aShoeBox:ShoeBox )
		
		sb = aShoebox
		
		Init( akey, amodifier, "LevelTool", aSelection, awin )
		
		Return Self
	EndMethod
	
	'----------------------------------------------------------------------------
	Method KeyUp:Int( akey:Int, amodifiers:Int )
				
		If ( akey = key )
		
			If (amodifiers & MODIFIER_SHIFT)
				
				If showSkinToggle
					sb.ShowLevelSkin()
				Else
					sb.HideLevelSkin()
				EndIf
				showSkinToggle = Not showSkinToggle

				Return True
			Else
						
				If showlevelToggle
					sb.ShowLevelIndicator()
				Else
					sb.HideLevelIndicator()
				EndIf
				showlevelToggle = Not showlevelToggle
				
				Return True
			EndIf
		EndIf
		
		Return Super.KeyUp( akey, amodifiers )
		
	EndMethod
		
	
	'----------------------------------------------------------------------------
	Method StoreUndoInfo()
			
		
	EndMethod	
	
EndType




