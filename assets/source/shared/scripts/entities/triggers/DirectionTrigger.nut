include_entity("triggers/Trigger");

class DirectionTrigger extends Trigger
</
	editorImage    = "editor.directiontrigger"
	libraryImage   = "editor.library.directiontrigger"
	placeable      = Placeable_Developer
	collisionRect  = [ 0.0, 1.0, 2.0, 2.0 ]
	sizeShapeColor = Colors.Trigger
	displayName    = "Trigger - Direction"
	group          = "04. Input Triggers"
	stickToEntity  = true
/>
{
	// Only works with rectangles
	</
		type        = "string"
		order       = 0
		conditional = "type != rectangle" // HACK: To hide this from the editor, but still pass it as a property
	/>
	type = "rectangle";
	
	</
		type           = "entityid_array"
		filter         = ["Trigger"]
		order          = 1.1
		group          = "Triggers"
		referenceColor = ReferenceColors.Enable
		description    = "When set, triggerLeft will receive an enter signal when entity leaves this trigger from the left"
	/>
	triggerLeft = null;
	
	</
		type           = "entityid_array"
		filter         = ["Trigger"]
		order          = 1.2
		group          = "Triggers"
		referenceColor = ReferenceColors.Enable
		description    = "When set, triggerRight will receive an enter signal when entity leaves this trigger from the right"
	/>
	triggerRight = null;
	
	</
		type           = "entityid_array"
		filter         = ["Trigger"]
		order          = 1.3
		group          = "Triggers"
		referenceColor = ReferenceColors.Enable
		description    = "When set, triggerTop will receive an enter signal when entity leaves this trigger from the top"
	/>
	triggerTop = null;
		
	</
		type           = "entityid_array"
		filter         = ["Trigger"]
		order          = 1.4
		group          = "Triggers"
		referenceColor = ReferenceColors.Enable
		description    = "When set, triggerBottom will receive an enter signal when entity leaves this trigger from the bottom"
	/>
	triggerBottom = null;
	
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function _triggerExit(p_entity, p_parent)
	{
		base._triggerExit(p_entity, p_parent);
		
		switch (determineHitDirection(p_entity))
		{
		case Direction_Left:  triggerList(triggerLeft,   p_entity, p_parent); break;
		case Direction_Right: triggerList(triggerRight,  p_entity, p_parent); break;
		case Direction_Up:    triggerList(triggerTop,    p_entity, p_parent); break;
		case Direction_Down:  triggerList(triggerBottom, p_entity, p_parent); break;
		default: break;
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods
	
	function triggerList(p_list, p_entity, p_parent)
	{
		if (p_list == null)
		{
			return;
		}
		
		foreach (triggerID in p_list)
		{
			local trigger = ::getEntityByID(triggerID);
			if (trigger != null)
			{
				trigger._triggerEnter(p_entity, p_parent);
				trigger._triggerExit(p_entity, p_parent);
			}
		}
	}
	
	function determineHitDirection(p_entity)
	{
		// Ray Testing taken from: http://tavianator.com/fast-branchless-raybounding-box-intersections
		local boxPos = getCenterPosition();
		local minx = boxPos.x - (width / 2.0);
		local miny = boxPos.y - (height / 2.0);
		local maxx = boxPos.x + (width / 2.0);
		local maxy = boxPos.y + (height / 2.0);
		
		local x0 = p_entity.getCenterPosition();
		local n = p_entity.getSpeed().normalize();
		local n_inv = ::Vector2(1.0 / n.x, 1.0 / n.y);
		
		local tx1 = (minx - x0.x) * n_inv.x;
		local tx2 = (maxx - x0.x) * n_inv.x;
		
		local tmin = ::min(tx1, tx2);
		local tmax = ::max(tx1, tx2);
		
		local ty1 = (miny - x0.y) * n_inv.y;
		local ty2 = (maxy - x0.y) * n_inv.y;
		
		tmin = ::max(tmin, ::min(ty1, ty2));
		tmax = ::min(tmax, ::max(ty1, ty2));
		
		// if tmin < 0, we're inside the box
		local epsilon = 0.0001;
		local hitpos = (tmin < 0) ? (n * tmax) + x0 : (n * tmin) + x0;
		
		// Determine face
		if      (::fabs(hitpos.x - minx) < epsilon) return Direction_Left;
		else if (::fabs(hitpos.x - maxx) < epsilon) return Direction_Right;
		else if (::fabs(hitpos.y - miny) < epsilon) return Direction_Down;
		// Must be up
		return Direction_Up;
	}
}
