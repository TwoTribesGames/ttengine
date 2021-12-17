SuperStrict

Import "ShoeTool.bmx"

Type LookTool Extends ShoeTool

	Field scrollkey:Int, scrollmouse:Int
	Field mouseBut:Int = 3
	
	'----------------------------------------------------------------------------
	Method InitLook:LookTool( akey:Int, amodifier:Int, aSelection:Selection, awin:Window )
		
		Init( akey, amodifier, "Look", aSelection, awin )
		
		mouseBut = TConfig.getIntValue("lookMouseButton", 3)
		Return Self
		
	EndMethod
	
	'----------------------------------------------------------------------------
	Method MouseMove:Int( x:Int, y:Int, speedx:Float, speedy:Float, scaledSpeedx:Float, scaledSpeedy:Float )
		
		If scrollkey And scrollmouse
			win.Camera.TurnEntity( speedy, -speedx, 0 )
			Return True
		EndIf
		
		Return Super.MouseMove( x, y, speedx, speedy, scaledSpeedx:Float, scaledSpeedy:Float )
		
	EndMethod
	
	'----------------------------------------------------------------------------
	Method MouseUp:Int( but:Int, x:Int, y:Int )
		
		If but = mouseBut And scrollmouse
			scrollmouse = False
			
			Return True
		EndIf
		
		Return Super.MouseUp( but, x, y )
		
	EndMethod
	
	'----------------------------------------------------------------------------
	Method MouseDown:Int( but:Int, x:Int, y:Int )
		Print(but)
		If but = mouseBut And scrollkey
			scrollmouse = True

			Return True
		EndIf
		
		Return Super.MouseDown( but, x, y )
		
	EndMethod
	
	'----------------------------------------------------------------------------
	Method KeyDown:Int( akey:Int, amodifiers:Int )
				
		If akey = key
			scrollkey = Not scrollkey'True
			
			If (scrollkey)
				
				If amodifiers & MODIFIER_CONTROL
					Local p:shoePlane = selected.getFirst()
					If p Then win.SetCamera(p.GetX(), -p.GetY())
				EndIf
			Else
				scrollkey = False
				win.Camera.RotateEntity(0, 0, 0)
				win.ResetCamera(True, False)
				win.LockCamera()
			EndIf
			
			Return True
		EndIf
		
		Return Super.KeyDown( akey, amodifiers )
		
	EndMethod
	
EndType


