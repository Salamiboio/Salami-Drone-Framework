class SAL_DroneBulletComponentClass: ScriptComponentClass
{
}

class SAL_DroneBulletComponent: ScriptComponent
{
	override void OnPostInit(IEntity owner)
	{
		#ifdef WORKBENCH
		
		SetEventMask(owner, EntityEvent.FRAME);
		
		#else
		
		if (RplSession.Mode() != RplMode.Dedicated)
			return;
		
		SetEventMask(owner, EntityEvent.FRAME);
		#endif
	}
	
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		GetGame().GetWorld().QueryEntitiesBySphere(owner.GetOrigin(), 0.5, FilterDrone, null);
	}
	
	bool FilterDrone(IEntity ent)
	{
		if (ent == GetOwner())
			return true;
		
		if (ent.FindComponent(SAL_DroneControllerComponent))
		{
			SAL_DroneHealthComponent.Cast(ent.FindComponent(SAL_DroneHealthComponent)).UpdateHealth(0);
		}
		return true;
	}
}