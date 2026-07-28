//Made by Salami._.
class SAL_ToggleJammer : SCR_InventoryAction
{
	IEntity m_ePlayer;
	IEntity m_ejammer;
	SAL_DroneConnectionManager m_DroneManager;
	string m_outName;
	
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
	{
		m_ePlayer = pUserEntity;
		m_ejammer = pOwnerEntity;
		
		SCR_PlayerController.Cast(GetGame().GetPlayerController()).ToggleJammer(SCR_PlayerController.GetRplId(m_ejammer));
	}
	
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		m_ejammer = pOwnerEntity;
		m_DroneManager = SAL_DroneConnectionManager.GetInstance();
	}
	
	override bool CanBePerformedScript(IEntity user)
	{
		if (!SCR_PlayerController.GetLocalControlledEntity())
			return false;

		if (m_DroneManager.m_aJammers.Contains(RplComponent.Cast(m_ejammer.FindComponent(RplComponent)).Id()))
			m_outName = "#Turn_Off_Jammer";
		else
			m_outName = "#Turn_On_Jammer";
		return true;
	}
	
	override bool CanBeShownScript(IEntity user)
	{
		if (m_ejammer.GetParent())
		{
			BaseInventoryStorageComponent storageComp = BaseInventoryStorageComponent.Cast(user.FindComponent(BaseInventoryStorageComponent));
			if (!storageComp)
				return false;
		
			if (!storageComp.Get(5))
				return false;
		
			if (!storageComp.Get(5).FindComponent(SAL_DroneJammerComponent))
				return false;
		}
		return true;
	}
	
	override bool GetActionNameScript(out string outName)
	{
		outName = m_outName;
		return true;
	}
	
}