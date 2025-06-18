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
	
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.CONTACT);
		m_DroneManager = SAL_DroneConnectionManager.GetInstance();
	}
	
	override void EOnContact(IEntity owner, IEntity other, Contact contact)
	{
		if (m_bIsDestroyed)
			return;
		vector contactNormal = contact.Normal;
		vector droneDown = -owner.GetTransformAxis(1);

		float downDot = vector.Dot(contactNormal.Normalized(), droneDown);
		if (downDot > 0.2)
			return;
		float impactSpeed = contact.GetRelativeNormalVelocityAfter() - contact.GetRelativeNormalVelocityBefore();
		if (impactSpeed < 1)
			return;
		
		float damage = m_fDamageBase * Math.Pow(impactSpeed, m_fDamagePower);
		damage = Math.Clamp(damage, 0.0, 100.0);
		
		m_fDroneHealth -= damage;
		
		if (m_fDroneHealth <= 0)
		{
			vector transform[4];
			owner.GetTransform(transform);
			EntitySpawnParams params = new EntitySpawnParams();
			params.Transform = transform;
			IEntity newDrone = GetGame().SpawnEntityPrefab(Resource.Load(m_sDroneWreckPrefab), GetGame().GetWorld(), params);		
			GetGame().GetCallqueue().CallLater(DeleteDrone, 200, false, owner.GetPhysics().GetVelocity(), newDrone);
			m_bIsDestroyed = true; 
		}
	}
	
	void DeleteDrone(vector linearVelocity, IEntity newDrone)
	{
		if (!newDrone)
			return;
		
		newDrone.GetPhysics().SetVelocity(linearVelocity);
		SCR_EntityHelper.DeleteEntityAndChildren(GetOwner());
	}
}