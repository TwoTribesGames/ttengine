SuperStrict

Import "ShoeTool.bmx"
Import "ScaleAction.bmx"

Import MaxGui.Drivers

Type ScaleTool Extends ShoeTool

	Field scale:Int, move:Int, keeptexscale:Int
	Field swidth:Int = True, sheight:Int = True
		
	'----------------------------------------------------------------------------
	Method InitScale:ScaleTool( akey:Int, amodifier:Int, aSelection:Selection, awin:Window )
		
		Init( akey, amodifier, "Scale", aSelection, awin )
		
		Return Self
	EndMethod
	
	'----------------------------------------------------------------------------
	Method UpdateText:Int()
		
		If Super.UpdateText()
			Local p:ShoePlane = selected.GetFirst()
			
			SetToolText( "width:" + p.GetWidth() + " height:" + p.GetHeight() )
		EndIf

	EndMethod
	
	'----------------------------------------------------------------------------
	Method MouseMove:Int( x:Int, y:Int, speedx:Float, speedy:Float, scaledSpeedx:Float, scaledSpeedy:Float )

		scaledSpeedy = scaledSpeedx
		
		Local scaleX:Float
		Local scaleY:Float
				
		Local ret:Int = False
		For Local p:ShoePlane = EachIn selected.GetSelection()
		
			Local width:Float  = p.previousState.GetWidth()
			Local height:Float = p.previousState.GetHeight()

			If scale
				scaleX = (scaledSpeedx / 10.0) + 1.0
				scaleY = (scaledSpeedy / 10.0) + 1.0
				If ( swidth And sheight )
					p.ScaleSize(scaleX, scaleY)
					If keeptexscale p.ScaleTex(scaledSpeedx, scaledSpeedy)
				Else If swidth   
					p.ScaleSize(scaleX, 1.0)
					If keeptexscale p.ScaleTex(scaledSpeedx, 0)
				Else
					p.ScaleSize(1.0, scaleY)
					If keeptexscale p.ScaleTex(0, scaledSpeedy)
				EndIf
				
				ret = True
			
			ElseIf move
				scaleX = scaledSpeedx
				scaleY = scaledSpeedy
				If ( swidth And sheight )
					If keeptexscale p.MoveTex( -(scaleX / width), -(scaleY / height))
				Else If swidth   
					If keeptexscale p.MoveTex( -(scaleX / width), 0)
				Else
					If keeptexscale p.MoveTex(0, -(scaleY / height))
				EndIf		
				
				ret = True
			EndIf
		Next	
		'EndIf
		
		If ret Return True Else Return Super.MouseMove( x, y, speedx, speedy, scaledSpeedx, scaledSpeedy )

	EndMethod
	
	'----------------------------------------------------------------------------
	Method MouseUp:Int( but:Int, x:Int, y:Int )
		Super.MouseUp( but, x, y )

		scale = False
		move  = False
				
		'If but = MOUSE_LEFT
			
			Local p:ShoePlane = selected.GetFirst()
			If p
			
				Local dwidth:Float  = Float(p.GetWidth())  / Float(p.previousState.GetWidth())
				Local dheight:Float = Float(p.GetHeight()) / Float(p.previousState.GetHeight())
				Print "  scaled by " + dwidth + ", " + dheight
				Local dtwidth:Float  = p.tileu / p.previousState.tileu
				Local dtheight:Float = p.tilev / p.previousState.tilev
				Local du:Float = p.posu - p.previousState.posu
				Local dv:Float = p.posv - p.previousState.posv
			Rem	
				Local dwidth:Float  = p.previousState.GetWidth()
				Local dheight:Float = p.previousState.GetHeight()
				Local dtwidth:Float  = p.previousState.tileu
				Local dtheight:Float = p.previousState.tilev
			EndRem				
				New ScaleAction.Scale( selected, dWidth, dHeight, dtwidth, dtheight, du, dv )
			EndIf		
			
			
			Return True
		'EndIf
		
	EndMethod
	
	'----------------------------------------------------------------------------
	Method MouseDown:Int( but:Int, x:Int, y:Int )
		Super.MouseDown( but, x, y )
		
		If but = MOUSE_LEFT
			scale = True
			Return True
			
		ElseIf but = MOUSE_RIGHT
			move = True
			Return True
		EndIf
		
	EndMethod
	
	
	'----------------------------------------------------------------------------
	Method KeyDown:Int( akey:Int, amodifiers:Int )
		
		'If Not scale
			If amodifiers & MODIFIER_OPTION
				swidth = False
			EndIf
			If amodifiers & MODIFIER_CONTROL
				sheight = False
			EndIf
			If amodifiers & MODIFIER_SHIFT
				keeptexscale = True
			EndIf
		'EndIf
				
		Return Super.KeyDown( akey, amodifiers )
		
	EndMethod
	
	'----------------------------------------------------------------------------
	Method KeyUp:Int( akey:Int, amodifiers:Int )
		
		swidth = True
		sheight = True
		keeptexscale = False
			
		
		Return Super.KeyUp( akey, amodifiers )
	EndMethod
	
	'----------------------------------------------------------------------------
	Method Random:Int(big:Int)
		
		Local range:Float = 0.1
		If big range = 0.5
		
		For Local p:ShoePlane = EachIn selected.GetSelection()
			Local _scale:Float = Float(Rnd(1 - range, 1 + range))
			p.Scale(_scale, _scale)
		Next

		StoreUndoInfo()
		Return False
	EndMethod
	
	'----------------------------------------------------------------------------
	Method OnActivate()
		Super.OnActivate()
		SetPointer(POINTER_SIZEWE)
	EndMethod
	
EndType

