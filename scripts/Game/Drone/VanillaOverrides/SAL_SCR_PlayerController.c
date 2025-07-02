modded class SCR_PlayerController
{
	CameraBase m_eOldCamera;
	float m_fMasterVolume;

	static RplId GetRplId(IEntity entity)
	{
		if (!entity)
			return RplId.Invalid();
		
		RplComponent rplComp = RplComponent.Cast(entity.FindComponent(RplComponent));
		return rplComp.Id();
	}
	
	//Adds some keybinds
	override protected void UpdateLocalPlayerController()
	{
		InputManager inputManager = GetGame().GetInputManager();
		super.UpdateLocalPlayerController();
		
		inputManager.AddActionListener("DisconnectDrone", EActionTrigger.DOWN, DisconnectDrone);
		inputManager.AddActionListener("ArmDroneWarhead", EActionTrigger.DOWN, ArmDroneWarhead);
	}
	
	
	void EnterDrone()
	{
		SAL_DroneConnectionManager droneManager = SAL_DroneConnectionManager.GetInstance();
		
		if (droneManager.GetPlayersDrone(GetPlayerId()) == null)
			return;
		
		IEntity drone = droneManager.GetPlayersDrone(GetPlayerId());
		
		if (drone.GetParent() != null)
			return;
		
		GetGame().GetInputManager().ActivateContext("DroneContext", 999999);
		
		SlotManagerComponent slotManager = SlotManagerComponent.Cast(drone.FindComponent(SlotManagerComponent));
		
		EntitySlotInfo cameraSlotHigher = slotManager.GetSlotByName("Camera");
		if (!cameraSlotHigher)
		{
			DisconnectDrone();
			return;
		}
		
		if (!cameraSlotHigher.GetAttachedEntity().FindComponent(SlotManagerComponent))
		{
			DisconnectDrone();
			return;
		}
			
		EntitySlotInfo cameraSlot = SlotManagerComponent.Cast(cameraSlotHigher.GetAttachedEntity().FindComponent(SlotManagerComponent)).GetSlotByName("Camera");
		
		if (!cameraSlot)
		{
			DisconnectDrone();
			return;
		}
		
		vector cameraTransform[4];
		cameraSlot.GetTransform(cameraTransform);
		
		EntitySpawnParams spawnParams = new EntitySpawnParams();
		spawnParams.Transform = cameraTransform;
		
		IEntity camera = GetGame().SpawnEntityPrefab(Resource.Load("{D10C3C304FC29655}Prefabs/Editor/Camera/DroneCamera.et"), GetWorld(), spawnParams);
		cameraSlot.AttachEntity(camera);
		
		m_eOldCamera = GetGame().GetCameraManager().CurrentCamera();
		
		CameraBase cameraBase = CameraBase.Cast(camera);
		GetGame().GetCameraManager().SetCamera(cameraBase);
		
		AudioSystem.SetMasterVolume(AudioSystem.SFX, 0.05);
	}
	
	void ExitDrone()
	{
		CameraBase droneCamera = GetGame().GetCameraManager().CurrentCamera();
		
		float volume;
		GetGame().GetEngineUserSettings().GetModule("AudioSettings").Get("VolumeSfx", volume);
		AudioSystem.SetMasterVolume(AudioSystem.SFX, volume);
		
		if (droneCamera)
			if (droneCamera.GetPrefabData().GetPrefabName() != "{D10C3C304FC29655}Prefabs/Editor/Camera/DroneCamera.et")
				return;
		
		GetGame().GetInputManager().ActivateContext("DroneContext");
		
		if (m_eOldCamera)
			GetGame().GetCameraManager().SetCamera(m_eOldCamera);
		
		if (droneCamera)
			SCR_EntityHelper.DeleteEntityAndChildren(droneCamera);
	}
	
	void DisconnectDrone()
	{
		SAL_DroneConnectionManager droneManager = SAL_DroneConnectionManager.GetInstance();
		
		if (!droneManager.IsPlayerDroneOwner(GetPlayerId()))
			return;
		
		if (droneManager.GetPlayersDrone(GetPlayerId()) != null)
			if (SAL_DroneControllerComponent.Cast(droneManager.GetPlayersDrone(GetPlayerId()).FindComponent(SAL_DroneControllerComponent)).m_bIsArmed)
				SAL_DroneControllerComponent.Cast(droneManager.GetPlayersDrone(GetPlayerId()).FindComponent(SAL_DroneControllerComponent)).ArmDrone();
		
		Rpc(RpcDo_DisconnectDrone, droneManager.GetPlayersDroneRplId(GetPlayerId()));
		
		GetGame().GetCallqueue().CallLater(ExitDrone, 100, false);
	}
	
	void ArmDroneWarhead()
	{
		SAL_DroneConnectionManager droneManager = SAL_DroneConnectionManager.GetInstance();
		
		if (!droneManager.IsPlayerDroneOwner(GetPlayerId()))
			return;
		
		SAL_DroneExplosionComponent expComp = SAL_DroneExplosionComponent.Cast(droneManager.GetPlayersDrone(GetPlayerId()).FindComponent(SAL_DroneExplosionComponent));
		if (!expComp)
			return;
		
		expComp.m_bIsArmed = !expComp.m_bIsArmed;
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcDo_DisconnectDrone(RplId drone)
	{
		SAL_DroneConnectionManager.GetInstance().DisconnectDrone(drone);
	}
	
	void ConnectPlayerToDrone(IEntity drone)
	{
		Rpc(RpcDo_ConnectPlayerToDrone, GetRplId(drone), GetPlayerId());
		//If the goggles are already on their head
		GetGame().GetCallqueue().CallLater(EnterDroneTest, 200, false);
	}
	
	void EnterDroneTest()
	{
		if (SCR_CharacterInventoryStorageComponent.Cast(GetLocalControlledEntity().FindComponent(SCR_CharacterInventoryStorageComponent)).Get(0))
			if (SCR_CharacterInventoryStorageComponent.Cast(GetLocalControlledEntity().FindComponent(SCR_CharacterInventoryStorageComponent)).Get(0).GetPrefabData().GetPrefabName() == "{8951045BFE8BC8E4}Prefabs/Characters/HeadGear/FPV_Goggles.et")
				EnterDrone();
	}
	
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcDo_ConnectPlayerToDrone(RplId droneId, int playerId)
	{
		if ( SAL_DroneConnectionManager.GetInstance().m_aDrones.Contains(droneId) )
			return;
		SAL_DroneConnectionManager.GetInstance().ConnectToDrone(droneId, playerId);
	}
	
	void SendTransformToServer(SAL_DroneNetworkPacket packet)
	{
		Rpc(RpcDo_SendTransformToServer, packet);
	}
	
	// === CLIENT → SERVER ===
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcDo_SendTransformToServer(SAL_DroneNetworkPacket packet)
	{
		if (!Replication.FindItem(packet.GetDrone()))
			return;
		
		IEntity drone = RplComponent.Cast(Replication.FindItem(packet.GetDrone())).GetEntity();
		vector transform[4];
		packet.GetTransform(transform);
		
		GenericEntity droneEntity = GenericEntity.Cast(drone);
		droneEntity.SetTransform(transform);
		droneEntity.Update();
		droneEntity.OnTransformReset();
		
		float rotorRPMs[4];
		packet.GetRotorRPMs(rotorRPMs);
		float averageRPM = (rotorRPMs[0] + rotorRPMs[1] + rotorRPMs[2] + rotorRPMs[3]) / 4;
		
		SAL_DroneBatteryComponent.Cast(droneEntity.FindComponent(SAL_DroneBatteryComponent)).m_fCurrentBattery = packet.GetBatteryLevel();
		SAL_DroneControllerComponent.Cast(drone.FindComponent(SAL_DroneControllerComponent)).m_vLastBroadcastedPosition1 = transform[0];
		SAL_DroneControllerComponent.Cast(drone.FindComponent(SAL_DroneControllerComponent)).m_vLastBroadcastedPosition2 = transform[1];
		SAL_DroneControllerComponent.Cast(drone.FindComponent(SAL_DroneControllerComponent)).m_vLastBroadcastedPosition3 = transform[2];
		SAL_DroneControllerComponent.Cast(drone.FindComponent(SAL_DroneControllerComponent)).m_vLastBroadcastedPosition4 = transform[3];
		SAL_DroneConnectionManager.GetInstance().ReplicateTransform(packet);
	}
	
	void ExplodeDrone(SAL_DroneNetworkPacket packet)
	{
		DisconnectDrone();
		Rpc(RpcDo_ExplodeDrone, packet);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcDo_ExplodeDrone(SAL_DroneNetworkPacket packet)
	{
		SAL_DroneConnectionManager.GetInstance().ExplodeDrone(packet);
	}
	
	void DisarmDrone(SAL_DroneNetworkPacket packet)
	{
		Rpc(RpcDo_DisarmDrone, packet);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcDo_DisarmDrone(SAL_DroneNetworkPacket packet)
	{
		SAL_DroneConnectionManager.GetInstance().DisarmDrone(packet);
		
		if (!Replication.FindItem(packet.GetDrone()))
			return;
		
		IEntity drone = RplComponent.Cast(Replication.FindItem(packet.GetDrone())).GetEntity();
		if (!drone)
			return;
		
		SAL_DroneControllerComponent.Cast(drone.FindComponent(SAL_DroneControllerComponent)).m_bIsArmed = packet.GetIsArmed();
	}
	
	void UpdateBattery(SAL_DroneNetworkPacket packet)
	{
		Rpc(Rpc_DoUpdateBattery, packet);
		
		if (!Replication.FindItem(packet.GetDrone()))
			return;
		
		IEntity drone = RplComponent.Cast(Replication.FindItem(packet.GetDrone())).GetEntity();
		if (!drone)
			return;
		
		
		SAL_DroneBatteryComponent battComp = SAL_DroneBatteryComponent.Cast(drone.FindComponent(SAL_DroneBatteryComponent));
		if (!battComp)
		return;
		
		battComp.m_fCurrentBattery = packet.GetBatteryLevel();
		battComp.m_fCurrentBatteryMax = packet.GetBatteryLevel();
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void Rpc_DoUpdateBattery(SAL_DroneNetworkPacket packet)
	{
		if (SAL_DroneConnectionManager.GetInstance())
			SAL_DroneConnectionManager.GetInstance().UpdateBattery(packet);
		
		if (!Replication.FindItem(packet.GetDrone()))
			return;
		
		IEntity drone = RplComponent.Cast(Replication.FindItem(packet.GetDrone())).GetEntity();
		if (!drone)
			return;
		
		
		SAL_DroneBatteryComponent battComp = SAL_DroneBatteryComponent.Cast(drone.FindComponent(SAL_DroneBatteryComponent));
		if (!battComp)
		return;
		
		battComp.m_fCurrentBattery = packet.GetBatteryLevel();
		battComp.m_fCurrentBatteryMax = packet.GetBatteryLevel();
	}
	
	void DestroyDrone(RplId droneId, ResourceName droneWreckPrefab)
	{
		Rpc(RpcDo_DestroyDrone, droneId, droneWreckPrefab);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcDo_DestroyDrone(RplId droneId, ResourceName droneWreckPrefab)
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
			
			SCR_EntityHelper.DeleteEntityAndChildren(drone);
		}
		else
		{
			vector transform[4];
			drone.GetTransform(transform);
			EntitySpawnParams params = new EntitySpawnParams();
			params.Transform = transform;
			IEntity newDrone = GetGame().SpawnEntityPrefab(Resource.Load(droneWreckPrefab), GetGame().GetWorld(), params);	
			
			SAL_DroneConnectionManager connectionManager = SAL_DroneConnectionManager.GetInstance();
		
			GetGame().GetCallqueue().CallLater(SAL_DroneConnectionManager.GetInstance().DeleteDrone, 200, false, drone.GetPhysics().GetVelocity(), newDrone, drone);
		}
	}
	
	void UpdateDroneHealth(SAL_DroneNetworkPacket packet)
	{
		Rpc(RpcDo_UpdateDroneHeatlh, packet);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcDo_UpdateDroneHeatlh(SAL_DroneNetworkPacket packet)
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
		SAL_DroneConnectionManager.GetInstance().BroadcastHealthUpdate(packet);
	}
	
	void SetCameraYaw(float input, RplId droneId)
	{
		if (!Replication.FindItem(droneId))
			return;
		
		IEntity drone = RplComponent.Cast(Replication.FindItem(droneId)).GetEntity();
		if (!drone)
			return;
		
		if (!drone.FindComponent(SlotManagerComponent))
			return;
		
		Rpc(RpcDo_SetCameraYaw, input, droneId);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
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
		SAL_DroneConnectionManager.GetInstance().SetCameraYaw(input, droneId);
	}
	
	void UpdateDroneFaction(IEntity drone, string factionKey)
	{
		RpcDo_UpdateDroneFaction(GetRplId(drone), factionKey);
		Rpc(RpcDo_UpdateDroneFaction, GetRplId(drone), factionKey);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcDo_UpdateDroneFaction(RplId droneId, string factionKey)
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
		
		SAL_DroneConnectionManager.GetInstance().UpdateDroneFactionServer(droneId, factionKey);
	}
	
	void DropGrenade(IEntity drone)
	{
		Rpc(RpcDo_DroneGrenade, GetRplId(drone));
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcDo_DroneGrenade(RplId droneId)
	{
		if (!Replication.FindItem(droneId))
			return;
		
		IEntity drone = RplComponent.Cast(Replication.FindItem(droneId)).GetEntity();
		if (!drone)
			return;
		
		SAL_DropperComponent dropperComp = SAL_DropperComponent.Cast(drone.FindComponent(SAL_DropperComponent));
		if (!dropperComp)
			return;
		
		SlotManagerComponent slotComp = SlotManagerComponent.Cast(drone.FindComponent(SlotManagerComponent));
		if (!slotComp)
			return;
		
		IEntity grenade = slotComp.GetSlotByName("GrenadeDropper").GetAttachedEntity();
		if (grenade == null)
			return;
		
		vector transform[4];
		grenade.GetTransform(transform);
		SCR_EntityHelper.DeleteEntityAndChildren(grenade);
		dropperComp.m_BGrenadeDropped = true;
		
		EntitySpawnParams params = new EntitySpawnParams();
		params.Transform = transform;
		GetGame().SpawnEntityPrefab(Resource.Load(dropperComp.m_DropperGrenade), GetGame().GetWorld(), params);
		SAL_DroneConnectionManager.GetInstance().DropGrenadeBroadcast(droneId);
		
	}
	
	void ReloadDrone(IEntity drone, IEntity grenade)
	{
		Rpc(RpcDo_ReloadDrone, GetRplId(drone), GetRplId(grenade), GetLocalControlledEntity().GetOrigin());
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcDo_ReloadDrone(RplId droneId, RplId grenadeId, vector playerOrigin)
	{
		if (!Replication.FindItem(droneId) || !Replication.FindItem(grenadeId))
			return;
		
		IEntity drone = RplComponent.Cast(Replication.FindItem(droneId)).GetEntity();
		if (!drone)
			return;
		
		IEntity grenade = RplComponent.Cast(Replication.FindItem(grenadeId)).GetEntity();
		if (!grenade)
			return;
		
		SlotManagerComponent slotComp = SlotManagerComponent.Cast(drone.FindComponent(SlotManagerComponent));
		if (!slotComp)
			return;
		EntitySpawnParams params = new EntitySpawnParams();
		params.Transform[3] = playerOrigin;
		
		SAL_DropperComponent dropperComp = SAL_DropperComponent.Cast(drone.FindComponent(SAL_DropperComponent));
		dropperComp.m_BGrenadeDropped = false;
		
		IEntity newGrenade = GetGame().SpawnEntityPrefab(Resource.Load(dropperComp.m_DropperGrenadePlaceholder), GetGame().GetWorld(), params);
		slotComp.GetSlotByName("GrenadeDropper").AttachEntity(newGrenade);
		
		SAL_DroneConnectionManager.GetInstance().ReloadGrenadeBroadcast(droneId);
		SCR_EntityHelper.DeleteEntityAndChildren(grenade);
	}
}