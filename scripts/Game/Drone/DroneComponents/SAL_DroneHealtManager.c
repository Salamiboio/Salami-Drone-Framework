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
	RplId m_DroneId;
	World m_World;
	SCR_DamageManagerComponent m_DamageManager;
	SAL_DroneControllerComponent m_DroneController;
	RplId m_Id;
	
	float m_fRplDroneHealth = 100;
	
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.CONTACT | EntityEvent.FIXEDFRAME | EntityEvent.INIT);
		
	}
	
	override void EOnInit(IEntity owner)
	{
		m_DroneManager = SAL_DroneConnectionManager.GetInstance();
		m_DroneId = RplComponent.Cast(owner.FindComponent(RplComponent)).Id();
		m_World = GetGame().GetWorld();
		m_DamageManager = SCR_DamageManagerComponent.Cast(owner.FindComponent(SCR_DamageManagerComponent));
		m_DroneController = SAL_DroneControllerComponent.Cast(owner.FindComponent(SAL_DroneControllerComponent));
		m_Id = m_DroneManager.GetRplId(owner);
	}
	
	override void EOnFixedFrame(IEntity owner, float timeSlice)
	{
		if (m_Id == -1)
			m_Id = RplComponent.Cast(owner.FindComponent(RplComponent)).Id();
		if (System.IsConsoleApp())
		{
			if (m_DroneController.m_bIsTriggered)
				return;
			
			if (m_DamageManager.IsDestroyed() && !m_bIsDestroyed)
			{
				m_bIsDestroyed = true;
				if (m_DroneManager.m_aActiveDrones.Contains(m_Id))
					AwardXPForKill();
				m_DroneManager.DestroyDroneServer(m_Id, m_sDroneWreckPrefab, m_DamageManager.GetInstigator());
			}
		}
		
		if (!owner) 
			return;
		
		if (!m_DroneManager)
			return;
		
		if (!m_DroneController.m_bIsActive)
			return;
		
		if (!m_DroneManager.m_aActiveDrones.Contains(m_Id))
		{
			if (!Replication.IsServer())
				return;
		}
		else
		{
			if (m_DroneController.m_iOwner != SCR_PlayerController.GetLocalPlayerId())
				return;
		}
		
		if (m_DroneController.m_bIsTriggered)
			return;
		
		EWaterSurfaceType waterType;
		
		GetWaterSurfaceY(owner.GetOrigin(), waterType);
		
		if ((m_fRplDroneHealth <= 0 && !m_bIsDestroyed) || (waterType != EWaterSurfaceType.WST_NONE && !m_bIsDestroyed))
		{
			m_bIsDestroyed = true;
			if (!System.IsConsoleApp())
				SCR_PlayerController.Cast(GetGame().GetPlayerController()).DestroyDroneClient(m_Id, m_sDroneWreckPrefab, SCR_PlayerController.GetLocalPlayerId());
			else
				m_DroneManager.DestroyDroneHealth(m_Id, m_sDroneWreckPrefab, SCR_PlayerController.GetLocalPlayerId());
		}
		
	}
	
	private void GetWaterSurfaceY(vector worldPos, out EWaterSurfaceType waterSurfaceType)
	{		
		vector waterSurfacePos;
		vector transformWS[4];
		vector obbExtents;

		ChimeraWorldUtils.TryGetWaterSurface(m_World, worldPos, waterSurfacePos, waterSurfaceType, transformWS, obbExtents);
		
		return;
	}

	override void EOnContact(IEntity owner, IEntity other, Contact contact)
	{
		if (!owner) 
			return;
		
		if (!m_DroneManager)
			return;
		
		if (!m_DroneController.m_bIsActive)
			return;
		
		if (!m_DroneManager.m_aActiveDrones.Contains(m_Id))
		{
			if (RplSession.Mode() == RplMode.Client)
				return;
		}
		else
		{
			if (m_DroneController.m_iOwner != SCR_PlayerController.GetLocalPlayerId())
				return;
		}

		vector contactNormal = contact.Normal.Normalized();
		vector droneUp = owner.GetTransformAxis(1);  // drone's local 'up' direction
		
		// Dot will be close to 1.0 if impact is from below (ground, landing)
		float upDot = vector.Dot(contactNormal, droneUp);
		
		if (upDot > 0.2)  // Impact from below (e.g. landing)
			return;
		float impactSpeed = contact.GetRelativeNormalVelocityAfter() - contact.GetRelativeNormalVelocityBefore();
		if (impactSpeed < 5)
			return;
		
		float damage = m_fDamageBase * impactSpeed * m_fDamagePower;
		damage = Math.Clamp(damage, 0.0, 100.0);
		
		SAL_DroneNetworkPacket packet = new SAL_DroneNetworkPacket();
		packet.SetDrone(m_Id);
		packet.SetHealth(m_fRplDroneHealth - damage);
		
		if (!System.IsConsoleApp())
			SCR_PlayerController.Cast(GetGame().GetPlayerController()).UpdateDroneHealth(packet);
		else
			m_DroneManager.BroadcastHealthUpdate(packet);
	}
	
	void AwardXPForKill()
	{
		SCR_XPHandlerComponent compXP = SCR_XPHandlerComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_XPHandlerComponent));

		if (compXP)
		{
			if (m_DamageManager.GetInstigator() == null)
				return;
			
			int killerId = m_DamageManager.GetInstigator().GetInstigatorPlayerID();
			
			if (!WasFriendlyFire())
				compXP.AwardXP(killerId, SCR_EXPRewards.ENEMY_KILL, 1);		
		}
			
	}
	
	bool WasFriendlyFire()
	{
		int droneOwnerId = m_DroneManager.GetDronesOwner(m_Id);
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		Faction droneFaction = factionManager.GetPlayerFaction(droneOwnerId);
		Faction killerFaction = factionManager.GetPlayerFaction(m_DamageManager.GetInstigator().GetInstigatorPlayerID());
		
		if (droneFaction == killerFaction)
			return true;
		else
			return false;
	}
}