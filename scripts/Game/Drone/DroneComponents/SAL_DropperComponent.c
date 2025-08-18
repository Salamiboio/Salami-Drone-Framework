class SAL_DropperComponentClass: ScriptComponentClass
{
}

class SAL_DropperComponent: ScriptComponent
{
	[Attribute("", UIWidgets.ResourceNamePicker, "Grenade Prefab to Drop", "et")] ResourceName m_DropperGrenade;
	[Attribute("", UIWidgets.ResourceNamePicker, "Grenade Prefab to Drop", "et")] ResourceName m_DropperGrenadeReload;
	[Attribute("", UIWidgets.ResourceNamePicker, "Grenade Prefab to Drop", "et")] ResourceName m_DropperGrenadePlaceholder;
	
	InputManager m_InputManager;
	SAL_DroneConnectionManager m_DroneManager;
	SAL_DroneControllerComponent m_DroneController;
	RplId m_Id;
	
	bool m_BGrenadeDropped = false;
	
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT | EntityEvent.FRAME);
	}
	
	override void EOnInit(IEntity owner)
	{
		m_InputManager = GetGame().GetInputManager();
		m_DroneManager = SAL_DroneConnectionManager.GetInstance();
		m_DroneController = SAL_DroneControllerComponent.Cast(owner.FindComponent(SAL_DroneControllerComponent));
		m_Id = SCR_PlayerController.GetRplId(owner);
	}
	
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		if (m_Id == -1)
			m_Id = RplComponent.Cast(owner.FindComponent(RplComponent)).Id();
		
		if (m_DroneController.m_iOwner != SCR_PlayerController.GetLocalPlayerId()  || !m_DroneManager.m_aActiveDrones.Contains(m_Id))
			return;
		
		if (!m_BGrenadeDropped && m_InputManager.GetActionValue("DropGrenadeDrone") == 1)
		{
			m_BGrenadeDropped = true;
			SCR_PlayerController.Cast(GetGame().GetPlayerController()).DropGrenade(owner);
		}
	}
	
	override bool RplSave(ScriptBitWriter writer)
	{
		writer.WriteBool(m_BGrenadeDropped);
		return true;
	}
	
	override bool RplLoad(ScriptBitReader reader)
    {
		reader.ReadBool(m_BGrenadeDropped);
		return true;
	}
}