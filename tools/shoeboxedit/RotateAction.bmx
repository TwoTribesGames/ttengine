SuperStrict

Import "Action.bmx"

Type RotateAction Extends Action
	
	Field dangle:Int
	
	'----------------------------------------------------------------------------
	Method Do( reverse:Int = False )
		' do the actual action	
		
		For Local p:ShoePlane = EachIn Selected.GetSelection()
			
			If reverse
				p.Rotate( -dangle )
			Else
				p.Rotate(  dangle )
			EndIf

		Next
		
	EndMethod
	
	'----------------------------------------------------------------------------
	' sets the movement vars and performs a "do" ( via Add )
	Method Rotate:RotateAction( aSelection:Selection, angle:Int )
		
		dangle = angle
		
		Add( aSelection )
	
		Return Self
	EndMethod
	
EndType


