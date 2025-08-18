modded class SCR_CharacterControllerComponent
{
	override void OnDeath(IEntity instigatorEntity, notnull Instigator instigator)
	{
		super.OnDeath(instigatorEntity, instigator);
		
		if (GetOwner() != SCR_PlayerController.GetLocalControlledEntity())
			return;
		
//		CameraBase camera = GetGame().GetCameraManager().CurrentCamera();
//		
//		if (!camera)
//			return;
//		
//		if (SAL_DroneConnectionManager.GetInstance().IsPlayerDroneOwner(SCR_PlayerController.GetLocalPlayerId()))
//			SCR_PlayerController.Cast(GetGame().GetPlayerController()).DisconnectDrone();
	}
	
	override void OnLifeStateChanged(ECharacterLifeState previousLifeState, ECharacterLifeState newLifeState)
	{
		super.OnLifeStateChanged(previousLifeState, newLifeState);
		if (GetOwner() != SCR_PlayerController.GetLocalControlledEntity())
			return;
		
		SAL_DroneConnectionManager droneManager = SAL_DroneConnectionManager.GetInstance();
		
		if (newLifeState == ECharacterLifeState.INCAPACITATED)
		{
			SCR_PlayerController.Cast(GetGame().GetPlayerController()).ExitDrone();
		}
		else if (newLifeState == ECharacterLifeState.DEAD)
		{
			SCR_PlayerController.Cast(GetGame().GetPlayerController()).DisconnectDrone();
			if (droneManager.IsPlayerAntennaOwner(SCR_PlayerController.GetLocalPlayerId()))
				SCR_PlayerController.Cast(GetGame().GetPlayerController()).DisconnectPlayerToAntenna(droneManager.GetPlayersAntenna(SCR_PlayerController.GetLocalPlayerId()));
			if (droneManager.IsPlayerCommandStationOwner(SCR_PlayerController.GetLocalPlayerId()))
				SCR_PlayerController.Cast(GetGame().GetPlayerController()).DisconnectPlayerToCommand(droneManager.GetPlayersCommandStation(SCR_PlayerController.GetLocalPlayerId()));
		}
	}
}