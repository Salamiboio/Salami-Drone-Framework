class SAL_DroneCleanUpComponentClass: ScriptComponentClass
{
}

class SAL_DroneCleanUpComponent: ScriptComponent
{
	[Attribute("5")] float m_fMinutesTillCleanup;
	SAL_DroneConnectionManager m_DroneManager;
	bool m_bIsConnected = false;
	
	float m_fCurrentMinutes = m_fMinutesTillCleanup * 60;
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.FRAME);
		m_DroneManager = SAL_DroneConnectionManager.GetInstance();
	}
	
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		if (!Replication.IsServer())
			return;
		

		if (owner.GetParent() || m_bIsConnected)
		{
			if (GetGame().GetCallqueue().GetRemainingTime(DeleteObject) != -1)
				GetGame().GetCallqueue().Remove(DeleteObject);
			
			return;
		}
		
		if (GetGame().GetCallqueue().GetRemainingTime(DeleteObject) == -1)
			GetGame().GetCallqueue().CallLater(DeleteObject, m_fMinutesTillCleanup * 60000, false, owner);
	}
	
	void DeleteObject(IEntity owner)
	{
		SCR_EntityHelper.DeleteEntityAndChildren(owner);
	}
	
	override bool RplSave(ScriptBitWriter writer)
	{
		
		writer.WriteBool(m_bIsConnected);
		return true;
	}
	
	override bool RplLoad(ScriptBitReader reader)
	{
		
		reader.ReadBool(m_bIsConnected);
		return true;
	}
}