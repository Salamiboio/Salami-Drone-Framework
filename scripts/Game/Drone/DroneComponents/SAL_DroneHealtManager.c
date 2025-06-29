class SAL_DroneHealthComponentClass: ScriptComponentClass
{

}

class SAL_DroneHealthComponent: ScriptComponent
{
	[Attribute("100.0")] float m_fDroneHealth;
	[Attribute("1")] float m_fDamageBase; 
	[Attribute("2.2")] float m_fDamagePower;    
	[Attribute(desc: "Used when drone is destroyed", params: "et")] ResourceName m_sDroneWreckPrefab;
	SAL_DroneConnectionManager m_DroneManager;
	bool m_bIsDestroyed = false;
	IEntity m_eDroneWreck;
	
	[RplProp()] float m_fRplDroneHealth = 100;
	
	override void OnPostInit(IEntity owner)
	{
		#ifdef WORKBENCH
		
		SetEventMask(owner, EntityEvent.CONTACT | EntityEvent.FIXEDFRAME);
		m_DroneManager = SAL_DroneConnectionManager.GetInstance();
		
		#else
		
		if (RplSession.Mode() != RplMode.Dedicated)
			return;
		SetEventMask(owner, EntityEvent.CONTACT | EntityEvent.FIXEDFRAME);
		m_DroneManager = SAL_DroneConnectionManager.GetInstance();
		
		#endif
	}
	
	void UpdateHealth(float input)
	{
		m_fRplDroneHealth = input;
		Replication.BumpMe();
	}
	
	override void EOnFixedFrame(IEntity owner, float timeSlice)
	{
		if (m_fRplDroneHealth <= 0 && !m_bIsDestroyed)
		{
			vector transform[4];
			owner.GetTransform(transform);
			EntitySpawnParams params = new EntitySpawnParams();
			params.Transform = transform;
			IEntity newDrone = GetGame().SpawnEntityPrefab(Resource.Load(m_sDroneWreckPrefab), GetGame().GetWorld(), params);	
			
			SAL_DroneConnectionManager connectionManager = SAL_DroneConnectionManager.GetInstance();
			RplId droneId = RplComponent.Cast(owner.FindComponent(RplComponent)).Id();
			
			if (connectionManager.m_aDrones.Contains(droneId))
				connectionManager.DisconnectDrone(droneId);		
			GetGame().GetCallqueue().CallLater(DeleteDrone, 200, false, owner.GetPhysics().GetVelocity(), newDrone);
			m_bIsDestroyed = true;
		}
	}

	override void EOnContact(IEntity owner, IEntity other, Contact contact)
	{
		
		vector contactNormal = contact.Normal.Normalized();
		vector droneUp = owner.GetTransformAxis(1);  // drone's local 'up' direction
		
		// Dot will be close to 1.0 if impact is from below (ground, landing)
		float upDot = vector.Dot(contactNormal, droneUp);
		if (upDot > 0.2)  // Impact from below (e.g. landing)
			return;
		float impactSpeed = contact.GetRelativeNormalVelocityAfter() - contact.GetRelativeNormalVelocityBefore();
		if (impactSpeed < 5)
			return;
		
		float damage = m_fDamageBase * Math.Pow(impactSpeed, m_fDamagePower);
		damage = Math.Clamp(damage, 0.0, 100.0);
		
		UpdateHealth(m_fRplDroneHealth - damage);
	}
	
	void DeleteDrone(vector linearVelocity, IEntity newDrone)
	{
		if (!newDrone)
			return;
		
		newDrone.GetPhysics().SetVelocity(linearVelocity);
		SCR_EntityHelper.DeleteEntityAndChildren(GetOwner());
	}
}