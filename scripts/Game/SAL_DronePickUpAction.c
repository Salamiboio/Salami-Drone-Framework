//Made by Salami._.
class SAL_DronePickUpAction : SCR_InventoryAction
{
	IEntity m_eDrone;
	
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
	{
		SAL_DroneClientComponent.GetInstance().PickUpDrone(pOwnerEntity);
	}
	
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		m_eDrone = pOwnerEntity;
	}
	
	override bool CanBeShownScript(IEntity user)
	{
		if(SAL_DroneGamemodeComponent.GetInstance().GetConnectedDrones().Find(RplComponent.Cast(m_eDrone.FindComponent(RplComponent)).Id()) != -1)
			return false;
		if(!SCR_InventoryStorageManagerComponent.Cast(user.FindComponent(SCR_InventoryStorageManagerComponent )).IsAnimationReady())
			return false;
		if(!SCR_InventoryStorageManagerComponent.Cast(user.FindComponent(SCR_InventoryStorageManagerComponent)).CanInsertResource(SAL_DroneComponent.Cast(m_eDrone.FindComponent(SAL_DroneComponent)).GetGroundItem(), EStoragePurpose.PURPOSE_DEPOSIT))
			return false;
		return true;
	}
}