SuperStrict

Import "ShoePlane.bmx"
Rem
Import bah.libxml
Import sidesign.minib3d
Import "TConfig.bmx"
EndRem
Type IncludePlane Extends ShoePlane
	
	Field offsetX:Float
	Field offsetY:Float
	Field offsetZ:Float
	Field scaleFactor:Float
	
	'----------------------------------------------------------------------------
	Method InitIncludePlane:IncludePlane ( node:TxmlNode, p_isDummy:Int = False)
		
		Super.Init( node, p_isDummy )
		
		offsetX     = Float(xmlNode.getAttribute( "x_offset" ) )
		offsetY     = Float(xmlNode.getAttribute( "y_offset" ) )
		offsetZ     = Float(xmlNode.getAttribute( "z_offset" ) )
		scaleFactor = Float(xmlNode.getAttribute( "scale"    ) )
		
		_scaleZ = True
		planeType = ShoePlaneType_Include
		
		Return Self
		
	EndMethod
	
	'----------------------------------------------------------------------------	
	Method Duplicate:ShoePlane(p_isDummy:Int = False, p_parent:TxmlNode = Null)
		
		Local xml:TxmlNode = DuplicateXML(p_isDummy, p_parent)
		
		Local t:ShoePlane = New IncludePlane.InitIncludePlane( xml, p_isDummy )
		
		Return t
	
	EndMethod
	
	'----------------------------------------------------------------------------	
	Method SetChildPlanes(p_planes:TList, p_moveChildren:Int)
		childPlanes = p_planes
		
		For Local p:ShoePlane = EachIn childPlanes
			p.Resize(scaleFactor)
			If (p_moveChildren) p.Move(offsetX, -offsetY, offsetZ)
		Next
	EndMethod
		
	'----------------------------------------------------------------------------	
	Method GetShoePlaneAttributes()
		
		Local x:Float        = Float(xmlNode.getAttribute( "x_offset" ) )
		Local y:Float        = Float(xmlNode.getAttribute( "y_offset" ) )
		Local z:Float        = Float(xmlNode.getAttribute( "z_offset" ) )
		
		Local width:Float    = Float(xmlNode.getAttribute( "scale" )) * 2
		If width <= 0 width = 2
		Local height:Float   = width

		InitShoeMesh( x,y,z, 0, width, height, TConfig.getStringValue("includetexture") )
		
	EndMethod
	
	'----------------------------------------------------------------------------
	Method InitShoeMesh( x:Float, y:Float, z:Float, rotation:Float, width:Float, height:Float, tex:String )
		CreatePlane()
	
		_scaleZ = True
		mesh.ScaleMesh(2, 2, 2)
		
		'Print "Init include " + tex
		SetPosition( x, y, z )
		SetSize( width, height )
		SetRotation( rotation )

		SetTexture( tex )

		?debug
		Print "Created IncludePlane : " + tex + "~t~t " + width  + "x" + height + "~t~t~t(" + x + ", " + y + ", " + z + ")~t~tangle: " + rotation
		?
	EndMethod
	
	'----------------------------------------------------------------------------	
	
	Method GetID:String()
		Return xmlNode.GetAttribute("filename")
	EndMethod

	'----------------------------------------------------------------------------
	Method GetToolText:String()	
		Local x:String = FloatToFixedPoint(GetX() / internalScale)
		Local y:String = FloatToFixedPoint(GetY() / internalScale)
		Local z:String = FloatToFixedPoint(GetZ() / internalScale)	
	
		Return "pid: " + pid + " " + .. 
		       "pos: (" + x + ", " + y + ", " + z + ") scale(" + FloatToFixedPoint(GetWidth() * 0.5) + ")   " + ..
		       "filename: (" + xmlNode.GetAttribute("filename") + ") "
	EndMethod

	Method Scale( ahorizontal:Float, avertical:Float, keepratio:Int = False )
		Super.Scale(ahorizontal * 0.0001, avertical * 0.0001, True )
	EndMethod
	
	Method ScaleSize( adw:Float, adh:Float )
		' Only supports uniform scaling
		Super.ScaleSize(adw, adw)
		For Local p:ShoePlane = EachIn childPlanes
			Local dx:Float = (GetX() - p.GetX())
			Local dy:Float = (GetY() - p.GetY())
			dx = dx - (adw * dx)
			dy = dy - (adw * dy)
			
			p.ScaleSize(adw, adw)
			
			p.Move(dx, -dy, 0)
		Next
	EndMethod
	
	'----------------------------------------------------------------------------
	Method Rotate( angle:Float )
		
	' do not rotate
		Return

	EndMethod
	
	Method ResizeScale(p_factor:Float)
		' do nothing
	EndMethod
	
	Method Move( dx:Float, dy:Float, dz:Float )
		Super.Move(dx, dy, dz)
		For Local p:ShoePlane = EachIn childPlanes
			p.Move(dx, dy, dz)
		Next
	EndMethod
	
	'----------------------------------------------------------------------------
	Method OnHide()
		Super.OnHide()
		
		For Local p:ShoePlane = EachIn childPlanes
			p.OnHide()
		Next		
	EndMethod
	
	'----------------------------------------------------------------------------	
	Method OnShow()
		Super.OnShow()
		
		For Local p:ShoePlane = EachIn childPlanes
			p.OnShow()
		Next		
	EndMethod
	
	'----------------------------------------------------------------------------	
	Method UpdateXML()
		
		' Store settings before unsetting
		Local filename:String = xmlNode.GetAttribute("filename")
			
		' FIXME: Unset all attributes, so that the order is guaranteed to be the same
		'For Local attribute:TxmlAttribute = EachIn xmlNode.getAttributeList()
		'	xmlNode.unsetAttribute(attribute.getName())
		'Next
		
		xmlNode.SetAttribute( "pid",      pid )
		xmlNode.SetAttribute( "filename", filename )
		xmlNode.SetAttribute( "x_offset", FloatToFixedPoint(GetX()) )
		xmlNode.SetAttribute( "x_offset", FloatToFixedPoint(GetX()) )
		xmlNode.SetAttribute( "y_offset", FloatToFixedPoint(GetY()) )
		xmlNode.SetAttribute( "z_offset", FloatToFixedPoint(GetZ()) )
		xmlNode.SetAttribute( "scale",    FloatToFixedPoint(GetWidth() * 0.5) )
	EndMethod
		
EndType

