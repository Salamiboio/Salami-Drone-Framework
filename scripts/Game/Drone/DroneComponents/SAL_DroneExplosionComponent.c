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
	RplId m_Id;
	
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.CONTACT);
		m_DroneManager = SAL_DroneConnectionManager.GetInstance();
		m_DroneController = SAL_DroneControllerComponent.Cast(owner.FindComponent(SAL_DroneControllerComponent));
		m_Id = m_DroneManager.GetRplId(owner);
	}
	
	override event protected void EOnContact(IEntity owner, IEntity other, Contact contact)
	{
		if (m_Id == -1)
			m_Id = RplComponent.Cast(owner.FindComponent(RplComponent)).Id();
		
		if (!owner) 
			return;
		
		if (!m_DroneManager)
			return;
		
		if (!m_DroneManager.m_aActiveDrones.Contains(m_Id))
		{
			if (RplSession.Mode() == RplMode.Client)
				return;
		}
		else
		{
			if (m_DroneController.m_iOwner != SCR_PlayerController.GetLocalPlayerId() )
				return;
		}	

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