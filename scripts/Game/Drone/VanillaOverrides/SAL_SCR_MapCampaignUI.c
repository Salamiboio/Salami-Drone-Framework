modded class SCR_MapCampaignUI
{
	override void Update(float timeSlice)
	{
		super.Update(timeSlice);
		
		if (!GetGame().GetCameraManager().CurrentCamera())
			return;
		
		if (GetGame().GetCameraManager().CurrentCamera().GetPrefabData().GetPrefabName() != "{D10C3C304FC29655}Prefabs/Editor/Camera/DroneCamera.et")
			return;
		
		vector min;
		vector max;
		m_MapEntity.GetMapVisibleFrame(min, max);
		foreach (Widget w, SCR_MapUIElement icon : m_mIcons)
		{
			vector markerPos = icon.GetPos();
			
			if ((markerPos[0] < min[0]) || (markerPos[0] > max[0]) || (markerPos[2] < min[2]) || (markerPos[2] > max[2]))
			{
				w.SetVisible(false);
			}	
			else
				w.SetVisible(true);
		}
	}
}