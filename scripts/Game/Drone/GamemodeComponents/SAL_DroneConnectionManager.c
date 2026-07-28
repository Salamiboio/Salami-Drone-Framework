class SAL_DroneConnectionManagerClass: SCR_BaseGameModeComponentClass
{
}


class SAL_DroneConnectionManager: SCR_BaseGameModeComponent
{
	[RplProp()] ref array<int> m_aDroneOwners = {};
	[RplProp()] ref array<ref SAL_DroneConnectContainer> m_aDrones = {};
	[RplProp()] ref array<RplId> m_aActiveDrones = {};
	[RplProp()] ref array<RplId> m_aJammers = {};
	static SAL_DroneConnectionManager m_sInstance;
	
	void SAL_DroneConnectionManager(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		m_sInstance = this;
	}
	
	override void OnPlayerDisconnected(int playerId, KickCauseCode cause, int timeout)
	{
		super.OnPlayerDisconnected(playerId, cause, timeout);
		if (!Replication.IsServer())
			return;
		
		if (!m_aDroneOwners.Contains(playerId))
			return;
		
		int index = m_aDroneOwners.Find(playerId);
		
		SAL_DroneConnectContainer container = m_aDrones.Get(index);
		
		if (container.GetDroneAntenna() != RplId.Invalid())
			DisconnectAntenna(container.GetDroneAntenna());
		
		if (!container)
			return;
		
		if (container.GetDroneCommandStation() != RplId.Invalid())
			DisconnectDroneCommandStation(container.GetDroneCommandStation());
		
		if (!container)
			return;
		
		if (container.GetDroneConnection(0) != RplId.Invalid())
			DisconnectDrone(container.GetDroneConnection(0), playerId);
		
		if (!container)
			return;
		
		if (container.GetDroneConnection(1) != RplId.Invalid())
			DisconnectDrone(container.GetDroneConnection(1), playerId);
		
		if (!container)
			return;
		
		if (container.GetDroneConnection(2) != RplId.Invalid())
			DisconnectDrone(container.GetDroneConnection(2), playerId);
		
		if (!container)
			return;
	}
	
	static SAL_DroneConnectionManager GetInstance()
	{
		return m_sInstance;
	}
	
	static RplId GetRplId(IEntity entity)
	{
		if (!entity)
			return RplId.Invalid();
		
		RplComponent rplComp = RplComponent.Cast(entity.FindComponent(RplComponent));
		return rplComp.Id();
	}
	
	void AddActiveDrone(RplId droneId)
	{
		m_aActiveDrones.Insert(droneId);
		Replication.BumpMe();
	}
	
	void RemoveActiveDrone(RplId droneId)
	{
		m_aActiveDrones.RemoveItem(droneId);
		Replication.BumpMe();
	}
	
	bool IsDroneConnected(RplId droneId)
	{
		if (!Replication.FindItem(droneId))
			return false;
		
		IEntity drone = RplComponent.Cast(Replication.FindItem(droneId)).GetEntity();
		if (!drone)
			return false;
		
		return SAL_DroneControllerComponent.Cast(drone.FindComponent(SAL_DroneControllerComponent)).m_bIsConnected;
	}
	
	bool IsAntennaConnected(RplId antennaId)
	{
		if (!Replication.FindItem(antennaId))
			return false;
		
		IEntity antenna = RplComponent.Cast(Replication.FindItem(antennaId)).GetEntity();
		if (!antenna)
			return false;
		
		return SAL_DroneAntennaComponent.Cast(antenna.FindComponent(SAL_DroneAntennaComponent)).m_bIsConnected;
	}
	
	bool IsCommandStationConnected(RplId commandId)
	{
		if (!Replication.FindItem(commandId))
			return false;
		
		IEntity commandStation = RplComponent.Cast(Replication.FindItem(commandId)).GetEntity();
		if (!commandStation)
			return false;
		
		return SAL_DroneCommandStationComponent.Cast(commandStation.FindComponent(SAL_DroneCommandStationComponent)).m_bIsConnected;
	}
	
	bool DoesPlayerHaveActiveDrone(int playerId)
	{
		if (!m_aDroneOwners.Contains(playerId))
			return false;
		
		int index = m_aDroneOwners.Find(playerId);
		
		SAL_DroneConnectContainer droneContainer = m_aDrones.Get(index);
		
		if (m_aActiveDrones.Contains(droneContainer.GetDroneConnection(0)))
			return true;
		else if (m_aActiveDrones.Contains(droneContainer.GetDroneConnection(1)))
			return true;
		else if (m_aActiveDrones.Contains(droneContainer.GetDroneConnection(2)))
			return true;
		
		return false;
	}
	
	bool IsDronePlayersRplId(RplId droneId)
	{
		if (!GetGame().GetPlayerController())
			return false;
		
		if (!m_aDroneOwners.Contains(SCR_PlayerController.GetLocalPlayerId()))
			return false;
		
		int index = m_aDroneOwners.Find(SCR_PlayerController.GetLocalPlayerId());
		
		SAL_DroneConnectContainer droneContainer = m_aDrones.Get(index);
		if (droneContainer.GetDroneConnection(0) == droneId)
			return true;
		else if (droneContainer.GetDroneConnection(1) == droneId)
			return true;
		else if (droneContainer.GetDroneConnection(2) == droneId)
			return true;
		
		return false;
	}
	
	bool IsDronePlayers(IEntity drone)
	{
		if (!GetGame().GetPlayerController())
			return false;
		
		if (!m_aDroneOwners.Contains(SCR_PlayerController.GetLocalPlayerId()))
			return false;
		
		int index = m_aDroneOwners.Find(SCR_PlayerController.GetLocalPlayerId());
		
		RplId droneId = GetRplId(drone);
		SAL_DroneConnectContainer droneContainer = m_aDrones.Get(index);
		
		if (droneContainer.GetDroneConnection(0) == droneId)
			return true;
		else if (droneContainer.GetDroneConnection(1) == droneId)
			return true;
		else if (droneContainer.GetDroneConnection(2) == droneId)
			return true;
		
		return false;
	}
	
	bool IsPlayerDroneOwner(int playerId)
	{
		if (!m_aDroneOwners.Contains(playerId))
			return false;
		
		int index = m_aDroneOwners.Find(playerId);
		
		SAL_DroneConnectContainer droneContainer = m_aDrones.Get(index);
		
		if (droneContainer.GetDroneCommandStation() == RplId.Invalid())
		{
			if (droneContainer.GetDroneConnection(0) != RplId.Invalid())
				return true;
			else
				return false;
		}
		else
		{
			int droneCounter = 0;
			if (droneContainer.GetDroneConnection(0) != RplId.Invalid())
				droneCounter++;
			if (droneContainer.GetDroneConnection(1) != RplId.Invalid())
				droneCounter++;
			if (droneContainer.GetDroneConnection(2) != RplId.Invalid())
				droneCounter++;
			
			if (droneCounter == 0)
				return false;
			else
				return true;
		}
	}
	
	bool IsPlayerAntennaOwner(int playerId)
	{
		if (!m_aDroneOwners.Contains(playerId))
			return false;
		
		int index = m_aDroneOwners.Find(playerId);
		
		SAL_DroneConnectContainer droneContainer = m_aDrones.Get(index);
		
		if (droneContainer.GetDroneAntenna() == RplId.Invalid())
			return false;
		else
			return true;
	}
	
	bool IsPlayerCommandStationOwner(int playerId)
	{
		if (!m_aDroneOwners.Contains(playerId))
			return false;
		
		int index = m_aDroneOwners.Find(playerId);
		
		SAL_DroneConnectContainer droneContainer = m_aDrones.Get(index);
		
		if (droneContainer.GetDroneCommandStation() == RplId.Invalid())
			return false;
		else
			return true;
	}
	
	IEntity GetPlayersDrone(int playerId)
	{
		if (!m_aDroneOwners.Contains(playerId))
			return null;
		
		int index = m_aDroneOwners.Find(playerId);
		
		SAL_DroneConnectContainer droneContainer = m_aDrones.Get(index);
		
		RplId droneId;
		
		if (droneContainer.GetDroneConnection(2) != RplId.Invalid())
			droneId = droneContainer.GetDroneConnection(2);
		else if (droneContainer.GetDroneConnection(1) != RplId.Invalid())
			droneId = droneContainer.GetDroneConnection(1);
		else if (droneContainer.GetDroneConnection(0) != RplId.Invalid())
			droneId = droneContainer.GetDroneConnection(0);
		
		if (Replication.FindItem(droneId))
			return RplComponent.Cast(Replication.FindItem(droneId)).GetEntity();
		else
			return null;
	}
	
	IEntity GetPlayersActiveDrone(int playerId)
	{
		if (!m_aDroneOwners.Contains(playerId))
			return null;
		
		int index = m_aDroneOwners.Find(playerId);
		
		SAL_DroneConnectContainer droneContainer = m_aDrones.Get(index);
		
		RplId droneId;
		
		if (m_aActiveDrones.Contains(droneContainer.GetDroneConnection(2)))
			droneId = droneContainer.GetDroneConnection(2);
		else if (m_aActiveDrones.Contains(droneContainer.GetDroneConnection(1)))
			droneId = droneContainer.GetDroneConnection(1);
		else if (m_aActiveDrones.Contains(droneContainer.GetDroneConnection(0)))
			droneId = droneContainer.GetDroneConnection(0);
		
		if (Replication.FindItem(droneId))
			return RplComponent.Cast(Replication.FindItem(droneId)).GetEntity();
		else
			return null;
	}
	
	array<RplId> GetPlayersDrones(int playerId)
	{
		ref array<RplId> drones = {};
		if (!m_aDroneOwners.Contains(playerId))
			return drones;
		
		int index = m_aDroneOwners.Find(playerId);
		
		SAL_DroneConnectContainer droneContainer = m_aDrones.Get(index);
		drones.Insert(droneContainer.GetDroneConnection(0));
		drones.Insert(droneContainer.GetDroneConnection(1));
		drones.Insert(droneContainer.GetDroneConnection(2));
		
		return drones;
	}
	
	IEntity GetPlayersAntenna(int playerId)
	{
		if (!m_aDroneOwners.Contains(playerId))
			return null;
		
		int index = m_aDroneOwners.Find(playerId);
		SAL_DroneConnectContainer droneContainer = m_aDrones.Get(index);
		
		RplId antennaId;
		
		if (droneContainer.GetDroneAntenna() != RplId.Invalid())
			antennaId = droneContainer.GetDroneAntenna();
		
		if (Replication.FindItem(antennaId))
			return RplComponent.Cast(Replication.FindItem(antennaId)).GetEntity();
		else
			return null;
	}
	
	IEntity GetPlayersCommandStation(int playerId)
	{
		if (!m_aDroneOwners.Contains(playerId))
			return null;
		
		int index = m_aDroneOwners.Find(playerId);
		SAL_DroneConnectContainer droneContainer = m_aDrones.Get(index);
		
		RplId antennaId;
		
		if (droneContainer.GetDroneCommandStation() != RplId.Invalid())
			antennaId = droneContainer.GetDroneCommandStation();
		
		if (Replication.FindItem(antennaId))
			return RplComponent.Cast(Replication.FindItem(antennaId)).GetEntity();
		else
			return null;
	}
	
		
	RplId GetPlayersDroneRplId(int playerId)
	{
		if (!m_aDroneOwners.Contains(playerId))
			return null;
		
		int index = m_aDroneOwners.Find(playerId);
		
		SAL_DroneConnectContainer droneContainer = m_aDrones.Get(index);
		
		RplId droneId = RplId.Invalid();
		
		if (droneContainer.GetDroneConnection(2) != RplId.Invalid())
			droneId = droneContainer.GetDroneConnection(2);
		else if (droneContainer.GetDroneConnection(1) != RplId.Invalid())
			droneId = droneContainer.GetDroneConnection(1);
		else if (droneContainer.GetDroneConnection(0) != RplId.Invalid())
			droneId = droneContainer.GetDroneConnection(0);
		
		return droneId;
	}
	
	RplId GetPlayersActiveDroneRplId(int playerId)
	{
		if (!m_aDroneOwners.Contains(playerId))
			return RplId.Invalid();
		
		int index = m_aDroneOwners.Find(playerId);
		
		SAL_DroneConnectContainer droneContainer = m_aDrones.Get(index);
		
		RplId droneId = RplId.Invalid();
		
		if (m_aActiveDrones.Contains(droneContainer.GetDroneConnection(2)))
			droneId = droneContainer.GetDroneConnection(2);
		else if (m_aActiveDrones.Contains(droneContainer.GetDroneConnection(1)))
			droneId = droneContainer.GetDroneConnection(1);
		else if (m_aActiveDrones.Contains(droneContainer.GetDroneConnection(0)))
			droneId = droneContainer.GetDroneConnection(0);
		
		return droneId;
	}
	
	int GetDronesOwner(RplId droneId)
	{
		if (!Replication.FindItem(droneId))
			return -1;
		
		IEntity drone = RplComponent.Cast(Replication.FindItem(droneId)).GetEntity();
		if (!drone)
			return -1;
		
		return SAL_DroneControllerComponent.Cast(drone.FindComponent(SAL_DroneControllerComponent)).m_iOwner;
	}
	
	int GetAntennaOwner(RplId antennaId)
	{
		if (!Replication.FindItem(antennaId))
			return -1;
		
		IEntity antenna = RplComponent.Cast(Replication.FindItem(antennaId)).GetEntity();
		if (!antenna)
			return -1;
		
		return SAL_DroneAntennaComponent.Cast(antenna.FindComponent(SAL_DroneAntennaComponent)).m_iOwner;
	}
	
	int GetCommandStationOwner(RplId commandId)
	{
		if (!Replication.FindItem(commandId))
			return -1;
		
		IEntity commandStation = RplComponent.Cast(Replication.FindItem(commandId)).GetEntity();
		if (!commandStation)
			return -1;
		
		return SAL_DroneCommandStationComponent.Cast(commandStation.FindComponent(SAL_DroneCommandStationComponent)).m_iOwner;
	}
	
	array<ref SAL_DroneConnectContainer> GetConnectedDrones()
	{
		return m_aDrones;
	}
	
	array<int> GetDroneOwners()
	{
		return m_aDroneOwners;
	}
	
	bool IsContainerEmpty(SAL_DroneConnectContainer container)
	{
		int emptySlots = 0;
		if (container.GetDroneConnection(0) == RplId.Invalid())
			emptySlots++;
		
		if (container.GetDroneConnection(1) == RplId.Invalid())
			emptySlots++;
		
		if (container.GetDroneConnection(2) == RplId.Invalid())
			emptySlots++;
		
		if (container.GetDroneAntenna() == RplId.Invalid())
			emptySlots++;
		
		if (container.GetDroneCommandStation() == RplId.Invalid())
			emptySlots++;
		
		if (emptySlots == 5)
			return true;
		else
			return false;
	}
	
	bool CanPlayerConnectDrone(int playerId)
	{
		if (!m_aDroneOwners.Contains(playerId))
			return true;
		
		int index = m_aDroneOwners.Find(playerId);
		
		int openSlots = 0;
		SAL_DroneConnectContainer droneContainer = m_aDrones.Get(index);
		
		if (droneContainer.GetDroneCommandStation() == RplId.Invalid())
		{
			if (droneContainer.GetDroneConnection(0) == RplId.Invalid())
				openSlots++;
		}
		else
		{
			if (droneContainer.GetDroneConnection(0) == RplId.Invalid())
				openSlots++;
		
			if (droneContainer.GetDroneConnection(1) == RplId.Invalid())
				openSlots++;
		
			if (droneContainer.GetDroneConnection(2) == RplId.Invalid())
				openSlots++;
		}
		
		if (openSlots == 0)
			return false;
		else
			return true;
	}
	
	void ConnectToDrone(RplId droneId, int playerId)
	{
		//Creates new connection manager if it doesn't exist
		if (!Replication.FindItem(droneId))
			return;
		
		int index = -1;
		if (!m_aDroneOwners.Contains(playerId))
		{
			SAL_DroneConnectContainer droneContainer = new SAL_DroneConnectContainer();
			index = m_aDrones.Insert(droneContainer);
			m_aDroneOwners.Insert(playerId);
		}
		else 
			index = m_aDroneOwners.Find(playerId);

		if (!CanPlayerConnectDrone(playerId))
			return;
		
		SAL_DroneConnectContainer playerDroneContainer = m_aDrones.Get(index);
		//Add drone to open connection slot
		if (playerDroneContainer.GetDroneCommandStation() == RplId.Invalid())
		{
			if (playerDroneContainer.GetDroneConnection(0) == RplId.Invalid())
				playerDroneContainer.SetDroneConnection(droneId, 0);
		}
		else
		{
			if (playerDroneContainer.GetDroneConnection(0) == RplId.Invalid())
				playerDroneContainer.SetDroneConnection(droneId, 0);
			else if (playerDroneContainer.GetDroneConnection(1) == RplId.Invalid())
				playerDroneContainer.SetDroneConnection(droneId, 1);
			else if (playerDroneContainer.GetDroneConnection(2) == RplId.Invalid())
				playerDroneContainer.SetDroneConnection(droneId, 2);
		}
		
		RplComponent rplComp = RplComponent.Cast(Replication.FindItem(droneId));
		RplIdentity rplIdentity = GetGame().GetPlayerManager().GetPlayerController(playerId).GetRplIdentity();
		rplComp.EnableStreamingConNode(rplIdentity, false);
		
		IEntity drone = RplComponent.Cast(Replication.FindItem(droneId)).GetEntity();
		if (!drone)
			return;
		SAL_DroneControllerComponent droneComp = SAL_DroneControllerComponent.Cast(drone.FindComponent(SAL_DroneControllerComponent));
		droneComp.m_bIsConnected = true;
		droneComp.m_iOwner = playerId;
		SAL_DroneCleanUpComponent cleanUp = SAL_DroneCleanUpComponent.Cast(drone.FindComponent(SAL_DroneCleanUpComponent));
		cleanUp.m_bIsConnected = true;
		Rpc(RpcDo_ConnectToDroneroadcast, droneId, playerId);
		Replication.BumpMe();

		SCR_ExplosiveTriggerComponent explComp = SCR_ExplosiveTriggerComponent.Cast(drone.FindComponent(SCR_ExplosiveTriggerComponent));
		if (GetDronesOwner(GetRplId(drone)) != -1 && explComp)
			explComp.GetInstigator().SetInstigatorByPlayerID(GetDronesOwner(GetRplId(drone)));
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_ConnectToDroneroadcast(RplId droneId, int playerId)
	{
		if (!Replication.FindItem(droneId))
			return;
		
		IEntity drone = RplComponent.Cast(Replication.FindItem(droneId)).GetEntity();
		if (!drone)
			return;
		SAL_DroneControllerComponent droneComp = SAL_DroneControllerComponent.Cast(drone.FindComponent(SAL_DroneControllerComponent));
		droneComp.m_bIsConnected = true;
		droneComp.m_iOwner = playerId;
		SAL_DroneCleanUpComponent cleanUp = SAL_DroneCleanUpComponent.Cast(drone.FindComponent(SAL_DroneCleanUpComponent));
		cleanUp.m_bIsConnected = true;
	}
	
	void DisconnectDrone(RplId droneId, int playerId)
	{
		int index = m_aDroneOwners.Find(playerId);
		if (index == -1)
			return;
		
		SAL_DroneConnectContainer droneContainer = m_aDrones.Get(index);
		if (droneContainer.GetDroneConnection(0) == droneId)
			droneContainer.SetDroneConnection(RplId.Invalid(), 0);
		else if (droneContainer.GetDroneConnection(1) == droneId)
			droneContainer.SetDroneConnection(RplId.Invalid(), 1);
		else if (droneContainer.GetDroneConnection(2) == droneId)
			droneContainer.SetDroneConnection(RplId.Invalid(), 2);
		
		if (Replication.FindItem(droneId))
		{
			RplComponent rplComp = RplComponent.Cast(Replication.FindItem(droneId));
			RplIdentity rplIdentity = GetGame().GetPlayerManager().GetPlayerController(m_aDroneOwners.Get(index)).GetRplIdentity();
			rplComp.EnableStreamingConNode(rplIdentity, true);
			IEntity drone = RplComponent.Cast(Replication.FindItem(droneId)).GetEntity();
			if (!drone)
				return;
			SAL_DroneControllerComponent droneComp = SAL_DroneControllerComponent.Cast(drone.FindComponent(SAL_DroneControllerComponent));
			droneComp.m_bIsConnected = false;
			droneComp.m_iOwner = -1;
			SAL_DroneCleanUpComponent cleanUp = SAL_DroneCleanUpComponent.Cast(drone.FindComponent(SAL_DroneCleanUpComponent));
			cleanUp.m_bIsConnected = false;
			if (droneComp.m_bIsArmed)
			{
				droneComp.m_bIsArmed = false;
				SAL_DroneNetworkPacket packet = new SAL_DroneNetworkPacket;
				packet.SetDrone(droneId);
				packet.SetIsArmed(false);	
				Rpc(RpcDo_DisarmDrone, packet);
			}
			Rpc(RpcDo_DisconnectToDroneroadcast, droneId);
 		}		
		
		if (IsContainerEmpty(droneContainer))
		{
			m_aDrones.RemoveOrdered(index);
			m_aDroneOwners.RemoveOrdered(index);
		}
		Replication.BumpMe();
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_DisconnectToDroneroadcast(RplId droneId)
	{
		if (!Replication.FindItem(droneId))
			return;
		
		IEntity drone = RplComponent.Cast(Replication.FindItem(droneId)).GetEntity();
		if (!drone)
			return;
		SAL_DroneControllerComponent droneComp = SAL_DroneControllerComponent.Cast(drone.FindComponent(SAL_DroneControllerComponent));
		droneComp.m_bIsConnected = false;
		droneComp.m_iOwner = -1;
		SAL_DroneCleanUpComponent cleanUp = SAL_DroneCleanUpComponent.Cast(drone.FindComponent(SAL_DroneCleanUpComponent));
		cleanUp.m_bIsConnected = false;
	}
	
	
	void ExplodeDrone(SAL_DroneNetworkPacket packet)
	{
		if (!Replication.FindItem(packet.GetDrone()))
			return;
		
		IEntity drone = RplComponent.Cast(Replication.FindItem(packet.GetDrone())).GetEntity();
		if (!drone)
			return;
		
		SCR_ExplosiveTriggerComponent explComp = SCR_ExplosiveTriggerComponent.Cast(drone.FindComponent(SCR_ExplosiveTriggerComponent));
		explComp.UseTrigger();
		GetGame().GetCallqueue().CallLater(SCR_EntityHelper.DeleteEntityAndChildren, 1000, false, drone);
	}
	
	void ExplodeDroneServer(SAL_DroneNetworkPacket packet)
	{	
		ExplodeDrone(packet);
	}
	
	
	void ReplicateTransform(SAL_DroneNetworkPacket packet)
	{
		if (packet.GetDrone() == RplId.Invalid())
			return;
		
		if (!Replication.FindItem(packet.GetDrone()))
			return;
		
		IEntity drone = RplComponent.Cast(Replication.FindItem(packet.GetDrone())).GetEntity();
		if (!drone)
			return;
		
		ref array<int> playerIds = {};
		PlayerManager playerMan = GetGame().GetPlayerManager();
		playerMan.GetPlayers(playerIds);
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (!core)
			return;
		
		foreach (int playerId: playerIds)
		{
			IEntity player = playerMan.GetPlayerControlledEntity(playerId);
			if (!player)
				continue;
			
			PlayerController pc = playerMan.GetPlayerController(playerId);
			if (!pc)
				continue;
			
			bool isInEditor = core.GetEditorManager(pc.GetPlayerId()).IsOpened();
			
			float distance = vector.Distance(player.GetOrigin(), drone.GetOrigin());
			if (distance >= 1500 && !isInEditor)
				continue;
			
			SCR_PlayerController.Cast(pc).ReplicateTransform(packet);
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
		
		if (!IsDronePlayers(drone))  // Only apply if not the controller
			SAL_DroneControllerComponent.Cast(drone.FindComponent(SAL_DroneControllerComponent)).m_bIsArmed = packet.GetIsArmed();
		
	}
	
	void RegisterJammer(RplId ownerId)
	{
		if (ownerId == RplId.Invalid())
			return;
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
		
		SAL_DroneControllerComponent droneController = SAL_DroneControllerComponent.Cast(drone.FindComponent(SAL_DroneControllerComponent));
		if (droneController.m_iOwner != SCR_PlayerController.GetLocalPlayerId())  // Only apply if not the controller
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
	
	void DestroyDroneServer(RplId droneId, ResourceName droneWreckPrefab, Instigator instagator)
	{
		if (!Replication.FindItem(droneId))
			return;
		
		IEntity drone = RplComponent.Cast(Replication.FindItem(droneId)).GetEntity();
		if (!drone)
			return;
		
		if (drone.FindComponent(SAL_DroneExplosionComponent))
		{
			SlotManagerComponent slotComp = SlotManagerComponent.Cast(drone.FindComponent(SlotManagerComponent));
			SCR_ExplosiveTriggerComponent explComp = SCR_ExplosiveTriggerComponent.Cast(drone.FindComponent(SCR_ExplosiveTriggerComponent));
			explComp.GetInstigator().SetInstigatorByPlayerID(instagator.GetInstigatorPlayerID());
			SAL_DroneNetworkPacket packet = new SAL_DroneNetworkPacket();
			packet.SetDrone(droneId);
			ExplodeDrone(packet);
		}
		else
		{
			vector transform[4];
			drone.GetTransform(transform);
			EntitySpawnParams params = new EntitySpawnParams();
			params.Transform = transform;
			IEntity newDrone = GetGame().SpawnEntityPrefab(Resource.Load(droneWreckPrefab), GetGame().GetWorld(), params);	
			
			GetGame().GetCallqueue().CallLater(DeleteDrone, 200, false, drone.GetPhysics().GetVelocity(), newDrone, drone);
		}		
	}
	
	void DestroyDroneServerHealth(RplId droneId, ResourceName droneWreckPrefab, int playerId)
	{
		if (!Replication.FindItem(droneId))
			return;
		
		IEntity drone = RplComponent.Cast(Replication.FindItem(droneId)).GetEntity();
		if (!drone)
			return;
		
		if (drone.FindComponent(SAL_DroneExplosionComponent))
		{
			SlotManagerComponent slotComp = SlotManagerComponent.Cast(drone.FindComponent(SlotManagerComponent));
			SCR_ExplosiveTriggerComponent explComp = SCR_ExplosiveTriggerComponent.Cast(drone.FindComponent(SCR_ExplosiveTriggerComponent));
			explComp.GetInstigator().SetInstigatorByPlayerID(playerId);
			SAL_DroneNetworkPacket packet = new SAL_DroneNetworkPacket();
			packet.SetDrone(droneId);
			ExplodeDrone(packet);
		}
		else
		{
			vector transform[4];
			drone.GetTransform(transform);
			EntitySpawnParams params = new EntitySpawnParams();
			params.Transform = transform;
			IEntity newDrone = GetGame().SpawnEntityPrefab(Resource.Load(droneWreckPrefab), GetGame().GetWorld(), params);	
			
			GetGame().GetCallqueue().CallLater(DeleteDrone, 200, false, drone.GetPhysics().GetVelocity(), newDrone, drone);
		}		
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
		
		factComp.SetAffiliatedFactionByKey(factionKey);
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
	
	void DestroyDroneHealth(RplId droneId, ResourceName droneWreckPrefab, int playerId)
	{
		if (!Replication.FindItem(droneId))
			return;
		
		IEntity drone = RplComponent.Cast(Replication.FindItem(droneId)).GetEntity();
		if (!drone)
			return;
		
		if (drone.FindComponent(SAL_DroneExplosionComponent))
		{
			SlotManagerComponent slotComp = SlotManagerComponent.Cast(drone.FindComponent(SlotManagerComponent));
			SCR_ExplosiveTriggerComponent explComp = SCR_ExplosiveTriggerComponent.Cast(drone.FindComponent(SCR_ExplosiveTriggerComponent));
			explComp.GetInstigator().SetInstigatorByPlayerID(playerId);
			SAL_DroneNetworkPacket packet = new SAL_DroneNetworkPacket();
			packet.SetDrone(droneId);
			SAL_DroneConnectionManager.GetInstance().ExplodeDrone(packet);
		}
		else
		{
			vector transform[4];
			drone.GetTransform(transform);
			EntitySpawnParams params = new EntitySpawnParams();
			params.Transform = transform;
			IEntity newDrone = GetGame().SpawnEntityPrefab(Resource.Load(droneWreckPrefab), GetGame().GetWorld(), params);	
			GetGame().GetCallqueue().CallLater(DeleteDrone, 200, false, drone.GetPhysics().GetVelocity(), newDrone, drone);
		}	
	}
	
	void ConnectAntenna(RplId antennaId, int playerId)
	{
		if (!Replication.FindItem(antennaId))
			return;
		
		int index = -1;
		if (!m_aDroneOwners.Contains(playerId))
		{
			SAL_DroneConnectContainer droneContainer = new SAL_DroneConnectContainer();
			index = m_aDrones.Insert(droneContainer);
			m_aDroneOwners.Insert(playerId);
		}
		else
			index = m_aDroneOwners.Find(playerId);
		
		if (index == -1)
			return;
		
		RplComponent rplComp = RplComponent.Cast(Replication.FindItem(antennaId));
		RplIdentity rplIdentity = GetGame().GetPlayerManager().GetPlayerController(playerId).GetRplIdentity();
		rplComp.EnableStreamingConNode(rplIdentity, false);
			
		IEntity antenna = RplComponent.Cast(Replication.FindItem(antennaId)).GetEntity();
		if (!antenna)
			return;
		SAL_DroneAntennaComponent antennaComp = SAL_DroneAntennaComponent.Cast(antenna.FindComponent(SAL_DroneAntennaComponent));
		antennaComp.m_bIsConnected = true;
		antennaComp.m_iOwner = playerId;
		SAL_DroneCleanUpComponent cleanUp = SAL_DroneCleanUpComponent.Cast(antenna.FindComponent(SAL_DroneCleanUpComponent));
		cleanUp.m_bIsConnected = true;
		
		Rpc(RpcDo_ConnectAntennaBroadcast, antennaId, playerId);
		
		SAL_DroneConnectContainer playerDroneContainer = m_aDrones.Get(index);
		playerDroneContainer.SetDroneAntenna(antennaId);
		
		
		Replication.BumpMe();
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_ConnectAntennaBroadcast(RplId antennaId, int playerId)
	{
		if (!Replication.FindItem(antennaId))
			return;
		
		IEntity antenna = RplComponent.Cast(Replication.FindItem(antennaId)).GetEntity();
		if (!antenna)
			return;
		SAL_DroneAntennaComponent antennaComp = SAL_DroneAntennaComponent.Cast(antenna.FindComponent(SAL_DroneAntennaComponent));
		antennaComp.m_bIsConnected = true;
		antennaComp.m_iOwner = playerId;
		SAL_DroneCleanUpComponent cleanUp = SAL_DroneCleanUpComponent.Cast(antenna.FindComponent(SAL_DroneCleanUpComponent));
		cleanUp.m_bIsConnected = true;
	}
	
	void DisconnectAntenna(RplId antennaId)
	{
		int playerId = GetAntennaOwner(antennaId);
		int index = m_aDroneOwners.Find(playerId);
		if (index == -1)
			return;
		
		SAL_DroneConnectContainer droneContainer = m_aDrones.Get(index);
		if (droneContainer.GetDroneAntenna() == antennaId)
			droneContainer.SetDroneAntenna(RplId.Invalid());
		
		if (Replication.FindItem(antennaId))
		{
			RplComponent rplComp = RplComponent.Cast(Replication.FindItem(antennaId));
			RplIdentity rplIdentity = GetGame().GetPlayerManager().GetPlayerController(playerId).GetRplIdentity();
			rplComp.EnableStreamingConNode(rplIdentity, true);
			
			IEntity antenna = RplComponent.Cast(Replication.FindItem(antennaId)).GetEntity();
			if (!antenna)
				return;
			SAL_DroneAntennaComponent antennaComp = SAL_DroneAntennaComponent.Cast(antenna.FindComponent(SAL_DroneAntennaComponent));
			antennaComp.m_bIsConnected = false;
			antennaComp.m_iOwner = -1;
			SAL_DroneCleanUpComponent cleanUp = SAL_DroneCleanUpComponent.Cast(antenna.FindComponent(SAL_DroneCleanUpComponent));
			cleanUp.m_bIsConnected = false;
			
			Rpc(RpcDo_DisConnectAntennaBroadcast, antennaId);
		}
		
		if (IsContainerEmpty(droneContainer))
		{
			m_aDrones.RemoveOrdered(index);
			m_aDroneOwners.RemoveOrdered(index);
		}
		
		Replication.BumpMe();
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_DisConnectAntennaBroadcast(RplId antennaId)
	{
		if (!Replication.FindItem(antennaId))
			return;
		
		IEntity antenna = RplComponent.Cast(Replication.FindItem(antennaId)).GetEntity();
		if (!antenna)
			return;
		SAL_DroneAntennaComponent antennaComp = SAL_DroneAntennaComponent.Cast(antenna.FindComponent(SAL_DroneAntennaComponent));
		antennaComp.m_bIsConnected = false;
		antennaComp.m_iOwner = -1;
		SAL_DroneCleanUpComponent cleanUp = SAL_DroneCleanUpComponent.Cast(antenna.FindComponent(SAL_DroneCleanUpComponent));
		cleanUp.m_bIsConnected = false;
	}
	
	void ConnectDroneCommandStation(RplId commandId, int playerId)
	{
		if (!Replication.FindItem(commandId))
			return;
		
		int index = -1;
		if (!m_aDroneOwners.Contains(playerId))
		{
			SAL_DroneConnectContainer droneContainer = new SAL_DroneConnectContainer();
			index = m_aDrones.Insert(droneContainer);
			m_aDroneOwners.Insert(playerId);
		}
		else
			index = m_aDroneOwners.Find(playerId);
		
		if (index == -1)
			return;
		
		
		RplComponent rplComp = RplComponent.Cast(Replication.FindItem(commandId));
		RplIdentity rplIdentity = GetGame().GetPlayerManager().GetPlayerController(m_aDroneOwners.Get(index)).GetRplIdentity();
		rplComp.EnableStreamingConNode(rplIdentity, false);
		
		IEntity commandStation = RplComponent.Cast(Replication.FindItem(commandId)).GetEntity();
		if (!commandStation)
			return;
		SAL_DroneCommandStationComponent commandComp = SAL_DroneCommandStationComponent.Cast(commandStation.FindComponent(SAL_DroneCommandStationComponent));
		commandComp.m_bIsConnected = true;
		commandComp.m_iOwner = playerId;
		SAL_DroneCleanUpComponent cleanUp = SAL_DroneCleanUpComponent.Cast(commandStation.FindComponent(SAL_DroneCleanUpComponent));
		cleanUp.m_bIsConnected = true;
		
		Rpc(RpcDo_ConnectDroneCommandStationBroadcast, commandId, playerId);
		
		SAL_DroneConnectContainer playerDroneContainer = m_aDrones.Get(index);
		playerDroneContainer.SetDroneCommandStation(commandId);
		Replication.BumpMe();
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_ConnectDroneCommandStationBroadcast(RplId commandId, int playerId)
	{
		if (!Replication.FindItem(commandId))
			return;
		
		IEntity commandStation = RplComponent.Cast(Replication.FindItem(commandId)).GetEntity();
		if (!commandStation)
			return;
		SAL_DroneCommandStationComponent commandComp = SAL_DroneCommandStationComponent.Cast(commandStation.FindComponent(SAL_DroneCommandStationComponent));
		commandComp.m_bIsConnected = true;
		commandComp.m_iOwner = playerId;
		SAL_DroneCleanUpComponent cleanUp = SAL_DroneCleanUpComponent.Cast(commandStation.FindComponent(SAL_DroneCleanUpComponent));
		cleanUp.m_bIsConnected = true;
	}
	
	void DisconnectDroneCommandStation(RplId commandId)
	{
		int playerId = GetCommandStationOwner(commandId);
		int index = m_aDroneOwners.Find(playerId);
		if (index == -1)
			return;
		
		if (Replication.FindItem(commandId))
		{
			RplComponent rplComp = RplComponent.Cast(Replication.FindItem(commandId));
			RplIdentity rplIdentity = GetGame().GetPlayerManager().GetPlayerController(m_aDroneOwners.Get(index)).GetRplIdentity();
			rplComp.EnableStreamingConNode(rplIdentity, true);
			
			IEntity commandStation = RplComponent.Cast(Replication.FindItem(commandId)).GetEntity();
			if (!commandStation)
			return;
			SAL_DroneCommandStationComponent commandComp = SAL_DroneCommandStationComponent.Cast(commandStation.FindComponent(SAL_DroneCommandStationComponent));
			commandComp.m_bIsConnected = false;
			commandComp.m_iOwner = -1;
			SAL_DroneCleanUpComponent cleanUp = SAL_DroneCleanUpComponent.Cast(commandStation.FindComponent(SAL_DroneCleanUpComponent));
			cleanUp.m_bIsConnected = false;
			
			Rpc(RpcDo_DisconnectDroneCommandStationBroadcast, commandId);
 		}	
		
		SAL_DroneConnectContainer droneContainer = m_aDrones.Get(index);
		if (droneContainer.GetDroneCommandStation() == commandId)
			droneContainer.SetDroneCommandStation(RplId.Invalid());
		
		if (droneContainer.GetDroneConnection(0) != RplId.Invalid())
			DisconnectDrone(droneContainer.GetDroneConnection(0), playerId);
		
		if (droneContainer.GetDroneConnection(1) != RplId.Invalid())
			DisconnectDrone(droneContainer.GetDroneConnection(1), playerId);
		
		if (droneContainer.GetDroneConnection(2) != RplId.Invalid())
			DisconnectDrone(droneContainer.GetDroneConnection(2), playerId);
		
		if (IsContainerEmpty(droneContainer))
		{
			m_aDrones.RemoveOrdered(index);
			m_aDroneOwners.RemoveOrdered(index);
		}
		
		Replication.BumpMe();
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_DisconnectDroneCommandStationBroadcast(RplId commandId)
	{
		if (!Replication.FindItem(commandId))
			return;
		
		IEntity commandStation = RplComponent.Cast(Replication.FindItem(commandId)).GetEntity();
		if (!commandStation)
			return;
		SAL_DroneCommandStationComponent commandComp = SAL_DroneCommandStationComponent.Cast(commandStation.FindComponent(SAL_DroneCommandStationComponent));
		commandComp.m_bIsConnected = false;
		commandComp.m_iOwner = -1;
		SAL_DroneCleanUpComponent cleanUp = SAL_DroneCleanUpComponent.Cast(commandStation.FindComponent(SAL_DroneCleanUpComponent));
		cleanUp.m_bIsConnected = false;
	}
	
	void TryInsertItemToInventory(IEntity newItem, BaseInventoryStorageComponent baseStorageComp, SCR_InventoryStorageManagerComponent storageComp)
	{
		if (!baseStorageComp)
			{
				storageComp.TryInsertItem(newItem);
				return;
			}
				
			if (newItem.FindComponent(BaseLoadoutClothComponent))
			{
				if (storageComp.CanReplaceItem(newItem, baseStorageComp, 0))
					storageComp.TryReplaceItem(newItem, baseStorageComp, 0);
				else
					storageComp.TryInsertItem(newItem);
			}
			else
				storageComp.TryInsertItem(newItem);
	}
	
	void BroadcastSlotAttachment(IEntity caseObject, string slotName, IEntity newItem)
	{
		Rpc(RpcDo_BroadcastSlotAttachment, GetRplId(caseObject), slotName, GetRplId(newItem));
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_BroadcastSlotAttachment(RplId caseId, string slotName, RplId newItem)
	{
		if (!Replication.FindItem(caseId) || !Replication.FindItem(newItem))
			return;
		
		IEntity caseObject = RplComponent.Cast(Replication.FindItem(caseId)).GetEntity();
		if (!caseObject)
			return;
		
		IEntity item = RplComponent.Cast(Replication.FindItem(newItem)).GetEntity();
		if (!item)
			return;
		
		SlotManagerComponent slotMan = SlotManagerComponent.Cast(caseObject.FindComponent(SlotManagerComponent));
		slotMan.GetSlotByName(slotName).AttachEntity(item);
	}
	
	void TeleportDroneBack(RplId droneId, vector transform[4])
	{
		Rpc(RpcDo_TeleportDroneBack, droneId, transform);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_TeleportDroneBack(RplId droneId, vector transform[4])
	{
		if (!Replication.FindItem(droneId))
			return;
		
		IEntity drone = RplComponent.Cast(Replication.FindItem(droneId)).GetEntity();
		if (!drone)
			return;
		
		if (!IsDronePlayers(drone))
			return;
		
		GenericEntity droneEntity = GenericEntity.Cast(drone);
		droneEntity.SetTransform(transform);
		droneEntity.Update();
		droneEntity.OnTransformReset();
	}
}