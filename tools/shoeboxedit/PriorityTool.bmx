SuperStrict

Import "ShoeTool.bmx"
Import "MoveAction.bmx"

Type MoveTool Extends ShoeTool


	Field move:Int, moveZ:Int
	Field win:Window
	
	'----------------------------------------------------------------------------
	Method InitMove:MoveTool( akey:Int, amodifier:Int, aSelection:Selection, awin:Window )
		
		Init( akey, amodifier, "Move", aSelection, awin )
		
		win = awin
		
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
	Method MouseMove:Int( x:Int, y:Int, speedx:Int, speedy:Int )
	
		If move Or moveZ
			MoveSelection( speedx, -speedy, speedy )
			
			Return True
		EndIf

		
		Return Super.MouseMove( x, y, speedx, speedy )
		
	EndMethod
	
	'----------------------------------------------------------------------------
	Method Tweak:Int( adx:Float, ady:Float, adz:float)
		'Print "tweak " + adx + " " + ady + " " + adz
		If Not ( move Or moveZ )
			move = True
			moveZ = True
			
			MoveSelection( adx, ady, adz )
			
			StoreUndoInfo()
			
			move = False
			moveZ = False
			Return True
		EndIf
		
		Return Super.Tweak( adx, ady, adz )
		
	EndMethod
	
	'----------------------------------------------------------------------------
	Method MoveSelection( adx:Int, ady:Int, adz:Int, snap:Int = 0 )
		
		If move
			dx:+ adx
			dy:+ ady
		EndIf
		If moveZ
			dz:+ adz
		EndIf
		
		For Local p:ShoePlane = EachIn selected.GetSelection()
			
			If move 
				p.Move( adx, ady, 0 )
			EndIf
			If moveZ 
				p.Move( 0, 0, adz )
			EndIf
			
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
	
	Rem
	'----------------------------------------------------------------------------
	Method KeyDown:Int( akey:Int, amodifiers:Int )
		Super.KeyDown( akey, amodifiers )
		
		If (amodifiers & MODIFIER_SHIFT) snap = True
		
		Return True
	EndMethod
	

	'----------------------------------------------------------------------------
	Method KeyUp:Int( akey:Int, amodifiers:Int )
		Super.KeyUp( akey, amodifiers )
		
		If (amodifiers & MODIFIER_SHIFT) snap = False
		
		Return True
	EndMethod
	EndRem
	
	Field dx:Int, dy:Int, dz:Int	
	'----------------------------------------------------------------------------
	Method StoreUndoInfo()
		
		New MoveAction.Move( selected, dx, dy, dz )'.Do()
		dx = 0
		dy = 0
		dz = 0
		
	EndMethod	
EndType
