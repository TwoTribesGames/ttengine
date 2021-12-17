SuperStrict

Import "ShoeTool.bmx"
Import "RotateAction.bmx"

Type RotateTool Extends ShoeTool

	Field rotate:Int
	
	Field rotateStep:Float[3]	
		
	'----------------------------------------------------------------------------
	Method InitRotate:RotateTool( akey:Int, amodifier:Int, aSelection:Selection, awin:Window )
		
		Init( akey, amodifier, "Rotate", aSelection, awin )
		
		rotateStep[0] = TConfig.getFloatValue("rotatestep_0", 0.01)
		rotateStep[1] = TConfig.getFloatValue("rotatestep_1", 1.0)
		rotateStep[2] = TConfig.getFloatValue("rotatestep_2", 30.0)
		
		Return Self
	EndMethod
	
	'----------------------------------------------------------------------------
	Method UpdateText:Int()
		
		If Super.UpdateText()
			Local p:ShoePlane = selected.GetFirst()
			
			SetToolText( "angle:" + p.GetAngle() )
		EndIf

	EndMethod
		
	'----------------------------------------------------------------------------
	Method MouseMove:Int( x:Int, y:Int, speedx:Float, speedy:Float, scaledSpeedx:Float, scaledSpeedy:Float )
		
		If rotate		
			RotateSelection( speedx )
			
			Return True
		EndIf
			
		Return Super.MouseMove( x, y, speedx, speedy, scaledSpeedx, scaledSpeedy )

	EndMethod
	
	'----------------------------------------------------------------------------
	Method Tweak:Int( adx:Float, ady:Float, adz:Float, adp:Int, astep:Int )
		
		If Not ( rotate )
			rotate = True
			
			adx :* rotateStep[astep]
			RotateSelection( -adx )
			
			FlipSelection(adp < 0, adp > 0)
			
			StoreUndoInfo()
						
			rotate = False
			Return True
		EndIf
		
		Return Super.Tweak( adx, ady, adz, adp )
		
	EndMethod

	'----------------------------------------------------------------------------
	Method Random:Int(big:Int)

		Local range:Int = 5
		If big range = 90
		
		For Local p:ShoePlane = EachIn selected.GetSelection()
			p.Rotate(Rand(-range, range))
		Next

		StoreUndoInfo()
		Return False
	EndMethod

	'----------------------------------------------------------------------------
	Method RotateSelection( aAngle:Float )
		
		For Local p:ShoePlane = EachIn selected.GetSelection()
			p.Rotate( aAngle )
		Next
	
	EndMethod

	'----------------------------------------------------------------------------
	Method FlipSelection( flipu:Int, flipv:Int )
		
		For Local p:ShoePlane = EachIn selected.GetSelection()
			p.FlipTex( flipu, flipv )
		Next
	
	EndMethod
	
	'----------------------------------------------------------------------------
	Method FlipXSelection(amodifiers:Int)
		If (amodifiers & MODIFIER_ALT)
			' Flip x values (used for flipping a whole shoebox)
			For Local p:ShoePlane = EachIn selected.GetSelection()
				p.FlipTex( True, False )
				p.SetRotation(-p.GetAngle())
				p.SetPosition(-p.GetX(), p.GetY(), p.GetZ())
			Next
		Else
			' X flip around average center 
			Local centerX:Float = 0.0
			Local totalPlanes:Int = 0
			For Local p:ShoePlane = EachIn selected.GetSelection()
				centerX :+ p.GetX()
				totalPlanes :+ 1
			Next
			If (totalPlanes > 0)
				centerX :/ totalPlanes
				centerX :* 2.0
				' Reposition planes around this center
				For Local p:ShoePlane = EachIn selected.GetSelection()
					p.FlipTex( True, False )
					p.SetRotation(-p.GetAngle())
					p.SetPosition(centerX - p.GetX(), p.GetY(), p.GetZ())
				Next
			End If
		EndIf 
	EndMethod	
	
	
	'----------------------------------------------------------------------------
	Method MouseUp:Int( but:Int, x:Int, y:Int )
		Super.MouseUp( but, x, y )
		
		If but = MOUSE_LEFT
			
			rotate = False
			
			StoreUndoInfo()
			
			Return True
		EndIf
		
	EndMethod
	
	'----------------------------------------------------------------------------
	Method MouseDown:Int( but:Int, x:Int, y:Int )
		Super.MouseDown( but, x, y )
		
		If but = MOUSE_LEFT
			rotate = True
			Return True
		EndIf
		
	EndMethod
	'----------------------------------------------------------------------------
	Method KeyDown:Int( akey:Int, amodifiers:Int )
		If akey = KEY_F And amodifiers & MODIFIER_SHIFT
			FlipXSelection(amodifiers )
		Else
			Return Super.KeyDown( akey, amodifiers )
		EndIf 	
	EndMethod
	
	'----------------------------------------------------------------------------
	Method KeyUp:Int( akey:Int, amodifiers:Int )

		If akey = KEY_0
			For Local p:ShoePlane = EachIn selected.GetSelection()
				p.SetRotation(0)
			Next
		EndIf	
		Super.KeyUp( akey, amodifiers )
	EndMethod
	
	'----------------------------------------------------------------------------
	Method StoreUndoInfo()
			
		
		Local p:ShoePlane = selected.GetFirst()
		If p
			Local dAngle:Float  = Float(p.GetAngle())  - Float(p.previousState.GetAngle())
						
			New RotateAction.Rotate( selected, dAngle )
		EndIf		
		
	EndMethod	
	
	
	'----------------------------------------------------------------------------
	Method OnActivate()
		Super.OnActivate()
		SetPointer(POINTER_SIZEWE)
	EndMethod
EndType

