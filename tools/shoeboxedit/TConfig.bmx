Strict

Import BaH.Libxml

Type TConfig
	
	Global configs:TMap = CreateMap()
	
	Function parseXML(XMLFileName:String)
		' parse the config file document
		
		Local XMLDoc:TxmlDoc = TxmlDoc.parseFile(XMLFileName)
		
		If XMLDoc = Null
			' something went wrong parsing the file
			RuntimeError("Could not parse: "+XMLFileName)
		EndIf
		
		Local node:TxmlNode = XMLdoc.GetRootElement()
	
		If node = Null Then
			RuntimeError "Empty document"
			XMLdoc.Free()
			Return
		End If
		
		If node.getName() <> "config"
			RuntimeError "Wrong document type, root element isn't called 'config'!"
			XMLdoc.Free()
			Return
		End If
		
		' start parsing the actual document
				
		Local children:TList = node.getChildren()
		
		For Local n:TxmlNode = EachIn children
			parse(n)
		Next
		
		XMLdoc.Free()
	EndFunction
	
	Function parse(node:TxmlNode, path:String="")
		
		Local name:String     = node.getName()
		Local pathname:String = name
		Local value:String    = node.getAttribute("value")
	
		If path
			pathname = path + "." + pathname
		EndIf
		
		If value
			' if the node had a value attribute, insert it to the big list
			?debug
			Print "Cofiguration added: "+pathname+", "+value
			?
			configs.insert(pathname, value)
		EndIf
	
		' parse children
		Local children:TList = node.getChildren()
		' return if the node has no children
		If Not children Return
		
		
		For Local n:TxmlNode = EachIn children
			
			
			parse(n, pathname)
			
		Next
		
	EndFunction
	
	Function getFloatValue:Float(name:String, aDefault:Float=0)
		Local v:Float = Float(String(configs.valueForKey(name)))
		
		If v 
			Return v
		Else
			Return aDefault
		EndIf
	EndFunction
	
	Function getIntValue:Int(name:String, aDefault:Int=0)
		Local v:Int = Int(String(configs.valueForKey(name)))

		If v 
			Return v
		Else
			Return aDefault
		EndIf
	EndFunction
	
	Function getBoolValue:Int(name:String, aDefault:Int=False)
		Local v:String = String(configs.valueForKey(name)).ToLower()

		If v = "true" Or v = "on" Or v = "yes" Or v = "1"
			Return True
		ElseIf v = "false" Or v = "off" Or v = "no" Or v = "0"
			Return False
		Else
			Return False
		EndIf
	EndFunction
	
	Function getStringValue:String(name:String, aDefault:String="")
		Local v:String = String(configs.valueForKey(name))

		If v 
			Return v
		Else
			Return aDefault
		EndIf
	EndFunction
	
EndType