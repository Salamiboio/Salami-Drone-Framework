class SAL_DroneHUDComponentClass: ScriptComponentClass
{
}

enum SAL_HUDType
{
	FPV,
	DJI
}

class SAL_DroneHUDComponent: ScriptComponent
{
	[Attribute("", UIWidgets.ResourceNamePicker, "Layout", "layout")] ResourceName m_DroneHUDLayout;
	[Attribute("0", UIWidgets.ComboBox, "Hud Type", "", ParamEnumArray.FromEnum(SAL_HUDType))] SAL_HUDType m_eHudType;
	
	
	SAL_DroneConnectionManager m_DroneConnectionManager;
	CameraManager m_CameraManager;
	SAL_CameraZoomComponent m_CameraZoom;
	SAL_DroneSignalComponent m_SignalComponent;
	SAL_DroneBatteryComponent m_BatteryComponent;
	SAL_DroneExplosionComponent m_ExplosiveComponent;
	SAL_DroneRangeFinderComponent m_RangeFinderComponent;
	SAL_DroneControllerComponent m_DroneController;
	
	int m_iHeight = 0;
	
	Widget m_wOverlayWidget;
	SCR_MapEntity m_MapEntity;
	
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.FIXEDFRAME | EntityEvent.INIT);
	}
	
	override void EOnInit(IEntity owner)
	{
		m_CameraManager = GetGame().GetCameraManager();
		m_DroneConnectionManager = SAL_DroneConnectionManager.GetInstance();
		m_CameraZoom = SAL_CameraZoomComponent.Cast(owner.FindComponent(SAL_CameraZoomComponent));
		m_SignalComponent = SAL_DroneSignalComponent.Cast(owner.FindComponent(SAL_DroneSignalComponent));
		m_BatteryComponent = SAL_DroneBatteryComponent.Cast(owner.FindComponent(SAL_DroneBatteryComponent));
		m_ExplosiveComponent = SAL_DroneExplosionComponent.Cast(owner.FindComponent(SAL_DroneExplosionComponent));
		m_RangeFinderComponent = SAL_DroneRangeFinderComponent.Cast(owner.FindComponent(SAL_DroneRangeFinderComponent));
		m_DroneController = SAL_DroneControllerComponent.Cast(owner.FindComponent(SAL_DroneControllerComponent));
	};
	
	float m_fTimer = 0;
	float m_fBatteryTimer = 0;
	override void EOnFixedFrame(IEntity owner, float timeSlice)
	{
		if (!m_CameraManager.CurrentCamera())
			return;
		
		if (m_CameraManager.CurrentCamera().GetPrefabData().GetPrefabName() != "{D10C3C304FC29655}Prefabs/Editor/Camera/DroneCamera.et" && m_wOverlayWidget)
		{
			if (SCR_PlayerController.GetLocalControlledEntity())
			{
				ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity()).GetWorld().SetCameraPostProcessEffect(ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity()).GetWorld().GetCurrentCameraId(), 9, PostProcessEffectType.None, "");
				ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity()).GetWorld().SetCameraPostProcessEffect(ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity()).GetWorld().GetCurrentCameraId(), 10, PostProcessEffectType.None, "");
			}
			if (m_MapEntity)
			{
				m_MapEntity.CloseMap();
			}
			delete m_wOverlayWidget;		
			GetGame().GetHUDManager().SetVisible(true);				
		}
		
		if (!m_DroneConnectionManager.IsDronePlayers(owner))
			return;
		
		if (m_CameraManager.CurrentCamera().GetPrefabData().GetPrefabName() != "{D10C3C304FC29655}Prefabs/Editor/Camera/DroneCamera.et")
			return;
		
		if (!m_wOverlayWidget)
		{
			m_wOverlayWidget = GetGame().GetWorkspace().CreateWidgets(m_DroneHUDLayout);			
			switch (m_eHudType)
			{
				case SAL_HUDType.FPV:
				{
					ImageWidget roll = ImageWidget.Cast(m_wOverlayWidget.FindWidget("Dots"));
					roll.LoadImageTexture(1, "{5021E90D8407E44F}UI/FPVHud/Data/FPV_DOTS_THAT_MOVE.edds");
					roll.SetImage(1);
					ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity()).GetWorld().SetCameraPostProcessEffect(ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity()).GetWorld().GetCurrentCameraId(), 9, PostProcessEffectType.ChromAber, "{C245539454FC3F58}UI/Materials/ScreenEffects_ChromAberrPPDrone.emat");
					ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity()).GetWorld().SetCameraPostProcessEffect(ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity()).GetWorld().GetCurrentCameraId(), 10, PostProcessEffectType.FilmGrain, "{F5DD5D01C23AD429}UI/Materials/Editor/FilmGrainDrone.emat");
					break;
				}
				
				case SAL_HUDType.DJI:
				{
					ImageWidget zoomLevel = ImageWidget.Cast(m_wOverlayWidget.FindAnyWidget("ZoomSlider"));
					zoomLevel.LoadImageTexture(1, "{EC0977F3BECC2866}UI/DJIHud/Data/zoomlevel1.edds");
					zoomLevel.SetImage(1);
					
					if (!m_MapEntity)
						m_MapEntity = SCR_MapEntity.GetMapInstance();
					
					if (m_MapEntity)
						GetGame().GetCallqueue().Call(OpenMap);
					break;
				}
			}	
			
			ImageWidget staticWidget = ImageWidget.Cast(m_wOverlayWidget.FindWidget("Static"));
			staticWidget.LoadImageTexture(1, "{980EF09C711CD338}UI/Textures/TVSTATIC.edds");
			staticWidget.LoadImageTexture(2, "{3A1849DED1F55B33}UI/Textures/TVSTATIC1.edds");
			staticWidget.LoadImageTexture(3, "{64F0609D6E78D53C}UI/Textures/TVSTATIC2.edds");
			staticWidget.LoadImageTexture(4, "{90F8270563A5BD48}UI/Textures/TVSTATIC3.edds");
			
			GetGame().GetHUDManager().SetVisible(false);
		}
		
		if (m_fTimer < 0.01)
		{
			m_fTimer += timeSlice;
			return;
		}
		else
			m_fTimer = 0;

		
		if (m_wOverlayWidget)
		{
			switch (m_eHudType)
			{
				case SAL_HUDType.FPV: 
				{
					vector transform[4];
					GetGame().GetWorld().GetCurrentCamera(transform);
					float roll = -Math3D.MatrixToAngles(transform)[2];
					ImageWidget.Cast(m_wOverlayWidget.FindWidget("Dots")).SetRotation(roll);
					
					TextWidget signalText = TextWidget.Cast(m_wOverlayWidget.FindWidget("SignalText"));
					int rssiInt = Math.Lerp(-30, -120, m_SignalComponent.m_fRSSI);
					string rssi = rssiInt.ToString();
					signalText.SetText(rssi);
					
					TextWidget lqText = TextWidget.Cast(m_wOverlayWidget.FindWidget("LQText"));
					int lqInt = Math.Lerp(100, 0, m_SignalComponent.m_fLQ);
					string lq = lqInt.ToString();
					lqText.SetTextFormat("LQ: %1%", lq);
					
					float batteryPercentage = m_BatteryComponent.m_fCurrentBattery/m_BatteryComponent.m_fCurrentBatteryMax;
					if (batteryPercentage <= 1 && batteryPercentage >= 0.75)
					{
						m_wOverlayWidget.FindWidget("BatteryLevel4").SetVisible(true);
						m_wOverlayWidget.FindWidget("BatteryLevel3").SetVisible(true);
						m_wOverlayWidget.FindWidget("BatteryLevel2").SetVisible(true);
						m_wOverlayWidget.FindWidget("BatteryLevel1").SetVisible(true);
					}
					else if (batteryPercentage <= 0.75 && batteryPercentage >= 0.5)
					{
						m_wOverlayWidget.FindWidget("BatteryLevel4").SetVisible(false);
						m_wOverlayWidget.FindWidget("BatteryLevel3").SetVisible(true);
						m_wOverlayWidget.FindWidget("BatteryLevel2").SetVisible(true);
						m_wOverlayWidget.FindWidget("BatteryLevel1").SetVisible(true);
					}
					else if (batteryPercentage <= 0.50 && batteryPercentage >= 0.25)
					{
						m_wOverlayWidget.FindWidget("BatteryLevel4").SetVisible(false);
						m_wOverlayWidget.FindWidget("BatteryLevel3").SetVisible(false);
						m_wOverlayWidget.FindWidget("BatteryLevel2").SetVisible(true);
						m_wOverlayWidget.FindWidget("BatteryLevel1").SetVisible(true);
					}
					else if (batteryPercentage <= 0.25 && batteryPercentage >= 0.10)
					{
						m_wOverlayWidget.FindWidget("BatteryLevel4").SetVisible(false);
						m_wOverlayWidget.FindWidget("BatteryLevel3").SetVisible(false);
						m_wOverlayWidget.FindWidget("BatteryLevel2").SetVisible(false);
						m_wOverlayWidget.FindWidget("BatteryLevel1").SetVisible(true);
					}
					else
					{
						m_fBatteryTimer += timeSlice;
						m_wOverlayWidget.FindWidget("BatteryLevel4").SetVisible(false);
						m_wOverlayWidget.FindWidget("BatteryLevel3").SetVisible(false);
						m_wOverlayWidget.FindWidget("BatteryLevel2").SetVisible(false);
						
						if (m_fBatteryTimer <= 0.5)
							m_wOverlayWidget.FindWidget("BatteryLevel1").SetVisible(true);
						else
						{
							m_wOverlayWidget.FindWidget("BatteryLevel1").SetVisible(false);
						}
						
						if (m_fBatteryTimer >= 1)
							m_fBatteryTimer = 0;
					}
					
					if (m_ExplosiveComponent)
					{
						Widget armed = m_wOverlayWidget.FindWidget("Armed");
						if (m_ExplosiveComponent.m_bIsArmed && !armed.IsVisible())
							armed.SetVisible(true);
						else if (!m_ExplosiveComponent.m_bIsArmed && armed.IsVisible())
							armed.SetVisible(false);
					}
					
					break;
				}
				
				case SAL_HUDType.DJI:
				{
					//Inspired from CSI thanks patman :)
					//---------------------------------------------------------------------------------------------------------------
					//---------------------------------------------------------------------------------------------------------------
					ImageWidget compass = ImageWidget.Cast(m_wOverlayWidget.FindWidget("Compass"));
					vector transform[4];
					GetGame().GetWorld().GetCurrentCamera(transform);
					float yaw = -Math3D.MatrixToAngles(transform)[0];
					compass.SetRotation(yaw);
					//---------------------------------------------------------------------------------------------------------------
					//---------------------------------------------------------------------------------------------------------------
					
					//Bearing BS
					//---------------------------------------------------------------------------------------------------------------
					//---------------------------------------------------------------------------------------------------------------
					int yawInt = -yaw;
			        
					if (yawInt < 0) 
					yawInt = 360 - Math.AbsFloat(yawInt);
					
					string bearingAdd = "";
							        
					if (yawInt >= 0 & yawInt < 10) 
					bearingAdd = "00";
					        
					if (yawInt >= 10 & yawInt < 100) 
						bearingAdd = "0";
					        
					string bearingStr = bearingAdd + (yawInt.ToString());
					TextWidget.Cast(m_wOverlayWidget.FindWidget("Bearing")).SetText(bearingStr);
					//---------------------------------------------------------------------------------------------------------------
					//---------------------------------------------------------------------------------------------------------------
					
					//Sets the height
					//---------------------------------------------------------------------------------------------------------------
					//---------------------------------------------------------------------------------------------------------------
					vector origin = owner.GetOrigin();
					vector end = origin + Vector(0, -1, 0) * 1000.0; // cast 1000 meters down
			
					TraceParam trace = new TraceParam();
					trace.Start = origin;
					trace.End = end;
					trace.Exclude = owner; // prevent hitting self
					trace.Flags = TraceFlags.WORLD;
			
					int height = Math.Round(GetGame().GetWorld().TraceMove(trace, null) * 10000);
					m_iHeight = height;
				
					string heightString = "";
							
					if((height/10) < 10)
						heightString = "00" + (height/10).ToString() + "," + height.ToString().Get(height.ToString().Length() - 1);
							
					if((height/100) < 100)
						heightString = "0" + (height/10).ToString() + "," + height.ToString().Get(height.ToString().Length() - 1);
							
					if((height/100) < 1000)
						heightString = (height/10).ToString() + "," + height.ToString().Get(height.ToString().Length() - 1);
							
					TextWidget.Cast(m_wOverlayWidget.FindAnyWidget("Altitude")).SetText(heightString);
					//---------------------------------------------------------------------------------------------------------------
					//---------------------------------------------------------------------------------------------------------------
					
					//Sets the speed
					//---------------------------------------------------------------------------------------------------------------
					//---------------------------------------------------------------------------------------------------------------
					int speed = owner.GetPhysics().GetVelocity().Length()*10;
					string speedString = "";
					
					if((speed/10) < 10)
						speedString = "00" + (speed/10).ToString() + "," + speed.ToString().Get(speed.ToString().Length() - 1);
							
					if((speed/100) < 100)
						speedString = "0" + (speed/10).ToString() + "," + speed.ToString().Get(speed.ToString().Length() - 1);
							
					if((speed/100) < 1000)
						speedString = (speed/10).ToString() + "," + speed.ToString().Get(speed.ToString().Length() - 1);
							
					TextWidget.Cast(m_wOverlayWidget.FindAnyWidget("Speed")).SetText(speedString);
					//---------------------------------------------------------------------------------------------------------------
					//---------------------------------------------------------------------------------------------------------------
					
					float fovDifference = Math.Clamp((m_CameraManager.CurrentCamera().GetVerticalFOV() - m_CameraZoom.m_iMaxZoom) / (m_CameraZoom.m_iMinZoom - m_CameraZoom.m_iMaxZoom), 0.0, 1.0);
					float zoomPos = Math.Lerp(-930, -540, fovDifference);
					FrameSlot.SetPosY(m_wOverlayWidget.FindAnyWidget("ZoomSlider"), zoomPos);
					
					//Signal BS
					//---------------------------------------------------------------------------------------------------------------
					//---------------------------------------------------------------------------------------------------------------
					TextWidget signalText = TextWidget.Cast(m_wOverlayWidget.FindWidget("SignalText"));
					int rssiInt = Math.Lerp(-30, -120, m_SignalComponent.m_fRSSI);
					string rssi = rssiInt.ToString();
					signalText.SetText(rssi);
					
					TextWidget lqText = TextWidget.Cast(m_wOverlayWidget.FindWidget("LQText"));
					int lqInt = Math.Lerp(100, 0, m_SignalComponent.m_fLQ);
					string lq = lqInt.ToString();
					lqText.SetTextFormat("LQ: %1%", lq);
					//---------------------------------------------------------------------------------------------------------------
					//---------------------------------------------------------------------------------------------------------------
					
					float batteryPercentage = m_BatteryComponent.m_fCurrentBattery/m_BatteryComponent.m_fCurrentBatteryMax;
					int currentPercent = Math.Round(100 * batteryPercentage);
					TextWidget.Cast(m_wOverlayWidget.FindWidget("BatteryText")).SetText(currentPercent.ToString());
					ProgressBarWidget.Cast(m_wOverlayWidget.FindWidget("BatteryBar")).SetCurrent(currentPercent);
					
					if (m_RangeFinderComponent)
					{
						if (m_RangeFinderComponent.m_bHasFired)
						{
							TextWidget gridWidget = TextWidget.Cast(m_wOverlayWidget.FindAnyWidget("Grid"));
							string finalGrid = "GRID: " + m_RangeFinderComponent.m_iGridX.ToString() + " - " +  m_RangeFinderComponent.m_iGridY.ToString();
							gridWidget.SetText(finalGrid);
							TextWidget distanceWidget = TextWidget.Cast(m_wOverlayWidget.FindAnyWidget("Distance"));
							string finalDistance = "DISTANCE: " + Math.Round(m_RangeFinderComponent.m_fHitDistance).ToString() + "m";
							distanceWidget.SetText(finalDistance);	
						}
					}
					ImageWidget.Cast(m_wOverlayWidget.FindWidget("DroneCursor")).SetRotation(yawInt);
					if (GetGame().GetMenuManager().GetTopMenu())
					{
						if (GetGame().GetMenuManager().GetTopMenu().IsInherited(SCR_InventoryMenuUI) && m_MapEntity)
							m_MapEntity.CloseMap();
					}
					else if (!m_MapEntity.IsOpen())
						GetGame().GetCallqueue().Call(OpenMap);
					
					if (m_MapEntity.IsOpen() && m_MapEntity.GetDelay() <= 0)
					{
						OpenMapWrapZoomChange();
						m_MapEntity.EnableGrid(false);
						m_MapEntity.EnableLegendPublic(false);
					}
						
					float rollPercentage = m_DroneController.m_fCurrentRollDeg/m_DroneController.m_DroneStablizerProfile.m_fMaxTiltDegrees;
					float rollRight = -540 - 50 * rollPercentage;
					float rollLeft = -540 + 50 * rollPercentage;
					FrameSlot.SetPosY(m_wOverlayWidget.FindWidget("LineRight"), rollRight);
					FrameSlot.SetPosY(m_wOverlayWidget.FindWidget("LineLeft"), rollLeft);
				}
			}
			
			ImageWidget staticWidget = ImageWidget.Cast(m_wOverlayWidget.FindWidget("Static"));
			
			int randomInt = Math.RandomInt(1, 5);
			switch(randomInt)
			{
				case 1: {staticWidget.SetImage(1); break;}
				case 2: {staticWidget.SetImage(2); break;}
				case 3: {staticWidget.SetImage(3); break;}
				case 4: {staticWidget.SetImage(4); break;}
			}
			staticWidget.SetColor(Color.FromRGBA(255, 255, 255, m_SignalComponent.m_fSignalStrength));
		}
	}
	
	void OpenMap()
	{
		GetGame().GetCallqueue().Call(OpenMapWrap); // Need two frames
	}
	
	/**
	 * Second step in opening map
	 */
	void OpenMapWrap()
	{
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return;
		
		SCR_MapConfigComponent configComp = SCR_MapConfigComponent.Cast(gameMode.FindComponent(SCR_MapConfigComponent));
		if (!configComp)
			return;
		
		MapConfiguration mapConfigFullscreen = m_MapEntity.SetupMapConfig(
			EMapEntityMode.FULLSCREEN, 
			"{1B8AC767E06A0ACD}Configs/Map/MapFullscreen.conf", 
			m_wOverlayWidget
		);
		
		m_MapEntity.OpenMap(mapConfigFullscreen);
		GetGame().GetCallqueue().Call(OpenMapWrapZoomChange);
	}
	
	void OpenMapWrapZoomChange()
	{
		GetGame().GetCallqueue().Call(OpenMapWrapZoomChangeWrap);
	}
	
	/**
	 * Final step in opening map (applies zoom) and pans to AO
	 */
	void OpenMapWrapZoomChangeWrap()
	{	
		vector center = GetOwner().GetOrigin();
		if (center)
		{
			if (m_MapEntity)
			{
				if (m_iHeight <= 0)
					m_MapEntity.ZoomPanSmooth(0, center[0], center[2], 0.001);
				else
					m_MapEntity.ZoomPanSmooth(200/m_iHeight, center[0], center[2], 0.001);
			}
				
		}
	}
	
	void ~SAL_DroneHUDComponent()
	{
		if (!GetGame().GetWorld())
			return;
		
		if (m_wOverlayWidget)
		{
			if (SCR_PlayerController.GetLocalControlledEntity())
			{
				ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity()).GetWorld().SetCameraPostProcessEffect(ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity()).GetWorld().GetCurrentCameraId(), 9, PostProcessEffectType.None, "");
				ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity()).GetWorld().SetCameraPostProcessEffect(ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity()).GetWorld().GetCurrentCameraId(), 10, PostProcessEffectType.None, "");
			}
			if (m_MapEntity)
			{
				m_MapEntity.CloseMap();
			}
			delete m_wOverlayWidget;
			GetGame().GetHUDManager().SetVisible(true);		
		}
			
	}
}