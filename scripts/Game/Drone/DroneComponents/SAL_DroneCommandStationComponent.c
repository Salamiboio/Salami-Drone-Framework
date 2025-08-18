class SAL_DroneCommandStationComponentClass: ScriptComponentClass
{
}

class SAL_DroneCommandStationComponent: ScriptComponent
{
	[Attribute("25")] int m_iCommandStationDistance;
	SAL_DroneConnectionManager m_DroneManager;
	SCR_GarbageSystem m_GarbageSystem;
	
	bool m_bIsConnected = false;
	int m_iOwner;
	
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.FIXEDFRAME);
		m_DroneManager = SAL_DroneConnectionManager.GetInstance();
		m_GarbageSystem = SCR_GarbageSystem.GetByEntityWorld(owner);
	}
	
	override void EOnFixedFrame(IEntity owner, float timeSlice)
	{
		if (m_GarbageSystem)
		{
			if (m_GarbageSystem.IsInserted(owner))
				m_GarbageSystem.Withdraw(owner);
		}
		else
			m_GarbageSystem = SCR_GarbageSystem.GetByEntityWorld(owner);
		
		if (m_DroneManager.GetPlayersCommandStation(SCR_PlayerController.GetLocalPlayerId()) != owner)
			return;
		
		if (vector.Distance(SCR_PlayerController.GetLocalControlledEntity().GetOrigin(), owner.GetOrigin()) > m_iCommandStationDistance)
			SCR_PlayerController.Cast(GetGame().GetPlayerController()).DisconnectPlayerToCommand(owner);
	}
	
	override bool RplSave(ScriptBitWriter writer)
	{
		
		writer.WriteBool(m_bIsConnected);
		writer.WriteInt(m_iOwner);
		return true;
	}
	
	override bool RplLoad(ScriptBitReader reader)
	{
		
		reader.ReadBool(m_bIsConnected);
		reader.ReadInt(m_iOwner);
		return true;
	}
}