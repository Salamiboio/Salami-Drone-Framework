class SAL_DroneConnectContainer
{
	protected RplId m_DroneConnection1 = RplId.Invalid();
	protected RplId m_DroneConnection2 = RplId.Invalid();
	protected RplId m_DroneConnection3 = RplId.Invalid();
	protected RplId m_DroneCommandStation = RplId.Invalid();
	protected RplId m_DroneAntenna = RplId.Invalid();
	
	//=========================================================================================================================
	void SetDroneConnection(RplId input, int index)
	{
		switch(index)
		{
			case 0: {m_DroneConnection1 = input; break;};
			case 1: {m_DroneConnection2 = input; break;};
			case 2: {m_DroneConnection3 = input; break;};
			default: {break;};
		}
	}
	
	void SetDroneCommandStation(RplId input)
	{
		m_DroneCommandStation = input;
	}
	
	void SetDroneAntenna(RplId input)
	{
		m_DroneAntenna = input;
	}
	
	//=========================================================================================================================
	RplId GetDroneConnection(int index)
	{
		switch(index)
		{
			case 0: {return m_DroneConnection1; break;};
			case 1: {return m_DroneConnection2; break;};
			case 2: {return m_DroneConnection3; break;};
			default: {return RplId.Invalid(); break;};
		}
		return RplId.Invalid();
	}
	
	RplId GetDroneCommandStation()
	{
		return m_DroneCommandStation;
	}
	
	RplId GetDroneAntenna()
	{
		return m_DroneAntenna;
	}
	
	//=========================================================================================================================
	static bool Extract(SAL_DroneConnectContainer instance, ScriptCtx ctx, SSnapSerializerBase snapshot)
	{	
		snapshot.SerializeInt(instance.m_DroneConnection1);
		snapshot.SerializeInt(instance.m_DroneConnection2);
		snapshot.SerializeInt(instance.m_DroneConnection3);
		snapshot.SerializeInt(instance.m_DroneCommandStation);
		snapshot.SerializeInt(instance.m_DroneAntenna);
		
		return true;
	}
	
	static bool Inject(SSnapSerializerBase snapshot, ScriptCtx ctx, SAL_DroneConnectContainer instance)
	{
		snapshot.SerializeInt(instance.m_DroneConnection1);
		snapshot.SerializeInt(instance.m_DroneConnection2);
		snapshot.SerializeInt(instance.m_DroneConnection3);
		snapshot.SerializeInt(instance.m_DroneCommandStation);
		snapshot.SerializeInt(instance.m_DroneAntenna);
		
		return true;
	}
	
	static void Encode(SSnapSerializerBase snapshot, ScriptCtx ctx, ScriptBitSerializer packet)
	{
		snapshot.EncodeInt(packet);
		snapshot.EncodeInt(packet);
		snapshot.EncodeInt(packet);
		snapshot.EncodeInt(packet);
		snapshot.EncodeInt(packet);
	}
	
	static bool Decode(ScriptBitSerializer packet, ScriptCtx ctx, SSnapSerializerBase snapshot)
	{
		snapshot.DecodeInt(packet);
		snapshot.DecodeInt(packet);
		snapshot.DecodeInt(packet);
		snapshot.DecodeInt(packet);
		snapshot.DecodeInt(packet);
		
		return true;
	}
	
	static bool SnapCompare(SSnapSerializerBase lhs, SSnapSerializerBase rhs, ScriptCtx ctx)
	{
		return lhs.CompareSnapshots(rhs, 4) 
		&& lhs.CompareSnapshots(rhs, 4) 
		&& lhs.CompareSnapshots(rhs, 4) 
		&& lhs.CompareSnapshots(rhs, 4) 
		&& lhs.CompareSnapshots(rhs, 4);
	}
	
	static bool PropCompare(SAL_DroneConnectContainer instance, SSnapSerializerBase snapshot, ScriptCtx ctx)
	{
		return snapshot.CompareInt(instance.m_DroneConnection1)
		&& snapshot.CompareInt(instance.m_DroneConnection2)
		&& snapshot.CompareInt(instance.m_DroneConnection3)
		&& snapshot.CompareInt(instance.m_DroneCommandStation)
		&& snapshot.CompareInt(instance.m_DroneAntenna);
	}
}