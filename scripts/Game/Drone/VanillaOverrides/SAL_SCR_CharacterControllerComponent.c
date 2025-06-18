modded class SCR_CharacterControllerComponent
{
	override void OnDeath(IEntity instigatorEntity, notnull Instigator instigator)
	{
		super.OnDeath(instigatorEntity, instigator);
		
		CameraBase camera = GetGame().GetCameraManager().CurrentCamera();
		
		if (!camera)
			return;
		
		if (camera.GetPrefabData().GetPrefabName() == "{D10C3C304FC29655}Prefabs/Editor/Camera/DroneCamera.et")
			SCR_PlayerController.Cast(GetGame().GetPlayerController()).DisconnectDrone();
	}
}