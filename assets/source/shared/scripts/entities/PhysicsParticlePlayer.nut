include_entity("triggers/Trigger");
include_entity("ParticlePlayer");

class PhysicsParticlePlayer extends ParticlePlayer
</
	editorImage    = "editor.physicsparticleplayer"
	libraryImage   = "editor.library.physicsparticleplayer"
	placeable      = Placeable_Developer
	movementset    = "Static"
	collisionRect  = [ 0.0, 0.0, 2.0, 2.0 ]  // center X, center Y, width, height
	group          = "99. Misc."
	stickToEntity  = true
/>
{
	</
		type           = "float"
		min            = 0.0
		max            = 100
		group          = "Movement"
		order          = 1.1
	/>
	speed = 30.0;
	
	</
		type           = "float"
		min            = 0.0
		max            = 100
		group          = "Movement"
		order          = 1.2
	/>
	randomSpeed = 10.0;
	
	</
		type           = "integer"
		min            = -180
		max            = 180
		group          = "Movement"
		order          = 1.3
	/>
	rotation = 45;
	
	</
		type           = "integer"
		min            = 0
		max            = 180
		group          = "Movement"
		order          = 1.4
	/>
	randomRotation = 45;
	
	</
		type           = "float"
		min            = -250.0
		max            = 0.0
		group          = "Movement"
		order          = 1.5
	/>
	gravity = -100.0;
	
	</
		type           = "float"
		min            = 0.0
		max            = 2.0
		group          = "Movement"
		order          = 1.6
	/>
	friction = 0.025;
	
	</
		type           = "string"
		choice         = ::getSoundCueNamesInBanks(["Effects"])
		group          = "Movement"
		order          = 1.7
	/>
	movingSoundEffect = null;
	
	</
		type           = "bool"
		group          = "Collision"
		order          = 2.0
	/>
	collisionWithSolid = true;
	
	</
		type           = "float"
		min            = 0.0
		max            = 5.0
		conditional    = "collisionWithSolid == true"
		group          = "Collision"
		order          = 2.1
	/>
	radius = 0.5;
	
	</
		type           = "float"
		min            = 0.1
		max            = 2.0
		conditional    = "collisionWithSolid == true"
		group          = "Collision"
		order          = 2.2
	/>
	bouncyness = 0.5;
	
	</
		type           = "float"
		min            = 0.0
		max            = 10.0
		conditional    = "collisionWithSolid == true"
		group          = "Collision"
		order          = 2.3
	/>
	collisionDrag = 0.4;
	
	</
		type           = "string"
		choice         = ::getSoundCueNamesInBanks(["Effects"])
		conditional    = "collisionWithSolid == true"
		group          = "Collision"
		order          = 2.4
	/>
	collisionSoundEffect = null;
	
	</
		type           = "float"
		min            = 0.0
		max            = 10.0
		description    = "Particle ends at timeout. Use 0.0 for infinite. When ending all parented Triggers will trigger."
		group          = "Ending"
		order          = 3.0
	/>
	endTimeout = 0.0;
	
	</
		type           = "float"
		min            = 0.0
		max            = 10.0
		group          = "Ending"
		order          = 3.1
	/>
	endRandomTimeout = 0.0;
	
	</
		type           = "integer"
		min            = 0
		max            = 10
		description    = "Particle ends when bouncing X times. Use 0 for infinite. When ending all parented Triggers will trigger."
		group          = "Ending"
		order          = 3.2
	/>
	endAtBounceCount = 0;
	
	</
		type           = "string"
		choice         = getParticleEffectNames()
		description    = "When set, it triggers a new particle effect when this one dies. Uses same settings for flip and layer."
		group          = "Ending"
		order          = 3.3
	/>
	endParticleEffect = null;
	
	</
		type           = "string"
		choice         = ::getSoundCueNamesInBanks(["Effects"])
		conditional    = "collisionWithSolid == true"
		group          = "Ending"
		order          = 3.4
	/>
	endSoundEffect = null;
	
	</
		type           = "entityid_array"
		filter         = ["PhysicsParticlePlayer"]
		conditional    = "fireworksMode == true"
		group          = "Ending"
		order          = 3.5
	/>
	endFireworksParts = null;
	
	</
		type           = "integer"
		min            = 1
		max            = 20
		conditional    = "endFireworksParts != null"
		group          = "Ending"
		order          = 3.6
	/>
	endFireworksPartCount = 5;
	
	</
		type           = "bool"
		order          = 101
		group          = "Misc."
	/>
	enabled = true;
	
	_movementSettings  = null;
	_bounceCount       = 0;
	_killedParts       = false;
	_deathEvent        = null;
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// onInit / onSpawn
	
	function onInit()
	{
		base.onInit();
		
		_movementSettings = PhysicsSettings(1.0, friction, 0.0, -1, -1);
		_movementSettings.setCollisionWithSolid(collisionWithSolid);
		_movementSettings.setBouncyness(bouncyness);
		_movementSettings.setCollisionDrag(collisionDrag);
		_movementSettings.setExtraForce(::Vector2(0, gravity));
		
		if (collisionWithSolid)
		{
			local diameter = radius * 2.0;
			setCollisionRectWithVectorRect(::VectorRect(::Vector2(0, 0), diameter, diameter));
		}
		
		_deathEvent = ::EventPublisher("deathevent");
	}
	
	function onSpawn()
	{
		base.onSpawn();
		
		// Kill associated fireworks part entities
		killParts();
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Callbacks
	
	function onEnabled()
	{
		base.onEnabled();
		
		local speedRange    = randomSpeed / 2.0;
		local rotationRange = randomRotation / 2.0;
		local movementSpeed = ::Vector2(0, ::max(0, speed + ::frnd_minmax(-speedRange, speedRange)));
		movementSpeed = movementSpeed.rotate(-rotation + ::rnd_minmax(-rotationRange, rotationRange));
		startMovementInDirection(::Vector2(0, 0), _movementSettings);
		setSpeed(movementSpeed);
		
		if (movingSoundEffect != null)
		{
			movingSoundEffect = playSoundEffect(movingSoundEffect);
		}
		
		local timeout = endTimeout + ::frnd_minmax(0, endRandomTimeout);
		if (timeout > 0.0)
		{
			startCallbackTimer("onEnding", timeout);
		}
	}
	
	function onSolidCollision(p_collisionNormal, p_speed)
	{
		++_bounceCount;
		if (endAtBounceCount > 0 && _bounceCount == endAtBounceCount)
		{
			customCallback("onEnding");
		}
		
		if (collisionSoundEffect != null)
		{
			playSoundEffect(collisionSoundEffect);
		}
	}
	
	function onDie()
	{
		base.onDie();
		
		killParts();
		_deathEvent.publish(this);
	}
	
	function onEnding()
	{
		if (endParticleEffect != null)
		{
			local effect = spawnParticleOneShot("particles/" + endParticleEffect, getCenterPosition(), false, 0.0, true, _layer, scale);
			handleFlipping(effect);
		}
		
		if (endSoundEffect != null)
		{
			playSoundEffect(endSoundEffect);
		}
		
		if (endFireworksParts != null)
		{
			local angle  = ::rnd_minmax(-180, 180);
			local spread = 360 / endFireworksPartCount;
			local parentSpeed = getSpeed();
			for (local i = 0; i < endFireworksPartCount; ++i)
			{
				local id = ::rnd_minmax(0, endFireworksParts.len() - 1);
				local part = ::respawnEntityAtPosition(endFireworksParts[id], getCenterPosition());
				
				// Distribute equally
				local speed = part.getSpeed().length();
				part.setSpeed((::getVectorFromAngle(angle) * speed) + parentSpeed);
				angle += spread;
			}
		}
		triggerChildTriggers(this);
		::killEntity(this);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// Methods
	
	function killParts()
	{
		if (_killedParts)
		{
			return;
		}
		_killedParts = true;
		
		// Kill associated fireworks part entities
		if (endFireworksParts != null)
		{
			foreach (entityID in endFireworksParts)
			{
				::killEntity(::getEntityByID(entityID));
			}
		}
	}
}
Trigger.makeTriggerParentable(PhysicsParticlePlayer);
