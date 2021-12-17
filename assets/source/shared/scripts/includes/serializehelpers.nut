/*

*/

// sensor


function Sensor::_serialize()
{
	return [
		// handle
		[
			["kt", "s"],
			["kv", "Handle"],
			["vt", "i"],
			["v", getHandleValue()]
		],
		// position
		[
			["kt", "s"],
			["kv", "Position"],
			["vt", "s"],
			["v", getPosition()]
		],
		// enabled
		[
			["kt", "s"],
			["kv", "Enabled"],
			["vt", "s"],
			["v", isEnabled()]
		]
	];
}
