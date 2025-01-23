//Made by Salami._.
class SAL_DroneConnectAction : SCR_InventoryAction
{
	IEntity m_ePlayer;
	IEntity m_eDrone;
	
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
	{
		m_ePlayer = pUserEntity;
		m_eDrone = pOwnerEntity;
		
		if(SAL_DroneGamemodeComponent.GetInstance().GetConnectedDrones().Find(RplComponent.Cast(m_eDrone.FindComponent(RplComponent)).Id()) != -1)
			return;
			
		SAL_DroneClientComponent.GetInstance().SetConnectedDrone(RplComponent.Cast(m_eDrone.FindComponent(RplComponent)).Id(), true);
	}
	
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		m_eDrone = pOwnerEntity;
	}
	
	bool HasController()
	{
		ref array<IEntity> rootItems = {};
		SCR_InventoryStorageManagerComponent.Cast(SCR_PlayerController.GetLocalControlledEntity().FindComponent(SCR_InventoryStorageManagerComponent)).GetAllRootItems(rootItems);
		foreach(IEntity item: rootItems)
		{
			if(item.GetPrefabData().GetPrefabName() == SAL_DroneComponent.Cast(m_eDrone.FindComponent(SAL_DroneComponent)).GetControllerPrefab())
				return true;
		}
		return false;
	}
	
	override bool CanBeShownScript(IEntity user)
	{
		if(!HasController())
			return false;
		
		if(SAL_DroneGamemodeComponent.GetInstance().GetConnectedDrones().Find(RplComponent.Cast(m_eDrone.FindComponent(RplComponent)).Id()) != -1)
			return false;
		
		if(SAL_DroneClientComponent.GetInstance().GetConnectedDrone() != null)
			return false;
		
		return true;
	}
	
}