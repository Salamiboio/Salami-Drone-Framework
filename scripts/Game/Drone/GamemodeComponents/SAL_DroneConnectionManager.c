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
		
		if (Replication.FindItem(droneId))
			return RplComponent.Cast(Replication.FindItem(droneId)).GetEntity();
		else
			return null;
	}
	
	int GetDronesOwner(IEntity drone)
	{
		RplComponent rplComp = RplComponent.Cast(drone.FindComponent(RplComponent));
		RplId droneId = rplComp.Id();
		
		if (!m_aDrones.Contains(droneId))
			return -1;
		
		return m_aDroneOwners.Get(m_aDrones.Find(droneId));
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
		
		if (Replication.FindItem(droneId))
		{
			RplComponent rplComp = RplComponent.Cast(Replication.FindItem(droneId));
			RplIdentity rplIdentity = GetGame().GetPlayerManager().GetPlayerController(playerId).GetRplIdentity();
			rplComp.EnableStreamingConNode(rplIdentity, true);
		}
		
		Replication.BumpMe();
	}
	
	void DisconnectDrone(RplId droneId)
	{
		int index = m_aDrones.Find(droneId);
		if (index == -1)
			return;
		
		if (Replication.FindItem(droneId))
		{
			RplComponent rplComp = RplComponent.Cast(Replication.FindItem(droneId));
			RplIdentity rplIdentity = GetGame().GetPlayerManager().GetPlayerController(m_aDroneOwners.Get(index)).GetRplIdentity();
			rplComp.EnableStreamingConNode(rplIdentity, false);
 		}
		
		m_aDrones.RemoveOrdered(index);
		m_aDroneOwners.RemoveOrdered(index);
		Replication.BumpMe();
	}
	
	void ExplodeDroneServer(SAL_DroneNetworkPacket packet)
	{
		if (!Replication.FindItem(packet.GetDrone()))
			return;
		
		IEntity drone = RplComponent.Cast(Replication.FindItem(packet.GetDrone())).GetEntity();
		
		vector transform[4];
		packet.GetTransform(transform);
		EntitySpawnParams params = new EntitySpawnParams();
		params.Transform = transform;
		GetGame().SpawnEntityPrefab(Resource.Load(packet.GetExplosion()), GetGame().GetWorld(), params);
		
		DisconnectDrone(packet.GetDrone());
		SCR_EntityHelper.DeleteEntityAndChildren(drone);
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
		
		if (!Replication.FindItem(packet.GetDrone()))
			return;
		
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
			
			if (packet.GetIsArmed())
			{
				for (int i = 0; i < 4; i++)
				{
					if (!Replication.FindItem(rotors[i]))
						 continue;
					
					IEntity rotor = RplComponent.Cast(Replication.FindItem(rotors[i])).GetEntity();
					if (!rotor) continue;
					
					float degPerSecond = rotorRPMs[i] * 6.0;
					float rotationAmount = m_aRotorSpinDir[i] * degPerSecond * packet.GetTimeSlice();
					vector oldAngles = rotor.GetLocalAngles();
					oldAngles[1] = oldAngles[1] + rotationAmount;
					rotor.SetAngles(oldAngles);
				}
			}
			SAL_DroneBatteryComponent.Cast(droneEntity.FindComponent(SAL_DroneBatteryComponent)).m_fCurrentBattery = packet.GetBatteryLevel();
			SAL_DroneControllerComponent.Cast(drone.FindComponent(SAL_DroneControllerComponent)).m_bIsArmed = packet.GetIsArmed();
			SAL_DroneControllerComponent.Cast(drone.FindComponent(SAL_DroneControllerComponent)).m_vLastBroadcastedPosition1 = transform[0];
			SAL_DroneControllerComponent.Cast(drone.FindComponent(SAL_DroneControllerComponent)).m_vLastBroadcastedPosition2 = transform[1];
			SAL_DroneControllerComponent.Cast(drone.FindComponent(SAL_DroneControllerComponent)).m_vLastBroadcastedPosition3 = transform[2];
			SAL_DroneControllerComponent.Cast(drone.FindComponent(SAL_DroneControllerComponent)).m_vLastBroadcastedPosition4 = transform[3];
		}
	}
	
	void DisarmDrone(SAL_DroneNetworkPacket packet)
	{
		Rpc(RpcDo_DisarmDrone, packet);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_DisarmDrone(SAL_DroneNetworkPacket packet)
	{
		if (!Replication.FindItem(packet.GetDrone()))
			return;
		
		IEntity drone = RplComponent.Cast(Replication.FindItem(packet.GetDrone())).GetEntity();
		if (!drone)
			return;
		
		if (!SAL_DroneConnectionManager.GetInstance().IsDronePlayers(drone))  // Only apply if not the controller
			SAL_DroneControllerComponent.Cast(drone.FindComponent(SAL_DroneControllerComponent)).m_bIsArmed = packet.GetIsArmed();
		
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
	
	void UpdateBattery(SAL_DroneNetworkPacket packet)
	{
		Rpc(RpcDo_UpdateBattery, packet);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_UpdateBattery(SAL_DroneNetworkPacket packet)
	{
		if (!Replication.FindItem(packet.GetDrone()))
			return;
		
		IEntity drone = RplComponent.Cast(Replication.FindItem(packet.GetDrone())).GetEntity();
		if (!drone)
			return;
		
		if (!SAL_DroneConnectionManager.GetInstance().IsDronePlayers(drone))  // Only apply if not the controller
		{
			SAL_DroneBatteryComponent battComp = SAL_DroneBatteryComponent.Cast(drone.FindComponent(SAL_DroneBatteryComponent));
			if (!battComp)
			return;
		
			battComp.m_fCurrentBattery = packet.GetBatteryLevel();
			battComp.m_fCurrentBatteryMax = packet.GetBatteryLevel();
		}
	}
	
	void BroadcastHealthUpdate(SAL_DroneNetworkPacket packet)
	{
		RpcDo_BroadcastHealthUpdate(packet);
		Rpc(RpcDo_BroadcastHealthUpdate, packet);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_BroadcastHealthUpdate(SAL_DroneNetworkPacket packet)
	{
		if (!Replication.FindItem(packet.GetDrone()))
			return;
		
		IEntity drone = RplComponent.Cast(Replication.FindItem(packet.GetDrone())).GetEntity();
		if (!drone)
			return;
		
		SAL_DroneHealthComponent healthComp = SAL_DroneHealthComponent.Cast(drone.FindComponent(SAL_DroneHealthComponent));
		if (!healthComp)
			return;
		
		healthComp.m_fRplDroneHealth = packet.GetHealth();
	}
	
	void DeleteDrone(vector linearVelocity, IEntity newDrone, IEntity oldDrone)
	{
		if (!newDrone)
			return;
		
		if (newDrone.GetPhysics())
			newDrone.GetPhysics().SetVelocity(linearVelocity);
		SCR_EntityHelper.DeleteEntityAndChildren(oldDrone);
	}
	
	void DestroyDroneServer(RplId droneId, ResourceName droneWreckPrefab)
	{
		if (!Replication.FindItem(droneId))
			return;
		
		IEntity drone = RplComponent.Cast(Replication.FindItem(droneId)).GetEntity();
		if (!drone)
			return;
		
		if (drone.FindComponent(SAL_DroneExplosionComponent))
		{
			SAL_DroneExplosionComponent explComp = SAL_DroneExplosionComponent.Cast(drone.FindComponent(SAL_DroneExplosionComponent));
			string explosion = explComp.m_sExplosionEffect;
			vector transform[4];
			drone.GetTransform(transform);
			EntitySpawnParams params = new EntitySpawnParams();
			params.Transform = transform;
			GetGame().SpawnEntityPrefab(Resource.Load(explosion), GetGame().GetWorld(), params);
			
			Rpc(RpcDo_DestroyDroneServer, droneId);
			GetGame().GetCallqueue().CallLater(SCR_EntityHelper.DeleteEntityAndChildren, 200, false, drone);
		}
		else
		{
			vector transform[4];
			drone.GetTransform(transform);
			EntitySpawnParams params = new EntitySpawnParams();
			params.Transform = transform;
			IEntity newDrone = GetGame().SpawnEntityPrefab(Resource.Load(droneWreckPrefab), GetGame().GetWorld(), params);	
			
			auto garbageSystem = SCR_GarbageSystem.GetByEntityWorld(newDrone);
			if (garbageSystem)
				garbageSystem.Insert(newDrone);
			
			Rpc(RpcDo_DestroyDroneServer, droneId);
			GetGame().GetCallqueue().CallLater(DeleteDrone, 200, false, drone.GetPhysics().GetVelocity(), newDrone, drone);
		}		
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_DestroyDroneServer(RplId droneId)
	{
		if (!Replication.FindItem(droneId))
			return;
		
		IEntity drone = RplComponent.Cast(Replication.FindItem(droneId)).GetEntity();
		if (!drone)
			return;
		
		if (!IsDronePlayers(drone))
			return;
		
		SCR_PlayerController.Cast(GetGame().GetPlayerController()).DisconnectDrone();
	}
	
	void SetCameraYaw(float input, RplId droneId)
	{
		Rpc(RpcDo_SetCameraYaw, input, droneId);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_SetCameraYaw(float input, RplId droneId)
	{
		if (!Replication.FindItem(droneId))
			return;
		
		IEntity drone = RplComponent.Cast(Replication.FindItem(droneId)).GetEntity();
		if (!drone)
			return;
		
		if (!drone.FindComponent(SlotManagerComponent))
			return;
		
		IEntity camera = SlotManagerComponent.Cast(drone.FindComponent(SlotManagerComponent)).GetSlotByName("Camera").GetAttachedEntity();
		if (!camera)
			return;
		
		SAL_GimbalCameraComponent camComp = SAL_GimbalCameraComponent.Cast(camera.FindComponent(SAL_GimbalCameraComponent));
		if (!camComp)
			return;
		
		camComp.m_fPitchOffset = input;
	}
	
	void UpdateDroneFactionServer(RplId droneId, string factionKey)
	{
		Rpc(RpcDo_UpdateDroneFactionServer, droneId, factionKey);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_UpdateDroneFactionServer(RplId droneId, string factionKey)
	{
		if (!Replication.FindItem(droneId))
			return;
		
		IEntity drone = RplComponent.Cast(Replication.FindItem(droneId)).GetEntity();
		if (!drone)
			return;
		
		FactionAffiliationComponent factComp = FactionAffiliationComponent.Cast(drone.FindComponent(FactionAffiliationComponent));
		if (!factComp)
			return;
		
		factComp.SetAffiliatedFaction(GetGame().GetFactionManager().GetFactionByKey(factionKey));
	}
	
	void DropGrenadeBroadcast(RplId droneid)
	{
		Rpc(RpcDo_DropDroneGrenadeBroadcast, droneid);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_DropDroneGrenadeBroadcast(RplId droneId)
	{
		if (!Replication.FindItem(droneId))
			return;
		
		IEntity drone = RplComponent.Cast(Replication.FindItem(droneId)).GetEntity();
		if (!drone)
			return;
		
		SAL_DropperComponent dropperComp = SAL_DropperComponent.Cast(drone.FindComponent(SAL_DropperComponent));
		if (!dropperComp)
			return;
		
		dropperComp.m_BGrenadeDropped = true;
	}
	
	void ReloadGrenadeBroadcast(RplId droneid)
	{
		Rpc(RpcDo_ReloadDroneGrenadeBroadcast, droneid);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_ReloadDroneGrenadeBroadcast(RplId droneId)
	{
		if (!Replication.FindItem(droneId))
			return;
		
		IEntity drone = RplComponent.Cast(Replication.FindItem(droneId)).GetEntity();
		if (!drone)
			return;
		
		SAL_DropperComponent dropperComp = SAL_DropperComponent.Cast(drone.FindComponent(SAL_DropperComponent));
		if (!dropperComp)
			return;
		
		dropperComp.m_BGrenadeDropped = false;
		
//		SlotManagerComponent slotComp = SlotManagerComponent.Cast(drone.FindComponent(SlotManagerComponent));
//		if (!slotComp)
//			return;
//		slotComp.GetSlotByName("GrenadeDropper").AttachEntity(grenade);
	}
}