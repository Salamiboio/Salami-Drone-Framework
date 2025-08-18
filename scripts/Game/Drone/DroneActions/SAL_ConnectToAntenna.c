//Made by Salami._.
class SAL_ConnectToAntenna : SCR_InventoryAction
{
	IEntity m_ePlayer;
	IEntity m_eDrone;
	SAL_DroneConnectionManager m_DroneManager;
	string m_outName;
	
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
	{
		m_ePlayer = pUserEntity;
		m_eDrone = pOwnerEntity;
		
		if (m_DroneManager.IsPlayerAntennaOwner(SCR_PlayerController.GetLocalPlayerId()))
			SCR_PlayerController.Cast(GetGame().GetPlayerController()).DisconnectPlayerToAntenna(pOwnerEntity);
		else
			SCR_PlayerController.Cast(GetGame().GetPlayerController()).ConnectPlayerToAntenna(pOwnerEntity);
	}
	
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		m_eDrone = pOwnerEntity;
		m_DroneManager = SAL_DroneConnectionManager.GetInstance();
	}
	
	override bool CanBePerformedScript(IEntity user)
	{
		if (!SCR_PlayerController.GetLocalControlledEntity())
			return false;
			
		BaseWeaponManagerComponent weaponMan = BaseWeaponManagerComponent.Cast(ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity()).FindComponent(BaseWeaponManagerComponent));
		if (!weaponMan)
			return false;

		if (!weaponMan.GetCurrentWeapon())
		{
			m_outName = "#Controller_Error";
			return false;
		}
		
		if (weaponMan.GetCurrentWeapon().GetOwner().GetPrefabData().GetPrefabName() != "{E2434ED1318D8476}Prefabs/Characters/Items/DroneController.et")
		{
			m_outName = "#Controller_Error";
			return false;
		}

		if (m_DroneManager.IsPlayerAntennaOwner(SCR_PlayerController.GetLocalPlayerId()))
			m_outName = "#Disconnect_Antenna";
		else
			m_outName = "#Connect_To_Antenna";
		return true;
	}
	
	override bool CanBeShownScript(IEntity user)
	{
		if (!SCR_PlayerController.GetLocalControlledEntity())
			return false;

		if (m_DroneManager.IsAntennaConnected(SCR_PlayerController.GetRplId(m_eDrone)) && m_DroneManager.GetPlayersAntenna(SCR_PlayerController.GetLocalPlayerId()) != m_eDrone)
			return false;
		
		return true;
	}
	
	override bool GetActionNameScript(out string outName)
	{
		outName = m_outName;
		return true;
	}
	
}