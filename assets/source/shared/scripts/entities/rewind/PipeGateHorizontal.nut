include_entity("rewind/PipeGate");

class PipeGateHorizontal extends PipeGate
</
	editorImage               = "editor.pipegate"
	libraryImage              = "editor.library.pipegate_horizontal"
	placeable                 = Placeable_Everyone
	movementset               = "Static"
	collisionRect             = [ 0.0, 0.5, 3.0, 1.0 ]
	sizeShapeColor            = Colors.PipeGate
/>
{
	</
		type   = "integer"
		choice = [90, 270]
		order  = 1
	/>
	rotation = 90;
	
	</
		type  = "integer"
		min   = 1
		max   = 1
		order = 2
		conditional = "height != 1" // HACK: To hide this from the editor, but still pass it as a property
	/>
	height = 1;
	
	</
		type  = "integer"
		min   = 2
		max   = 10
		order = 3
	/>
	width = 3;
}
