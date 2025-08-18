//------------------------------------------------------------------------------------------------
//! modded version for to be used with the inventory 2.0 
class SAL_CaseEquipItem : SCR_EquipWeaponAction
{
	#ifndef DISABLE_INVENTORY
	SAL_DroneConnectionManager m_DroneManager;
	IEntity m_ItemObject;
	
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		super.Init(pOwnerEntity, pManagerComponent);
		m_DroneManager = SAL_DroneConnectionManager.GetInstance();;
		m_ItemObject = pOwnerEntity;
	}
	
	override bool CanBeShownScript(IEntity user)
	{
		if (m_ItemObject.GetParent() != null)
			return false;
		
		return super.CanBeShownScript(user);
		
	}
	#endif
};