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
	
	bool m_BGrenadeDropped = false;
	
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT | EntityEvent.FRAME);
	}
	
	override void EOnInit(IEntity owner)
	{
		m_InputManager = GetGame().GetInputManager();
		m_DroneManager = SAL_DroneConnectionManager.GetInstance();
	}
	
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		if (!m_DroneManager.IsDronePlayers(owner))
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