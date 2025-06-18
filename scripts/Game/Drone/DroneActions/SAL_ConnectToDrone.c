//Made by Salami._.
class SAL_ConnectToDrone : SCR_InventoryAction
{
	IEntity m_ePlayer;
	IEntity m_eDrone;
	SAL_DroneConnectionManager m_DroneManager;
	
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
	{
		m_ePlayer = pUserEntity;
		m_eDrone = pOwnerEntity;
		
		SCR_PlayerController.Cast(GetGame().GetPlayerController()).ConnectPlayerToDrone(pOwnerEntity);
	}
	
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		m_eDrone = pOwnerEntity;
		m_DroneManager = SAL_DroneConnectionManager.GetInstance();
	}
//	
//	bool HasController()
//	{
//		ref array<IEntity> rootItems = {};
//		SCR_InventoryStorageManagerComponent.Cast(SCR_PlayerController.GetLocalControlledEntity().FindComponent(SCR_InventoryStorageManagerComponent)).GetAllRootItems(rootItems);
//		foreach(IEntity item: rootItems)
//		{
//			if(item.GetPrefabData().GetPrefabName() == SAL_DroneComponent.Cast(m_eDrone.FindComponent(SAL_DroneComponent)).GetControllerPrefab())
//				return true;
//		}
//		return false;
//	}
	
	override bool CanBeShownScript(IEntity user)
	{
//		if(!HasController())
//			return false;

		if (m_DroneManager.GetConnectedDrones().Find(RplComponent.Cast(m_eDrone.FindComponent(RplComponent)).Id()) != -1)
			return false;
		
		if (m_DroneManager.IsPlayerDroneOwner(SCR_PlayerController.GetLocalPlayerId()))
			return false;
		
		return true;
	}
	
}