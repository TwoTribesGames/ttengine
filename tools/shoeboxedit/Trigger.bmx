SuperStrict

Import "ShoePlane.bmx"
Rem
Import bah.libxml
Import sidesign.minib3d
Import "TConfig.bmx"
EndRem
Type Trigger Extends ShoePlane

	'----------------------------------------------------------------------------
	Method InitTrigger:Trigger( node:TxmlNode, p_isDummy:Int = False )
		
		Super.Init( node, p_isDummy )
	
		Return Self
		
	EndMethod

	'----------------------------------------------------------------------------	
	Method Duplicate:ShoePlane(p_isDummy:Int = False, p_parent:TxmlNode = Null)
		
		Local xmlnode:TxmlNode = DuplicateXML(p_isDummy, p_parent)
		
		Local t:ShoePlane = New Trigger.InitTrigger( xmlNode, p_isDummy )
		
		Return t
	
	EndMethod
		
	'----------------------------------------------------------------------------	
	Method GetShoePlaneAttributes()
		
		Local x:Int        = Int(xmlNode.getAttribute( "x" ) )
		Local y:Int        = Int(xmlNode.getAttribute( "y" ) )
		Local z:Int        = Int(xmlNode.getAttribute( "z" ) )
		
		Local width:Int    = Int(xmlNode.getAttribute("width"))
		Local height:Int   = Int(xmlNode.getAttribute("height"))
		InitShoeMesh( x,y,z, 0, width, height, TConfig.getStringValue("triggertexture") )
		
	EndMethod
	
	'----------------------------------------------------------------------------
	Method InitShoeMesh( x:Float, y:Float, z:Float, rotation:Float, width:Float, height:Float, tex:String )
		
		CreatePlane()
		SetSize( width, height )
		SetTexture( tex )
		SetPosition( x, y, z )
	
		SetPriority(1000)

		?debug
		Print "Created Trigger: " + tex + "~t~t " + width + "x" + height + "~t~t~t(" + x + ", " + y + ", " + z + ")~t~tangle: " + rotation
		?
	EndMethod
		
	'----------------------------------------------------------------------------
	Method GetToolText:String()
		Local x:String = FloatToFixedPoint(GetX() / internalScale)
		Local y:String = FloatToFixedPoint(GetY() / internalScale)
	
		Return "pid: " + pid + " " + .. 
		       "pos: (" + x + ", " + y + ") w(" + FloatToFixedPoint(GetWidth()) + ", " + FloatToFixedPoint(GetHeight()) + ")   " + ..
		       "script: (" + xmlNode.GetAttribute("script_filename") + ") " + ..
		       "name: (" + xmlNode.GetAttribute("trigger_name") + ")"
		
	EndMethod

	'----------------------------------------------------------------------------
	Method Rotate( angle:Float )
	
	' do not rotate
		Return

	EndMethod
	
	'----------------------------------------------------------------------------	
	Method UpdateXML()
	
		' Store settings before unsetting
		Local script:String = xmlNode.GetAttribute("script_filename")
		Local name:String = xmlNode.GetAttribute("trigger_name")
			
		' FIXME: Unset all attributes, so that the order is guaranteed to be the same
		'For Local attribute:TxmlAttribute = EachIn xmlNode.getAttributeList()
		'	xmlNode.unsetAttribute(attribute.getName())
		'Next
		
		xmlNode.SetAttribute( "pid", pid )
		xmlNode.SetAttribute( "script_filename", script )
		xmlNode.SetAttribute( "script_filename", name )
		xmlNode.SetAttribute( "x", FloatToFixedPoint(GetX() / internalScale) )
		xmlNode.SetAttribute( "y", FloatToFixedPoint(GetY() / internalScale) )
		
		xmlNode.SetAttribute( "width", FloatToFixedPoint(GetWidth()) )
		xmlNode.SetAttribute( "height", FloatToFixedPoint(GetHeight()) )
	
	EndMethod
		
EndType

