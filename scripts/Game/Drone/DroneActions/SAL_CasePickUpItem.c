//------------------------------------------------------------------------------------------------
//! modded version for to be used with the inventory 2.0 
class SAL_CasePickUpItem : SCR_PickUpItemAction
{
	#ifndef DISABLE_INVENTORY
	SAL_DroneConnectionManager m_DroneManager;
	IEntity m_ItemObject;
	//------------------------------------------------------------------------------------------------
	override protected void PerformActionInternal(SCR_InventoryStorageManagerComponent manager, IEntity pOwnerEntity, IEntity pUserEntity)
	{
		manager.InsertItem( pOwnerEntity );
	}
	
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		super.Init(pOwnerEntity, pManagerComponent);
		m_DroneManager = SAL_DroneConnectionManager.GetInstance();;
		m_ItemObject = pOwnerEntity;
	}
	
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
 	{
		if ( !super.CanBePerformedScript( user ) )
			return false;
		
		return true;
 	}
	
	override bool CanBeShownScript(IEntity user)
	{
		if (m_ItemObject.GetParent() != null)
			return false;
		
		return super.CanBeShownScript(user);
		
	}
	#endif
};