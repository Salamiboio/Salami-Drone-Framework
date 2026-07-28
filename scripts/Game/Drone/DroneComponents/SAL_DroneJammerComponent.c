class SAL_DroneJammerComponentClass: ScriptComponentClass
{
}

class SAL_DroneJammerComponent: ScriptComponent
{
	[Attribute("300")] float m_fJammingRange;
	RplId m_Id;

	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT);
	}
	
	override void EOnInit(IEntity owner)
	{
		#ifdef WORKBENCH
		GetGame().GetCallqueue().CallLater(RegisterJammer, 100, false, owner);
		#else
		if (!Replication.IsServer())
			return;
		
		GetGame().GetCallqueue().CallLater(RegisterJammer, 100, false, owner);
		#endif
	}
	
	void RegisterJammer(IEntity owner)
	{
		SAL_DroneConnectionManager.GetInstance().RegisterJammer(RplComponent.Cast(owner.FindComponent(RplComponent)).Id());
		m_Id = RplComponent.Cast(owner.FindComponent(RplComponent)).Id();
	}
	
	void ~SAL_DroneJammerComponent()
	{
		#ifdef WORKBENCH
		
		if (!GetGame().GetWorld())
			return;
		
		if(SAL_DroneConnectionManager.GetInstance())
				SAL_DroneConnectionManager.GetInstance().UnregisterJammer(m_Id);
		#else
		if (!Replication.IsServer())
			return;
		
		if (!GetGame().GetWorld())
			return;
		
		if(SAL_DroneConnectionManager.GetInstance())
				SAL_DroneConnectionManager.GetInstance().UnregisterJammer(m_Id);
		#endif
	}
}