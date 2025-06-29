[BaseContainerProps()]
modded class SCR_MapCursorModule
{
	override void Update(float timeSlice)
	{
		if(GetGame().GetCameraManager().CurrentCamera())
			if (GetGame().GetCameraManager().CurrentCamera().GetPrefabData().GetPrefabName() == "{D10C3C304FC29655}Prefabs/Editor/Camera/DroneCamera.et")
			{
				m_CustomCursor.GetCursor().SetVisible(false);
				m_wCrossGrid.SetVisible(false);
				return;
			}
			else
			{
				m_wCrossGrid.SetVisible(true);		
				m_CustomCursor.GetCursor().SetVisible(true);
			}
				
		
		if (m_bIsDisabled)
			return;
		
		m_bRecentlyTraced = false;
		
		// update current pos
		GetCursorPosition(m_CursorInfo.x, m_CursorInfo.y);

		// frame handlers
		HandleMove();
		HandleHover(timeSlice);
		HandlePan(timeSlice);
		HandleZoom();

		// crosshair grid lines
		if (m_bEnableMapCrosshairVisuals && (m_CursorState & CUSTOM_CURSOR_LOCKED) == 0)
			UpdateCrosshairUI();
	}
}