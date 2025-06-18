class SAL_CameraZoomComponentClass: ScriptComponentClass
{
}

class SAL_CameraZoomComponent: ScriptComponent
{
	[Attribute("5")] int m_iMaxZoom;
	[Attribute("70")] int m_iMinZoom;
	[Attribute("1.0")] float m_fZoomSensitivity;
	
	InputManager m_InputManager;
	SAL_DroneConnectionManager m_DroneConnectionManager;
	CameraManager m_CameraManager;
	
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.FIXEDFRAME | EntityEvent.INIT);
	}
	
	override void EOnInit(IEntity owner)
	{
		m_InputManager = GetGame().GetInputManager();
		m_DroneConnectionManager = SAL_DroneConnectionManager.GetInstance();
		m_CameraManager = GetGame().GetCameraManager();
	}
	
	override void EOnFixedFrame(IEntity owner, float timeSlice)
	{
		if (!m_DroneConnectionManager.IsDronePlayers(owner))
			return;
		
		if (!m_CameraManager.CurrentCamera())
			return;
		
		if (m_CameraManager.CurrentCamera().GetPrefabData().GetPrefabName() != "{D10C3C304FC29655}Prefabs/Editor/Camera/DroneCamera.et")
			return;
		
		float zoomCommand = -m_InputManager.GetActionValue("DroneZoom");
		if (zoomCommand == 0)
			return;
		
		float currentFOV = m_CameraManager.CurrentCamera().GetVerticalFOV();
		float newFov = currentFOV + (m_fZoomSensitivity * zoomCommand);
		
		newFov = Math.Clamp(newFov, m_iMaxZoom, m_iMinZoom);

		m_CameraManager.CurrentCamera().SetVerticalFOV(newFov);
	}
}