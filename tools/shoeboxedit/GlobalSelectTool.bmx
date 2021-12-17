SuperStrict

Import "ShoeTool.bmx"
Import "ShoeBox.bmx"

Type GlobalSelectTool Extends ShoeTool
	
	Field sb:ShoeBox
	
	'----------------------------------------------------------------------------
	Method InitGlobalSelect:GlobalSelectTool( akey:Int, amodifier:Int, aSelection:Selection, awin:Window, aShoeBox:ShoeBox )
		
		sb = aShoebox
		
		Init( akey, amodifier, "Select", aSelection, awin )
		
		Return Self
		
	EndMethod
	
Rem	
	'----------------------------------------------------------------------------
	Method KeyDown:Int( akey:Int, amodifiers:Int )
		
		Return Super.KeyDown( akey, amodifiers )
		
	EndMethod
EndRem	
	'----------------------------------------------------------------------------
	Method KeyDown:Int( akey:Int, amodifiers:Int )
		
		
		If ( amodifiers & MODIFIER_CONTROL )
			
			If ( akey = KEY_D )
				
				selected.Clear()
				Return True

			ElseIf ( akey = KEY_A And amodifiers & MODIFIER_SHIFT )
				
				For Local sp:ShoePlane = EachIn sb.GetAllShoePlanes()
					If sp.GetZ() > 0 Then selected.Add( sp )
				Next
				
				Return True
			
			ElseIf ( akey = KEY_A And amodifiers & MODIFIER_ALT )
				
				For Local sp:ShoePlane = EachIn sb.GetAllShoePlanes()
					If sp.GetZ() < 0 Then selected.Add( sp )
				Next
				
				Return True
									
			ElseIf ( akey = KEY_A )
				
				For Local sp:ShoePlane = EachIn sb.GetAllShoePlanes()
					selected.Add( sp )
				Next
				
				Return True
			
			ElseIf ( akey = KEY_Y )
				
				For Local sp:ShoePlane = EachIn sb.GetAllShoePlanes()
					Local minc:Float = win.Camera.pz - win.Camera.range_near
					Local maxc:Float = win.Camera.pz - win.Camera.range_far

					If sp.mesh And sp.mesh.pz < minc And sp.mesh.pz > maxc
						selected.Add( sp )
					EndIf
				Next
				
				Return True


			ElseIf ( akey = KEY_I )
				
				Local Current:Selection = selected.Copy()
				
				For Local sp:ShoePlane = EachIn sb.GetAllShoePlanes()
					selected.Add( sp )
				Next
				
				For Local sp:ShoePlane = EachIn Current.GetSelection()
					selected.Remove( sp )
				Next
				
				Return True			
				
			ElseIf ( aKey = KEY_M )
			
				If Not selected.IsEmpty()
				
					Local sp:ShoePlane = selected.GetFirst()
					Local id:String = sp.GetID()
					
					Local r:Float = 1
					Local g:Float = 1
					Local b:Float = 1
					
					If (sp.mesh <> Null)
						r = sp.mesh.brush.red
						g = sp.mesh.brush.green
						b = sp.mesh.brush.blue
					EndIf
					
					If Not (amodifiers & MODIFIER_SHIFT) selected.Clear()
					
					Local matchColor:Int = (amodifiers & MODIFIER_ALT)
					
					For Local sp:ShoePlane = EachIn sb.GetAllShoePlanes()
						
						' alwats match on texture name
						Local match:Int = sp.GetID() = id
						
						If (matchColor And sp.mesh <> Null)
							match = match And r = sp.mesh.brush.red And g = sp.mesh.brush.green And b = sp.mesh.brush.blue
						EndIf
						
						If match selected.Add( sp )
						
					Next
				EndIf
				
			EndIf
		EndIf
		
		Return Super.KeyUp( akey, amodifiers )
		
	EndMethod
	
EndType


