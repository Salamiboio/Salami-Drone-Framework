//Made by Salami._.
class SAL_ConnectToCommandStation : SCR_InventoryAction
{
	IEntity m_ePlayer;
	IEntity m_eDrone;
	SAL_DroneConnectionManager m_DroneManager;
	string m_outName;
	
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
	{
		m_ePlayer = pUserEntity;
		m_eDrone = pOwnerEntity;
		
		if (m_DroneManager.IsPlayerCommandStationOwner(SCR_PlayerController.GetLocalPlayerId()))
			SCR_PlayerController.Cast(GetGame().GetPlayerController()).DisconnectPlayerToCommand(pOwnerEntity);
		else
			SCR_PlayerController.Cast(GetGame().GetPlayerController()).ConnectPlayerToCommand(pOwnerEntity);
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

		if (m_DroneManager.IsPlayerCommandStationOwner(SCR_PlayerController.GetLocalPlayerId()))
			m_outName = "#Disconnect_Command_Station";
		else
			m_outName = "#Connect_To_Command_Station";
		return true;
	}
	
	override bool CanBeShownScript(IEntity user)
	{
		if (!SCR_PlayerController.GetLocalControlledEntity())
			return false;

		if (m_DroneManager.IsCommandStationConnected(SCR_PlayerController.GetRplId(m_eDrone)) && m_DroneManager.GetPlayersCommandStation(SCR_PlayerController.GetLocalPlayerId()) != m_eDrone)
			return false;
		
		return true;
	}
	
	override bool GetActionNameScript(out string outName)
	{
		outName = m_outName;
		return true;
	}
	
}