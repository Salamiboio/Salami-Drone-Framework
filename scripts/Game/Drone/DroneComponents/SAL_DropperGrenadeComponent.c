class SAL_DropperGrenadeComponentClass: ScriptComponentClass
{
}

class SAL_DropperGrenadeComponent: ScriptComponent
{
	[Attribute("5.0")] float m_fFuzeTime;
	[Attribute("", UIWidgets.ResourceNamePicker, "Explosion", "et")] ResourceName m_Explosion;
	
	float m_fCurrentFuze = m_fFuzeTime;
	
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
		if (owner.GetParent() != null && owner.GetPhysics().GetSimulationState() != 0)
			owner.GetPhysics().ChangeSimulationState(SimulationState.NONE);
		
		if (owner.GetParent() != null)
			return;
		
		if (owner.GetPhysics().GetSimulationState() == 0)
			owner.GetPhysics().ChangeSimulationState(SimulationState.SIMULATION);
		
		m_fCurrentFuze -= timeSlice;
		
		if (m_fCurrentFuze >= 0)
			return;
		
		vector transform[4];
		owner.GetTransform(transform);
		EntitySpawnParams params = new EntitySpawnParams();
		params.Transform = transform;
		GetGame().SpawnEntityPrefab(Resource.Load(m_Explosion), GetGame().GetWorld(), params);
			
		SCR_EntityHelper.DeleteEntityAndChildren(owner);
	}
}