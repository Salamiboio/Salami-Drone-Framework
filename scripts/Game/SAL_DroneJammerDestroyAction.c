//Made by Salami._.
class SAL_DroneJammerDestroyAction : SCR_InventoryAction
{
	IEntity m_eBag;
	EquipedLoadoutStorageComponent m_cLoadoutStorage
	
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
	{
		if(ChimeraCharacter.Cast(pOwnerEntity))
			SAL_DroneClientComponent.GetInstance().DestroyDroneJammer(m_cLoadoutStorage.GetClothFromArea(LoadoutBackpackArea));
		else
			SAL_DroneClientComponent.GetInstance().DestroyDroneJammer(pOwnerEntity);
	}
	
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		m_eBag = pOwnerEntity;
	}
	
	override bool CanBeShownScript(IEntity user)
	{
		
		if(ChimeraCharacter.Cast(m_eBag))
		{
			m_cLoadoutStorage = SCR_CharacterInventoryStorageComponent.Cast(m_eBag.FindComponent(SCR_CharacterInventoryStorageComponent));
			if(!m_cLoadoutStorage.GetClothFromArea(LoadoutBackpackArea))
				return false;
			if(m_cLoadoutStorage.GetClothFromArea(LoadoutBackpackArea).FindComponent(SAL_DroneJammerComponent))
				return true;
			return false;
		}
		else
			return true;
	}
}