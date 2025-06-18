modded class SCR_PlayerController
{
	CameraBase m_eOldCamera;

	static RplId GetRplId(IEntity entity)
	{
		RplComponent rplComp = RplComponent.Cast(entity.FindComponent(RplComponent));
		return rplComp.Id();
	}
	
	//Adds some keybinds
	override protected void UpdateLocalPlayerController()
	{
		InputManager inputManager = GetGame().GetInputManager();
		super.UpdateLocalPlayerController();
		
		inputManager.AddActionListener("DisconnectDrone", EActionTrigger.DOWN, DisconnectDrone);
	}
	
	
	void EnterDrone()
	{
		SAL_DroneConnectionManager droneManager = SAL_DroneConnectionManager.GetInstance();
		
		if (droneManager.GetPlayersDrone(GetPlayerId()) == null)
			return;
		
		GetGame().GetInputManager().ActivateContext("DroneContext", 999999);
		
		IEntity drone = droneManager.GetPlayersDrone(GetPlayerId());
		
		SlotManagerComponent slotManager = SlotManagerComponent.Cast(drone.FindComponent(SlotManagerComponent));
		
		EntitySlotInfo cameraSlotHigher = slotManager.GetSlotByName("Camera");
		EntitySlotInfo cameraSlot = SlotManagerComponent.Cast(cameraSlotHigher.GetAttachedEntity().FindComponent(SlotManagerComponent)).GetSlotByName("Camera");
		vector cameraTransform[4];
		cameraSlot.GetTransform(cameraTransform);
		
		EntitySpawnParams spawnParams = new EntitySpawnParams();
		spawnParams.Transform = cameraTransform;
		
		IEntity camera = GetGame().SpawnEntityPrefab(Resource.Load("{D10C3C304FC29655}Prefabs/Editor/Camera/DroneCamera.et"), GetWorld(), spawnParams);
		cameraSlot.AttachEntity(camera);
		
		m_eOldCamera = GetGame().GetCameraManager().CurrentCamera();
		
		CameraBase cameraBase = CameraBase.Cast(camera);
		GetGame().GetCameraManager().SetCamera(cameraBase);
	}
	
	void ExitDrone()
	{
		CameraBase droneCamera = GetGame().GetCameraManager().CurrentCamera();
		if (droneCamera.GetPrefabData().GetPrefabName() != "{D10C3C304FC29655}Prefabs/Editor/Camera/DroneCamera.et")
			return;
		
		GetGame().GetInputManager().ActivateContext("DroneContext");
		
		GetGame().GetCameraManager().SetCamera(m_eOldCamera);
		SCR_EntityHelper.DeleteEntityAndChildren(droneCamera);
	}
	
	void DisconnectDrone()
	{
		SAL_DroneConnectionManager droneManager = SAL_DroneConnectionManager.GetInstance();
		
		if (!droneManager.IsPlayerDroneOwner(GetPlayerId()))
			return;
		
		if (SAL_DroneControllerComponent.Cast(droneManager.GetPlayersDrone(GetPlayerId()).FindComponent(SAL_DroneControllerComponent)).m_bIsArmed)
			return;
		
		Rpc(RpcDo_DisconnectDrone, droneManager.GetPlayersDroneRplId(GetPlayerId()));
		
		ExitDrone()
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
		
		SAL_DroneConnectionManager.GetInstance().ReplicateTransform(packet);
	}
	
	void ExplodeDrone(SAL_DroneNetworkPacket packet)
	{
		Rpc(RpcDo_ExplodeDrone, packet);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcDo_ExplodeDrone(SAL_DroneNetworkPacket packet)
	{
		if (!Replication.FindItem(packet.GetDrone()))
			return;
		
		IEntity drone = RplComponent.Cast(Replication.FindItem(packet.GetDrone())).GetEntity();
		
		vector transform[4];
		packet.GetTransform(transform);
		EntitySpawnParams params = new EntitySpawnParams();
		params.Transform = transform;
		GetGame().SpawnEntityPrefab(Resource.Load(packet.GetExplosion()), GetGame().GetWorld(), params);
		
		SCR_EntityHelper.DeleteEntityAndChildren(drone);
	}
	
	void DisarmDrone(SAL_DroneNetworkPacket packet)
	{
		Rpc(RpcDo_DisarmDrone, packet);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcDo_DisarmDrone(SAL_DroneNetworkPacket packet)
	{
		SAL_DroneConnectionManager.GetInstance().DisarmDrone(packet);
	}
}