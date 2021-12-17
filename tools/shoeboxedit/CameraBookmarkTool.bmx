SuperStrict

Import "ShoeTool.bmx"

Type CameraBookmarkTool Extends ShoeTool


	Field px#, py#
	
	'----------------------------------------------------------------------------
	Method InitCameraBookmark:CameraBookmarkTool( akey:Int, amodifier:Int, aSelection:Selection, awin:Window )
		
		Init( akey, amodifier, "Camera Bookmark", aSelection, awin )
		
		px = win.Camera.px
		py = win.Camera.py
		Return Self
		
	EndMethod
	
	
	'----------------------------------------------------------------------------
	Method KeyDown:Int( akey:Int, amodifiers:Int )
		
		If akey = key 
			Local tpx# = px 
			Local tpy# = py
			
			px = win.Camera.px
			py = win.Camera.py
			
			'Print "CAM: " + tpx + ", " + tpy + " "+ win.camera.pz
			win.SetCamera(tpx, tpy)
			
			Return Super.KeyDown( akey, amodifiers )
		EndIf
				
	EndMethod
	
	
EndType


