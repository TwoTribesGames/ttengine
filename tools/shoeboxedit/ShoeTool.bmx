SuperStrict

Import sidesign.minib3d
Import "Selection.bmx"
Import "ShoePlane.bmx"
Import "Window.bmx"
Import "TextArea.bmx"

Import MaxGui.Drivers

Type ShoeTool
	
	Field key:Int
	Field modifier:Int
	Field name:String
	Field selected:Selection
	Field win:Window		
	
	'----------------------------------------------------------------------------
	Method Init:ShoeTool( akey:Int, amodifier:Int, aname:String, aSelection:Selection, awin:Window )
	
		key      = akey
		modifier = amodifier
		name		= aname
		selected = aSelection
		win      = awin
		
	EndMethod
	
	
	'----------------------------------------------------------------------------
	Method CheckHotKey:Int( akey:Int, amodifiers:Int )
		
		If ( ( Not key ) Or ( key = akey ) ) And ( ( Not modifier ) Or ( modifier & amodifiers ) )
			Return True
		EndIf
		
		Return False
		
	EndMethod
	

	' input handlers
	'  return true to block further propagation of the handler
	'----------------------------------------------------------------------------
	Method MouseMove:Int( x:Int, y:Int, speedx:Float, speedy:Float, scaledSpeedx:Float, scaledSpeedy:Float )

		?debug
'		Print name + ": mousemove " + speedx + ", "+ speedy
		?
		
		Return False
	EndMethod
	
	'----------------------------------------------------------------------------
	Method MouseUp:Int( but:Int, x:Int, y:Int )
		?debug
'		Print name + ": mouseup"
		?
		'ShowMouse()
		
		Return False
	EndMethod
	
	'----------------------------------------------------------------------------
	Method MouseDown:Int( but:Int, x:Int, y:Int )
		?debug
'		Print name + ": mousedown"	
		?
		'HideMouse()
		
		Return False
	EndMethod
	
	'----------------------------------------------------------------------------
	Method KeyDown:Int( akey:Int, amodifiers:Int )
		?debug
'		Print name + ": keydown"	
		?
		
		Return False
	EndMethod
	
	'----------------------------------------------------------------------------
	Method KeyUp:Int( akey:Int, amodifiers:Int )
		?debug
'		Print name + ": keyup"	
		?
		
		Return False
	EndMethod
	
	'----------------------------------------------------------------------------
	Method MouseWheel:Int( speed:Int, x:Int, y:Int )
		?debug
'		Print name + ": mousewheel"	
		?
		
		Return False
	EndMethod
	
	'----------------------------------------------------------------------------
	Method Tweak:Int( dx:Float, dy:Float, dz:Float, dp:Int, astep:Int = 0)
		
		Return False	
	EndMethod
	
	'----------------------------------------------------------------------------
	Method Random:Int(big:Int)
	
		Return False
	EndMethod
	
	'----------------------------------------------------------------------------
	Method Update()
		
		UpdateText()
		
	EndMethod

	'----------------------------------------------------------------------------
	Method StoreUndoInfo()
		
	EndMethod	
	
	'----------------------------------------------------------------------------
	' if true is returned, no tooltip has been set, false means default tooltip
	Method UpdateText:Int()
		
		If selected.Size() = 1
			Local p:ShoePlane = selected.GetFirst()
			
			SetToolText( p.GetToolText() )
			'Return True
			Return False
			
		ElseIf selected.Size() = 0
			SetToolText( "Nothing selected" )
			Return False
			
		Else
			SetToolText( "Multiple planes selected" )
			Return False
			
		EndIf

	EndMethod
	
	'----------------------------------------------------------------------------
	Method OnActivate()
		SetToolText( "" )
	EndMethod
	
	'----------------------------------------------------------------------------
	Method OnDeactivate()
		SetToolText( "" )
		SetPointer(POINTER_DEFAULT)
	EndMethod
	
	'----------------------------------------------------------------------------
	Method SetToolText( aText:String )

		win.ToolText.SetText( name + " [" + selected.Size() + "]~n" + aText )
		
	EndMethod
	
EndType
