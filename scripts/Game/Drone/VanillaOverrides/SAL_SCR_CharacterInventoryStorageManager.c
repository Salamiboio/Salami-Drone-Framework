modded class SCR_CharacterInventoryStorageComponent
{
	override void HandleOnItemAddedToInventory( IEntity item, BaseInventoryStorageComponent storageOwner )
	{
		super.HandleOnItemAddedToInventory(item, storageOwner);
		
		if (!item)
			return;
		
		if (!GetGame().GetPlayerController())
			return;
		
		if (item.GetPrefabData().GetPrefabName() == "{8951045BFE8BC8E4}Prefabs/Characters/HeadGear/FPV_Goggles.et")
		{
			if (LoadoutSlotInfo.Cast(storageOwner.FindItemSlot(item)))
			{
				SCR_PlayerController.Cast(GetGame().GetPlayerController()).EnterDrone();
			}
			else
				SCR_PlayerController.Cast(GetGame().GetPlayerController()).ExitDrone();
		}
		
		if (item.FindComponent(SAL_DroneControllerComponent))
		{
			if (!GetGame().GetCameraManager().CurrentCamera())
				return;
			
			if (GetGame().GetCameraManager().CurrentCamera().GetPrefabData().GetPrefabName() == "{D10C3C304FC29655}Prefabs/Editor/Camera/DroneCamera.et")
				SCR_PlayerController.Cast(GetGame().GetPlayerController()).ExitDrone();
		}
	}
	
	override void HandleOnItemRemovedFromInventory( IEntity item, BaseInventoryStorageComponent storageOwner )
	{
		super.HandleOnItemRemovedFromInventory(item, storageOwner);
		
		if (!item)
			return;
		
		if (!GetGame().GetPlayerController())
			return;
		
		GetGame().GetCallqueue().CallLater(GoggleCheck, 100, false, item, storageOwner);
	}
	
	void GoggleCheck(IEntity item, BaseInventoryStorageComponent storageOwner)
	{
		if (!item)
			return;	
		
		if (item.GetPrefabData().GetPrefabName() == "{8951045BFE8BC8E4}Prefabs/Characters/HeadGear/FPV_Goggles.et")
			if (!InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent)).GetParentSlot())
				SCR_PlayerController.Cast(GetGame().GetPlayerController()).ExitDrone();
		
		if (item.FindComponent(SAL_DroneControllerComponent))
		{
			//GoggleCheck to throw em into the drone
			if (SAL_DroneConnectionManager.GetInstance().IsDronePlayers(item))
				if(SCR_CharacterInventoryStorageComponent.Cast(SCR_PlayerController.GetLocalControlledEntity().FindComponent(SCR_CharacterInventoryStorageComponent)).Get(0))
					if (SCR_CharacterInventoryStorageComponent.Cast(SCR_PlayerController.GetLocalControlledEntity().FindComponent(SCR_CharacterInventoryStorageComponent)).Get(0).GetPrefabData().GetPrefabName() == "{8951045BFE8BC8E4}Prefabs/Characters/HeadGear/FPV_Goggles.et")
							SCR_PlayerController.Cast(GetGame().GetPlayerController()).EnterDrone();
		}
			

	}
}