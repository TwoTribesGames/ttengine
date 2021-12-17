SuperStrict

Import sidesign.minib3d
Import "TextArea.bmx"
Import "TConfig.bmx"
Import "PlaneHelper.bmx"

Type Window
	
	Field Camera:TCamera
	Field FOV:Float, zoom:Float = 0
	
	Field Caption:TextArea
	Field ToolText:TextArea
	Field CamPos:TextArea
	
	Field w#, h#
	Field cameraSpeedx:Float, cameraSpeedy:Float, cameraDamping:Float
	
	Field cameraHeight:Float
	
	Field nearFogMarker:TMesh, farFogMarker:TMesh
	
	Const SLICESIZE:Int = 2

	Const WIDESCREEN:Int = False
			
	'----------------------------------------------------------------------------
	Method Init:Window( width:Int, height:Int, title:String="")
		AppTitle = title
		Graphics3D( width, height, 32, 2 )
		
		w = width
		h = height
		
		cameraHeight = height
		FOV = TConfig.getFloatValue("camerafov", 45)
		
		' create cam
		Camera:TCamera = CreateCamera()

		nearFogMarker = CreateSphere(16, Camera)
		nearFogMarker.EntityColor(200, 100, 50)
		nearFogMarker.ScaleMesh(32, 32, 0)
		nearFogMarker.EntityFX(1 + 4 + 8 + 32) 
		nearFogMarker.EntityOrder(4)
		nearFogMarker.HideEntity()
		
		farFogMarker  = CreateSphere(16, Camera)
		farFogMarker.EntityColor(50, 100, 200)
		farFogMarker.ScaleMesh(32, 32, 0)
		farFogMarker.EntityFX(1 + 4 + 8 + 32) 
		farFogMarker.EntityOrder(4)
		farFogMarker.HideEntity()
		
		Camera.CameraClsColor( 128,128,128 )
		Camera.CameraViewport( 0, 0, width, height )
		ResetCamera()

		'SetCameraFOV( Camera, FOV )
		Camera.CameraZoom( FOV ) ' i changed the minib3d camerazoom to set the FOVy
		
		AmbientLight( 255,255,255 )
		
		Caption  = New TextArea.Init( "Caption" )
		ToolText = New TextArea.Init( "ToolText~nx:0~ty:0" )
		CamPos   = New TextArea.Init( "(,)" )
		
		SetFogColor(200, 225, 240)
		'SetFogRange(100, 500)
		
		cameraDamping = TConfig.getFloatValue("cameradamping", 0.96)
		Return Self
	EndMethod

	'----------------------------------------------------------------------------
	Method Render()
		'BeginMax2D()

		'Cls()
		'SetColor( 255,255,255 )	
		'DrawRect( 0, 19, 642, 482 )
		'DrawLine( 0, 19, GraphicsWidth(), 19 )
		'DrawLine( 0, 321+19, GraphicsWidth(), 321+19 )
		

		
		'EndMax2d()
			
		Rem?debug
		If KeyDown(KEY_LSHIFT) Or KeyDown(KEY_RSHIFT)
			Local lr:Int = KeyDown(KEY_LEFT) - KeyDown(KEY_RIGHT) 	
			Local ud:Int = KeyDown(KEY_UP) - KeyDown(KEY_DOWN)
			
			camera.MoveEntity( 0,0, ud * 10 )
			camera.TurnEntity( 0, lr, 0 )
		EndIf
		EndRem?

		If Not lockedcam Then UpdateCameraPos()
		
		UpdateWorld()
		RenderWorld()
		
		SetColor(255,255,255)
		' Disabled campos for now
		'CamPos.Render( 5, 10 )
		Caption.Render( 5, 0 )
		ToolText.Render( 5, h - 40 )
		
		Flip( 0 )
		
		
	EndMethod	
	
	'----------------------------------------------------------------------------
	Method MoveCamera( dx:Float, dy:Float )
		Camera.MoveEntity( dx, dy, 0 )
		
		SetCameraSpeed(dx * 0.6, dy * 0.6)
	EndMethod
	
	'----------------------------------------------------------------------------	
	Method GetWidth:Int()
		Return w
	EndMethod
	
	Method GetHeight:Int()
		Return h
	EndMethod

	'----------------------------------------------------------------------------
	Field lockedcam:Int = True
	Method FreeCamera()
		lockedcam = False
	EndMethod
	
	Method LockCamera()
		lockedcam = True
		SetCameraSpeed(0,0)
	EndMethod

	'----------------------------------------------------------------------------
	Method UpdateCameraPos()
		Camera.MoveEntity( cameraSpeedx, cameraSpeedy, 0 )
		cameraSpeedx = cameraSpeedx * cameraDamping
		cameraSpeedy = cameraSpeedy * cameraDamping 
	EndMethod
	
	'----------------------------------------------------------------------------
	Method SetCamera( dx:Float, dy:Float )
		Camera.MoveEntity( dx - camera.px, dy - camera.py, 0 )
		SetCameraSpeed(0,0)
	EndMethod
	
	'----------------------------------------------------------------------------
	Method SetCameraSpeed( dx:Float, dy:Float)
		cameraSpeedx = dx
		cameraSpeedy = dy
	EndMethod
		
	'----------------------------------------------------------------------------
	Method ZoomCamera( dz:Float )
		
		'Camera.MoveEntity( 0, 0, dz )
		'SetCameraRange( 1, Camera.range_far + dz )
		zoom :+ dz
		zoom = Max(-FOV + dz, zoom)
		
		Local newZoom:Float = FOV + zoom
		If (newZoom < 1.0)   newZoom = 1.0
		If (newZoom > 170.0) newZoom = 170.0
		
		zoom = newZoom - FOV
		
		Camera.CameraZoom(newZoom)
		
	EndMethod
	
	'----------------------------------------------------------------------------
	Method ResetCamera(keepxy:Int = False, resetFOV:Int = True)
		
		Local x:Float, y:Float
		If keepxy
			x = Camera.px
			y = Camera.py
		EndIf
		Camera.PositionEntity( x, y, -GetCameraDistance() )
		
		If resetFOV
			zoom = 0
			Camera.CameraZoom(FOV)
		EndIf
	EndMethod
	
	'----------------------------------------------------------------------------
	Method GetCameraDistance:Float()
		
		Return TConfig.getFloatValue("cameradistance", 25 )'* 32)'( cameraHeight * 0.5 ) / Tan( 0.5 * FOV )
		
	EndMethod
	
	
	Field storedNear:Float, storedFar:Float
	'----------------------------------------------------------------------------
	Method ShowSlice( slicePos:Int )
		
		storedNear = Camera.range_near
		storedFar  = Camera.range_far
		
		SetSlice( slicePos )
		
	EndMethod
	
	'----------------------------------------------------------------------------
	Method HideSlice()
	
		Camera.CameraRange( storedNear, storedFar )
		
	EndMethod
	
	'----------------------------------------------------------------------------
	Method SetSlice( slicePos:Int )
		
		Camera.CameraRange( GetCameraDistance() + slicePos - SLICESIZE, GetCameraDistance() + slicePos + SLICESIZE )
		
	EndMethod

	'----------------------------------------------------------------------------
	Method SetCameraRange( near:Int, far:Int )
		
		'Camera.CameraRange( GetCameraDistance() + near - 1, GetCameraDistance() + far + 1  )
		Camera.CameraRange( 1, GetCameraDistance() + far + 1  )

	EndMethod
	
	Method getNormalizedZCoord:Float(worldZ:Float)
	'DebugStop()
		Return (Camera.pz - Camera.range_near - worldZ) / (Camera.range_far - Camera.range_near)
	EndMethod
	
	'----------------------------------------------------------------------------
	Method SetFog(near:Float, far:Float, r:Int, g:Int, b:Int)
		ShowFog()
		SetFogColor(r, g, b)
		SetFogRange(near, Far)
	EndMethod
	
	Method SetFogColor(r:Int, g:Int, b:Int)
		Camera.CameraFogColor(r, g, b)
	EndMethod
	
	Method SetFogRange(near:Float, far:Float)
		nearFogMarker.PositionEntity(15, 15, near)
		farFogMarker.PositionEntity(-15,-15, far)
		Camera.CameraFogRange(near, far)
	EndMethod
	
	Method ShowFog()
		'nearFogMarker.ShowEntity()
		'farFogMarker.ShowEntity()
		Camera.CameraFogMode(1) 
	EndMethod
	
	Method HideFog()
		nearFogMarker.HideEntity()
		farFogMarker.HideEntity()
		Camera.CameraFogMode(0)
	EndMethod
	'----------------------------------------------------------------------------
	Method ClearScene()
	
	
	EndMethod
	
	'----------------------------------------------------------------------------
	Method GetObjectFrom:TEntity( x:Int, y:Int )
		Local ent:TEntity = Camera.CameraPick( MouseX(), MouseY() )
		
		If ( ent )
			Return ent
		EndIf

		Return Null		
	EndMethod
	
	'----------------------------------------------------------------------------
	Method ScreenToWorld(p_screenX:Int, p_screenY:Int, p_worldX:Float Var, p_worldY:Float Var)
		Local zoom:Float  = Float(FOV + zoom)
		Local screenWidth:Float = GetWidth()
		Local screenHeight:Float = GetHeight()
		Local aspectRatio:Float  = screenWidth / screenHeight
		Local camHeight:Float = 2.0 * (Tan(0.5 * zoom) * GetCameraDistance());
		Local camWidth:Float  = aspectRatio * camHeight
			
		p_worldX = (p_screenX/ screenWidth ) * camWidth
		p_worldY = (p_screenY/ screenHeight) * camHeight
	EndMethod
	
EndType

Rem

Function SetCameraFOV(Camera:TCamera, FOV:Float)
	
	Local vertvof:Float = 2 / ( 1.5 * Cos( FOV ) )
	Camera.CameraZoom( vertvof )

'	Camera.CameraZoom( 1.0 / Tan( FOV/2.0 ) )
	
End Function
endrem