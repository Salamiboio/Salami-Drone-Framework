class SAL_ReloadDrone : SCR_InventoryAction
{
	IEntity m_ePlayer;
	IEntity m_eDrone;
	SAL_DroneConnectionManager m_DroneManager;
	SAL_DroneControllerComponent m_DroneController;
	SAL_DropperComponent m_DropperComponent;
	SlotManagerComponent m_SlotManager;
	ResourceName m_rGrenadePrefab;
	IEntity m_eGrenade;
	
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
	{
		SCR_PlayerController.Cast(GetGame().GetPlayerController()).ReloadDrone(pOwnerEntity, m_eGrenade);
	}
	
	bool HasGrenade(IEntity user)
	{
		InventoryStorageManagerComponent invComp = InventoryStorageManagerComponent.Cast(user.FindComponent(InventoryStorageManagerComponent));
		if (!invComp)
			return false;
		
		ref array<IEntity> items = {};
		invComp.GetItems(items);
		
		foreach (IEntity item: items)
		{
			if (item.GetPrefabData().GetPrefabName() == m_rGrenadePrefab)
			{
				m_eGrenade = item;
				return true;
			}
				
		}
		return false;
	}
	
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		m_eDrone = pOwnerEntity;
		m_DroneManager = SAL_DroneConnectionManager.GetInstance();
		m_DroneController = SAL_DroneControllerComponent.Cast(pOwnerEntity.FindComponent(SAL_DroneControllerComponent));
		m_DropperComponent = SAL_DropperComponent.Cast(pOwnerEntity.FindComponent(SAL_DropperComponent));
		m_rGrenadePrefab = m_DropperComponent.m_DropperGrenadeReload;
		m_SlotManager =  SlotManagerComponent.Cast(pOwnerEntity.FindComponent(SlotManagerComponent));
	}

	
	override bool CanBeShownScript(IEntity user)
	{
		if (!HasGrenade(user))
			return false;
		
		if (!m_DropperComponent.m_BGrenadeDropped)
			return false;
		
		return true;
	}
}