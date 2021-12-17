SuperStrict

Import "ShoeTool.bmx"
Import "TConfig.bmx"

Type ZoomTool Extends ShoeTool

	Field zoomstep:Float
	Field scrollstep:Float
	
	'----------------------------------------------------------------------------
	Method InitZoom:ZoomTool( akey:Int, amodifier:Int, aSelection:Selection, awin:Window )
		
		Init( akey, amodifier, "Zoom", aSelection, awin )
		zoomstep   = TConfig.GetFloatValue("zoomstep", 1.0)
		scrollstep = TConfig.GetFloatValue("zoomscrollstep", 1.0)

		Return Self
		
	EndMethod
	
	
	'----------------------------------------------------------------------------
	Method KeyDown:Int( akey:Int, amodifiers:Int )

		If ( amodifiers & MODIFIER_CONTROL )
				
			If akey = KEY_EQUALS 
				win.ZoomCamera(-zoomstep)
	
				Return True
			ElseIf akey = KEY_MINUS
				win.ZoomCamera(zoomstep)
	
				Return True
			ElseIf akey = KEY_0
				If amodifiers & MODIFIER_SHIFT
					win.ResetCamera(False)
				Else
					win.ResetCamera(True)
				EndIf
			EndIf
		
		EndIf
		
		Return Super.KeyDown( akey, amodifiers )
		
	EndMethod
	
	Method MouseWheel:Int( speed:Int, x:Int, y:Int )
		win.ZoomCamera(speed * -scrollstep)
		
		Return Super.MouseWheel(speed, x, y)
	EndMethod
	Rem
	'----------------------------------------------------------------------------
	Method KeyUp:Int( akey:Int, amodifiers:Int )
				
		If akey = key
			scrollkey = False
			Return True
		EndIf
		
		Return Super.KeyUp( akey, amodifiers )
		
	EndMethod
	EndRem
EndType

