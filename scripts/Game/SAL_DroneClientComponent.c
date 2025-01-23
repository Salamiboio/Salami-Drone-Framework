//Made by Salami._.
[EntityEditorProps(category: "GameScripted", description: "Handles communcation to server", color: "")]
class SAL_DroneClientComponentClass: ScriptComponentClass
{
}

class SAL_DroneClientComponent : ScriptComponent
{	
	
	protected IEntity m_eConnectedDrone = null;
	protected RplId m_rConnectedDroneRplId;
	protected SCR_CharacterControllerComponent m_sCharacterController;
	protected bool m_bIsInDrone = false;
	protected SignalsManagerComponent m_SignalsManagerComponent;
	protected int m_iIsUsed;
	protected IEntity m_ePlayer;
	protected Widget m_wOverlayWidget;
	protected Widget m_wStaticWidget;
	protected ImageWidget m_wStaticImageWidget;
	protected ImageWidget m_wOverlayImageWidget;
	protected ResourceName m_rControllerPrefab = "";
	protected ResourceName m_rOverlayWidgetLayout;
	protected float m_fCameraFOV = 70;
	protected int m_iZoomLevel = -302;
	protected IEntity m_eClosestBag;
	protected SAL_DroneJammerComponent m_cDroneJammerComponent;
	protected float m_fMasterVolume;
	protected float m_fvoiceVolume;
	protected float m_iGroundBuffer = 0;
	protected SAL_DroneComponent m_cDroneComponent;
	protected CameraBase m_eCurrentCamera;
	
	//Get Members
	//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	static SAL_DroneClientComponent GetInstance()
	{
		if (GetGame().GetPlayerController())
			return SAL_DroneClientComponent.Cast(GetGame().GetPlayerController().FindComponent(SAL_DroneClientComponent));
		else
			return null;
	}
	
	ResourceName GetControllerPrefab()
	{
		return m_rControllerPrefab;
	}
	
	bool GetIsInDrone()
	{
		return m_bIsInDrone;
	}
	
	IEntity GetConnectedDrone()
	{
		return m_eConnectedDrone;
	}
	
	RplId GetRplId(IEntity entity)
	{
		return RplComponent.Cast(entity.FindComponent(RplComponent)).Id();
	}
	
	//Update for when flying the drone
	//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void Update()
	{
		//Basic checks for disconnect
		if(vector.Distance(m_ePlayer.GetOrigin(), m_eConnectedDrone.GetOrigin()) > m_cDroneComponent.GetDroneRange() || m_eConnectedDrone == null || SCR_EditorManagerEntity.GetInstance().IsOpened())
		{
			GetGame().GetCallqueue().Remove(Update);
			ExitDrone();
			if (m_wOverlayWidget)
				m_wOverlayWidget.RemoveFromHierarchy();
			m_wOverlayWidget = null;
			if (m_wStaticWidget)
				m_wStaticWidget.RemoveFromHierarchy();
			m_wStaticWidget = null;
			return;
		}	
		
		//Jammer BS, does it exist, if it does blah blah blah
		FilterJammers();
		bool power = false;
		if(m_eClosestBag)
		{
			m_cDroneJammerComponent = SAL_DroneJammerComponent.Cast(m_eClosestBag.FindComponent(SAL_DroneJammerComponent));
			power = SAL_DroneGamemodeComponent.GetInstance().GetJammerPowers().Get(SAL_DroneGamemodeComponent.GetInstance().GetJammerBags().Find(RplComponent.Cast(m_eClosestBag.FindComponent(RplComponent)).Id()));
		
			if(vector.Distance(m_eClosestBag.GetOrigin(), m_eConnectedDrone.GetOrigin()) <= m_cDroneJammerComponent.GetJammerRange() && power)
			{
				GetGame().GetCallqueue().Remove(Update);
				ExitDrone();
				SetConnectedDrone(m_rConnectedDroneRplId, false);
				if (m_wOverlayWidget)
					m_wOverlayWidget.RemoveFromHierarchy();
				m_wOverlayWidget = null;
				if (m_wStaticWidget)
					m_wStaticWidget.RemoveFromHierarchy();
				m_wStaticWidget = null;
				return;
			}
		}
		
		//Erm... what the duece does this player just not exist
		if(!m_ePlayer.FindComponent(SCR_CharacterDamageManagerComponent))
		{
			GetGame().GetCallqueue().Remove(Update);
			ExitDrone();
			SetConnectedDrone(m_rConnectedDroneRplId, false);
			if (m_wOverlayWidget)
					m_wOverlayWidget.RemoveFromHierarchy();
				m_wOverlayWidget = null;
			if (m_wStaticWidget)
					m_wStaticWidget.RemoveFromHierarchy();
				m_wStaticWidget = null;
			return;
		}
		
		//Oh no I died 
		if(SCR_CharacterDamageManagerComponent.Cast(m_ePlayer.FindComponent(SCR_CharacterDamageManagerComponent)).GetState() == EDamageState.DESTROYED)
		{
			GetGame().GetCallqueue().Remove(Update);
			ExitDrone();
			SetConnectedDrone(m_rConnectedDroneRplId, false);
			if (m_wOverlayWidget)
					m_wOverlayWidget.RemoveFromHierarchy();
				m_wOverlayWidget = null;
			if (m_wStaticWidget)
					m_wStaticWidget.RemoveFromHierarchy();
				m_wStaticWidget = null;
			return;
		}

		
		//Oh no it died
		if(SCR_HelicopterDamageManagerComponent.Cast(m_eConnectedDrone.FindComponent(SCR_HelicopterDamageManagerComponent)).GetState() == EDamageState.DESTROYED)
		{
			DroneExitAnimation(m_rConnectedDroneRplId);
			return;
		}
		
		//Third person and it's consequences on milsim
		if(SCR_CharacterCameraHandlerComponent.Cast(SCR_PlayerController.GetLocalControlledEntity().FindComponent(SCR_CharacterCameraHandlerComponent)).IsInThirdPerson() && m_wOverlayWidget)
		{
			if (m_wOverlayWidget)
				m_wOverlayWidget.RemoveFromHierarchy();
			m_wOverlayWidget = null;
			if (m_wStaticWidget)
				m_wStaticWidget.RemoveFromHierarchy();
			m_wStaticWidget = null;
			
			ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity()).GetWorld().SetCameraPostProcessEffect(ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity()).GetWorld().GetCurrentCameraId(), 9, PostProcessEffectType.None, "");
			ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity()).GetWorld().SetCameraPostProcessEffect(ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity()).GetWorld().GetCurrentCameraId(), 10, PostProcessEffectType.None, "");

			AudioSystem.SetMasterVolume(AudioSystem.SFX, m_fMasterVolume);
		}
		
		//Gigachas first person user, redraws the UI
		if(!SCR_CharacterCameraHandlerComponent.Cast(SCR_PlayerController.GetLocalControlledEntity().FindComponent(SCR_CharacterCameraHandlerComponent)).IsInThirdPerson() && !m_wOverlayWidget)
		{
			SwitchCamera(m_eConnectedDrone, SCR_PlayerController.GetLocalControlledEntity());
			
			if (m_wOverlayWidget)
				m_wOverlayWidget.RemoveFromHierarchy();
			m_wOverlayWidget = null;
			if (m_wStaticWidget)
				m_wStaticWidget.RemoveFromHierarchy();
			m_wStaticWidget = null;
			
			ResourceName staticWidgetLayout = "{FC6EB950BCCD1161}UI/Textures/TVStatic.layout";
			m_wStaticWidget = GetGame().GetWorkspace().CreateWidgets(staticWidgetLayout);
			
			m_wStaticImageWidget = ImageWidget.Cast(m_wStaticWidget.GetChildren());
		
			m_wStaticImageWidget.LoadImageTexture(1, "{980EF09C711CD338}UI/Textures/TVSTATIC.edds");
			m_wStaticImageWidget.LoadImageTexture(2, "{3A1849DED1F55B33}UI/Textures/TVSTATIC1.edds");
			m_wStaticImageWidget.LoadImageTexture(3, "{64F0609D6E78D53C}UI/Textures/TVSTATIC2.edds");
			m_wStaticImageWidget.LoadImageTexture(4, "{90F8270563A5BD48}UI/Textures/TVSTATIC3.edds");
			
			m_wOverlayWidget = GetGame().GetWorkspace().CreateWidgets(m_rOverlayWidgetLayout);
			
			if(m_cDroneComponent.GetHud().m_bZoomLevel)
			{
				m_wOverlayImageWidget = ImageWidget.Cast(m_wOverlayWidget.FindAnyWidget(m_cDroneComponent.GetHud().m_sZoomWidget));
				
				m_wOverlayImageWidget.LoadImageTexture(1, "{EC0977F3BECC2866}UI/DJIHud/Data/zoomlevel1.edds");
				m_wOverlayImageWidget.LoadImageTexture(2, "{B2E15EB00141A669}UI/DJIHud/Data/zoomlevel2.edds");
				m_wOverlayImageWidget.LoadImageTexture(3, "{46E919280C9CCE1D}UI/DJIHud/Data/zoomlevel3.edds");
				m_wOverlayImageWidget.LoadImageTexture(4, "{0F310C377E5ABA77}UI/DJIHud/Data/zoomlevel4.edds");
				m_wOverlayImageWidget.LoadImageTexture(5, "{FB394BAF7387D203}UI/DJIHud/Data/zoomlevel5.edds");
				m_wOverlayImageWidget.LoadImageTexture(6, "{A5D162ECCC0A5C0C}UI/DJIHud/Data/zoomlevel6.edds");
			};
			
			if(m_cDroneComponent.GetHud().m_bMoveRoll)
			{
				m_wOverlayImageWidget = ImageWidget.Cast(m_wOverlayWidget.FindAnyWidget(m_cDroneComponent.GetHud().m_sRollWidget));
				m_wOverlayImageWidget.LoadImageTexture(1, "{5021E90D8407E44F}UI/FPVHud/Data/FPV_DOTS_THAT_MOVE.edds");
				m_wOverlayImageWidget.SetImage(1);
			}
			
			if(m_cDroneComponent.GetHud().m_bRecordSymbol)
			{
				ImageWidget recImageWidget = ImageWidget.Cast(m_wOverlayWidget.FindAnyWidget(m_cDroneComponent.GetHud().m_sRecordWidget));
				recImageWidget.LoadImageTexture(1, "{B112098DDBA3BC0D}UI/FPVHud/Data/RecSymbol.edds");
				recImageWidget.SetImage(1);
				GetGame().GetCallqueue().CallLater(RecAnimation, 1000, true, recImageWidget);
			}
			
			ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity()).GetWorld().SetCameraPostProcessEffect(ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity()).GetWorld().GetCurrentCameraId(), 9, PostProcessEffectType.ChromAber, "{C245539454FC3F58}UI/Materials/ScreenEffects_ChromAberrPPDrone.emat");
			ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity()).GetWorld().SetCameraPostProcessEffect(ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity()).GetWorld().GetCurrentCameraId(), 10, PostProcessEffectType.FilmGrain, "{F5DD5D01C23AD429}UI/Materials/Editor/FilmGrainDrone.emat");

			AudioSystem.SetMasterVolume(AudioSystem.SFX, 0.05);
		}
		
			
		//Static animation and calculations
		if(m_wStaticWidget)
		{
			int randomInt = Math.RandomInt(1, 5);
			m_wStaticImageWidget = ImageWidget.Cast(m_wStaticWidget.GetChildren());
			
			switch(randomInt)
			{
				case 1: {m_wStaticImageWidget.SetImage(1); break;}
				case 2: {m_wStaticImageWidget.SetImage(2); break;}
				case 3: {m_wStaticImageWidget.SetImage(3); break;}
				case 4: {m_wStaticImageWidget.SetImage(4); break;}
			}
			int staticStrength;
			int distanceFrom = vector.Distance(m_ePlayer.GetOrigin(), m_eConnectedDrone.GetOrigin());
			int maxDistance = m_cDroneComponent.GetDroneRange();
			float distancePercentage = distanceFrom/maxDistance;
			if(!power)
				staticStrength =  Math.Round((distancePercentage*255)*distancePercentage);
			
			if(power)
				if(vector.Distance(m_eClosestBag.GetOrigin(), m_eConnectedDrone.GetOrigin()) < (m_cDroneJammerComponent.GetJammerRange() * m_cDroneJammerComponent.GetJammerMultiplier()))
					staticStrength = Math.Round(((distancePercentage*255)*distancePercentage)) +  Math.Round(((((vector.Distance(m_eClosestBag.GetOrigin(), m_eConnectedDrone.GetOrigin()) - m_cDroneJammerComponent.GetJammerRange())/((m_cDroneJammerComponent.GetJammerRange() * m_cDroneJammerComponent.GetJammerMultiplier()) - m_cDroneJammerComponent.GetJammerRange()) - 1) * -1) * 255) * (((vector.Distance(m_eClosestBag.GetOrigin(), m_eConnectedDrone.GetOrigin()) - m_cDroneJammerComponent.GetJammerRange())/((m_cDroneJammerComponent.GetJammerRange() * m_cDroneJammerComponent.GetJammerMultiplier()) - m_cDroneJammerComponent.GetJammerRange()) - 1) * -1));			
				else
					staticStrength =  Math.Round((distancePercentage*255)*distancePercentage);
			if(m_cDroneComponent.GetLOS())
			{
				vector droneSignal = m_eConnectedDrone.GetOrigin();
				droneSignal[1] = droneSignal[1] + 0.5;
				droneSignal[2] = droneSignal[2] - 0.5;
				vector playerSignal = m_ePlayer.GetOrigin();
				playerSignal[1] = playerSignal[1] + 2;
				ref array<int> playerIDs = {};
				GetGame().GetPlayerManager().GetAllPlayers(playerIDs);
				ref array<IEntity> allEntities = {};
				foreach(int playerID: playerIDs)
				{
					allEntities.Insert(GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID));
				}
				ref array<AIAgent> allAgents = {};
				if(GetGame().GetAIWorld())
				{
					GetGame().GetAIWorld().GetAIAgents(allAgents);
					foreach(AIAgent agent: allAgents)
					{
						allEntities.Insert(agent.GetControlledEntity());
					}
				}
				allEntities.Insert(m_eConnectedDrone);
				float groundInterference = RayCastLine(droneSignal, playerSignal, EPhysicsLayerDefs.Terrain, allEntities);
				if(groundInterference > 0)
					m_iGroundBuffer += 1;
				
				if(m_iGroundBuffer > 0 && groundInterference == 0)
					m_iGroundBuffer -= 1;
				
				if(m_iGroundBuffer > 0)
					staticStrength += 255 * ((m_iGroundBuffer/5)/100);
				
				if(m_iGroundBuffer >= 500)
				{
					GetGame().GetCallqueue().Remove(Update);
					ExitDrone();
					SetConnectedDrone(m_rConnectedDroneRplId, false);
					if (m_wOverlayWidget)
						m_wOverlayWidget.RemoveFromHierarchy();
					m_wOverlayWidget = null;
					if (m_wStaticWidget)
						m_wStaticWidget.RemoveFromHierarchy();
					m_wStaticWidget = null;
					return;
				}
			}
			
			if(staticStrength > 255)
				staticStrength = 255;
			m_wStaticImageWidget.SetColor(Color.FromRGBA(255, 255, 255, staticStrength));
		}
		
		//Overlay animations
		if(m_wOverlayWidget)
		{		
			SAL_DroneHUDConfig hudConfig = m_cDroneComponent.GetHud();
			
			if(hudConfig.m_bMoveCompass)
				MoveCompass(hudConfig.m_sCompassWidget);
			
			if(hudConfig.m_bBearing)
				SetBearing(hudConfig.m_sBearingWidget);
			
			if(hudConfig.m_bAltitude)
				SetAltitude(hudConfig.m_sAltitudeWidget);
			
			if(hudConfig.m_bSpeed)
				SetSpeed(hudConfig.m_sSpeedWidget);
			
			if(hudConfig.m_bMoveRoll)
				SetRoll(hudConfig.m_sRollWidget);
		}
		
		//Make sure the player can get out
		if(!GetGame().GetInputManager().IsContextActive("DroneContext"))
			GetGame().GetInputManager().ActivateContext("DroneContext", 99999);
		if(!GetGame().GetInputManager().IsActionActive("ExitDrone"))
			GetGame().GetInputManager().AddActionListener("ExitDrone", EActionTrigger.PRESSED, ExitDroneAction);
	}
	
	//Drone destroyed animation members
	//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	protected int m_delay = 0;
	void DroneExitAnimation(RplId droneID)
	{
		if(m_delay == 20)
		{
			IEntity previouseController = SCR_PlayerController.GetLocalControlledEntity();
			GetGame().GetCallqueue().CallLater(DeleteFinalEntity, 10000, false, previouseController);
			if (m_wOverlayWidget)
				m_wOverlayWidget.RemoveFromHierarchy();
			m_wOverlayWidget = null;
			if (m_wStaticWidget)
				m_wStaticWidget.RemoveFromHierarchy();
			m_wStaticWidget = null;
			m_iGroundBuffer = 0;
			ExitDrone();
			SetConnectedDrone(droneID, false);	
			SCR_EditorManagerEntity editorManagerEntity = SCR_EditorManagerEntity.GetInstance();
			editorManagerEntity.Close();
			m_delay = 0;
			return;
		}
		
		if(m_delay == 0)
		{
			GetGame().GetCallqueue().Remove(Update);
			if (m_wOverlayWidget)
				m_wOverlayWidget.RemoveFromHierarchy();
			m_wOverlayWidget = null;
		}
		
		int randomInt = Math.RandomInt(1, 5);
			
		switch(randomInt)
		{
			case 1: {m_wStaticImageWidget.SetImage(1); break;}
			case 2: {m_wStaticImageWidget.SetImage(2); break;}
			case 3: {m_wStaticImageWidget.SetImage(3); break;}
			case 4: {m_wStaticImageWidget.SetImage(4); break;}
		}
		m_wStaticImageWidget.SetColor(Color.FromRGBA(255, 255, 255, 255));
		m_delay++;
		GetGame().GetCallqueue().CallLater(DroneExitAnimation, 50, false, droneID);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcDo_SpawnDestroyedEntity(RplId entityID, int playerID)
	{
		IEntity entity = RplComponent.Cast(Replication.FindItem(entityID)).GetEntity();
		EntitySpawnParams params = new EntitySpawnParams();
		entity.GetTransform(params.Transform);
		params.Transform[3] + Vector(0, 250, 0);
		IEntity previousDroneController = SCR_PlayerController.GetLocalControlledEntity();
	}
	
	void DeleteFinalEntity(IEntity entity)
	{
		Rpc(RpcDo_DeleteFinalEntity, RplComponent.Cast(entity.FindComponent(RplComponent)).Id());
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcDo_DeleteFinalEntity(RplId entityID)
	{

		delete RplComponent.Cast(Replication.FindItem(entityID)).GetEntity();
	}
	
	//Drone connection members
	//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void SetConnectedDrone(RplId droneID, bool input)
	{	
		if(input == false)
		{
			m_eConnectedDrone = null;
			m_rConnectedDroneRplId = null;
			m_cDroneComponent = null;
			m_iGroundBuffer = 0;
			SetDroneConnection(false, droneID);
			GetGame().GetInputManager().ActivateContext("DroneContext");
		}
		else
		{
			m_eConnectedDrone = RplComponent.Cast(Replication.FindItem(droneID)).GetEntity();
			m_rConnectedDroneRplId = droneID;
			m_cDroneComponent = SAL_DroneComponent.Cast(m_eConnectedDrone.FindComponent(SAL_DroneComponent));
			SetDroneConnection(true, droneID);
			if(m_bIsInDrone)
				ExitDrone();
			GetGame().GetInputManager().ActivateContext("DroneContext", 999999);
		}
	}
	
	void SetDroneConnection(bool input, RplId droneID)
	{
		Rpc(RpcDo_SetDroneConnection, input, droneID, SCR_PlayerController.GetLocalControlledEntityFaction().GetFactionKey());
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcDo_SetDroneConnection(bool input, RplId droneID, string factionKey)
	{
		if(input == true)
		{
			BaseCompartmentManagerComponent compartmentManger = BaseCompartmentManagerComponent.Cast(RplComponent.Cast(Replication.FindItem(droneID)).GetEntity().FindComponent(BaseCompartmentManagerComponent));
		
			ref array<BaseCompartmentSlot> compartments = {};
			compartmentManger.GetCompartments(compartments);
			
			BaseCompartmentSlot slot = compartments[0];
			IEntity droneAI = slot.AttachedOccupant();
			if(!droneAI)
				return;
			FactionAffiliationComponent.Cast(droneAI.FindComponent(FactionAffiliationComponent)).SetAffiliatedFaction(GetGame().GetFactionManager().GetFactionByKey(factionKey));
			SCR_FactionAffiliationComponent.Cast(RplComponent.Cast(Replication.FindItem(droneID)).GetEntity().FindComponent(SCR_FactionAffiliationComponent)).SetAffiliatedFaction(GetGame().GetFactionManager().GetFactionByKey(factionKey));
			SAL_DroneGamemodeComponent.GetInstance().ConnectDrone(droneID);
		}
		else
		{
			BaseCompartmentManagerComponent compartmentManger = BaseCompartmentManagerComponent.Cast(RplComponent.Cast(Replication.FindItem(droneID)).GetEntity().FindComponent(BaseCompartmentManagerComponent));
		
			ref array<BaseCompartmentSlot> compartments = {};
			compartmentManger.GetCompartments(compartments);
			
			BaseCompartmentSlot slot = compartments[0];
			IEntity droneAI = slot.AttachedOccupant();
			if(!droneAI)
				return;
			FactionAffiliationComponent.Cast(droneAI.FindComponent(FactionAffiliationComponent)).SetAffiliatedFaction(null);
			SCR_FactionAffiliationComponent.Cast(RplComponent.Cast(Replication.FindItem(droneID)).GetEntity().FindComponent(SCR_FactionAffiliationComponent)).SetAffiliatedFaction(null);
			SAL_DroneGamemodeComponent.GetInstance().DisconnectDrone(droneID);
		}
	}
	protected void ExitDroneAction(float value, EActionTrigger reason)
	{
		if(m_eConnectedDrone != null)
			ExitDrone();
	}
	
	//Swithc to drone memebers
	//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void SwitchToDrone()
	{
		if(m_bIsInDrone)
			return;
		
		if(!m_eConnectedDrone)
			return;
		
		if(SCR_HelicopterDamageManagerComponent.Cast(m_eConnectedDrone.FindComponent(SCR_HelicopterDamageManagerComponent)).GetState() == EDamageState.DESTROYED)
			return;
		
		BaseCompartmentManagerComponent compartmentManger = BaseCompartmentManagerComponent.Cast(m_eConnectedDrone.FindComponent(BaseCompartmentManagerComponent));
		if (!compartmentManger)
			return;
		
		ref array<BaseCompartmentSlot> compartments = {};
		compartmentManger.GetCompartments(compartments);
		BaseCompartmentSlot slot = compartments[0];
		if (!slot)
			return;
		
		SwitchCamera(m_eConnectedDrone, slot.GetOccupant());
		
		m_ePlayer = SCR_PlayerController.GetLocalControlledEntity();
		
		m_SignalsManagerComponent = SignalsManagerComponent.Cast(SCR_PlayerController.GetLocalControlledEntity().FindComponent(SignalsManagerComponent));
		
		m_bIsInDrone = true;
		RplId droneSlotID = GetRplId(m_eConnectedDrone);
		GetGame().GetCallqueue().CallLater(Update, 10, true);
		Rpc(RpcDo_SwitchToDrone, droneSlotID, SCR_PlayerController.GetLocalPlayerId());
		
		if (m_wOverlayWidget)
			m_wOverlayWidget.RemoveFromHierarchy();
		m_wOverlayWidget = null;
		
		ResourceName staticWidgetLayout = "{FC6EB950BCCD1161}UI/Textures/TVStatic.layout";
		m_wStaticWidget = GetGame().GetWorkspace().CreateWidgets(staticWidgetLayout);
		
		m_wStaticImageWidget = ImageWidget.Cast(m_wStaticWidget.GetChildren());
		
		m_wStaticImageWidget.LoadImageTexture(1, "{980EF09C711CD338}UI/Textures/TVSTATIC.edds");
		m_wStaticImageWidget.LoadImageTexture(2, "{3A1849DED1F55B33}UI/Textures/TVSTATIC1.edds");
		m_wStaticImageWidget.LoadImageTexture(3, "{64F0609D6E78D53C}UI/Textures/TVSTATIC2.edds");
		m_wStaticImageWidget.LoadImageTexture(4, "{90F8270563A5BD48}UI/Textures/TVSTATIC3.edds");

		m_rOverlayWidgetLayout = m_cDroneComponent.GetHud().m_rHUDLayout;
		m_wOverlayWidget = GetGame().GetWorkspace().CreateWidgets(m_rOverlayWidgetLayout);
		
		if(m_cDroneComponent.GetHud().m_bZoomLevel)
		{
			m_wOverlayImageWidget = ImageWidget.Cast(m_wOverlayWidget.FindAnyWidget(m_cDroneComponent.GetHud().m_sZoomWidget));
				
			m_wOverlayImageWidget.LoadImageTexture(1, "{EC0977F3BECC2866}UI/DJIHud/Data/zoomlevel1.edds");
			m_wOverlayImageWidget.LoadImageTexture(2, "{B2E15EB00141A669}UI/DJIHud/Data/zoomlevel2.edds");
			m_wOverlayImageWidget.LoadImageTexture(3, "{46E919280C9CCE1D}UI/DJIHud/Data/zoomlevel3.edds");
			m_wOverlayImageWidget.LoadImageTexture(4, "{0F310C377E5ABA77}UI/DJIHud/Data/zoomlevel4.edds");
			m_wOverlayImageWidget.LoadImageTexture(5, "{FB394BAF7387D203}UI/DJIHud/Data/zoomlevel5.edds");
			m_wOverlayImageWidget.LoadImageTexture(6, "{A5D162ECCC0A5C0C}UI/DJIHud/Data/zoomlevel6.edds");
		};
			
		if(m_cDroneComponent.GetHud().m_bMoveRoll)
		{
			m_wOverlayImageWidget = ImageWidget.Cast(m_wOverlayWidget.FindAnyWidget(m_cDroneComponent.GetHud().m_sRollWidget));
			m_wOverlayImageWidget.LoadImageTexture(1, "{5021E90D8407E44F}UI/FPVHud/Data/FPV_DOTS_THAT_MOVE.edds");
			m_wOverlayImageWidget.SetImage(1);
		}
			
		if(m_cDroneComponent.GetHud().m_bRecordSymbol)
		{
			ImageWidget recImageWidget = ImageWidget.Cast(m_wOverlayWidget.FindAnyWidget(m_cDroneComponent.GetHud().m_sRecordWidget));
			recImageWidget.LoadImageTexture(1, "{B112098DDBA3BC0D}UI/FPVHud/Data/RecSymbol.edds");
			recImageWidget.SetImage(1);
			GetGame().GetCallqueue().CallLater(RecAnimation, 1000, true, recImageWidget);
		}
		
		GetGame().GetInputManager().AddActionListener("ExitDrone", EActionTrigger.PRESSED, ExitDroneAction);
		
		if(m_cDroneComponent.GetCanMoveCamera())
		{
			GetGame().GetInputManager().AddActionListener("CameraUp", EActionTrigger.VALUE, CameraUp);
			GetGame().GetInputManager().AddActionListener("CameraRight", EActionTrigger.VALUE, CameraRight);
			GetGame().GetInputManager().AddActionListener("ResetCamera", EActionTrigger.PRESSED, CenterCamera);
			GetGame().GetInputManager().AddActionListener("AdjustCameraFOV", EActionTrigger.VALUE, CameraZoom);
		}
		
		if(m_cDroneComponent.GetCanTarget())
			GetGame().GetInputManager().AddActionListener("DroneLaser", EActionTrigger.DOWN, GetGrid);
		
		ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity()).GetWorld().SetCameraPostProcessEffect(ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity()).GetWorld().GetCurrentCameraId(), 9, PostProcessEffectType.ChromAber, "{C245539454FC3F58}UI/Materials/ScreenEffects_ChromAberrPPDrone.emat");
		ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity()).GetWorld().SetCameraPostProcessEffect(ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity()).GetWorld().GetCurrentCameraId(), 10, PostProcessEffectType.FilmGrain, "{F5DD5D01C23AD429}UI/Materials/Editor/FilmGrainDrone.emat");

		m_fMasterVolume = AudioSystem.GetMasterVolume(AudioSystem.SFX);
		AudioSystem.SetMasterVolume(AudioSystem.SFX, 0.05);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcDo_SwitchToDrone(RplId droneID, int playerID)
	{
		IEntity drone = RplComponent.Cast(Replication.FindItem(droneID)).GetEntity();
		BaseCompartmentManagerComponent compartmentManger = BaseCompartmentManagerComponent.Cast(drone.FindComponent(BaseCompartmentManagerComponent));
		
		ref array<BaseCompartmentSlot> compartments = {};
		compartmentManger.GetCompartments(compartments);
		
		BaseCompartmentSlot slot = compartments[0];
		IEntity droneAI = slot.AttachedOccupant();
		
		SCR_InventoryStorageManagerComponent playerStorage = SCR_InventoryStorageManagerComponent.Cast(GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID).FindComponent(SCR_InventoryStorageManagerComponent));
		SCR_InventoryStorageManagerComponent droneStorage = SCR_InventoryStorageManagerComponent.Cast(droneAI.FindComponent(SCR_InventoryStorageManagerComponent));
		ref array<IEntity> itemsPlayer = {};
		ref array<IEntity> itemsDrone = {};
		IEntity radioPlayer;
		IEntity radioDrone;
		playerStorage.GetItems(itemsPlayer);
		droneStorage.GetItems(itemsDrone);
		foreach(IEntity item: itemsPlayer)
		{
			if(item.FindComponent(SCR_RadioComponent))
				radioPlayer = item;
		}
		if(radioPlayer)
		{
			foreach(IEntity item: itemsDrone)
			{
				if(item.FindComponent(SCR_RadioComponent))
					radioDrone = item;
			}
			BaseRadioComponent radioComponent = SCR_RadioComponent.Cast(radioPlayer.FindComponent(SCR_RadioComponent)).GetRadioComponent();
			SCR_RadioComponent.Cast(radioDrone.FindComponent(SCR_RadioComponent)).GetRadioComponent().SetEncryptionKey(radioComponent.GetEncryptionKey());
			for(int i = 0; i < radioComponent.TransceiversCount(); i++)
			{
				BaseTransceiver transceiverPlayer = radioComponent.GetTransceiver(i);
				BaseTransceiver transceiverDrone = SCR_RadioComponent.Cast(radioDrone.FindComponent(SCR_RadioComponent)).GetRadioComponent().GetTransceiver(i);
				if(!transceiverPlayer)
					break;
				if(!transceiverDrone)
					break;
				transceiverDrone.SetFrequency(transceiverPlayer.GetFrequency());
			}
			
		}
		
		if(!drone)
			return;
		
		SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerID)).SetInitialMainEntity(droneAI);
	}
	
	void SwitchCamera(IEntity drone, IEntity droneController)
	{
		CameraBase camera = CameraBase.Cast(SlotManagerComponent.Cast(drone.FindComponent(SlotManagerComponent)).GetSlotByName("Camera").GetAttachedEntity());
		m_eCurrentCamera = GetGame().GetCameraManager().CurrentCamera();
		GetGame().GetCameraManager().SetCamera(camera);
	}
	
	//Exit drone members
	//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void ExitDrone()
	{
		if(!m_bIsInDrone)
			return;
		
		if (m_wOverlayWidget)
			m_wOverlayWidget.RemoveFromHierarchy();
		m_wOverlayWidget = null;
		
		if (m_wStaticWidget)
			m_wStaticWidget.RemoveFromHierarchy();
		m_wStaticWidget = null;
		
		
		m_bIsInDrone = false;
		GetGame().GetCameraManager().SetCamera(m_eCurrentCamera);
		GetGame().GetCallqueue().Remove(Update);
		GetGame().GetCallqueue().Remove(RecAnimation);
		Rpc(RpcDo_ExitDrone, GetRplId(m_ePlayer), m_rConnectedDroneRplId, SCR_PlayerController.GetLocalPlayerId());
		
		GetGame().GetInputManager().RemoveActionListener("ExitDrone", EActionTrigger.PRESSED, ExitDroneAction);
		GetGame().GetInputManager().RemoveActionListener("CameraUp", EActionTrigger.VALUE, CameraUp);
		GetGame().GetInputManager().RemoveActionListener("CameraRight", EActionTrigger.VALUE, CameraRight);
		GetGame().GetInputManager().RemoveActionListener("ResetCamera", EActionTrigger.PRESSED, CenterCamera);
		GetGame().GetInputManager().RemoveActionListener("AdjustCameraFOV", EActionTrigger.VALUE, CameraZoom);
		GetGame().GetInputManager().RemoveActionListener("DroneLaser", EActionTrigger.DOWN, GetGrid);
		
		AudioSystem.SetMasterVolume(AudioSystem.SFX, m_fMasterVolume);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcDo_ExitDrone(RplId originalPlayer, RplId droneID, int playerID)
	{
		IEntity drone;
		IEntity player;
		IEntity droneAI;
		ActionsManagerComponent actionsManagerComponent
		if(Replication.FindItem(droneID))
			drone = RplComponent.Cast(Replication.FindItem(droneID)).GetEntity();
		if(Replication.FindItem(originalPlayer))
			player = RplComponent.Cast(Replication.FindItem(originalPlayer)).GetEntity();
		
		if(!player.FindComponent(SCR_CharacterDamageManagerComponent))
		{
			DamageManagerComponent damageManager = DamageManagerComponent.Cast(GetGame().GetPlayerManager().GetPlayerController(playerID).FindComponent(DamageManagerComponent));
			damageManager.SetHealthScaled(0);
			return;
		}
		
		if(SCR_CharacterDamageManagerComponent.Cast(player.FindComponent(SCR_CharacterDamageManagerComponent)).GetState() == EDamageState.DESTROYED)
		{
			DamageManagerComponent damageManager = DamageManagerComponent.Cast(GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID).FindComponent(DamageManagerComponent));
			damageManager.SetHealthScaled(0);
			return;
		}
		
		SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerID)).SetInitialMainEntity(player);
	}
	
	//Pick up drone members
	//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void PickUpDrone(IEntity drone)
	{
		Rpc(RpcDo_PickUpDrone, GetRplId(drone), SCR_PlayerController.GetLocalPlayerId());
		SCR_CharacterControllerComponent.Cast(SCR_PlayerController.GetLocalControlledEntity().FindComponent(SCR_CharacterControllerComponent)).TryPlayItemGesture(EItemGesture.EItemGesturePickUp);	
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcDo_PickUpDrone(RplId droneID, int playerID)
	{
		IEntity drone = RplComponent.Cast(Replication.FindItem(droneID)).GetEntity();
		ResourceName droneItem = SAL_DroneComponent.Cast(drone.FindComponent(SAL_DroneComponent)).GetGroundItem();
		if(!SCR_InventoryStorageManagerComponent.Cast(GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID).FindComponent(SCR_InventoryStorageManagerComponent)).CanInsertResource(droneItem, EStoragePurpose.PURPOSE_DEPOSIT))
			return;
		EntitySpawnParams params = new EntitySpawnParams();
		drone.GetTransform(params.Transform);
		params.Transform[3] + Vector(0, 250, 0);
		IEntity droneController = GetGame().SpawnEntityPrefab(Resource.Load(droneItem), null, params);

		SCR_InventoryStorageManagerComponent.Cast(GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID).FindComponent(SCR_InventoryStorageManagerComponent)).InsertItem(droneController);
		delete drone;
	}	
	
	//Camera Movement Vectors
	//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void CameraUp(float value, EActionTrigger reason)
	{
		if(value == 1)
		{
			IEntity cameraPoint = SlotManagerComponent.Cast(m_eConnectedDrone.FindComponent(SlotManagerComponent)).GetSlotByName("Camera").GetAttachedEntity();
			vector angles = cameraPoint.GetLocalYawPitchRoll();
			angles[1] = angles[1] + 1;
			
			if(angles[1] > m_cDroneComponent.GetMaxUp()[0])
				return;
			cameraPoint.SetYawPitchRoll(angles);
		}
		else if(value == -1)
		{
			IEntity cameraPoint = SlotManagerComponent.Cast(m_eConnectedDrone.FindComponent(SlotManagerComponent)).GetSlotByName("Camera").GetAttachedEntity();
			vector angles = cameraPoint.GetLocalYawPitchRoll();
			angles[1] = angles[1] - 1;
			
			if(angles[1] < m_cDroneComponent.GetMaxDown()[0])
				return;
			cameraPoint.SetYawPitchRoll(angles);
		}
	}
	
	void CameraRight(float value, EActionTrigger reason)
	{
		if(value == 1)
		{
			IEntity cameraPoint = SlotManagerComponent.Cast(m_eConnectedDrone.FindComponent(SlotManagerComponent)).GetSlotByName("Camera").GetAttachedEntity();
			vector angles = cameraPoint.GetLocalYawPitchRoll();
			angles[0] = angles[0] + 1;
			cameraPoint.SetYawPitchRoll(angles);
		} else if (value == -1)
		{
			IEntity cameraPoint = SlotManagerComponent.Cast(m_eConnectedDrone.FindComponent(SlotManagerComponent)).GetSlotByName("Camera").GetAttachedEntity();
			vector angles = cameraPoint.GetLocalYawPitchRoll();
			angles[0] = angles[0] - 1;
			cameraPoint.SetYawPitchRoll(angles);
		}
	}
	
	void CenterCamera(float value, EActionTrigger reason)
	{
		GetGame().GetCallqueue().CallLater(CenterCameraUp, 50, true);
		GetGame().GetCallqueue().CallLater(CenterCameraRight, 50, true);
	}
	
	void CenterCameraUp()
	{
		IEntity cameraPoint = SlotManagerComponent.Cast(m_eConnectedDrone.FindComponent(SlotManagerComponent)).GetSlotByName("Camera").GetAttachedEntity();
		vector angles = cameraPoint.GetLocalYawPitchRoll();	
		if(angles[1] > -1 && angles[1] < 1)
		{
			angles[1] = 0;
			cameraPoint.SetYawPitchRoll(angles);	
			GetGame().GetCallqueue().Remove(CenterCameraUp);
		}
		if(angles[1] < -1)
		{
			angles[1] = angles[1] + 1;
			cameraPoint.SetYawPitchRoll(angles);	
		}
		else if(angles[1] > 1)
		{
			angles[1] = angles[1] - 1;
			cameraPoint.SetYawPitchRoll(angles);	
		}
	}
	
	void CenterCameraRight()
	{
		IEntity cameraPoint = SlotManagerComponent.Cast(m_eConnectedDrone.FindComponent(SlotManagerComponent)).GetSlotByName("Camera").GetAttachedEntity();
		vector angles = cameraPoint.GetLocalYawPitchRoll();	
		if(angles[0] > -1 && angles[0] < 1)
		{
			angles[0] = 0;
			cameraPoint.SetYawPitchRoll(angles);	
			GetGame().GetCallqueue().Remove(CenterCameraRight);
		}
		if(angles[0] < -1)
		{
			angles[0] = angles[0] + 1;
			cameraPoint.SetYawPitchRoll(angles);	
		}
		else if(angles[0] > 1)
		{
			angles[0] = angles[0] - 1;
			cameraPoint.SetYawPitchRoll(angles);	
		}
	}
	
	void CameraZoom(float value, EActionTrigger reason)
	{
		float newFOV = m_fCameraFOV + 10 * value;
		if(newFOV < 20 || newFOV > 70)
			return;
		
		m_fCameraFOV = newFOV;
		
		if(!m_wOverlayImageWidget)
			return;
		
		switch(m_fCameraFOV)
		{
			case 70: {m_wOverlayImageWidget.SetImage(1); break;}
			case 60: {m_wOverlayImageWidget.SetImage(2); break;}
			case 50: {m_wOverlayImageWidget.SetImage(3); break;}
			case 40: {m_wOverlayImageWidget.SetImage(4); break;}
			case 30: {m_wOverlayImageWidget.SetImage(5); break;}
			case 20: {m_wOverlayImageWidget.SetImage(6); break;}
		}
		
		GetGame().GetCameraManager().CurrentCamera().SetFOVDegree(m_fCameraFOV);
	}
	
	void RecAnimation(ImageWidget imageWidget)
	{
		if(imageWidget.IsVisible())
			imageWidget.SetVisible(false);
		else
			imageWidget.SetVisible(true);
	}
	
	//Inspired from the RHS RangeFinder
	void GetGrid()
	{
		vector transform[4];
		GetGame().GetWorld().GetCurrentCamera(transform);
		float maxRange = m_cDroneComponent.GetMaxRange();
		vector targetPos = transform[3] + (transform[2].Normalized() * maxRange);
		float hitDistance = RayCastLine(transform[3], targetPos, EPhysicsLayerDefs.Projectile) * maxRange;
		vector hitPos
		if (hitDistance > 0)
			hitPos = transform[3] + (transform[2].Normalized() * hitDistance);
		int gridX;
		int gridY;
		SCR_MapEntity.GetGridPos(hitPos, gridX, gridY, 0, 4);
		TextWidget gridWidget = TextWidget.Cast(m_wOverlayWidget.FindAnyWidget("Grid"));
		string finalGrid = "GRID: " + gridX.ToString() + " - " + gridY.ToString();
		gridWidget.SetText(finalGrid);
		TextWidget distanceWidget = TextWidget.Cast(m_wOverlayWidget.FindAnyWidget("Distance"));
		string finalDistance = "DISTANCE: " + Math.Round(hitDistance).ToString() + "m";
		distanceWidget.SetText(finalDistance);
	}
	
	//Written by an RHS Dev
	private static float RayCastLine(vector start, vector end, EPhysicsLayerDefs layer, array<IEntity> entityArray = null)
	{
		autoptr TraceParam p = new TraceParam;
		ChimeraCharacter ce = ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
		
		p.Exclude = ce;
		p.Flags = TraceFlags.DEFAULT | TraceFlags.ANY_CONTACT;
		p.LayerMask = layer;
		p.Start = start;
		p.End = end;
		if(entityArray)
			p.ExcludeArray = entityArray;
		
		return TraceMoveHit(p);
	}
	
	//Written by an RHS Dev
	private static float TraceMoveHit(TraceParam p)
	{
		float hit = GetGame().GetWorld().TraceMove(p, null);
		
		if (!p.TraceEnt)
			return 0;

		
		return hit;
	}
	
	//Jammer Members
	//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void FilterJammers()
	{
		ref array<RplId> jammerBagIDs = SAL_DroneGamemodeComponent.GetInstance().GetJammerBags();
		ref array<string> jammerBagFactions = SAL_DroneGamemodeComponent.GetInstance().GetJammerFactions();
		ref array<bool> jammerBagPowers = SAL_DroneGamemodeComponent.GetInstance().GetJammerPowers();
		if(jammerBagIDs.Count() == 0)
			return;
		for(int i = 0; i < jammerBagIDs.Count(); i++)
		{
			if(!Replication.FindItem(jammerBagIDs.Get(i)))
				continue;
			
			if(!jammerBagPowers.Get(i))
				continue;
			
			IEntity jammerBag = RplComponent.Cast(Replication.FindItem(jammerBagIDs.Get(i))).GetEntity();	
			SAL_DroneJammerComponent droneJammerComponent = SAL_DroneJammerComponent.Cast(jammerBag.FindComponent(SAL_DroneJammerComponent));
			if(jammerBagFactions.Get(i) == SCR_PlayerController.GetLocalControlledEntityFaction().GetFactionKey())
				continue;
			
			if(vector.Distance(m_eConnectedDrone.GetOrigin(), jammerBag.GetOrigin()) < (droneJammerComponent.GetJammerRange() * droneJammerComponent.GetJammerMultiplier()))
				if(m_eClosestBag)
					if(vector.Distance(m_eConnectedDrone.GetOrigin(), jammerBag.GetOrigin()) < vector.Distance(m_eConnectedDrone.GetOrigin(), m_eClosestBag.GetOrigin()))
						m_eClosestBag = jammerBag;
				if(!m_eClosestBag)
					m_eClosestBag = jammerBag;
		}
	}
	
	void UpdatePowers(IEntity entity, bool power)
	{
		Rpc(RpcDo_UpdatePower, GetRplId(entity), power);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcDo_UpdatePower(RplId entityID, bool power)
	{
		SAL_DroneGamemodeComponent.GetInstance().RpcDo_UpdatePower(entityID, power);
	}
	
	void DestroyDroneJammer(IEntity entity)
	{
		Rpc(RpcDo_DestroyDroneJammer, GetRplId(entity));
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcDo_DestroyDroneJammer(RplId entityID)
	{
		IEntity jammerBag = RplComponent.Cast(Replication.FindItem(entityID)).GetEntity();
		
		EntitySpawnParams params = new EntitySpawnParams();
		jammerBag.GetTransform(params.Transform);
		IEntity destroyedJammerBag =  GetGame().SpawnEntityPrefab(Resource.Load(SAL_DroneJammerComponent.Cast(jammerBag.FindComponent(SAL_DroneJammerComponent)).GetDestroyedBag()), null, params);
		
		
		ParticleEffectEntitySpawnParams spawnParams();
		spawnParams.Parent = destroyedJammerBag;
		ParticleEffectEntity emitter = ParticleEffectEntity.SpawnParticleEffect("{87DC8999EBCF0195}Particles/JammerDestroyed.ptc", spawnParams);
		
		if(jammerBag.GetParent())
		{
			SCR_InventoryStorageManagerComponent playerStorage = SCR_InventoryStorageManagerComponent.Cast(jammerBag.GetParent().FindComponent(SCR_InventoryStorageManagerComponent));
			delete jammerBag;
			playerStorage.TryInsertItem(destroyedJammerBag);
		}
		else
			delete jammerBag;
	}
	
	//Drone Reload Members
	//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void ReloadDrone(IEntity drone, IEntity item)
	{
		Rpc(RpcDo_ReloadDrone, GetRplId(drone), GetRplId(item));
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcDo_ReloadDrone(RplId droneID, RplId itemID)
	{
		IEntity drone = RplComponent.Cast(Replication.FindItem(droneID)).GetEntity();
		IEntity item = RplComponent.Cast(Replication.FindItem(itemID)).GetEntity();
		
		EntitySpawnParams params = new EntitySpawnParams();
		drone.GetTransform(params.Transform);
		IEntity newGrenade =  GetGame().SpawnEntityPrefab(Resource.Load(SAL_DroneComponent.Cast(drone.FindComponent(SAL_DroneComponent)).GetAmmoMagazine()), null, params);
		
		delete WeaponSlotComponent.Cast(drone.FindComponent(WeaponSlotComponent)).GetWeaponEntity();
		
		WeaponSlotComponent.Cast(drone.FindComponent(WeaponSlotComponent)).SetWeapon(newGrenade);
		delete item;
	}
	
	//Drone Hud Update Members
	//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//Inspired by CSI
	void MoveCompass(string widgetName)
	{
		//Inspired from CSI thanks patman :)
		ImageWidget compass = ImageWidget.Cast(m_wOverlayWidget.FindAnyWidget(widgetName));
		vector transform[4];
		GetGame().GetWorld().GetCurrentCamera(transform);
		float yaw = -Math3D.MatrixToAngles(transform)[0];
		compass.SetRotation(yaw);
	}
	
	//Inspired by CSI
	void SetBearing(string widgetName)
	{
		vector transform[4];
		GetGame().GetWorld().GetCurrentCamera(transform);
		float yaw = -Math3D.MatrixToAngles(transform)[0];
		int yawInt = -yaw;
			        
		if (yawInt < 0) 
		yawInt = 360 - Math.AbsFloat(yawInt);
		
		string bearingAdd = "";
				        
		if (yawInt >= 0 & yawInt < 10) 
		bearingAdd = "00";
		        
		if (yawInt >= 10 & yawInt < 100) 
			bearingAdd = "0";
		        
		string bearingStr = bearingAdd + (yawInt.ToString());
		TextWidget.Cast(m_wOverlayWidget.FindAnyWidget("Bearing")).SetText(bearingStr);
	}
	
	void SetAltitude(string widgetName)
	{
		int height = VehicleHelicopterSimulation.Cast(m_eConnectedDrone.FindComponent(VehicleHelicopterSimulation)).GetAltitudeAGL()*10;
				
		string heightString = "";
				
		if((height/10) < 10)
			heightString = "00" + (height/10).ToString() + "," + height.ToString().Get(height.ToString().Length() - 1);
				
		if((height/100) < 100)
			heightString = "0" + (height/10).ToString() + "," + height.ToString().Get(height.ToString().Length() - 1);
				
		if((height/100) < 1000)
			heightString = (height/10).ToString() + "," + height.ToString().Get(height.ToString().Length() - 1);
				
		TextWidget.Cast(m_wOverlayWidget.FindAnyWidget("Altitude")).SetText(heightString);
	}
	
	void SetSpeed(string widgetName)
	{
		int speed = m_eConnectedDrone.GetPhysics().GetVelocity().Length() * 3.6 * 10;
		string speedString = "";
		
		if((speed/10) < 10)
			speedString = "00" + (speed/10).ToString() + "," + speed.ToString().Get(speed.ToString().Length() - 1);
				
		if((speed/100) < 100)
			speedString = "0" + (speed/10).ToString() + "," + speed.ToString().Get(speed.ToString().Length() - 1);
				
		if((speed/100) < 1000)
			speedString = (speed/10).ToString() + "," + speed.ToString().Get(speed.ToString().Length() - 1);
				
		TextWidget.Cast(m_wOverlayWidget.FindAnyWidget("Speed")).SetText(speedString);
	}
	
	void SetRoll(string widgetName)
	{
		ImageWidget rollPipper = ImageWidget.Cast(m_wOverlayWidget.FindAnyWidget("Dots"));
		vector transform[4];
		GetGame().GetWorld().GetCurrentCamera(transform);
		float roll = -Math3D.MatrixToAngles(transform)[2];
		rollPipper.SetRotation(roll);
	}
}