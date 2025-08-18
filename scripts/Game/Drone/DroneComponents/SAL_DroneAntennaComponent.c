class SAL_DroneAntennaComponentClass: ScriptComponentClass
{
}

class SAL_DroneAntennaComponent: ScriptComponent
{
	[Attribute("5")] int m_iAntennaRangeMultiplier;
	[Attribute("25")] int m_iAntennaDistance;
	SAL_DroneConnectionManager m_DroneManager;
	SCR_GarbageSystem m_GarbageSystem;
	
	bool m_bIsConnected = false;
	int m_iOwner = -1;
	
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
		
		if (m_DroneManager.GetPlayersAntenna(SCR_PlayerController.GetLocalPlayerId()) != owner)
			return;
		
		if (vector.Distance(SCR_PlayerController.GetLocalControlledEntity().GetOrigin(), owner.GetOrigin()) > m_iAntennaDistance)
			SCR_PlayerController.Cast(GetGame().GetPlayerController()).DisconnectPlayerToAntenna(owner);
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