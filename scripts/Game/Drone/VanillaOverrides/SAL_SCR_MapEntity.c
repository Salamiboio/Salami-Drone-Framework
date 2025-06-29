modded class SCR_MapEntity
{
	int GetDelay()
	{
		return m_iDelayCounter;
	}
	
	void EnableLegendPublic(bool input)
	{
		EnableLegend(input);
	}
	
	override protected void OnMapOpen(MapConfiguration config)
	{						
		// init zoom & layers
		m_MapWidget.SetSizeInUnits(Vector(m_iMapSizeX, m_iMapSizeY, 0));	// unit size to meters
		UpdateZoomBounds();
		AssignViewLayer(true);
		
		if (m_bDoUpdate)	// when resolution changes, zoom to the same PPU to update zoom and pos
		{
			ZoomSmooth(m_fZoomPPU, reinitZoom: true);
			m_bDoUpdate = false;
		}
		
		// activate modules & components
		ActivateModules(config.Modules);
		ActivateComponents(config.Components);
		ActivateOtherComponents(config.OtherComponents);
		
		m_bDoReload = false;
		
		if (s_OnMapOpen)
			s_OnMapOpen.Invoke(config);

		if (GetGame().GetCameraManager().CurrentCamera())
		{
			if (GetGame().GetCameraManager().CurrentCamera().GetPrefabData().GetPrefabName() == "{D10C3C304FC29655}Prefabs/Editor/Camera/DroneCamera.et")
			{
				EnableVisualisation(true);
				return;
			}
			else if (config.MapEntityMode == EMapEntityMode.FULLSCREEN)
				SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_HUD_MAP_OPEN);
		}
		else if (config.MapEntityMode == EMapEntityMode.FULLSCREEN)
			SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_HUD_MAP_OPEN);
		
		EnableVisualisation(true);
	}
	
	override protected void OnMapClose()
	{
		if (s_OnMapClose)
			s_OnMapClose.Invoke(m_ActiveMapCfg);
		
		if (m_ActiveMapCfg.MapEntityMode == EMapEntityMode.FULLSCREEN)
		{
			if (GetGame().GetCameraManager().CurrentCamera())
			{
				if (GetGame().GetCameraManager().CurrentCamera().GetPrefabData().GetPrefabName() != "{D10C3C304FC29655}Prefabs/Editor/Camera/DroneCamera.et")
					SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_HUD_MAP_CLOSE);
			}
			else
				SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_HUD_MAP_CLOSE);
			
			PlayerController controller = GetGame().GetPlayerController();
			if (controller)
			{
				ChimeraCharacter char = ChimeraCharacter.Cast(controller.GetControlledEntity());
				if (char)
					SCR_CharacterControllerComponent.Cast(char.GetCharacterController()).m_OnLifeStateChanged.Insert(OnLifeStateChanged);
			}
			
			SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
			if (gameMode)
				gameMode.GetOnPlayerDeleted().Remove(OnPlayerDeleted);
		}
		
		if ( m_ActiveMapCfg.OtherComponents & EMapOtherComponents.LEGEND_SCALE)
			EnableLegend(false);
		
		EnableVisualisation(false);
		
		Cleanup();
	}
}