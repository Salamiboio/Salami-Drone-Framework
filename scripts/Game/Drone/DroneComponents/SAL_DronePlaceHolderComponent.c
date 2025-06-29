class SAL_DronePlaceHolderComponentClass: ScriptComponentClass
{
}

class SAL_DronePlaceHolderComponent: ScriptComponent
{
	[Attribute(desc: "Drone this will spawn", params: "et")] ResourceName m_sDroneToSpawn;
	bool m_bHasParent = false;
	
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		SetEventMask(owner, EntityEvent.FRAME);
	}
	
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		#ifdef WORKBENCH
		if (owner.GetParent() != null)
			return;
		
		vector transform[4];
		owner.GetTransform(transform);
		EntitySpawnParams params = new EntitySpawnParams();
		params.Transform = transform;
		GetGame().SpawnEntityPrefab(Resource.Load(m_sDroneToSpawn), GetGame().GetWorld(), params);
		
		SCR_EntityHelper.DeleteEntityAndChildren(owner);
		#else
		if (RplSession.Mode() == RplMode.Client)
			return;
			
		if (owner.GetParent() != null)
			return;
		
		vector transform[4];
		owner.GetTransform(transform);
		EntitySpawnParams params = new EntitySpawnParams();
		params.Transform = transform;
		GetGame().SpawnEntityPrefab(Resource.Load(m_sDroneToSpawn), GetGame().GetWorld(), params);
		
		SCR_EntityHelper.DeleteEntityAndChildren(owner);
		#endif
	}
}
