class SAL_DropperGrenadeComponentClass: ScriptComponentClass
{
}

class SAL_DropperGrenadeComponent: ScriptComponent
{
	[Attribute("5.0")] float m_fFuzeTime;
	[Attribute("0")] bool m_bImpactFuze;
	
	float m_fCurrentFuze = m_fFuzeTime;
	float m_fLastHeight = 10000;
	Instigator m_Instagator;
	IEntity m_eDrone;
	float m_fDropperBuffer = 0;
	
	override void OnPostInit(IEntity owner)
	{
		#ifdef WORKBENCH
		if (!m_bImpactFuze)
			SetEventMask(owner, EntityEvent.FRAME);
		else
			SetEventMask(owner, EntityEvent.FRAME | EntityEvent.CONTACT);
		#else
		if (RplSession.Mode() == RplMode.Client)
			return;
		
		if (!m_bImpactFuze)
			SetEventMask(owner, EntityEvent.FRAME);
		else
			SetEventMask(owner, EntityEvent.FRAME | EntityEvent.CONTACT);
		#endif
		
	}
	
	override void EOnFrame(IEntity owner, float timeSlice)
	{

		if (owner.GetParent() != null)
			return;	

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
				explComp.GetInstigator().SetInstigatorByPlayerID(SCR_PlayerController.GetLocalPlayerId());
				m_Instagator = explComp.GetInstigator();
			}
		}

		if (m_bImpactFuze)
		{
			m_fDropperBuffer += timeSlice;
			return;
		}
		
		m_fCurrentFuze -= timeSlice;
		
		if (m_fCurrentFuze >= 0)
			return;
		
		ExplodeGrenade(owner);
	}
	
	override void EOnContact(IEntity owner, IEntity other, Contact contact)
	{
		if (m_fDropperBuffer <= 0.5)
			return;
		
		ExplodeGrenade(owner);
	}
	
	bool FilterDrone(IEntity ent)
	{
		if (ent == GetOwner())
			return true;
		
		if (ent.FindComponent(SAL_DroneControllerComponent))
			m_eDrone = ent;
		return true;
	}
	
	void ExplodeGrenade(IEntity owner)
	{
		if (!owner)
			return;
		SCR_ExplosiveTriggerComponent explComp = SCR_ExplosiveTriggerComponent.Cast(owner.FindComponent(SCR_ExplosiveTriggerComponent));
		if (explComp)
			explComp.UseTrigger();
		GetGame().GetCallqueue().CallLater(SCR_EntityHelper.DeleteEntityAndChildren, 1000, false, owner);
	}
}