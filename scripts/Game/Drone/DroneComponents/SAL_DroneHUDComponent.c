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
	RplComponent m_RplComp;
	UserSettings m_VideoSettings;
	RplId m_Id;
	
	int m_iOldFPS;
	int m_iHeight = 0;
	bool m_bNghtSightOn = false;
	
	Widget m_wOverlayWidget;
	SCR_MapEntity m_MapEntity;
	
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.FRAME | EntityEvent.INIT);
	}
	
	void ToggleNightSight()
	{
		m_bNghtSightOn = m_DroneController.m_bNightSightActive;
		if (m_bNghtSightOn)
			ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity()).GetWorld().SetCameraPostProcessEffect(ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity()).GetWorld().GetCurrentCameraId(), 8, PostProcessEffectType.Colors, "{41B8CF95B51D4F8A}Prefabs/Editor/Camera/CameraPPEffectNight.emat");
		else
			ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity()).GetWorld().SetCameraPostProcessEffect(ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity()).GetWorld().GetCurrentCameraId(), 8, PostProcessEffectType.Colors, "{BED69BE8F08A9063}Prefabs/Editor/Camera/CameraPPEffect.emat");
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
		m_Id = SCR_PlayerController.GetRplId(owner);
		if (!m_RplComp)
			m_RplComp = RplComponent.Cast(owner.FindComponent(RplComponent));
		ArmaReforgerScripted game = GetGame();
		UserSettings engineUserSettings = game.GetEngineUserSettings();
		m_VideoSettings = engineUserSettings.GetModule("VideoUserSettings");
	};
	
	void CloseHud()
	{
		if (SCR_PlayerController.GetLocalControlledEntity())
		{
			ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity()).GetWorld().SetCameraPostProcessEffect(ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity()).GetWorld().GetCurrentCameraId(), 8, PostProcessEffectType.None, "");
			ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity()).GetWorld().SetCameraPostProcessEffect(ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity()).GetWorld().GetCurrentCameraId(), 9, PostProcessEffectType.None, "");
			ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity()).GetWorld().SetCameraPostProcessEffect(ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity()).GetWorld().GetCurrentCameraId(), 7, PostProcessEffectType.None, "");
		}
		
		if (m_eHudType == SAL_HUDType.FPV)
		{
			m_VideoSettings.Set("MaxFps", m_iOldFPS);
		}
		
		if (m_MapEntity)
		{
			m_MapEntity.CloseMap();
		}
		delete m_wOverlayWidget;		
		GetGame().GetHUDManager().SetVisible(true);		
	}
	
	float m_fTimer = 0;
	float m_fBatteryTimer = 0;
	float m_fBatteryCheck = 0;
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		if (System.IsConsoleApp())
			return;
		
		if (m_Id == -1)
			m_Id = RplComponent.Cast(owner.FindComponent(RplComponent)).Id();
		
		if (!m_CameraManager.CurrentCamera())
			return;
		
		if (m_wOverlayWidget && !m_DroneConnectionManager.m_aActiveDrones.Contains(m_Id))
			CloseHud();
		
		if (m_DroneController.m_iOwner != SCR_PlayerController.GetLocalPlayerId() || !m_DroneConnectionManager.m_aActiveDrones.Contains(m_Id))
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
					if (m_bNghtSightOn)
						ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity()).GetWorld().SetCameraPostProcessEffect(ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity()).GetWorld().GetCurrentCameraId(), 8, PostProcessEffectType.Colors, "{41B8CF95B51D4F8A}Prefabs/Editor/Camera/CameraPPEffectNight.emat");
					else
						ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity()).GetWorld().SetCameraPostProcessEffect(ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity()).GetWorld().GetCurrentCameraId(), 8, PostProcessEffectType.Colors, "{BED69BE8F08A9063}Prefabs/Editor/Camera/CameraPPEffect.emat");
					ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity()).GetWorld().SetCameraPostProcessEffect(ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity()).GetWorld().GetCurrentCameraId(), 9, PostProcessEffectType.ChromAber, "{FDDEEF9952D91012}Prefabs/Editor/Camera/FPVCameraChromAb.emat");
					ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity()).GetWorld().SetCameraPostProcessEffect(ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity()).GetWorld().GetCurrentCameraId(), 7, PostProcessEffectType.FilmGrain, "{54CDAA35A1831F7F}Prefabs/Editor/Camera/FPVCameraSharpen.emat");
					m_VideoSettings.Get("MaxFps", m_iOldFPS);
					m_VideoSettings.Set("MaxFps", 30);
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
						GetGame().GetCallqueue().CallLater(OpenMap);
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

		
		if (m_wOverlayWidget)
		{
			if (m_bNghtSightOn != m_DroneController.m_bNightSightActive)
				ToggleNightSight();
			switch (m_eHudType)
			{
				case SAL_HUDType.FPV: 
				{
					vector transform[4];
					m_DroneController.m_Drone.GetTransform(transform);
					float roll = -Math3D.MatrixToAngles(transform)[2];
					ImageWidget dots = ImageWidget.Cast(m_wOverlayWidget.FindWidget("Dots"));
					dots.SetRotation(roll);
					float pitch = Math3D.MatrixToAngles(transform)[1];
					float offsetY = pitch * 3.0;
					
					FrameSlot.SetPos(dots, -150, offsetY + -150);
					
					TextWidget signalText = TextWidget.Cast(m_wOverlayWidget.FindWidget("SignalText"));
					int rssiInt = Math.Lerp(-30, -120, m_SignalComponent.m_fRSSI);
					string rssi = rssiInt.ToString();
					signalText.SetText(rssi);
					
					TextWidget lqText = TextWidget.Cast(m_wOverlayWidget.FindWidget("LQText"));
					int lqInt = Math.Lerp(100, 0, m_SignalComponent.m_fLQ);
					string lq = lqInt.ToString();
					lqText.SetTextFormat("LQ: %1%", lq);
					
					TextWidget date = TextWidget.Cast(m_wOverlayWidget.FindAnyWidget("Date"));
					ChimeraWorld world = ChimeraWorld.CastFrom(GetGame().GetWorld());
					int year, month, day;
					world.GetTimeAndWeatherManager().GetDate(year, month, day);
					date.SetText(year.ToString() + " - " + month.ToString(2) + " - " + day.ToString(2));
					
					TimeContainer time = world.GetTimeAndWeatherManager().GetTime();
					TextWidget timeWidget = TextWidget.Cast(m_wOverlayWidget.FindAnyWidget("Time"));
					timeWidget.SetText(time.m_iHours.ToString(2) + ":" + time.m_iMinutes.ToString(2) + ":" + time.m_iSeconds.ToString(2));
					
					float batteryPercentage = (m_BatteryComponent.m_fCurrentBattery/m_BatteryComponent.m_fCurrentBatteryMax) * 100;
					float voltPerCell = ComputeVoltageFromPercent(batteryPercentage);
					
					TextWidget allVolts = TextWidget.Cast(m_wOverlayWidget.FindAnyWidget("AllVolts"));
					allVolts.SetText((voltPerCell * 4).ToString(-1, 2) + "v");
					
					TextWidget cellVolts = TextWidget.Cast(m_wOverlayWidget.FindAnyWidget("CellVolts"));
					cellVolts.SetText(voltPerCell.ToString(-1, 2) + "v");
					
					float ampsUsed = ComputeAhUsedFromPercent(m_BatteryComponent.m_fCurrentBatteryMax, batteryPercentage);
					TextWidget usedAmps = TextWidget.Cast(m_wOverlayWidget.FindAnyWidget("UsedAmps"));
					usedAmps.SetText(ampsUsed.ToString(-1, 0));
					
					float ampsCurrent = ComputeInstantAmps(m_DroneController.m_fWeight * 1000, m_DroneController.m_iThrottle);
					TextWidget currentAmps = TextWidget.Cast(m_wOverlayWidget.FindAnyWidget("CurrentAmps"));
					currentAmps.SetText((ampsCurrent * 4).ToString(-1, 2) + " A");
					
					TextWidget headingWidget = TextWidget.Cast(m_wOverlayWidget.FindAnyWidget("Heading"));
					float yaw = -Math3D.MatrixToAngles(transform)[0];
					int yawInt = -yaw;
			        
					if (yawInt < 0) 
						yawInt = 360 - Math.AbsFloat(yawInt);
					headingWidget.SetText(yawInt.ToString(3));
					
					vector origin = owner.GetOrigin();
					vector end = origin + Vector(0, -1, 0) * 1000.0; // cast 1000 meters down
			
					TraceParam trace = new TraceParam();
					trace.Start = origin;
					trace.End = end;
					trace.Exclude = owner; // prevent hitting self
					trace.Flags = TraceFlags.WORLD;
			
					float height = GetGame().GetWorld().TraceMove(trace, null) * 1000;
							
					TextWidget.Cast(m_wOverlayWidget.FindAnyWidget("Altitude")).SetText(height.ToString(-1, 1) + "m");
					float batteryChange = (34 * (batteryPercentage/100));
					Widget battLevel1 = m_wOverlayWidget.FindAnyWidget("BatteryLevel1");
					Widget battLevel2 = m_wOverlayWidget.FindAnyWidget("BatteryLevel2");
					FrameSlot.SetSizeY(battLevel1, batteryChange);
					FrameSlot.SetSizeY(battLevel2, batteryChange);
					FrameSlot.SetPosY(battLevel1, -131 + (34 - batteryChange));
					FrameSlot.SetPosY(battLevel2, -67 + (34 - batteryChange));
					
					if (m_fTimer >= 1)
					{
						m_fTimer = 0;
						if (batteryPercentage < 20)
						{
							Widget batteryFrame = m_wOverlayWidget.FindAnyWidget("BatteryFrame");
							batteryFrame.SetVisible(!batteryFrame.IsVisible());
							Widget lowBattery = m_wOverlayWidget.FindAnyWidget("LowBattery");
							lowBattery.SetVisible(batteryFrame.IsVisible());
						}
						else
							m_wOverlayWidget.FindAnyWidget("LowBattery").SetVisible(false);
					}
					else
						m_fTimer += timeSlice;
					
					
					if (m_ExplosiveComponent)
					{
						Widget armed = m_wOverlayWidget.FindWidget("Armed");
						armed.SetVisible(m_ExplosiveComponent.m_bIsWarheadArmed);
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
					
					if (m_fBatteryCheck > 1.5)
					{
						m_fBatteryCheck = 0;
						if (m_DroneController.m_iThrottle > 0)
						{
							float timeLeft = m_BatteryComponent.m_fCurrentBattery/ (60 * (m_DroneController.m_iThrottle + Math.AbsFloat(m_DroneController.m_iPitch) + Math.AbsFloat(m_DroneController.m_iRoll)));
							float timeLeftLeftover = timeLeft - Math.Floor(timeLeft);
							float secondsLeft = timeLeftLeftover * 60;
							string timeLeftString = Math.Floor(timeLeft).ToString() + "'" + Math.Round(secondsLeft).ToString() + "\"";
							TextWidget.Cast(m_wOverlayWidget.FindWidget("BatteryTime")).SetText(timeLeftString);
						}
						else
							TextWidget.Cast(m_wOverlayWidget.FindWidget("BatteryTime")).SetText("00'00\"");
					}
					else
						m_fBatteryCheck += timeSlice;
					
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
					
					SlotManagerComponent slotMan = SlotManagerComponent.Cast(owner.FindComponent(SlotManagerComponent));
					if (!slotMan)
						break;
					
					EntitySlotInfo grenadeSlot = slotMan.GetSlotByName("GrenadeDropper");
					if (!grenadeSlot)
						break;
					
					if (grenadeSlot.GetAttachedEntity() == null)
						m_wOverlayWidget.FindWidget("Grenade").SetVisible(false);
					else
						m_wOverlayWidget.FindWidget("Grenade").SetVisible(true);
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
			
			BaseWeaponManagerComponent weaponMan = BaseWeaponManagerComponent.Cast(ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity()).FindComponent(BaseWeaponManagerComponent));
			if (!weaponMan)
				return;
			
			if (!weaponMan.GetCurrentWeapon())
			{
				m_wOverlayWidget.FindWidget("ControllerWarning").SetVisible(true);
				return;
			}
			
			if (weaponMan.GetCurrentWeapon().GetOwner().GetPrefabData().GetPrefabName() != "{E2434ED1318D8476}Prefabs/Characters/Items/DroneController.et")
			{
				m_wOverlayWidget.FindWidget("ControllerWarning").SetVisible(true);
				return;
			}
			else
				m_wOverlayWidget.FindWidget("ControllerWarning").SetVisible(false);
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
	
	float ComputeInstantAmps(float weightGrams, float throttle01)
	{
	    throttle01 = Math.Clamp(throttle01, 0, 1);
	    return (weightGrams / 100.0) * throttle01;
	}
	
	float ComputeAhUsedFromPercent(float batterySize, float batteryPercent)
	{
		float batteryCapacityAh = 12600 * (batterySize/1200);
	    float pct = Math.Clamp(batteryPercent, 0, 100);
	    return batteryCapacityAh * (1.0 - (pct / 100.0));
	}
	
	float ComputeVoltageFromPercent(float pct)
	{
	    pct = Math.Clamp(pct, 0, 100);
	
	    float minV = 3.3;
	    float maxV = 4.2;
	
	    float cellV = minV + (maxV - minV) * Math.Pow(pct / 100.0, 1.1);
	    return cellV;
	}

	
	void ~SAL_DroneHUDComponent()
	{
		if (!GetGame().GetWorld())
			return;
		
		if (m_wOverlayWidget)
			CloseHud();
			
	}
}