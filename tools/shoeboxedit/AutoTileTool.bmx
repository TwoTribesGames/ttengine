SuperStrict

Import "ShoeBox.bmx"
Import "ShoeTool.bmx"

Type AutoTileTool Extends ShoeTool

	Field sb:ShoeBox
	
	'----------------------------------------------------------------------------
	Method InitAutoTile:AutoTileTool( akey:Int, amodifier:Int, aSelection:Selection, awin:Window, aShoeBox:ShoeBox )
		
		Init( akey, amodifier, "Texture", aSelection, awin )
		
		sb = aShoeBox
		
		Return Self
		
	EndMethod
	
	
	'----------------------------------------------------------------------------
	Method KeyDown:Int( akey:Int, amodifiers:Int )
				
		If akey = key 'And amodifiers & modifier
			
			If Not selected.IsEmpty()

				Local first:ShoePlane = selected.GetFirst()
				Local scaleu:Float = first.tileu / first.GetWidth()
				Local scalev:Float = first.tilev / first.GetHeight()

				For Local p:ShoePlane = EachIn selected.GetSelection()

					Local u:Float = first.posu + ((p.getMinX() - first.GetMinX()) * scaleu)
					Local v:Float = first.posv + ((p.getMinY() - first.GetMinY()) * scalev)
					
					Local tu:Float = p.GetWidth()  * scaleu
					Local tv:Float = p.getHeight() * scalev
					
					p.SetTexPos(u, v)
					p.SetTexSize(tu, tv)
				Next
			EndIf
			
			Return True
		EndIf
		
		Return Super.KeyDown( akey, amodifiers )
		
	EndMethod
	
EndType



