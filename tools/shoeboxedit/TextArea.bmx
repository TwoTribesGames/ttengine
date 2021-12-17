SuperStrict

Type TextArea

	Field text:String
	Field lineHeight:Int
	
	'----------------------------------------------------------------------------
	Method Init:TextArea( aText:String )
		
		SetText( aText )
		lineHeight = TextHeight( "|" )	' we're not switching fonts, so this won't change
		
		Return Self
		
	EndMethod
	
	'----------------------------------------------------------------------------
	Method SetText( aText:String )
		
		text = aText
		
	EndMethod

	'----------------------------------------------------------------------------
	Method AddText( aText:String )
		
		text :+ aText
		
	EndMethod	
	
	'----------------------------------------------------------------------------
	Method Render( x:Int, y:Int )
		
		For Local t:String = EachIn ( text.Split( "~n" ))
			GLDrawText( t, x, y )
			y:+ lineHeight
		Next
		
	EndMethod
	
EndType
