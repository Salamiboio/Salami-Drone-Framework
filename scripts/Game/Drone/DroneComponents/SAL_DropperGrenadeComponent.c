class SAL_DropperGrenadeComponentClass: ScriptComponentClass
{
}

class SAL_DropperGrenadeComponent: ScriptComponent
{
	[Attribute("5.0")] float m_fFuzeTime;
	[Attribute("0")] bool m_bImpactFuze;
	[Attribute("", UIWidgets.ResourceNamePicker, "Explosion", "et")] ResourceName m_Explosion;
	
	float m_fCurrentFuze = m_fFuzeTime;
	Instigator m_Instagator;
	IEntity m_eDrone;
	
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
		
		if (!m_Instagator)
		{
			if (!m_eDrone)
				GetGame().GetWorld().QueryEntitiesBySphere(owner.GetOrigin(), 0.5, FilterDrone, null);
			else
			{
				SlotManagerComponent slotComp = SlotManagerComponent.Cast(owner.FindComponent(SlotManagerComponent));
				if (!slotComp.GetSlotByName("Explosion"))
					return;
		
				IEntity explosion = slotComp.GetSlotByName("Explosion").GetAttachedEntity();
				if (!explosion)
					return;
			
				SCR_ExplosiveTriggerComponent explComp = SCR_ExplosiveTriggerComponent.Cast(explosion.FindComponent(SCR_ExplosiveTriggerComponent));
				explComp.GetInstigator().SetInstigatorByPlayerID(SAL_DroneConnectionManager.GetInstance().GetDronesOwner(m_eDrone));
				m_Instagator = explComp.GetInstigator();
			}
		}
		
		if (m_bImpactFuze)
		{
			if (IsOnGround(owner))
				ExplodeGrenade(owner);
			
			return;
		}
		else
		{
			m_fCurrentFuze -= timeSlice;
		
			if (m_fCurrentFuze >= 0)
				return;
		
			ExplodeGrenade(owner);
		}
	}
	
	bool FilterDrone(IEntity ent)
	{
		if (ent == GetOwner())
			return true;
		
		if (ent.FindComponent(SAL_DroneControllerComponent))
			m_eDrone = ent;
		return true;
	}
	
	bool IsOnGround(IEntity owner)
	{
		vector origin = owner.GetOrigin();
		vector end = origin + Vector(0, -1, 0) * 1000.0; // cast 1000 meters down
			
		TraceParam trace = new TraceParam();
		trace.Start = origin;
		trace.End = end;
		trace.Exclude = owner; // prevent hitting self
		trace.Flags = TraceFlags.WORLD;
			
		float height = GetGame().GetWorld().TraceMove(trace, null) * 10000;
		return height <= 1.015;
	}
	
	void ExplodeGrenade(IEntity owner)
	{
		SlotManagerComponent slotComp = SlotManagerComponent.Cast(owner.FindComponent(SlotManagerComponent));
		if (!slotComp.GetSlotByName("Explosion"))
			return;
		
		IEntity explosion = slotComp.GetSlotByName("Explosion").GetAttachedEntity();
		if (!explosion)
			return;
			
		SCR_ExplosiveTriggerComponent explComp = SCR_ExplosiveTriggerComponent.Cast(explosion.FindComponent(SCR_ExplosiveTriggerComponent));
		explComp.UseTrigger();
		GetGame().GetCallqueue().CallLater(SCR_EntityHelper.DeleteEntityAndChildren, 1000, false, owner);
	}
}