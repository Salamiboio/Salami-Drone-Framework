class SAL_DroneBulletComponentClass: ScriptComponentClass
{
}

class SAL_DroneBulletComponent: ScriptComponent
{
	
	bool m_bIsDestroyed = false;
	override void OnPostInit(IEntity owner)
	{
		#ifdef WORKBENCH
		
		SetEventMask(owner, EntityEvent.FRAME);
		
		#else
		
		if (RplSession.Mode() == RplMode.Client)
			return;
		
		SetEventMask(owner, EntityEvent.FRAME);
		#endif
	}
	
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		if (m_bIsDestroyed)
			return;
		
		if (!GetGame().GetWorld().QueryEntitiesBySphere(owner.GetOrigin(), 0.5, FilterDrone, null))
		{
			m_bIsDestroyed = true;
			ClearEventMask(owner, EntityEvent.FRAME);
		}
			
	}
	
	bool FilterDrone(IEntity ent)
	{
		if (ent == GetOwner())
			return true;
		
		if (m_bIsDestroyed)
			return true;
		
		if (ent.FindComponent(SAL_DroneControllerComponent))
		{
			if (SAL_DroneControllerComponent.Cast(ent.FindComponent(SAL_DroneControllerComponent)).m_bIsDestroyed)
				return true;
			
			SAL_DroneControllerComponent.Cast(ent.FindComponent(SAL_DroneControllerComponent)).m_bIsDestroyed = true;
			SAL_DroneConnectionManager.GetInstance().DestroyDroneServer(RplComponent.Cast(ent.FindComponent(RplComponent)).Id(), SAL_DroneHealthComponent.Cast(ent.FindComponent(SAL_DroneHealthComponent)).m_sDroneWreckPrefab);
			return false;
		}
		return true;
	}
}