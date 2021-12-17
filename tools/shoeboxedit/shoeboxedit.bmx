SuperStrict

Import sidesign.minib3d
Import "TConfig.bmx"
Import "Window.bmx"
Import "Editor.bmx"

' load xml configuration/tweaks before doing anything else.
TConfig.parseXML( CurrentDir() + "/config.xml" )

Local resize:Float = 0
Local level:String = ""

For Local i:Int = 1 Until AppArgs.length
	Local arg:String = AppArgs[i]
	
	Select arg
		Case "-scale"
			resize = Float(AppArgs[i + 1])
			i :+ 1
		'Case "-level"
		'	level = AppArgs[i + 1]
		'	i :+ 1
	EndSelect		
Next

Local win:Window  = New Window.Init( TConfig.getIntValue("windowwidth", 1024), TConfig.getIntValue("windowheight", 768), "Shoebox Editor" )

Local edit:Editor = New Editor.Init( win )

If (resize <> 0 And level <> "")
	edit.box.Resize(resize)
	edit.box.SaveToXML(level)
	End
Else 
	edit.Run()
EndIf