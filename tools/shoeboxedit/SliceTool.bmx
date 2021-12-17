SuperStrict

Import "ShoeTool.bmx"

Type SliceTool Extends ShoeTool

	Field Slicekey:Int, Slicemouse:Int
	Field sliceZ:Int 

	
	'----------------------------------------------------------------------------
	Method InitSlice:SliceTool( akey:Int, amodifier:Int, aSelection:Selection, awin:Window )
		
		Init( akey, amodifier, "Slice", aSelection, awin )
		
		sliceZ = 0
		
		Return Self
		
	EndMethod
Rem
	'----------------------------------------------------------------------------
	Method UpdateText:Int()
		
		SetToolText( "Slice from " + (sliceZ - win.SLICESIZE) + " to " + ( sliceZ + win.SLICESIZE ) )
		
	EndMethod
EndRem	
	'----------------------------------------------------------------------------
	Method MouseMove:Int( x:Int, y:Int, speedx:Float, speedy:Float, scaledSpeedx:Float, scaledSpeedy:Float )
		
		If SliceKey And Slicemouse
			sliceZ:- speedy
			win.SetSlice( sliceZ )
		EndIf
		
		Return Super.MouseMove( x, y, speedx, speedy, scaledSpeedx, scaledSpeedy )
		
	EndMethod


	'----------------------------------------------------------------------------
	Method MouseUp:Int( but:Int, x:Int, y:Int )
		
		If but = MOUSE_RIGHT And Slicemouse
			Slicemouse = False
			Return True
		EndIf
		
		Return Super.MouseUp( but, x, y )
		
	EndMethod
	
	'----------------------------------------------------------------------------
	Method MouseDown:Int( but:Int, x:Int, y:Int )
		
		If but = MOUSE_RIGHT And Slicekey
			Slicemouse = True
			Return True
		EndIf
		
		Return Super.MouseDown( but, x, y )
		
	EndMethod
	
	'----------------------------------------------------------------------------
	Method ShowSlice()
		
		If selected.Size() = 1
			sliceZ = -ShoePlane( selected.GetFirst() ).GetZ()
		EndIf
		
		win.ShowSlice( sliceZ )
		
		SliceKey = True
		
	EndMethod

	'----------------------------------------------------------------------------
	Method HideSlice()
		
		win.HideSlice()

		SliceKey = False
		
	EndMethod
	
	'----------------------------------------------------------------------------
	Method KeyDown:Int( akey:Int, amodifiers:Int )
				
		If akey = key And Not SliceKey
			ShowSlice()
			Return False 
		EndIf
		
		Return Super.KeyDown( akey, amodifiers )
		
	EndMethod
	
	'----------------------------------------------------------------------------
	Method KeyUp:Int( akey:Int, amodifiers:Int )
				
		If akey = key And SliceKey
			
			HideSlice()
			Return False
		
		ElseIf akey = KEY_ENTER
			
			If SliceKey
				HideSlice()
			Else
				ShowSlice()
			EndIf
			
			Return False
		EndIf
		
		Return Super.KeyUp( akey, amodifiers )
		
	EndMethod
	
	'----------------------------------------------------------------------------
	Method MouseWheel:Int( speed:Int, x:Int, y:Int )
		
		If SliceKey
			sliceZ:- speed
			win.SetSlice( sliceZ )
			
			win.ToolText.SetText( "Slice at " + sliceZ )
		EndIf
		
		Return Super.MouseWheel( speed, x, y )
	EndMethod
	
EndType


