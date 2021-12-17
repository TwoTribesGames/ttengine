SuperStrict

Type Timer
	
	Field starttime:Int
	Field endtime:Int
	Field duration:Int
	Field Tag:String = "Timer"
	
	Method Start:Timer(aTag:String = "Timer")
		starttime = MilliSecs()
		endtime   = -1
		duration  = 0
		Tag       = aTag
		
		Return Self
	EndMethod
	
	Method Pause()
		duration :+ MilliSecs() - starttime	
	EndMethod
	
	Method Resume()
		starttime = MilliSecs()
	EndMethod
	
	Method Stop()
		endtime  = MilliSecs()
		duration :+ endtime - starttime
	EndMethod
	
	Method Report:String()
		Stop()
		Return Self.ToString()
	EndMethod
	
	Method ToString:String()
		Return "[" + Tag + "] Running For: " + duration + "ms"
	EndMethod
EndType
