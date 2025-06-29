modded class SCR_EditorManagerEntity
{
	override void StartEvents(EEditorEventOperation type = EEditorEventOperation.NONE)
	{
		// Call base implementation
		super.StartEvents(type);
		
		SAL_DroneConnectionManager droneManager = SAL_DroneConnectionManager.GetInstance();
		
		// Skip custom behavior if no CRF_Gamemode is active
		if (!SAL_DroneConnectionManager)
			return;
		
		// Handle editor opening - close all CRF menus
		if (type == EEditorEventOperation.OPEN)
		{	
			CameraManager cameraManager = GetGame().GetCameraManager();
			
			if (!cameraManager.CurrentCamera())
				return;
		
			if (cameraManager.CurrentCamera().GetPrefabData().GetPrefabName() == "{D10C3C304FC29655}Prefabs/Editor/Camera/DroneCamera.et")
				SCR_PlayerController.Cast(GetGame().GetPlayerController()).ExitDrone();
		}
		// Handle editor closing - reopen appropriate UI
		else if (type == EEditorEventOperation.CLOSE)
		{	
			if (!SCR_PlayerController.GetLocalControlledEntity())
				return;
			
			if (!droneManager.IsPlayerDroneOwner(SCR_PlayerController.GetLocalPlayerId()))
				return;
			
			if (SCR_CharacterInventoryStorageComponent.Cast(SCR_PlayerController.GetLocalControlledEntity().FindComponent(SCR_CharacterInventoryStorageComponent)).Get(0) == null)
				return;
			
			if (SCR_CharacterInventoryStorageComponent.Cast(SCR_PlayerController.GetLocalControlledEntity().FindComponent(SCR_CharacterInventoryStorageComponent)).Get(0).GetPrefabData().GetPrefabName() == "{8951045BFE8BC8E4}Prefabs/Characters/HeadGear/FPV_Goggles.et")
				GetGame().GetCallqueue().CallLater(OpenDrone, 1000, false);
		}
	}
	
	void OpenDrone()
	{
		SCR_PlayerController.Cast(GetGame().GetPlayerController()).EnterDrone();
	}
}