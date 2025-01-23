//Made by Salami._.
class SAL_DroneJammerTurnOnAction : SCR_InventoryAction
{
	IEntity m_eBag;
	EquipedLoadoutStorageComponent m_cLoadoutStorage
	
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
	{
		if(ChimeraCharacter.Cast(pOwnerEntity))
		{
			SAL_DroneClientComponent.GetInstance().UpdatePowers(m_cLoadoutStorage.GetClothFromArea(LoadoutBackpackArea), true);
			
		}
		else
			SAL_DroneClientComponent.GetInstance().UpdatePowers(pOwnerEntity, true);
		
		
		
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
			if(m_cLoadoutStorage.GetClothFromArea(LoadoutBackpackArea).FindComponent(SAL_DroneJammerComponent) && !SAL_DroneGamemodeComponent.GetInstance().GetJammerPowers().Get(SAL_DroneGamemodeComponent.GetInstance().GetJammerBags().Find(RplComponent.Cast(m_cLoadoutStorage.GetClothFromArea(LoadoutBackpackArea).FindComponent(RplComponent)).Id())))
				return true;
			return false;
		}
		else
		{
			if(!SAL_DroneGamemodeComponent.GetInstance().GetJammerPowers().Get(SAL_DroneGamemodeComponent.GetInstance().GetJammerBags().Find(RplComponent.Cast(m_eBag.FindComponent(RplComponent)).Id())))
				return true;
			return false;
		}
	}
}