SuperStrict

Import "ShoeTool.bmx"
Import "MoveAction.bmx"

Type MoveTool Extends ShoeTool


	Field move:Int, moveZ:Int
	Field win:Window
	
	Field keeptexscale:Int
	
	Field moveStep:Float[3]
	Field priorityStep:Float[3]
	
	'----------------------------------------------------------------------------
	Method InitMove:MoveTool( akey:Int, amodifier:Int, aSelection:Selection, awin:Window )
		
		Init( akey, amodifier, "Move", aSelection, awin )
		
		win = awin
		
		moveStep[0] = TConfig.getFloatValue("movestep_0", 0.01)
		moveStep[1] = TConfig.getFloatValue("movestep_1", 1.0)
		moveStep[2] = TConfig.getFloatValue("movestep_2", 30.0)
		
		priorityStep[0] = TConfig.getFloatValue("prioritystep_0", 1.0)
		priorityStep[1] = TConfig.getFloatValue("prioritystep_1", 10.0)
		priorityStep[2] = TConfig.getFloatValue("prioritystep_2", 30.0)
		
		Return Self
		
	EndMethod

	'----------------------------------------------------------------------------
	Method UpdateText:Int()
		
		If Super.UpdateText()
			Local p:ShoePlane = selected.GetFirst()
			
			SetToolText( "x:" + p.GetX() + " y:" + p.GetY() + " z:" + p.GetZ() )
		EndIf

	EndMethod
		
	'----------------------------------------------------------------------------
	Method MouseMove:Int( x:Int, y:Int, speedx:Float, speedy:Float, scaledSpeedx:Float, scaledSpeedy:Float )
	
		If move Or moveZ
			MoveSelection( scaledSpeedx, -scaledSpeedy, scaledSpeedy, 0, keeptexscale)
			
			Return True
		EndIf

		
		Return Super.MouseMove( x, y, speedx, speedy, scaledSpeedx, scaledSpeedy )
		
	EndMethod
	
	'----------------------------------------------------------------------------
	Method Tweak:Int( adx:Float, ady:Float, adz:Float, adp:Int, astep:Int )
		
		If Not ( move Or moveZ )
			move = True
			moveZ = True
			
			adx :* moveStep[astep]
			ady :* moveStep[astep]
			adz :* moveStep[astep]
			adp :* priorityStep[astep];
			'Print "tweak " + adx + " " + ady + " " + adz + " astep: " + astep
			MoveSelection( adx, ady, adz, adp )
			
			StoreUndoInfo()
			
			move = False
			moveZ = False
			Return True
		EndIf
		
		Return Super.Tweak( adx, ady, adz, adp )
		
	EndMethod
	
	'----------------------------------------------------------------------------
	Method Random:Int(big:Int)
	
		Local range:Float = 1
		If big range = 10
		
		For Local p:ShoePlane = EachIn selected.GetSelection()
			p.Move( Rnd(-range, range), Rnd(-range, range), 0 )
		Next

		StoreUndoInfo()
		Return False
	EndMethod

	'----------------------------------------------------------------------------
	Method MoveSelection( adx:Float, ady:Float, adz:Float, adp:Int, akeeptexscale:Int = False )
		
		If move
			dx:+ adx
			dy:+ ady
		EndIf
		If moveZ
			dz:+ adz
		EndIf
		
		dp:+ adp
		
		For Local p:ShoePlane = EachIn selected.GetSelection()
			
			If move 
				p.Move( adx, ady, 0 )
				If akeeptexscale p.MoveTex(adx / p.GetWidth(), -ady / p.GetHeight())
			EndIf
			If moveZ 
				p.Move( 0, 0, adz )
			EndIf
			p.MovePriority( adp )
			
'			If akeeptexscale p.MoveTex(-adx/p.GetWidth(), -ady/p.GetHeight())

			
		Next
		
	EndMethod
	
	'----------------------------------------------------------------------------
	Method MouseUp:Int( but:Int, x:Int, y:Int )
		
		If ( but = MOUSE_LEFT Or but = MOUSE_RIGHT ) And ( move Or moveZ )
			
			move = False
			moveZ = False
			
			StoreUndoInfo()

			Return True
		EndIf		
		
		
		Return Super.MouseUp( but, x, y )
		
	EndMethod
	
	'----------------------------------------------------------------------------
	Method MouseDown:Int( but:Int, x:Int, y:Int )
				
		If but = MOUSE_LEFT
			move = True
			dx = 0
			dy = 0
		EndIf	

		If but = MOUSE_RIGHT
			moveZ = True
			dz = 0
		EndIf
				
		If move Or moveZ Return True
		
		Return Super.MouseDown( but, x, y )
		
	EndMethod
	
	'Rem
	'----------------------------------------------------------------------------
	Method KeyDown:Int( akey:Int, amodifiers:Int )
		
		'If (amodifiers & MODIFIER_SHIFT) snap = True
		If amodifiers & MODIFIER_SHIFT & False
			keeptexscale = True
		
			Return False
		EndIf
		
		Return Super.KeyDown( akey, amodifiers )
	EndMethod
	

	'----------------------------------------------------------------------------
	Method KeyUp:Int( akey:Int, amodifiers:Int )

		If amodifiers & MODIFIER_SHIFT & False
			keeptexscale = False
		
			Return False
		ElseIf akey = KEY_0
		
			For Local p:ShoePlane = EachIn selected.GetSelection()
				p.setZ(0)
				
				If amodifiers & MODIFIER_CONTROL
					p.setPosition(0,0,0)
				EndIf 
			Next
		
		EndIf
		
		Return Super.KeyUp( akey, amodifiers )
	EndMethod
	'EndRem
	
	Field dx:Int, dy:Int, dz:Int	, dp:Int
	'----------------------------------------------------------------------------
	Method StoreUndoInfo()
		
		New MoveAction.Move( selected, dx, dy, dz, dp )'.Do()
		dx = 0
		dy = 0
		dz = 0
		dp = 0
		
	EndMethod	
	
	
	'----------------------------------------------------------------------------
	Method OnActivate()
		Super.OnActivate()
		SetPointer(POINTER_SIZEALL)
	EndMethod
EndType
