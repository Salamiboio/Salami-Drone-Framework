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
		inputManager.AddActionListener("SwitchDrone", EActionTrigger.DOWN, SwitchDrone);
		inputManager.AddActionListener("SwitchDroneDown", EActionTrigger.DOWN, SwitchDroneDown);
	}
	
	void SwitchDroneDown()
	{
		SwitchDrone(-1);
	}
	
	void SwitchDrone(float value)
	{
		SAL_DroneConnectionManager droneManager = SAL_DroneConnectionManager.GetInstance();
		if (droneManager.GetPlayersCommandStation(GetPlayerId()) == null)
			return;
		
		if (droneManager.GetPlayersDrone(GetPlayerId()) == null)
			return;
		
		if (!GetGame().GetCameraManager().CurrentCamera().GetParent())
			return;
		
		if (!GetGame().GetCameraManager().CurrentCamera().GetParent().GetParent())
			return;
		
		IEntity currentDrone = GetGame().GetCameraManager().CurrentCamera().GetParent().GetParent();
		RplId currentDroneId = GetRplId(currentDrone);
		ref array<RplId> droneRplIds = droneManager.GetPlayersDrones(GetPlayerId());
		int newIndex;
		foreach(RplId droneId: droneRplIds)
		{
			if (droneId != currentDroneId)
				continue;
			
			int index = droneRplIds.Find(droneId);
			newIndex = index + value;
			
			if (newIndex > 2)
				newIndex = 0;
			if (newIndex < 0)
				newIndex = 2;
		}
		
		while (droneRplIds.Get(newIndex) == RplId.Invalid())
		{
			newIndex += value;
			if (newIndex > 2)
				newIndex = 0;
			if (newIndex < 0)
				newIndex = 2;
		}
		
		ExitDrone();
		EnterDrone(droneRplIds.Get(newIndex));
	}
	
	
	void EnterDrone(RplId droneId)
	{
		SAL_DroneConnectionManager droneManager = SAL_DroneConnectionManager.GetInstance();
		
		if (droneManager.GetPlayersDrone(GetPlayerId()) == null)
			return;
		
		if (!Replication.FindItem(droneId))
		{
			DisconnectSpecificDrone(droneId);
			return;
		}
		
		IEntity drone = RplComponent.Cast(Replication.FindItem(droneId)).GetEntity();
		
		if (drone.GetParent() != null)
			return;
		
		GetGame().GetInputManager().ActivateContext("DroneContext", 999999);
		AddActiveDrone(droneId);
		
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
		RplId oldDroneId = SAL_DroneConnectionManager.GetInstance().GetPlayersActiveDroneRplId(SCR_PlayerController.GetLocalPlayerId());
		RemoveActiveDrone(oldDroneId);
		
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
		
		if (droneManager.GetPlayersActiveDrone(GetPlayerId()) != null)
			if (SAL_DroneControllerComponent.Cast(droneManager.GetPlayersActiveDrone(GetPlayerId()).FindComponent(SAL_DroneControllerComponent)).m_bIsArmed)
				SAL_DroneControllerComponent.Cast(droneManager.GetPlayersActiveDrone(GetPlayerId()).FindComponent(SAL_DroneControllerComponent)).ArmDrone();
			
		if (droneManager.DoesPlayerHaveActiveDrone(GetPlayerId()))
			GetGame().GetCallqueue().CallLater(NewDroneCheck, 150, false, droneManager.GetPlayersActiveDroneRplId(GetPlayerId()));
		if (droneManager.DoesPlayerHaveActiveDrone(GetPlayerId()))
			GetGame().GetCallqueue().CallLater(DisconnectDroneDelay, 150, false, droneManager.GetPlayersActiveDroneRplId(GetPlayerId()), GetPlayerId());
		else
			GetGame().GetCallqueue().CallLater(DisconnectDroneDelay, 150, false, droneManager.GetPlayersDroneRplId(GetPlayerId()), GetPlayerId());
		GetGame().GetCallqueue().CallLater(ExitDrone, 100, false);
	}
	
	void DisconnectDroneDelay(RplId droneId, int playerId)
	{
		Rpc(RpcDo_DisconnectDrone, droneId, playerId);
	}
	
	void DisconnectSpecificDrone(RplId droneId)
	{
		SAL_DroneConnectionManager droneManager = SAL_DroneConnectionManager.GetInstance();
		RplId activeDroneId = droneManager.GetPlayersActiveDroneRplId(GetPlayerId());
		if (activeDroneId == droneId)
			GetGame().GetCallqueue().CallLater(NewDroneCheck, 130, false, activeDroneId);	
		
		GetGame().GetCallqueue().CallLater(DisconnectDroneDelay, 150, false, droneId, GetPlayerId());
		if (activeDroneId == droneId)
			GetGame().GetCallqueue().CallLater(ExitDrone, 100, false);
		
	}
	
	void NewDroneCheck(RplId oldDrone)
	{
		SAL_DroneConnectionManager droneManager = SAL_DroneConnectionManager.GetInstance();
		
		if (!droneManager.GetPlayersCommandStation(GetPlayerId()))
			return;
		
		if (!droneManager.IsPlayerDroneOwner(GetPlayerId()))
			return;
	
		ref array<RplId> droneRplIds = droneManager.GetPlayersDrones(GetPlayerId());
		foreach (RplId droneId: droneRplIds)
		{
			if (droneId == RplId.Invalid())
				continue;
			
			if (droneId == oldDrone)
				continue;
			
			EnterDrone(droneId);
			return;
		}
	}
	
	void ArmDroneWarhead()
	{
		SAL_DroneConnectionManager droneManager = SAL_DroneConnectionManager.GetInstance();
		
		if (!droneManager.IsPlayerDroneOwner(GetPlayerId()))
			return;
		
		SAL_DroneExplosionComponent expComp = SAL_DroneExplosionComponent.Cast(droneManager.GetPlayersActiveDrone(GetPlayerId()).FindComponent(SAL_DroneExplosionComponent));
		if (!expComp)
			return;
		
		expComp.m_bIsArmed = !expComp.m_bIsArmed;
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcDo_DisconnectDrone(RplId drone, int playerId)
	{
		SAL_DroneConnectionManager.GetInstance().DisconnectDrone(drone, playerId);
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
				EnterDrone(SAL_DroneConnectionManager.GetInstance().GetPlayersDroneRplId(SCR_PlayerController.GetLocalPlayerId()));
	}
	
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcDo_ConnectPlayerToDrone(RplId droneId, int playerId)
	{
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
		
		if (vector.Distance(drone.GetOrigin(), transform[3]) > 10)
		{
			SAL_DroneConnectionManager.GetInstance().TeleportDroneBack(packet.GetDrone(), transform);
			return;
		}
		
		GenericEntity droneEntity = GenericEntity.Cast(drone);
		droneEntity.SetTransform(transform);
		droneEntity.Update();
		droneEntity.OnTransformReset();
		
		SAL_DroneControllerComponent droneController = SAL_DroneControllerComponent.Cast(drone.FindComponent(SAL_DroneControllerComponent));
		
		SAL_DroneBatteryComponent.Cast(droneEntity.FindComponent(SAL_DroneBatteryComponent)).m_fCurrentBattery = packet.GetBatteryLevel();
		droneController.m_vLastBroadcastedPosition1 = transform[0];
		droneController.m_vLastBroadcastedPosition2 = transform[1];
		droneController.m_vLastBroadcastedPosition3 = transform[2];
		droneController.m_vLastBroadcastedPosition4 = transform[3];
		droneController.m_vCurrentVelocity = packet.GetCurrentVelocity();
		SAL_DroneConnectionManager.GetInstance().ReplicateTransform(packet);
	}
	
	void ExplodeDrone(SAL_DroneNetworkPacket packet)
	{
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
			
	void DestroyDroneClient(RplId droneId, ResourceName droneWreckPrefab, int playerId)
	{
		Rpc(RpcDo_DestroyDroneClient, droneId, droneWreckPrefab, playerId);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcDo_DestroyDroneClient(RplId droneId, ResourceName droneWreckPrefab, int playerId)
	{
		SAL_DroneConnectionManager.GetInstance().DestroyDroneHealth(droneId, droneWreckPrefab, playerId);
	}
	
	void ConnectPlayerToAntenna(IEntity antenna)
	{
		Rpc(RpcDo_ConnectPlayerToAntenna, GetRplId(antenna), GetPlayerId());
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcDo_ConnectPlayerToAntenna(RplId antennaId, int playerId)
	{
		if (SAL_DroneConnectionManager.GetInstance().IsAntennaConnected(antennaId))
			return;
		SAL_DroneConnectionManager.GetInstance().ConnectAntenna(antennaId, playerId);
	}
	
	void DisconnectPlayerToAntenna(IEntity antenna)
	{
		Rpc(RpcDo_DisconnectPlayerToAntenna, GetRplId(antenna));
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcDo_DisconnectPlayerToAntenna(RplId antennaId)
	{
		SAL_DroneConnectionManager.GetInstance().DisconnectAntenna(antennaId);
	}
	
	void ConnectPlayerToCommand(IEntity antenna)
	{
		Rpc(RpcDo_ConnectPlayerToCommand, GetRplId(antenna), GetPlayerId());
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcDo_ConnectPlayerToCommand(RplId antennaId, int playerId)
	{
		if (SAL_DroneConnectionManager.GetInstance().IsCommandStationConnected(antennaId))
			return;
		SAL_DroneConnectionManager.GetInstance().ConnectDroneCommandStation(antennaId, playerId);
	}
	
	void DisconnectPlayerToCommand(IEntity antenna)
	{
		Rpc(RpcDo_DisconnectPlayerToCommand, GetRplId(antenna));
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcDo_DisconnectPlayerToCommand(RplId antennaId)
	{
		SAL_DroneConnectionManager.GetInstance().DisconnectDroneCommandStation(antennaId);
	}
	
	void ToggleJammer(RplId jammerId)
	{
		Rpc(RpcDo_ToggleJammer, jammerId);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcDo_ToggleJammer(RplId jammerId)
	{
		if (SAL_DroneConnectionManager.GetInstance().m_aJammers.Contains(jammerId))
			SAL_DroneConnectionManager.GetInstance().UnregisterJammer(jammerId);
		else
			SAL_DroneConnectionManager.GetInstance().RegisterJammer(jammerId);
	}
	
	void AddOrRemoveItem(RplId caseId, RplId itemId, string slotName)
	{
		Rpc(RpcDo_AddOrRemoveItem, caseId, itemId, slotName, GetPlayerId());
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]	
	void RpcDo_AddOrRemoveItem(RplId caseId, RplId itemId, string slotName, int playerId)
	{
		if (!Replication.FindItem(caseId) || !Replication.FindItem(itemId))
			return;
		
		IEntity caseObject = RplComponent.Cast(Replication.FindItem(caseId)).GetEntity();
		if (!caseObject)
			return;
		
		IEntity itemObject = RplComponent.Cast(Replication.FindItem(itemId)).GetEntity();
		if (!itemObject)
			return;
		
		SlotManagerComponent slotComp = SlotManagerComponent.Cast(caseObject.FindComponent(SlotManagerComponent));
		if (!slotComp)
			return;
		
		EntitySlotInfo slot = slotComp.GetSlotByName(slotName);
		if (!slot)
			return;
		
		if (slot.GetAttachedEntity())
		{
			IEntity player = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
			
			SCR_InventoryStorageManagerComponent storageComp = SCR_InventoryStorageManagerComponent.Cast(player.FindComponent(SCR_InventoryStorageManagerComponent));
			if (!storageComp)
				return;

			string itemPrefab = itemObject.GetPrefabData().GetPrefabName();
			EntitySpawnParams spawnParams = new EntitySpawnParams();
			vector transform[4];
			itemObject.GetTransform(transform);
			spawnParams.Transform = transform;
			IEntity newItem = GetGame().SpawnEntityPrefab(Resource.Load(itemPrefab), GetGame().GetWorld(), spawnParams);
			SCR_EntityHelper.DeleteEntityAndChildren(itemObject);
			
			BaseInventoryStorageComponent baseStorageComp = BaseInventoryStorageComponent.Cast(player.FindComponent(BaseInventoryStorageComponent));
			
			GetGame().GetCallqueue().CallLater(SAL_DroneConnectionManager.GetInstance().TryInsertItemToInventory, 200, false, newItem, baseStorageComp, storageComp);
		}
		else
		{
			IEntity player = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
			
			SCR_InventoryStorageManagerComponent storageComp = SCR_InventoryStorageManagerComponent.Cast(player.FindComponent(SCR_InventoryStorageManagerComponent));
			if (!storageComp)
				return;
			
			string itemPrefab = itemObject.GetPrefabData().GetPrefabName();
			EntitySpawnParams spawnParams = new EntitySpawnParams();
			vector transform[4];
			itemObject.GetTransform(transform);
			spawnParams.Transform = transform;
			IEntity newItem = GetGame().SpawnEntityPrefab(Resource.Load(itemPrefab), GetGame().GetWorld(), spawnParams);
			SCR_EntityHelper.DeleteEntityAndChildren(itemObject);
			
			slot.AttachEntity(newItem);
			GetGame().GetCallqueue().CallLater(SAL_DroneConnectionManager.GetInstance().BroadcastSlotAttachment, 500, false, caseObject, slotName, newItem);
		}
	}
	
	void AddActiveDrone(RplId droneId)
	{
		Rpc(RpcDo_AddActiveDrone, droneId);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]	
	void RpcDo_AddActiveDrone(RplId droneId)
	{
		SAL_DroneConnectionManager.GetInstance().AddActiveDrone(droneId);
	}
	
	void RemoveActiveDrone(RplId droneId)
	{
		Rpc(RpcDo_RemoveActiveDrone, droneId);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]	
	void RpcDo_RemoveActiveDrone(RplId droneId)
	{
		SAL_DroneConnectionManager.GetInstance().RemoveActiveDrone(droneId);
	}
}