SuperStrict

Import "ShoeTool.bmx"
Import "ShoeBox.bmx"

Type HelpTool Extends ShoeTool
	
	Field sb:ShoeBox
	
	'----------------------------------------------------------------------------
	Method InitHelp:HelpTool( akey:Int, amodifier:Int, aSelection:Selection, awin:Window, aShoeBox:ShoeBox )
		
		Init( akey, amodifier, "Help", aSelection, awin )
		
		sb = aShoebox
		
		Return Self
		
	EndMethod
	
	'----------------------------------------------------------------------------
	Method UpdateText:Int()
		
		Super.UpdateText()
	
	EndMethod
	
	
	'----------------------------------------------------------------------------
	Method KeyUp:Int( akey:Int, amodifiers:Int )
		
		
		If CheckHotKey( akey, amodifiers )
		
			OpenURL("http://wiki.twotribes.com/Projects/Toki_Tori_2/Technology/Shoebox_Editor")
			
			Return True
		EndIf
		
		Return Super.KeyUp( akey, amodifiers )
		
	EndMethod
	
EndType


