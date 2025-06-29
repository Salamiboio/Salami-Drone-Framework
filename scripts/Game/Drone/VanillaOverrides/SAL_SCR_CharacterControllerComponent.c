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
		
		if (newLifeState == ECharacterLifeState.INCAPACITATED)
		{
		
			CameraBase camera = GetGame().GetCameraManager().CurrentCamera();
			
			if (!camera)
				return;
			
			if (camera.GetPrefabData().GetPrefabName() == "{D10C3C304FC29655}Prefabs/Editor/Camera/DroneCamera.et")
				SCR_PlayerController.Cast(GetGame().GetPlayerController()).ExitDrone();
		}
		else if (newLifeState == ECharacterLifeState.DEAD)
		{
			CameraBase camera = GetGame().GetCameraManager().CurrentCamera();
		
			if (!camera)
				return;
			
			if (SAL_DroneConnectionManager.GetInstance().IsPlayerDroneOwner(SCR_PlayerController.GetLocalPlayerId()))
				SCR_PlayerController.Cast(GetGame().GetPlayerController()).DisconnectDrone();
		}
	}
}