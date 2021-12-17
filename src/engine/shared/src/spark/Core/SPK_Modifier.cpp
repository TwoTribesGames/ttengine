//////////////////////////////////////////////////////////////////////////////////
// SPARK particle engine														//
// Copyright (C) 2008-2009 - Julien Fryer - julienfryer@gmail.com				//
//																				//
// This software is provided 'as-is', without any express or implied			//
// warranty.  In no event will the authors be held liable for any damages		//
// arising from the use of this software.										//
//																				//
// Permission is granted to anyone to use this software for any purpose,		//
// including commercial applications, and to alter it and redistribute it		//
// freely, subject to the following restrictions:								//
//																				//
// 1. The origin of this software must not be misrepresented; you must not		//
//    claim that you wrote the original software. If you use this software		//
//    in a product, an acknowledgment in the product documentation would be		//
//    appreciated but is not required.											//
// 2. Altered source versions must be plainly marked as such, and must not be	//
//    misrepresented as being the original software.							//
// 3. This notice may not be removed or altered from any source distribution.	//
//////////////////////////////////////////////////////////////////////////////////

#include <spark/Core/SPK_Modifier.h>

namespace SPK
{
	Vector3D Modifier::intersection;
	Vector3D Modifier::normal;

	Modifier::Modifier(int p_availableTriggers,ModifierTrigger p_trigger,bool p_needsIntersection,bool p_needsNormal,Zone* p_zone) :
		Registerable(),
		Transformable(),
		BufferHandler(),
		needsIntersection(p_needsIntersection),
		needsNormal(p_needsNormal),
		trigger(p_trigger),
		availableTriggers(p_availableTriggers),
		zone(p_zone),
		full(false),
		active(true),
		local(false)
	{}

	void Modifier::registerChildren(bool registerAll)
	{
		Registerable::registerChildren(registerAll);
		registerChild(zone,registerAll);
	}

	void Modifier::copyChildren(const Modifier& modifier,bool createBase)
	{
		Registerable::copyChildren(modifier,createBase);
		zone = dynamic_cast<Zone*>(copyChild(modifier.zone,createBase));
	}

	void Modifier::destroyChildren(bool keepChildren)
	{
		destroyChild(zone,keepChildren);
		Registerable::destroyChildren(keepChildren);
	}

	Registerable* Modifier::findByName(const std::string& p_name)
	{
		Registerable* object = Registerable::findByName(p_name);
		if ((object != NULL)||(zone == NULL))
			return object;

		return zone->findByName(p_name);
	}

	void Modifier::setZone(Zone* p_zone,bool p_full)
	{
		decrementChildReference(this->zone);
		incrementChildReference(p_zone);

		this->zone = p_zone;
		this->full = p_full;
	}

	bool Modifier::setTrigger(ModifierTrigger p_trigger)
	{
		if ((p_trigger & availableTriggers) != 0)
		{
			this->trigger = p_trigger;
			return true;
		}

		return false;
	}

	void Modifier::beginProcess(Group& group)
	{
		savedActive = active;
		
		if (!active)
			return;
		
		if (!prepareBuffers(group))
			active = false; // if buffers of the modifier in the group are not ready, the modifier is made incative for the frame
	}
}
