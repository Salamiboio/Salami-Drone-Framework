//------------------------------------------------------------------------------------------------
//! modded version for to be used with the inventory 2.0 
class SAL_PickUpDroneAction : SCR_PickUpItemAction
{
	#ifndef DISABLE_INVENTORY
	SAL_DroneConnectionManager m_DroneManager;
	IEntity m_drone;
	//------------------------------------------------------------------------------------------------
	override protected void PerformActionInternal(SCR_InventoryStorageManagerComponent manager, IEntity pOwnerEntity, IEntity pUserEntity)
	{
		manager.InsertItem( pOwnerEntity );
	}
	
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		super.Init(pOwnerEntity, pManagerComponent);
		m_DroneManager = SAL_DroneConnectionManager.GetInstance();;
		m_drone = pOwnerEntity;
	}
	
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
 	{
		if (m_DroneManager.IsDronePlayers(m_drone))
			return false;
		
		if ( !super.CanBePerformedScript( user ) )
			return false;
		
		return true;
 	}
	#endif
};