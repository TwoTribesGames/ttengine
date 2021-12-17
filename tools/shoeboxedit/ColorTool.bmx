SuperStrict

Import "ShoeBox.bmx"
Import "ShoeTool.bmx"
Import MaxGui.Drivers



Type ColorTool Extends ShoeTool

	Field sb:ShoeBox
	
	'----------------------------------------------------------------------------
	Method InitColor:ColorTool ( akey:Int, amodifier:Int, aSelection:Selection, awin:Window, aShoeBox:ShoeBox )
		
		Init( akey, amodifier, "Color", aSelection, awin )
		
		sb = aShoeBox
		
		Return Self
		
	EndMethod
	
	
	'----------------------------------------------------------------------------
	Method KeyDown:Int( akey:Int, amodifiers:Int )
				
		If akey = key And ((amodifiers & modifier) | (modifier = 0)) And Not selected.IsEmpty()
		
			Local r:Int = 255, g:Int = 255, b:Int = 255, a:Int = 255
			If selected.GetFirst() <> Null
				Local p:ShoePlane = selected.GetFirst()
				r = p.r
				g = p.g
				b = p.b
				a = p.a
			EndIf
			
			If RequestColor(r, g, b)
				r = RequestedRed()
				g = RequestedGreen()
				b = RequestedBlue()
			EndIf
			' FIXME: Alpha value should be part of RequestColor
			If RequestColor(a, a, a)
				a = RequestedRed()
			EndIf
			
			For Local p:ShoePlane = EachIn selected.GetSelection()
				p.SetColor(r, g, b, a)
			Next
			
			Return True
		EndIf
		
		Return Super.KeyDown( akey, amodifiers )
		
	EndMethod
	
EndType



