class SAL_DropperGrenadeComponentClass: ScriptComponentClass
{
}

class SAL_DropperGrenadeComponent: ScriptComponent
{
	[Attribute("5.0")] float m_fFuzeTime;
	[Attribute("0")] bool m_bImpactFuze;
	[Attribute("0")] bool m_bIsMine;
	[Attribute("0")] bool m_bIsFlare;
	[Attribute("", params: "et")] ResourceName m_rMine;
	
	float m_fCurrentFuze = m_fFuzeTime;
	float m_fLastHeight = 10000;
	Instigator m_Instagator;
	IEntity m_eDrone;
	float m_fDropperBuffer = 0;
	
	override void OnPostInit(IEntity owner)
	{
		#ifdef WORKBENCH
		if (!m_bImpactFuze || m_bIsMine)
			SetEventMask(owner, EntityEvent.FRAME| EntityEvent.INIT);
		else
			SetEventMask(owner, EntityEvent.FRAME | EntityEvent.CONTACT| EntityEvent.INIT);
		#else
		if (!Replication.IsServer())
			return;
		
		if (!m_bImpactFuze || m_bIsMine)
			SetEventMask(owner, EntityEvent.FRAME | EntityEvent.INIT);
		else
			SetEventMask(owner, EntityEvent.FRAME | EntityEvent.CONTACT| EntityEvent.INIT);
		#endif
		
	}
	
	override void EOnFrame(IEntity owner, float timeSlice)
	{

		if (owner.GetParent() != null)
			return;	
		
		if (m_bIsFlare)
		{
			SCR_ExplosiveTriggerComponent explComp = SCR_ExplosiveTriggerComponent.Cast(owner.FindComponent(SCR_ExplosiveTriggerComponent));
			if (explComp)
				explComp.UseTrigger();
			GetGame().GetCallqueue().CallLater(SCR_EntityHelper.DeleteEntityAndChildren, 1000, false, owner);
			return;
		}

		if (!m_Instagator)
		{
			if (!m_eDrone)
				GetGame().GetWorld().QueryEntitiesBySphere(owner.GetOrigin(), 0.5, FilterDrone, null);
			else if (!m_bIsMine)
			{
				SCR_ExplosiveTriggerComponent explComp = SCR_ExplosiveTriggerComponent.Cast(owner.FindComponent(SCR_ExplosiveTriggerComponent));
				SAL_DroneControllerComponent droneController = SAL_DroneControllerComponent.Cast(m_eDrone.FindComponent(SAL_DroneControllerComponent));
				
				explComp.GetInstigator().SetInstigatorByPlayerID(droneController.m_iOwner);
				m_Instagator = explComp.GetInstigator();
			}
		}
		
		if (m_bIsMine)
		{
			MineCheck(owner);
			return;
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
	
	void MineCheck(IEntity owner)
	{
		vector transform[4];
		owner.GetTransform(transform);
		vector newPos;
		SCR_TerrainHelper.SnapToGeometry(newPos, transform[3], null);
		
		if (vector.Distance(newPos, transform[3]) < 0.1)
		{
			transform[3] = newPos;
			EntitySpawnParams params = new EntitySpawnParams();
			params.Transform = transform;
			IEntity newMine = GetGame().SpawnEntityPrefab(Resource.Load(m_rMine), null, params);
			SCR_PressureTriggerComponent triggerComp = SCR_PressureTriggerComponent.Cast(newMine.FindComponent(SCR_PressureTriggerComponent));
			if (!triggerComp)
				return;

			SAL_DroneControllerComponent droneController = SAL_DroneControllerComponent.Cast(m_eDrone.FindComponent(SAL_DroneControllerComponent));
				
			triggerComp.GetInstigator().SetInstigatorByPlayerID(droneController.m_iOwner);
			triggerComp.ActivateTrigger();
			if (newMine)
				SCR_EntityHelper.DeleteEntityAndChildren(owner);
		}
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
		
		vector transform[4];
		owner.GetTransform(transform);
		vector newPos;
		SCR_TerrainHelper.SnapToGeometry(newPos, transform[3], null);
		transform[3] = newPos;
		transform[3][1] = transform[3][1] + 0.2;
		owner.SetWorldTransform(transform);
		owner.Update();
		GenericEntity.Cast(owner).OnTransformReset();
		
		SCR_ExplosiveTriggerComponent explComp = SCR_ExplosiveTriggerComponent.Cast(owner.FindComponent(SCR_ExplosiveTriggerComponent));
		if (explComp)
			explComp.UseTrigger();
		GetGame().GetCallqueue().CallLater(SCR_EntityHelper.DeleteEntityAndChildren, 1000, false, owner);
	}
}