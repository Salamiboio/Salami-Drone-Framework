class SAL_DroneExplosionComponentClass: ScriptComponentClass
{
}

class SAL_DroneExplosionComponent: ScriptComponent
{
	const float HIT_FRONT_THRESHOLD = 0.5; // ~45 degrees cone
	SAL_DroneConnectionManager m_DroneManager;
	SAL_DroneControllerComponent m_DroneController;
	bool m_bIsArmed = false;
	[Attribute(desc: "Explosion effect for destruction", params: "et")] ResourceName m_sExplosionEffect;
	
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.CONTACT);
		m_DroneManager = SAL_DroneConnectionManager.GetInstance();
		m_DroneController = SAL_DroneControllerComponent.Cast(owner.FindComponent(SAL_DroneControllerComponent));
	}
	
	override event protected void EOnContact(IEntity owner, IEntity other, Contact contact)
	{
		if (!owner) 
			return;
		
		if (!m_DroneManager)
			return;
		
		if (!m_DroneManager.IsDronePlayers(owner))
			return;

		if (!m_bIsArmed)
			return;
		
		vector forward = owner.GetTransformAxis(2); // Z = forward
		vector impactDir = -contact.Normal.Normalized(); // Impact direction is opposite contact normal

		float dot = vector.Dot(forward.Normalized(), impactDir.Normalized());

		if (dot > HIT_FRONT_THRESHOLD)
		{
			m_DroneController.m_bIsTriggered = true;
		}
	}
}