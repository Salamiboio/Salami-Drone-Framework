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
	}
	
	override void EOnFixedFrame(IEntity owner, float timeSlice)
	{
		#ifdef WORKBENCH
		if (m_DamageManager.IsDestroyed() && !m_bIsDestroyed)
		{
			m_bIsDestroyed = true;
			if (m_DroneManager.GetDronesOwner(GetOwner()) != -1)
				AwardXPForKill();
			m_DroneManager.DestroyDroneServer(RplComponent.Cast(owner.FindComponent(RplComponent)).Id(), m_sDroneWreckPrefab);
		}
		#else
		if (RplSession.Mode() != RplMode.Client)
		{
			if (m_DamageManager.IsDestroyed() && !m_bIsDestroyed)
			{
				m_bIsDestroyed = true;
				if (m_DroneManager.GetDronesOwner(GetOwner()) != -1)
					AwardXPForKill();
				m_DroneManager.DestroyDroneServer(RplComponent.Cast(owner.FindComponent(RplComponent)).Id(), m_sDroneWreckPrefab);
			}
		}
		#endif
		
		if (!owner) 
			return;
		
		if (!m_DroneManager)
			return;
		
		if (!m_DroneManager.IsDronePlayers(owner))
			return;
		
		EWaterSurfaceType waterType;
		
		GetWaterSurfaceY(owner.GetOrigin(), waterType);
		
		if ((m_fRplDroneHealth <= 0 && !m_bIsDestroyed) || (waterType != EWaterSurfaceType.WST_NONE && !m_bIsDestroyed))
		{
			m_bIsDestroyed = true;
			SCR_PlayerController.Cast(GetGame().GetPlayerController()).DisconnectDrone();
			SCR_PlayerController.Cast(GetGame().GetPlayerController()).DestroyDrone(RplComponent.Cast(owner.FindComponent(RplComponent)).Id(), m_sDroneWreckPrefab);
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
		
		if (!m_DroneManager.IsDronePlayers(owner))
			return;

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
		packet.SetDrone(RplComponent.Cast(owner.FindComponent(RplComponent)).Id());
		packet.SetHealth(m_fRplDroneHealth - damage);
		
		SCR_PlayerController.Cast(GetGame().GetPlayerController()).UpdateDroneHealth(packet);
	}
	
	void AwardXPForKill()
	{
		SCR_XPHandlerComponent compXP = SCR_XPHandlerComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_XPHandlerComponent));

		if (compXP)
		{
			if (m_DamageManager.GetInstigator() == null)
				return;
			
			int killerId = m_DamageManager.GetInstigator().GetInstigatorPlayerID();
			
			if (WasFriendlyFire())
				compXP.AwardXP(killerId, SCR_EXPRewards.FRIENDLY_KILL, 1);
			else
				compXP.AwardXP(killerId, SCR_EXPRewards.ENEMY_KILL, 1);
		}
			
	}
	
	bool WasFriendlyFire()
	{
		int droneOwnerId = m_DroneManager.GetDronesOwner(GetOwner());
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		Faction droneFaction = factionManager.GetPlayerFaction(droneOwnerId);
		Faction killerFaction = factionManager.GetPlayerFaction(m_DamageManager.GetInstigator().GetInstigatorPlayerID());
		
		if (droneFaction == killerFaction)
			return true;
		else
			return false;
	}
}