modded class SCR_InventoryStorageManagerComponent
{
	override void InsertItem( IEntity pItem, BaseInventoryStorageComponent pStorageTo = null, BaseInventoryStorageComponent pStorageFrom = null, SCR_InvCallBack cb = null  )
	{
		SAL_DroneConnectionManager droneManager = SAL_DroneConnectionManager.GetInstance();
		if (pItem.FindComponent(SAL_DroneControllerComponent))
			if (droneManager.IsDroneConnected(SAL_DroneConnectionManager.GetRplId(pItem)))
				return;
		
		if (pItem.FindComponent(SAL_DroneAntennaComponent))
			if (droneManager.IsAntennaConnected(SAL_DroneConnectionManager.GetRplId(pItem)))
				return;
		
		if (pItem.FindComponent(SAL_DroneCommandStationComponent))
			if (droneManager.IsCommandStationConnected(SAL_DroneConnectionManager.GetRplId(pItem)))
				return;
		
		super.InsertItem(pItem, pStorageTo, pStorageFrom, cb);
	}
}