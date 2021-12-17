SuperStrict

Import "ShoeTool.bmx"
Import "ShoeBox.bmx"

Type HideTool Extends ShoeTool
	
	Field sb:ShoeBox
	
	'----------------------------------------------------------------------------
	Method InitHide:HideTool( akey:Int, amodifier:Int, aSelection:Selection, awin:Window, aShoeBox:ShoeBox )
		
		Init( akey, amodifier, "Hide", aSelection, awin )
		
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
		
			If ( amodifiers & MODIFIER_ALT )
				Local show:Int = ( amodifiers & MODIFIER_SHIFT )

				For Local ip:IncludePlane = EachIn sb.GetAllShoePlanes()
					If show ip.OnShow() Else ip.OnHide()
				Next

			Else If ( amodifiers & MODIFIER_SHIFT )
				For Local p:ShoePlane = EachIn sb.GetAllShoePlanes()
					p.OnShow()
				Next
			
			Else
				For Local p:ShoePlane = EachIn selected.GetSelection()
					If p.isVisible p.OnHide() Else p.OnShow()
				Next
			EndIf
			
			Return True
		EndIf
		
		Return Super.KeyUp( akey, amodifiers )
		
	EndMethod
	
EndType


