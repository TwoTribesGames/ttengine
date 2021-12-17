SuperStrict

Import "ShoeTool.bmx"
Import "ShoeBox.bmx"
Import MaxGui.Drivers

Type FogTool Extends ShoeTool

	Field near:Float = 1500, far:Float = 3000
	Field activeNear:Int = False, activeFar:Int = False

	Field sb:ShoeBox
	'----------------------------------------------------------------------------
	Method InitFog:FogTool( akey:Int, amodifier:Int, aSelection:Selection, awin:Window, aShoeBox:ShoeBox )
		
		Init( akey, amodifier, "Fog", aSelection, awin )
		
		sb = aShoeBox
		
		Return Self
		
	EndMethod

	'----------------------------------------------------------------------------
	Method MouseMove:Int( x:Int, y:Int, speedx:Float, speedy:Float, scaledSpeedx:Float, scaledSpeedy:Float )
		
		If activeNear near = near + speedy
		If activeFar  far  = far  + speedy
		
		If activeNear Or activeFar
			'near = Min(near,  far - 1)
			'far  = Max(far,  near + 1)
			
			?debug 
				Print near + ", " + far
			?
			SetToolText( "near: " + near + " far: " + far )

			sb.SetFogRange(near, far)
		EndIf
		
		Return Super.MouseMove( x, y, speedx, speedy, scaledSpeedx:Float, scaledSpeedy:Float )
		
	EndMethod


	Method SetToolText( aText:String )
		aText :+ "near: " + (25 - near) + " far: " + (25 - far)
		Super.setToolText( aText )
			
	EndMethod
	
	'----------------------------------------------------------------------------
	Method MouseUp:Int( but:Int, x:Int, y:Int )
		Rem
		If but = MOUSE_RIGHT And Slicemouse
			Slicemouse = False
			Return True
		EndIf
		EndRem
		activeNear = False
		activeFar  = False
		Return Super.MouseUp( but, x, y )
		
	EndMethod
	
	'----------------------------------------------------------------------------
	Method MouseDown:Int( but:Int, x:Int, y:Int )
		Rem
		If but = MOUSE_RIGHT And Slicekey
			Slicemouse = True
			Return True
		EndIf
		EndRem
		activeNear = (but = 1)
		activeFar  = (but = 2)
		Return Super.MouseDown( but, x, y )
		
	EndMethod

	'----------------------------------------------------------------------------
	Method KeyDown:Int( akey:Int, amodifiers:Int )
				
		If akey = key

			Return False 
		EndIf
		
		Return Super.KeyDown( akey, amodifiers )
		
	EndMethod
	
	'----------------------------------------------------------------------------
	Method KeyUp:Int( akey:Int, amodifiers:Int )
				
		If akey = key
			Return False
		ElseIf akey = KEY_K And selected.IsEmpty()
				
			If RequestColor(sb.GetFogR(), sb.GetFogG(), sb.GetFogB())				
				Local r:Int = RequestedRed()
				Local g:Int = RequestedGreen()
				Local b:Int = RequestedBlue()

				sb.SetFogColor(r, g, b)
			EndIf
			Return True
		ElseIf akey = KEY_I
		
			For Local p:ShoePlane = EachIn selected.GetSelection()
				p.SetIgnoreFog(True)
			Next
			Return True
		ElseIf akey = KEY_O
		
			For Local p:ShoePlane = EachIn selected.GetSelection()
				p.SetIgnoreFog(False)
			Next
			Return True
		EndIf
		
		Return Super.KeyUp( akey, amodifiers )
		
	EndMethod
	
	Method OnActivate()
		sb.ShowFog()
		'sb.SetFogRange(near, far)
		near = sb.GetFogNear()
		far  = sb.GetFogFar()
	EndMethod
	
	Method OnDeactivate()
		sb.HideFog()
	EndMethod
	
	Rem
	'----------------------------------------------------------------------------
	Method MouseWheel:Int( speed:Int, x:Int, y:Int )
		
		If SliceKey
			sliceZ:- speed
			win.SetSlice( sliceZ )
			
			win.ToolText.SetText( "Slice at " + sliceZ )
		EndIf
		
		Return Super.MouseWheel( speed, x, y )
	EndMethod
	EndRem
EndType


