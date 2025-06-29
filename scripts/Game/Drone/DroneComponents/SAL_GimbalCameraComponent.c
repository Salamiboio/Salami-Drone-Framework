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
	
	InputManager m_InputManager;

	float m_fPitchOffset = 0.0;
	
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.FRAME | EntityEvent.INIT);
	}
	
	override void EOnInit(IEntity owner)
	{
		m_CameraEntity = owner;
		
		m_InputManager = GetGame().GetInputManager();
		m_CameraManager = GetGame().GetCameraManager();
		m_DroneConnectionManager = SAL_DroneConnectionManager.GetInstance();
		if (m_Parent)
			m_CameraZoom = SAL_CameraZoomComponent.Cast(m_Parent.FindComponent(SAL_CameraZoomComponent));
	}

	override void EOnFrame(IEntity owner, float timeSlice)
	{
		if (m_Parent == null)
		{
			m_Parent = owner.GetParent();
			if (m_Parent)
				m_CameraZoom = SAL_CameraZoomComponent.Cast(m_Parent.FindComponent(SAL_CameraZoomComponent));
			return;
		}
			
		
		if (!m_CameraManager.CurrentCamera())
			return;
		
		if (m_DroneConnectionManager.IsDronePlayers(m_Parent))
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
				SCR_PlayerController.Cast(GetGame().GetPlayerController()).SetCameraYaw(m_fPitchOffset, RplComponent.Cast(owner.FindComponent(RplComponent)).Id());
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
	
		// Smooth interpolation
		float smoothing = Math.Min(timeSlice * 60.0, 1.0);  // Clamp to avoid overshoot
	
		vector smoothedAngles;
		smoothedAngles[0] = targetAngles[0];  // Pitch
		smoothedAngles[1] = SCR_Math.LerpAngle(cameraAngles[1], targetAngles[1], smoothing);  // Yaw
		smoothedAngles[2] = SCR_Math.LerpAngle(cameraAngles[2], targetAngles[2], smoothing);  // Roll
		
		if (smoothedAngles[1] < 270 && smoothedAngles[1] > 100)
			smoothedAngles[1] = 270;
	
		m_CameraEntity.SetYawPitchRoll(smoothedAngles);
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