//Made by Salami._.
class SAL_DroneReloadAction : SCR_InventoryAction
{
	IEntity m_eDrone;
	IEntity m_eAmmo;
	
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
	{
		SAL_DroneClientComponent.GetInstance().ReloadDrone(m_eDrone, m_eAmmo);
	}
	
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		m_eDrone = pOwnerEntity;
	}
	
	override bool CanBeShownScript(IEntity user)
	{
		if(WeaponSlotComponent.Cast(m_eDrone.FindComponent(WeaponSlotComponent)).GetCurrentMagazine())
			return false;
		
		ref array<IEntity> items = {};
		SCR_InventoryStorageManagerComponent.Cast(user.FindComponent(SCR_InventoryStorageManagerComponent)).GetItems(items);
		foreach(IEntity item: items)
		{
			if(item.GetPrefabData())
				if(item.GetPrefabData().GetPrefabName() == SAL_DroneComponent.Cast(m_eDrone.FindComponent(SAL_DroneComponent)).GetAmmo())
				{
					m_eAmmo = item;
					return true;
				}	
		}
		return false;
	}
}