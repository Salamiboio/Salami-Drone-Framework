class SAL_DroneJammerComponentClass: ScriptComponentClass
{
}

class SAL_DroneJammerComponent: ScriptComponent
{
	[Attribute("300")] float m_fJammingRange;

	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT);
	}
	
	override void EOnInit(IEntity owner)
	{
		#ifdef WORKBENCH
		GetGame().GetCallqueue().CallLater(RegisterJammer, 100, false, owner);
		#else
		if (RplSession.Mode() != RplMode.Dedicated)
			return;
		GetGame().GetCallqueue().CallLater(RegisterJammer, 100, false, owner);
		#endif
	}
	
	void RegisterJammer(IEntity owner)
	{
		SAL_DroneConnectionManager.GetInstance().RegisterJammer(RplComponent.Cast(owner.FindComponent(RplComponent)).Id());
		
	}
	
	void ~SAL_DroneJammerComponent()
	{
		#ifdef WORKBENCH
		
		if (!GetGame().GetWorld())
			return;
		
		if(SAL_DroneConnectionManager.GetInstance())
			SAL_DroneConnectionManager.GetInstance().UnregisterJammer(RplComponent.Cast(GetOwner().FindComponent(RplComponent)).Id());
		#else
		if (RplSession.Mode() != RplMode.Dedicated)
			return;
		
		if (!GetGame().GetWorld())
			return;
		
		if(SAL_DroneConnectionManager.GetInstance())
			SAL_DroneConnectionManager.GetInstance().UnregisterJammer(RplComponent.Cast(GetOwner().FindComponent(RplComponent)).Id());
		#endif
	}
}