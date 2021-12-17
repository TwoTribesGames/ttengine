SuperStrict

Import "ShoePlane.bmx"

Type Popup Extends ShoePlane

	'----------------------------------------------------------------------------
	Method InitPopup:Popup( node:TxmlNode, p_isDummy:Int = False )
		
		Super.Init( node, p_isDummy )
	
		Return Self
		
	EndMethod

	'----------------------------------------------------------------------------	
	Method Duplicate:ShoePlane(p_isDummy:Int = False)
		
		Local xmlnode:TxmlNode = DuplicateXML()
		
		Local t:ShoePlane = New Popup.InitPopup( xmlNode, p_isDummy )
		
		Return t
	
	EndMethod
		
	'----------------------------------------------------------------------------	
	Method GetShoePlaneAttributes()
		
		Local x:Int        = Int(xmlNode.getAttribute( "x" ) )
		Local y:Int        = Int(xmlNode.getAttribute( "y" ) )
		
		Local width:Int    = Int(xmlNode.getAttribute("width"))
		Local height:Int   = Int(xmlNode.getAttribute("height"))
		
		InitShoeMesh( x,y,0, 0, width, height, TConfig.getStringValue("popuptexture") )
		
	EndMethod
	
	'----------------------------------------------------------------------------
	Method InitShoeMesh( x:Float, y:Float, z:Float, rotation:Float, width:Float, height:Float, tex:String )
		
		CreatePlane()
		SetSize( width, height )
		SetTexture( tex )
		SetPosition( x, y, z )
	
		SetPriority(1000)

		?debug
		Print "Created Popup: " + tex + "~t~t " + width + "x" + height + "~t~t~t(" + x + ", " + y + ", " + z + ")~t~tangle: " + rotation
		?
	EndMethod
		
	'----------------------------------------------------------------------------
	Method GetToolText:String()	
	
		Return "pos(" + GetX() + ", " + GetY() + ") w(" + GetWidth() + ", " + GetHeight() + ")   " + ..
		       "popup(" + xmlNode.GetAttribute("loc_id") + ") " + ..
		       "script(" + xmlNode.GetAttribute("script_filename") + ") " + ..
		       "name(" + xmlNode.GetAttribute("popup_name") + ")"
		
	EndMethod

	'----------------------------------------------------------------------------
	Method Rotate( angle:Float)
	
	' do not rotate
		Return

	EndMethod
	
	'----------------------------------------------------------------------------	
	Method UpdateXML()
	
		xmlNode.SetAttribute( "x", FloatToFixedPoint(GetX() / internalScale) )
		xmlNode.SetAttribute( "y", FloatToFixedPoint(GetY() / internalScale) )

		xmlNode.SetAttribute( "width", FloatToFixedPoint(GetWidth()) )
		xmlNode.SetAttribute( "height", FloatToFixedPoint(GetHeight()) )
	
	EndMethod
		
EndType

