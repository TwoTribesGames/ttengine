SuperStrict

Import "ShoeTool.bmx"
Import "ScaleAction.bmx"

Import MaxGui.Drivers

Type SpecialScaleTool Extends ShoeTool

	Field keeptexscale:Int
	
	Field entity:TEntity	
	Field plane:ShoePlane
	
	Field startPos:TVector
	Field currentPos:TVector
	
	'----------------------------------------------------------------------------
	Method InitScale:SpecialScaleTool ( akey:Int, amodifier:Int, aSelection:Selection, awin:Window )
		
		Init( akey, amodifier, "SpecialScale", aSelection, awin )
		
		Return Self
	EndMethod
	
	'----------------------------------------------------------------------------
	Method UpdateText:Int()
		
		If Super.UpdateText()
			Local p:ShoePlane = selected.GetFirst()
			
			'SetToolText( "width:" + p.GetWidth() + " height:" + p.GetHeight() )
			Print "?"
		EndIf

	EndMethod
	
	'----------------------------------------------------------------------------
	Method MouseMove:Int( x:Int, y:Int, speedx:Float, speedy:Float, scaledSpeedx:Float, scaledSpeedy:Float )

		
		If entity
	
			currentPos.x :+ speedx
			currentPos.y :+ speedy
			
			'Local v:TVector = project2Dto3D(win.Camera, x, y)
			'TPick.Camera2WorldCoords(win.Camera, x, y)
			'Print "world x y: " + v.x + ", " + v.y
			
			TEntity.TFormVector(speedx, speedy, 0, Null, entity)		
			Print "    -> " + TFormedX() + ", " + TFormedY()
			
			TEntity.TFormPoint(x, y, 0, Null, entity)		
			Print "    -] " + TFormedX() + ", " + TFormedY()

			
			plane.SetPosition(currentPos.x, currentPos.y, entity.pz)
			
			
			plane.move(TFormedX(), TFormedY(), 0)
			
			Return True
		EndIf
		
		Return Super.MouseMove( x, y, speedx, speedy, scaledSpeedx:Float, scaledSpeedy:Float )
	EndMethod
	
	'----------------------------------------------------------------------------
	Method MouseUp:Int( but:Int, x:Int, y:Int )
		Super.MouseUp( but, x, y )

		entity = Null
		
		
	EndMethod
	
	'----------------------------------------------------------------------------
	Method MouseDown:Int( but:Int, x:Int, y:Int )
		Super.MouseDown( but, x, y )
		
		If but = MOUSE_LEFT
		
			entity = win.GetObjectFrom( x, y )
			
			Local x:Float = PickedX()
			Local y:Float = PickedY()
			
			'Print "picked: " + x + ", " + y
			
			TEntity.TFormPoint(x, y, entity.pz, Null, entity)
			
			Print "local: " + TFormedX() + ", " + TFormedY()
			
			plane = ShoePlane.GetShoePlaneFromMesh(entity)

			startPos = TVector.Create(x, y, 0)
			currentPos = TVector.Create(0,0,0)
			
			Return entity <> Null And plane <> Null
		EndIf
		
	EndMethod
	
	
	'----------------------------------------------------------------------------
	Method KeyDown:Int( akey:Int, amodifiers:Int )
		
		If amodifiers & MODIFIER_SHIFT
			keeptexscale = True
		EndIf
				
		Return Super.KeyDown( akey, amodifiers )
		
	EndMethod
	
	'----------------------------------------------------------------------------
	Method KeyUp:Int( akey:Int, amodifiers:Int )
		
		keeptexscale = False
			
		Return Super.KeyUp( akey, amodifiers )
	EndMethod
	

	'----------------------------------------------------------------------------
	Method OnActivate()
		Super.OnActivate()
		SetPointer(POINTER_SIZEWE)
	EndMethod
	
EndType

