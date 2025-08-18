class SAL_GimbalCameraComponentClass: ScriptComponentClass
{
}

enum SAL_GimbalMode
{
	Roll,
	Pitch,
	Combined
}

class SAL_GimbalCameraComponent: ScriptComponent
{
	[Attribute("0", UIWidgets.ComboBox, "", enums: ParamEnumArray.FromEnum(SAL_GimbalMode))] int m_GimbalMode;
	protected IEntity m_CameraEntity;
	protected IEntity m_Parent;
	CameraManager m_CameraManager;
	SAL_DroneConnectionManager m_DroneConnectionManager;
	SAL_CameraZoomComponent m_CameraZoom;
	SCR_PlayerController m_PC;
	RplId m_DroneId;
	SAL_DroneControllerComponent m_DroneController;
	EntitySlotInfo m_CameraSlot;
	RplId m_Id;
	RplComponent m_RplComp;
	
	InputManager m_InputManager;

	float m_fPitchOffset = 0.0;
	
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.POSTFRAME | EntityEvent.INIT);
	}
	
	override void EOnInit(IEntity owner)
	{
		m_CameraEntity = owner;
		
		m_InputManager = GetGame().GetInputManager();
		m_CameraManager = GetGame().GetCameraManager();
		m_DroneConnectionManager = SAL_DroneConnectionManager.GetInstance();
		if (m_Parent)
		{
			SlotManagerComponent slotMan = SlotManagerComponent.Cast(m_Parent.FindComponent(SlotManagerComponent));
			m_CameraSlot = slotMan.GetSlotByName("CameraBody");
			m_CameraZoom = SAL_CameraZoomComponent.Cast(m_Parent.FindComponent(SAL_CameraZoomComponent));
			m_DroneController = SAL_DroneControllerComponent.Cast(m_Parent.FindComponent(SAL_DroneControllerComponent));
			m_Id = SCR_PlayerController.GetRplId(owner);
			if (!m_RplComp)
				m_RplComp = RplComponent.Cast(m_Parent.FindComponent(RplComponent));
		
		}
	}

	override void EOnPostFrame(IEntity owner, float timeSlice)
	{
		if (m_Id == -1)
			m_Id = RplComponent.Cast(owner.FindComponent(RplComponent)).Id();
		
		if (m_Parent == null)
		{
			m_Parent = owner.GetParent();
			if (m_Parent)
			{
				SlotManagerComponent slotMan = SlotManagerComponent.Cast(m_Parent.FindComponent(SlotManagerComponent));
				m_CameraSlot = slotMan.GetSlotByName("CameraBody");
				m_CameraZoom = SAL_CameraZoomComponent.Cast(m_Parent.FindComponent(SAL_CameraZoomComponent));
				m_DroneController = SAL_DroneControllerComponent.Cast(m_Parent.FindComponent(SAL_DroneControllerComponent));
				m_Id = SCR_PlayerController.GetRplId(owner);
				if (!m_RplComp)
					m_RplComp = RplComponent.Cast(m_Parent.FindComponent(RplComponent));
			}
			return;
		}
		
		if (System.IsConsoleApp())
			return;
			
		
		if (!m_CameraManager.CurrentCamera())
			return;
		
		if (!m_DroneController.m_bIsActive)
			return;
		
		if (m_DroneController.m_iOwner != SCR_PlayerController.GetLocalPlayerId() || !m_DroneConnectionManager.m_aActiveDrones.Contains(m_Id))
		{	
			float pitchCommand = m_InputManager.GetActionValue("CameraPitch");
			if (pitchCommand != 0)
			{
				if (m_CameraZoom)
				{
					float fovDifference = Math.Clamp((m_CameraManager.CurrentCamera().GetVerticalFOV() - m_CameraZoom.m_iMaxZoom) / (m_CameraZoom.m_iMinZoom - m_CameraZoom.m_iMaxZoom), 0.01, 1.0);
					m_fPitchOffset += pitchCommand * fovDifference;
				}
				else
					m_fPitchOffset += pitchCommand;
			
				m_fPitchOffset = Math.Clamp(m_fPitchOffset, -89.5, 35);	
				SCR_PlayerController.Cast(GetGame().GetPlayerController()).SetCameraYaw(m_fPitchOffset, m_Id);
			}
		}
		
		vector cameraAngles = m_CameraEntity.GetLocalYawPitchRoll();
		vector parentAngles = m_Parent.GetLocalYawPitchRoll();
		vector targetAngles;
		if (m_GimbalMode == SAL_GimbalMode.Roll)
		{
			targetAngles = Vector(
			cameraAngles[0],         // Yaw from Camera
			-cameraAngles[1],         // Stabalize Pitch
			-parentAngles[2]          // Stabalize Roll
			);
		} 
		else if (m_GimbalMode == SAL_GimbalMode.Pitch)
		{
			targetAngles = Vector(
			cameraAngles[0],         // Yaw from Camera
			-parentAngles[1] + m_fPitchOffset, // Stabalize Pitch
			-cameraAngles[2]          // Stabalize Roll
			);
		}
		else
		{
			targetAngles = Vector(
			cameraAngles[0],         // Yaw from Camera
			-parentAngles[1] + m_fPitchOffset, // Stabalize Pitch
			-parentAngles[2]          // Stabalize Roll
			);
		}
		
		if (targetAngles[0] != 0)
			targetAngles[0] = 0;
		
		if (targetAngles[1] < 270 && targetAngles[1] > 100)
			targetAngles[1] = 270;
	
	
		m_CameraEntity.SetYawPitchRoll(targetAngles);
		
		if (!m_CameraSlot)
		{
			SlotManagerComponent slotMan = SlotManagerComponent.Cast(m_Parent.FindComponent(SlotManagerComponent));
			m_CameraSlot = slotMan.GetSlotByName("CameraBody");
			return;
		}
		vector cameraTransform[4];
		if (owner.GetPrefabData().GetPrefabName() == "{DD1F5C844352D9A7}Prefabs/Drones/DJIBase/CameraEntity.et")
		{
			if (!m_CameraSlot.GetAttachedEntity())
				return;
			m_CameraSlot.GetAttachedEntity().GetTransform(cameraTransform);
			cameraTransform[3] = cameraTransform[3] + (cameraTransform[2] * 0.1);
			m_CameraEntity.SetTransform(cameraTransform);
			m_CameraEntity.Update();
			GenericEntity.Cast(m_CameraEntity).OnTransformReset();
		}
	}
	
	override bool RplSave(ScriptBitWriter writer)
	{
		writer.WriteFloat(m_fPitchOffset);
		
		return true;
	}
	
	override bool RplLoad(ScriptBitReader reader)
	{
		reader.ReadFloat(m_fPitchOffset);
		
		return true;
	}
}