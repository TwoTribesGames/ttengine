SuperStrict

Import "ShoeTool.bmx"

Type ScrollTool Extends ShoeTool

	Field scrollkey:Int, scrollmouse:Int
	
	'----------------------------------------------------------------------------
	Method InitScroll:ScrollTool( akey:Int, amodifier:Int, aSelection:Selection, awin:Window )
		
		Init( akey, amodifier, "Scroll", aSelection, awin )
		
		
		Return Self
		
	EndMethod
	
	'----------------------------------------------------------------------------
	Method MouseMove:Int( x:Int, y:Int, speedx:Float, speedy:Float, scaledSpeedx:Float, scaledSpeedy:Float )

		If scrollkey And scrollmouse
			win.MoveCamera( -scaledSpeedx, scaledSpeedy )
			Return True
		EndIf
		
		Return Super.MouseMove( x, y, speedx, speedy, scaledSpeedx:Float, scaledSpeedy:Float )
		
	EndMethod
	
	'----------------------------------------------------------------------------
	Method MouseUp:Int( but:Int, x:Int, y:Int )
		
		If but = MOUSE_LEFT And scrollmouse
			scrollmouse = False
			win.FreeCamera()
			Return True
		EndIf
		
		Return Super.MouseUp( but, x, y )
		
	EndMethod
	
	'----------------------------------------------------------------------------
	Method MouseDown:Int( but:Int, x:Int, y:Int )
		
		If but = MOUSE_LEFT And scrollkey
			scrollmouse = True
			win.LockCamera()
			Return True
		EndIf
		
		Return Super.MouseDown( but, x, y )
		
	EndMethod
	
	'----------------------------------------------------------------------------
	Method KeyDown:Int( akey:Int, amodifiers:Int )
				
		If akey = key
			scrollkey = True
			
			If amodifiers & MODIFIER_CONTROL
				Local p:shoePlane = selected.getFirst()
				If p Then win.SetCamera(p.GetX(), -p.GetY())
			EndIf
			
			Return True
		EndIf
		
		Return Super.KeyDown( akey, amodifiers )
		
	EndMethod
	
	'----------------------------------------------------------------------------
	Method KeyUp:Int( akey:Int, amodifiers:Int )
				
		If akey = key
			scrollkey = False
			win.LockCamera()			
			Return True
		EndIf
		
		Return Super.KeyUp( akey, amodifiers )
		
	EndMethod
	
EndType

