include_entity("rewind/PipeGate");

class PipeGateVertical extends PipeGate
</
	editorImage               = "editor.pipegate"
	libraryImage              = "editor.library.pipegate_vertical"
	placeable                 = Placeable_Everyone
	movementset               = "Static"
	collisionRect             = [ 0.0, 1.5, 1.0, 3.0 ]
	sizeShapeColor            = Colors.PipeGate
	sizeShapeFromEntityCenter = false
/>
{
	</
		type   = "integer"
		choice = [0, 180]
		order  = 1
	/>
	rotation = 0;
	
	</
		type  = "integer"
		min   = 2
		max   = 10
		order = 2
	/>
	height = 3;
	
	</
		type  = "integer"
		min   = 1
		max   = 1
		order = 3
		conditional = "width != 1" // HACK: To hide this from the editor, but still pass it as a property
	/>
	width = 1;
}
