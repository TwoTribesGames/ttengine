SuperStrict

Import "Action.bmx"

Type MoveAction Extends Action
	
	Field dx:Float, dy:Float, dz:float, dp:Int
	
	'----------------------------------------------------------------------------
	Method Do( reverse:Int = False )
		' do the actual action	
		
		For Local p:ShoePlane = EachIn Selected.GetSelection()
			
			If reverse
				p.Move( -dx, -dy, -dz )
				p.MovePriority(-dp)
			Else
				p.Move(  dx,  dy,  dz )
				p.MovePriority(dp)
			EndIf

		Next
		
	EndMethod
	
	'----------------------------------------------------------------------------
	' sets the movement vars and performs a "do" ( via Add )
	Method Move:MoveAction( aSelection:Selection, adx:Float, ady:Float, adz:Float, adp:Int )
		
		dx = adx
		dy = ady
		dz = adz
		dp = adp
		
		Add( aSelection )
	
		Return Self
	EndMethod
	
EndType

