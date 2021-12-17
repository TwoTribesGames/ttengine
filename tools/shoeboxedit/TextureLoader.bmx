SuperStrict


Type TextureLoader

	Global _textures:TMap = New TMap
	Global _missingImage:TPixmap
	
	Function GetTexture:TPixmap(url:String)
	
		Local hash:String = GetRealTextureName(url)
		
		If _textures.Contains(hash)
			Return TPixmap(_textures.ValueForKey(hash))
		Else
			Local pm:TPixmap = LoadPixmap(hash)
			
			If (pm <> Null)
				_textures.Insert(hash, pm)
				pm._source = hash
			EndIf
			
			Return pm
		EndIf
	EndFunction
	
	Function GetRealTextureName:String(url:String)
		
		url = RealPath(url)
		
		Local special:Int = url.FindLast:Int("__")
		
		If (special > -1)
			Print ("Found special delimiter!")
			Local ext:String = "." + ExtractExt(url)
			url = url[..special] + ext
			
			Print ("New file: " + url)
			DebugStop()
		EndIf
		
		Return url
	EndFunction
	
	Function Reset()
		_textures.Clear()
	EndFunction
EndType
