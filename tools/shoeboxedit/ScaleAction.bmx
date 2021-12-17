SuperStrict

Import "Action.bmx"

Type ScaleAction Extends Action
	
	Field dwidth:Float, dheight:Float, dtheight:Float, dtwidth:Float, du:Float, dv:Float
	
	'----------------------------------------------------------------------------
	Method Do( reverse:Int = False )
		' do the actual action	
		Rem
		'For Local p:ShoePlane = EachIn Selected.GetSelection()
		Local p:ShoePlane = selected.GetFirst()
			
			If reverse
'Print "  reverse scaled by: " + dwidth + " and " + dheight
'Print "  reverse scaled by: " + dtheight + " and " + dtwidth 
			
				p.Scale(    1/dwidth  , 1/dheight )
				p.ScaleTex( 1/dtheight, 1/dtwidth)
				p.MoveTex(-du, -dv)
			Else
'Print "          scaled by: " + dwidth + " and " + dheight
'Print "          scaled by: " + dtheight + " and " + dtwidth 
				p.Scale(     dwidth  , dheight )
				p.ScaleTex(  dtheight, dtwidth )
				p.MoveTex(du, dv)
			EndIf

		'Next
		EndRem
	EndMethod
	
	'----------------------------------------------------------------------------
	' sets the movement vars and performs a "do" ( via Add )
	Method Scale:ScaleAction( aSelection:Selection, width:Float, height:Float, tileu:Float, tilev:Float, u:Float, v:Float )
		
		Rem
		dwidth  = width
		dheight = height
		dtwidth  = tileu
		dtheight = tilev
		du = u
		dv = v
		Add( aSelection )
	
		Return Self
		EndRem
	EndMethod
	
EndType



