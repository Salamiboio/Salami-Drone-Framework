class SAL_DroneConnectionManagerClass: ScriptComponentClass
{
}


class SAL_DroneConnectionManager: ScriptComponent
{
	[RplProp()] ref array<int> m_aDroneOwners = {};
	[RplProp()] ref array<RplId> m_aDrones = {};
	[RplProp()] ref array<RplId> m_aJammers = {};
	protected int m_aRotorSpinDir[4] = { 1, -1, -1, 1 };
	
	static SAL_DroneConnectionManager GetInstance()
	{
		if (GetGame().GetGameMode())
			return SAL_DroneConnectionManager.Cast(GetGame().GetGameMode().FindComponent(SAL_DroneConnectionManager));
		else
			return null;
	}
	
	static RplId GetRplId(IEntity entity)
	{
		if (!entity)
			return RplId.Invalid();
		
		RplComponent rplComp = RplComponent.Cast(entity.FindComponent(RplComponent));
		return rplComp.Id();
	}
	
	bool IsDronePlayers(IEntity drone)
	{
		if (m_aDrones.Find(SCR_PlayerController.GetRplId(drone)) == -1)
			return false;
		return m_aDroneOwners.Get(m_aDrones.Find(SCR_PlayerController.GetRplId(drone))) == SCR_PlayerController.GetLocalPlayerId();
	}
	
	bool IsPlayerDroneOwner(int playerId)
	{
		return m_aDroneOwners.Contains(playerId);
	}
	
	IEntity GetPlayersDrone(int playerId)
	{
		if (!m_aDroneOwners.Contains(playerId))
			return null;
		
		RplId droneId = m_aDrones.Get(m_aDroneOwners.Find(playerId));
		return RplComponent.Cast(Replication.FindItem(droneId)).GetEntity();
	}
	
	RplId GetPlayersDroneRplId(int playerId)
	{
		if (!m_aDroneOwners.Contains(playerId))
			return null;

		return m_aDrones.Get(m_aDroneOwners.Find(playerId));
	}
	
	array<RplId> GetConnectedDrones()
	{
		return m_aDrones;
	}
	
	array<int> GetDroneOwners()
	{
		return m_aDroneOwners;
	}
	
	void ConnectToDrone(RplId droneId, int playerId)
	{
		m_aDrones.Insert(droneId);
		m_aDroneOwners.Insert(playerId);
		Replication.BumpMe();
	}
	
	void DisconnectDrone(RplId droneId)
	{
		int index = m_aDrones.Find(droneId);
		m_aDrones.RemoveOrdered(index);
		m_aDroneOwners.RemoveOrdered(index);
		Replication.BumpMe();
	}
	
	void ReplicateTransform(SAL_DroneNetworkPacket packet)
	{
		Rpc(RpcDo_ReplicateTransform, packet);
	}
	
	// === SERVER → ALL CLIENTS ===
	[RplRpc(RplChannel.Unreliable, RplRcver.Broadcast)]
	void RpcDo_ReplicateTransform(SAL_DroneNetworkPacket packet)
	{
		vector transform[4];
		packet.GetTransform(transform);
		
		IEntity drone = RplComponent.Cast(Replication.FindItem(packet.GetDrone())).GetEntity();
		if (!drone)
			return;
		
		if (!SAL_DroneConnectionManager.GetInstance())
			return;
		
		if (!SAL_DroneConnectionManager.GetInstance().IsDronePlayers(drone))  // Only apply if not the controller
		{
			GenericEntity droneEntity = GenericEntity.Cast(drone);
			droneEntity.SetTransform(transform);
			droneEntity.Update();
			droneEntity.OnTransformReset();
			
			RplId rotors[4];
			packet.GetRotors(rotors);
			
			float rotorRPMs[4];
			packet.GetRotorRPMs(rotorRPMs);
			float averageRPM = (rotorRPMs[0] + rotorRPMs[1] + rotorRPMs[2] + rotorRPMs[3]) / 4;
			
			SAL_DroneSoundComponent soundComp = SAL_DroneSoundComponent.Cast(drone.FindComponent(SAL_DroneSoundComponent));
			soundComp.m_fAverageRotorRPM = averageRPM;
			if (!soundComp.m_bEngineOn && packet.GetIsArmed())
				soundComp.StartEngine();
			
			for (int i = 0; i < 4; i++)
			{
				IEntity rotor = RplComponent.Cast(Replication.FindItem(rotors[i])).GetEntity();
				if (!rotor) continue;
				
				float degPerSecond = rotorRPMs[i] * 6.0;
				float rotationAmount = m_aRotorSpinDir[i] * degPerSecond * packet.GetTimeSlice();
				vector oldAngles = rotor.GetLocalAngles();
				oldAngles[1] = oldAngles[1] + rotationAmount;
				rotor.SetAngles(oldAngles);
			}
		}
	}
	
	void DisarmDrone(SAL_DroneNetworkPacket packet)
	{
		Rpc(RpcDo_DisarmDrone, packet);
	}
	
	[RplRpc(RplChannel.Unreliable, RplRcver.Broadcast)]
	void RpcDo_DisarmDrone(SAL_DroneNetworkPacket packet)
	{
		IEntity drone = RplComponent.Cast(Replication.FindItem(packet.GetDrone())).GetEntity();
		if (!drone)
			return;
		
		if (!SAL_DroneConnectionManager.GetInstance().IsDronePlayers(drone))  // Only apply if not the controller
		{
			SAL_DroneSoundComponent soundComp = SAL_DroneSoundComponent.Cast(drone.FindComponent(SAL_DroneSoundComponent));
			
			if (soundComp.m_bEngineOn && !packet.GetIsArmed())
				soundComp.ShutOffEngine();
			
			SAL_DroneControllerComponent.Cast(drone.FindComponent(SAL_DroneControllerComponent)).m_bIsArmed = packet.GetIsArmed();
		}
	}
	
	void RegisterJammer(RplId ownerId)
	{
		m_aJammers.Insert(ownerId);
		Replication.BumpMe();
	}
	
	void UnregisterJammer(RplId ownerId)
	{
		int index = m_aJammers.Find(ownerId);
		if (index == -1)
			return;
		
		m_aJammers.RemoveOrdered(index);
		Replication.BumpMe();
	}
}