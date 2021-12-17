SuperStrict

Import "ShoeBox.bmx"
Import "ShoeTool.bmx"

Import PUB.FreeProcess

Rem
Global proc:TProcess



Function RunApp(myapp$)
    ' check if application is still running
    If proc<>Null
        If proc.Status()
            Notify "program still running!"
            Return
        EndIf
    EndIf

    ' run program
    proc=TProcess.Create(myapp$,0)
    If Not Proc
        Notify "Failed to launch "+myapp$
    EndIf
End Function
EndRem

Type OpenXMLTool Extends ShoeTool

	Field sb:ShoeBox
	
	'----------------------------------------------------------------------------
	Method InitOpenXML:OpenXMLTool( akey:Int, amodifier:Int, aSelection:Selection, awin:Window, aShoeBox:ShoeBox )
		
		Init( akey, amodifier, "Open", aSelection, awin )
		
		sb = aShoeBox
		
		Return Self
		
	EndMethod
	
	
	'----------------------------------------------------------------------------
	Method KeyDown:Int( akey:Int, amodifiers:Int )
				
		If akey = key And amodifiers & modifier
			' clear selection
			If Confirm( "Opening an external XML editor will save your current changes, continue?" )
				sb.Save()
				CreateProcess:TProcess( TConfig.getStringValue("xmleditor") + " " + sb.m_shoeboxFilename, 1)
				
				Return True
			EndIf
		EndIf
		
		Return Super.KeyDown( akey, amodifiers )
		
	EndMethod
	
EndType




