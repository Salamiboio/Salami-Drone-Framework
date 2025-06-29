class SAL_ReplaceBattery : SCR_InventoryAction
{
	IEntity m_ePlayer;
	IEntity m_eDrone;
	SAL_DroneConnectionManager m_DroneManager;
	SAL_DroneControllerComponent m_DroneController;
	SAL_DroneBatteryComponent m_DroneBatteryComp;
	
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
	{
		m_ePlayer = pUserEntity;
		m_eDrone = pOwnerEntity;
		
		IEntity largestBattery = GetLargestBattery();
		SAL_DroneNetworkPacket packet = new SAL_DroneNetworkPacket();
		packet.SetDrone(RplComponent.Cast(pOwnerEntity.FindComponent(RplComponent)).Id());
		packet.SetBatteryLevel(SAL_BatteryComponent.Cast(largestBattery.FindComponent(SAL_BatteryComponent)).m_fBatteryStorage);
		SCR_PlayerController.Cast(GetGame().GetPlayerController()).UpdateBattery(packet);
		delete largestBattery;
	}
	
	IEntity GetLargestBattery()
	{
		InventoryStorageManagerComponent invComp = InventoryStorageManagerComponent.Cast(m_ePlayer.FindComponent(InventoryStorageManagerComponent));
		if (!invComp)
			return null;
		
		IEntity largestBattery;
		ref array<IEntity> items = {};
		invComp.GetItems(items);
		
		foreach (IEntity item: items)
		{
			if (!item.FindComponent(SAL_BatteryComponent))
				continue;
			
			if (SAL_BatteryComponent.Cast(item.FindComponent(SAL_BatteryComponent)).m_eBatteryType != m_DroneBatteryComp.m_eBatteryType)
				continue;
			
			if (!largestBattery)
			{
				largestBattery = item;
				continue;
			}
			
			SAL_BatteryComponent largestBattComp = SAL_BatteryComponent.Cast(largestBattery.FindComponent(SAL_BatteryComponent));
			SAL_BatteryComponent itemBattComp = SAL_BatteryComponent.Cast(item.FindComponent(SAL_BatteryComponent));
			
			if (largestBattComp.m_fBatteryStorage < itemBattComp.m_fBatteryStorage)
			{
				largestBattery = item;
				continue;
			}
		}
		
		return largestBattery;
	}
	
	bool HasBattery(IEntity user)
	{
		InventoryStorageManagerComponent invComp = InventoryStorageManagerComponent.Cast(user.FindComponent(InventoryStorageManagerComponent));
		if (!invComp)
			return false;
		
		IEntity largestBattery;
		ref array<IEntity> items = {};
		invComp.GetItems(items);
		
		foreach (IEntity item: items)
		{
			if (!item.FindComponent(SAL_BatteryComponent))
				continue;
			
			if (SAL_BatteryComponent.Cast(item.FindComponent(SAL_BatteryComponent)).m_eBatteryType != m_DroneBatteryComp.m_eBatteryType)
				continue;
			
			return true;
		}
		return false;
	}
	
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		m_eDrone = pOwnerEntity;
		m_DroneManager = SAL_DroneConnectionManager.GetInstance();
		m_DroneController = SAL_DroneControllerComponent.Cast(pOwnerEntity.FindComponent(SAL_DroneControllerComponent));
		m_DroneBatteryComp = SAL_DroneBatteryComponent.Cast(pOwnerEntity.FindComponent(SAL_DroneBatteryComponent));
	}

	
	override bool CanBeShownScript(IEntity user)
	{
		if (m_DroneController.m_bIsArmed)
			return false;
		
		if (!HasBattery(user))
			return false;
		
		
		
		return true;
	}
}