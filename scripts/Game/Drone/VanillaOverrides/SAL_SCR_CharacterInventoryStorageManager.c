modded class SCR_CharacterInventoryStorageComponent
{
	override void HandleOnItemAddedToInventory( IEntity item, BaseInventoryStorageComponent storageOwner )
	{
		super.HandleOnItemAddedToInventory(item, storageOwner);
		
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
	}
	
	override void HandleOnItemRemovedFromInventory( IEntity item, BaseInventoryStorageComponent storageOwner )
	{
		super.HandleOnItemRemovedFromInventory(item, storageOwner);
		
		if (!GetGame().GetPlayerController())
			return;
		
		GetGame().GetCallqueue().CallLater(GoggleCheck, 100, false, item, storageOwner);
	}
	
	void GoggleCheck(IEntity item, BaseInventoryStorageComponent storageOwner)
	{
		if (item.GetPrefabData().GetPrefabName() == "{8951045BFE8BC8E4}Prefabs/Characters/HeadGear/FPV_Goggles.et")
			if (!InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent)).GetParentSlot())
				SCR_PlayerController.Cast(GetGame().GetPlayerController()).ExitDrone();
	}
}