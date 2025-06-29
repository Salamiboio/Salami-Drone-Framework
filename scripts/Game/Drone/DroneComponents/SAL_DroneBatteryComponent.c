class SAL_DroneBatteryComponentClass: ScriptComponentClass
{
}

enum SAL_BatteryType
{
	FPV,
	DJI
}

class SAL_DroneBatteryComponent: ScriptComponent
{
	[Attribute("1800")] float m_fMaxBattery;
	[Attribute("0", UIWidgets.ComboBox, "Battery Type", "", ParamEnumArray.FromEnum(SAL_HUDType))] SAL_BatteryType m_eBatteryType;
	
	float m_fCurrentBatteryMax = m_fMaxBattery;
	float m_fCurrentBattery = m_fMaxBattery;
	
	SAL_DroneControllerComponent m_DroneControllernManager;
	SAL_DroneConnectionManager m_DroneManager;
	
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT | EntityEvent.FIXEDFRAME);
	}
	
	override void EOnInit(IEntity owner)
	{
		m_DroneControllernManager = SAL_DroneControllerComponent.Cast(owner.FindComponent(SAL_DroneControllerComponent));
		m_DroneManager = SAL_DroneConnectionManager.GetInstance();
	}
	
	float m_fBatteryCheck = 0;
	override void EOnFixedFrame(IEntity owner, float timeSlice)
	{
		if (!m_DroneManager.IsDronePlayers(owner))
			return;
		
		if (!m_DroneControllernManager.m_bIsArmed)
			return;
		
		if (m_fBatteryCheck < 1)
		{
			m_fBatteryCheck += timeSlice;
			return;
		}
			
		m_fBatteryCheck = 0;
		
		m_fCurrentBattery -= m_DroneControllernManager.m_iThrottle;
		
		if (m_fCurrentBattery <= 0)
		{
			SAL_DroneControllerComponent.Cast(owner.FindComponent(SAL_DroneControllerComponent)).ArmDrone();
			SCR_PlayerController.Cast(GetGame().GetPlayerController()).DisconnectDrone();
		}
	}
}