//Made by Salami._.
[ComponentEditorProps(category: "GameScripted/GameMode/Components", description: "Base for gamemode scripted component.")]
class SAL_DroneGamemodeComponentClass : SCR_BaseGameModeComponentClass
{
}

class SAL_DroneGamemodeComponent : SCR_BaseGameModeComponent
{
	
	[RplProp()]
	protected ref array<RplId> m_aDronesConnected = {};
	
	[RplProp()]
	protected RplId m_rToInsert = RplId.Invalid();
	
	[RplProp()]
	protected ref array<RplId> m_aJammerBags = {};
	
	[RplProp()]
	protected RplId m_rJammerBag;
	
	[RplProp()]
	protected ref array<string> m_aJammerBagFactions = {};
	
	[RplProp()]
	protected string m_sJammerBagFaction;
	
	[RplProp()]
	protected ref array<bool> m_aJammerBagPowers = {};
	
	[RplProp()]
	protected bool m_bJammerBagPower;
	
	//Setup
	//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	override void OnControllableSpawned(IEntity entity)
	{
		super.OnControllableSpawned(entity);
		
		if(entity.FindComponent(SCR_InventoryStorageManagerComponent))
		{
			SCR_InventoryStorageManagerComponent inventoryManager = SCR_InventoryStorageManagerComponent.Cast(entity.FindComponent(SCR_InventoryStorageManagerComponent));
			inventoryManager.m_OnItemRemovedInvoker.Insert(SpawnDroneGround);
			inventoryManager.m_OnItemAddedInvoker.Insert(GroundJammerBagCheck);
		}
			
		if(entity.GetPrefabData())
			if(entity.GetPrefabData().GetPrefabName() == "{FD8FB88F6413745F}Prefabs/Characters/DroneControllerPrefab.et")
				GetGame().GetCallqueue().CallLater(SetObjectInvisible, 10, false, entity);
		
	}
	
	
	//Get Members
	//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------		
	static SAL_DroneGamemodeComponent GetInstance()
	{
		if (GetGame().GetGameMode())
			return SAL_DroneGamemodeComponent.Cast(GetGame().GetGameMode().FindComponent(SAL_DroneGamemodeComponent));
		else
			return null;
	}
	
	RplId GetRplId(IEntity entity)
	{
		return RplComponent.Cast(entity.FindComponent(RplComponent)).Id();
	}
	
	array<RplId> GetConnectedDrones()
	{
		return m_aDronesConnected;
	}
	
	array<RplId> GetJammerBags()
	{
		return m_aJammerBags;
	}
	
	array<string> GetJammerFactions()
	{
		return m_aJammerBagFactions;
	}
	
	array<bool> GetJammerPowers()
	{
		return m_aJammerBagPowers;
	}
	
	void UpdateJammers(IEntity entity, string factionKey, bool power)
	{
		Rpc(RpcDo_UpdateJammers, RplComponent.Cast(entity.FindComponent(RplComponent)).Id(), factionKey, power);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcDo_UpdateJammers(RplId entityID, string factionKey, bool power)
	{
		m_rJammerBag = entityID;
		Replication.BumpMe();
		m_aJammerBags.Insert(m_rJammerBag);
		Replication.BumpMe();
		m_sJammerBagFaction = factionKey;
		Replication.BumpMe();
		m_aJammerBagFactions.Insert(m_sJammerBagFaction);
		Replication.BumpMe();
		m_bJammerBagPower = power;
		Replication.BumpMe();
		m_aJammerBagPowers.Insert(m_bJammerBagPower);
		Replication.BumpMe();
	}
	
	void UpdatePower(IEntity entity, bool power)
	{
		Rpc(RpcDo_UpdatePower, GetRplId(entity), power);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcDo_UpdatePower(RplId entityID, bool power)
	{
		m_aJammerBagPowers.Set(m_aJammerBags.Find(entityID), power);
		Replication.BumpMe();
	}
	
	void UpdateFaction(IEntity entity, string faction)
	{
		Rpc(RpcDo_UpdateFaction, GetRplId(entity), faction);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcDo_UpdateFaction(RplId entityID, string faction)
	{
		m_aJammerBagFactions.Set(m_aJammerBags.Find(entityID), faction);
		Replication.BumpMe();
	}
	
	//Drone controller members
	//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void SpawnDroneController(IEntity entity)
	{
		Rpc(RpcDo_SpawnDroneController, GetRplId(entity))
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcDo_SpawnDroneController(RplId entityID)
	{
		IEntity entity = RplComponent.Cast(Replication.FindItem(entityID)).GetEntity();
		EntitySpawnParams params = new EntitySpawnParams();
		entity.GetTransform(params.Transform);
		params.Transform[3] + Vector(0, 250, 0);
		IEntity droneController = GetGame().SpawnEntityPrefab(Resource.Load("{FD8FB88F6413745F}Prefabs/Characters/DroneControllerPrefab.et"), null, params);
		
		BaseCompartmentManagerComponent compartmentManger = BaseCompartmentManagerComponent.Cast(entity.FindComponent(BaseCompartmentManagerComponent));
		if (!compartmentManger)
			return;
		
		ref array<BaseCompartmentSlot> compartments = {};
		compartmentManger.GetCompartments(compartments);
		BaseCompartmentSlot slot = compartments[0];
		if (!slot)
			return;
		
		SCR_CompartmentAccessComponent compartmentAccess = SCR_CompartmentAccessComponent.Cast(droneController.FindComponent(SCR_CompartmentAccessComponent));
		if (!compartmentAccess)
			return;
		
		compartmentAccess.GetInVehicle(entity, slot, true, -1, ECloseDoorAfterActions.INVALID, true);
	}
	
	void SetObjectInvisible(IEntity entity)
	{
		SCR_ChimeraCharacter.Cast(entity).SetVisibility(false);
		SCR_DamageManagerComponent.Cast(entity.FindComponent(SCR_DamageManagerComponent)).EnableDamageHandling(false);
	}
	
	//Spawns drone from inventory item on ground
	//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void SpawnDroneGround(IEntity item, BaseInventoryStorageComponent storageOwner)
	{
		if(item.FindComponent(SAL_DroneItemComponent))
			GetGame().GetCallqueue().CallLater(SpawnDrone, 100, false, item);
	}
	
	void SpawnDrone(IEntity entity)
	{
		if(!entity)
			return;
		Rpc(RpcDo_SpawnDrone, GetRplId(entity));
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcDo_SpawnDrone(RplId entityID)
	{
		IEntity entity = RplComponent.Cast(Replication.FindItem(entityID)).GetEntity();
		EntitySpawnParams spawnParams = new EntitySpawnParams();
		spawnParams.TransformMode = ETransformMode.WORLD;
		spawnParams.Transform[3] = entity.GetOrigin();
		SCR_TerrainHelper.OrientToTerrain(spawnParams.Transform);
		
		IEntity droneController = GetGame().SpawnEntityPrefab(Resource.Load(SAL_DroneItemComponent.Cast(entity.FindComponent(SAL_DroneItemComponent)).GetDroneToSpawn()), null, spawnParams);
		delete entity;
	}
	
	void GroundJammerBagCheck(IEntity item, BaseInventoryStorageComponent storageOwner)
	{
		if(item.FindComponent(SAL_DroneJammerComponent))
			GetGame().GetCallqueue().CallLater(DelayedGroundJammerBagCheck, 150, false, item);
	}
	
	void DelayedGroundJammerBagCheck(IEntity item)
	{
		Rpc(RpcDo_GroundJammerBagCheck, GetRplId(item));
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcDo_GroundJammerBagCheck(RplId itemID)
	{
		if(m_aJammerBagFactions.Get(m_aJammerBags.Find(itemID)) == "")
		{
			m_aJammerBagFactions.Set(m_aJammerBags.Find(itemID), SCR_FactionManager.SGetPlayerFaction(GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(RplComponent.Cast(Replication.FindItem(itemID)).GetEntity().GetParent())).GetFactionKey());
			Replication.BumpMe();
		}	
	}
	
	//Drone Connection Members
	//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void ConnectDrone(RplId droneID)
	{
		m_rToInsert = droneID;
		Replication.BumpMe();
		m_aDronesConnected.Insert(m_rToInsert);
		Replication.BumpMe();
	}
	
	void DisconnectDrone(RplId droneID)
	{
		m_rToInsert = droneID;
		Replication.BumpMe();
		m_aDronesConnected.RemoveItem(m_rToInsert);
		Replication.BumpMe();
	}	
}