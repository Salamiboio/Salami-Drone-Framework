class SAL_DroneNetworkPacket 
{
	protected RplId m_Drone;
	protected vector m_vDroneTransform1;
	protected vector m_vDroneTransform2;
	protected vector m_vDroneTransform3;
	protected vector m_vDroneTransform4;
	protected float m_aRotorRPMs1;
	protected float m_aRotorRPMs2;
	protected float m_aRotorRPMs3;
	protected float m_aRotorRPMs4;
	protected RplId m_aRotors1;
	protected RplId m_aRotors2;
	protected RplId m_aRotors3;
	protected RplId m_aRotors4;
	protected float m_fTimeSlice;
	protected bool m_bIsTrigger;
	protected string m_sExplosion;
	protected bool m_bIsArmed;
	protected float m_fbatteryLevel;
	protected float m_fHealth;
	
	//------------------------------------------------------------------------------------------------
	// SETTERS
	//------------------------------------------------------------------------------------------------
	
	void SetDrone(RplId input)
	{
		m_Drone = input;
	}
	
	void SetTransform(vector input[4])
	{
		m_vDroneTransform1 = input[0];
		m_vDroneTransform2 = input[1];
		m_vDroneTransform3 = input[2];
		m_vDroneTransform4 = input[3];
	}
	
	void SetRotorRPMs(float input[4])
	{
		m_aRotorRPMs1 = input[0];
		m_aRotorRPMs2 = input[1];
		m_aRotorRPMs3 = input[2];
		m_aRotorRPMs4 = input[3];
	}
	
	void SetRotors(RplId input[4])
	{
		m_aRotors1 = input[0];
		m_aRotors2 = input[1];
		m_aRotors3 = input[2];
		m_aRotors4 = input[3];
	}
	
	void SetTimeSlice(float input)
	{
		m_fTimeSlice = input;
	}
	
	void SetIsTriggerd(bool input)
	{
		m_bIsTrigger = input;
	}
	
	void SetExplosion(string input)
	{
		m_sExplosion = input;
	}
	
	void SetIsArmed(bool input)
	{
		m_bIsArmed = input;
	}
	
	void SetBatteryLevel(float input)
	{
		m_fbatteryLevel = input;
	}
	
	void SetHealth(float input)
	{
		m_fHealth = input;
	}
	
	//------------------------------------------------------------------------------------------------
	// GETTERS
	//------------------------------------------------------------------------------------------------
	
	RplId GetDrone()
	{
		return m_Drone;
	}
	
	void GetTransform(out vector transform[4])
	{
		transform = {m_vDroneTransform1, m_vDroneTransform2, m_vDroneTransform3, m_vDroneTransform4};
	}
	
	void GetRotorRPMs(out float RPMs[4])
	{
		RPMs = {m_aRotorRPMs1, m_aRotorRPMs2, m_aRotorRPMs3, m_aRotorRPMs4};
	}
	
	void GetRotors(out RplId rotors[4])
	{
		rotors = {m_aRotors1, m_aRotors2, m_aRotors3, m_aRotors4};
	}
	
	float GetTimeSlice()
	{
		return m_fTimeSlice;
	}
	
	bool GetIsTriggered()
	{
		return m_bIsTrigger;
	}
	
	string GetExplosion()
	{
		return m_sExplosion;
	}
	
	bool GetIsArmed()
	{
		return m_bIsArmed;
	}
	
	float GetBatteryLevel()
	{
		return m_fbatteryLevel;
	}
	
	float GetHealth()
	{
		return m_fHealth;
	}
	
	//------------------------------------------------------------------------------------------------
	// REPLICATION STUFF
	//------------------------------------------------------------------------------------------------
	bool RplSave(ScriptBitWriter writer)
	{
		writer.WriteRplId(m_Drone);
		
		writer.WriteVector(m_vDroneTransform1);
		writer.WriteVector(m_vDroneTransform2);
		writer.WriteVector(m_vDroneTransform3);
		writer.WriteVector(m_vDroneTransform4);
		
		writer.WriteFloat(m_aRotorRPMs1);
		writer.WriteFloat(m_aRotorRPMs2);
		writer.WriteFloat(m_aRotorRPMs3);
		writer.WriteFloat(m_aRotorRPMs4);
		
		writer.WriteRplId(m_aRotors1);
		writer.WriteRplId(m_aRotors2);
		writer.WriteRplId(m_aRotors3);
		writer.WriteRplId(m_aRotors4);
		
		writer.WriteFloat(m_fTimeSlice);
		
		writer.WriteBool(m_bIsTrigger);
		
		writer.WriteString(m_sExplosion);
		
		writer.WriteBool(m_bIsArmed);
		
		writer.WriteFloat(m_fbatteryLevel);
		
		writer.WriteFloat(m_fHealth);
			
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	bool RplLoad(ScriptBitReader reader)
	{
		reader.ReadRplId(m_Drone);
		
		reader.ReadVector(m_vDroneTransform1);
		reader.ReadVector(m_vDroneTransform2);
		reader.ReadVector(m_vDroneTransform3);
		reader.ReadVector(m_vDroneTransform4);
		
		reader.ReadFloat(m_aRotorRPMs1);
		reader.ReadFloat(m_aRotorRPMs2);
		reader.ReadFloat(m_aRotorRPMs3);
		reader.ReadFloat(m_aRotorRPMs4);
		
		reader.ReadRplId(m_aRotors1);
		reader.ReadRplId(m_aRotors2);
		reader.ReadRplId(m_aRotors3);
		reader.ReadRplId(m_aRotors4);
		
		reader.ReadFloat(m_fTimeSlice);
		
		reader.ReadBool(m_bIsTrigger);
		
		reader.ReadString(m_sExplosion);
		
		reader.ReadBool(m_bIsArmed);
		
		reader.ReadFloat(m_fbatteryLevel);
		
		reader.ReadFloat(m_fHealth);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	static bool Extract(SAL_DroneNetworkPacket instance, ScriptCtx ctx, SSnapSerializerBase snapshot)
	{	
		snapshot.SerializeInt(instance.m_Drone);
		
		snapshot.SerializeVector(instance.m_vDroneTransform1);
		snapshot.SerializeVector(instance.m_vDroneTransform2);
		snapshot.SerializeVector(instance.m_vDroneTransform3);
		snapshot.SerializeVector(instance.m_vDroneTransform4);
		
		snapshot.SerializeFloat(instance.m_aRotorRPMs1);
		snapshot.SerializeFloat(instance.m_aRotorRPMs2);
		snapshot.SerializeFloat(instance.m_aRotorRPMs3);
		snapshot.SerializeFloat(instance.m_aRotorRPMs4);
		
		snapshot.SerializeInt(instance.m_aRotors1);
		snapshot.SerializeInt(instance.m_aRotors2);
		snapshot.SerializeInt(instance.m_aRotors3);
		snapshot.SerializeInt(instance.m_aRotors4);
		
		snapshot.SerializeFloat(instance.m_fTimeSlice);
		
		snapshot.SerializeBool(instance.m_bIsTrigger);
		
		snapshot.SerializeString(instance.m_sExplosion);
		
		snapshot.SerializeBool(instance.m_bIsArmed);
		
		snapshot.SerializeFloat(instance.m_fbatteryLevel);
		
		snapshot.SerializeFloat(instance.m_fHealth);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	static bool Inject(SSnapSerializerBase snapshot, ScriptCtx ctx, SAL_DroneNetworkPacket instance)
	{
		snapshot.SerializeInt(instance.m_Drone);
		
		snapshot.SerializeVector(instance.m_vDroneTransform1);
		snapshot.SerializeVector(instance.m_vDroneTransform2);
		snapshot.SerializeVector(instance.m_vDroneTransform3);
		snapshot.SerializeVector(instance.m_vDroneTransform4);
		
		snapshot.SerializeFloat(instance.m_aRotorRPMs1);
		snapshot.SerializeFloat(instance.m_aRotorRPMs2);
		snapshot.SerializeFloat(instance.m_aRotorRPMs3);
		snapshot.SerializeFloat(instance.m_aRotorRPMs4);
		
		snapshot.SerializeInt(instance.m_aRotors1);
		snapshot.SerializeInt(instance.m_aRotors2);
		snapshot.SerializeInt(instance.m_aRotors3);
		snapshot.SerializeInt(instance.m_aRotors4);
		
		snapshot.SerializeFloat(instance.m_fTimeSlice);
		
		snapshot.SerializeBool(instance.m_bIsTrigger);
		
		snapshot.SerializeString(instance.m_sExplosion);
		
		snapshot.SerializeBool(instance.m_bIsArmed);
		
		snapshot.SerializeFloat(instance.m_fbatteryLevel);
		
		snapshot.SerializeFloat(instance.m_fHealth);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	static void Encode(SSnapSerializerBase snapshot, ScriptCtx ctx, ScriptBitSerializer packet)
	{
		snapshot.EncodeInt(packet);
		
		snapshot.EncodeVector(packet);
		snapshot.EncodeVector(packet);
		snapshot.EncodeVector(packet);
		snapshot.EncodeVector(packet);
		
		snapshot.EncodeFloat(packet);
		snapshot.EncodeFloat(packet);
		snapshot.EncodeFloat(packet);
		snapshot.EncodeFloat(packet);
		
		snapshot.EncodeInt(packet);
		snapshot.EncodeInt(packet);
		snapshot.EncodeInt(packet);
		snapshot.EncodeInt(packet);
		
		snapshot.EncodeFloat(packet);
		
		snapshot.EncodeBool(packet);
		
		snapshot.EncodeString(packet);
		
		snapshot.EncodeBool(packet);
		
		snapshot.EncodeFloat(packet);
		
		snapshot.EncodeFloat(packet);
	}
	
	//------------------------------------------------------------------------------------------------
	static bool Decode(ScriptBitSerializer packet, ScriptCtx ctx, SSnapSerializerBase snapshot)
	{
		snapshot.DecodeInt(packet);
		
		snapshot.DecodeVector(packet);
		snapshot.DecodeVector(packet);
		snapshot.DecodeVector(packet);
		snapshot.DecodeVector(packet);
		
		snapshot.DecodeFloat(packet);
		snapshot.DecodeFloat(packet);
		snapshot.DecodeFloat(packet);
		snapshot.DecodeFloat(packet);
		
		snapshot.DecodeInt(packet);
		snapshot.DecodeInt(packet);
		snapshot.DecodeInt(packet);
		snapshot.DecodeInt(packet);
		
		snapshot.DecodeFloat(packet);
		
		snapshot.DecodeBool(packet);
		
		snapshot.DecodeString(packet);
		
		snapshot.DecodeBool(packet);
		
		snapshot.DecodeFloat(packet);
		
		snapshot.DecodeFloat(packet);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	static bool SnapCompare(SSnapSerializerBase lhs, SSnapSerializerBase rhs, ScriptCtx ctx)
	{
		bool bool1 = lhs.CompareSnapshots(rhs, 4)
			&& lhs.CompareSnapshots(rhs, 12)
			&& lhs.CompareSnapshots(rhs, 12)
			&& lhs.CompareSnapshots(rhs, 12)
			&& lhs.CompareSnapshots(rhs, 12)
			&& lhs.CompareSnapshots(rhs, 4)
			&& lhs.CompareSnapshots(rhs, 4)
			&& lhs.CompareSnapshots(rhs, 4)
			&& lhs.CompareSnapshots(rhs, 4)
			&& lhs.CompareSnapshots(rhs, 4)
			&& lhs.CompareSnapshots(rhs, 4);
		
		bool bool2 = lhs.CompareSnapshots(rhs, 4)
			&& lhs.CompareSnapshots(rhs, 4)
			&& lhs.CompareSnapshots(rhs, 4)
			&& lhs.CompareSnapshots(rhs, 4)
			&& lhs.CompareStringSnapshots(rhs)
			&& lhs.CompareSnapshots(rhs, 4)
			&& lhs.CompareSnapshots(rhs, 4)
			&& lhs.CompareSnapshots(rhs, 4);;
		
		return bool1 && bool2;
			
	}
	
	//------------------------------------------------------------------------------------------------
	static bool PropCompare(SAL_DroneNetworkPacket instance, SSnapSerializerBase snapshot, ScriptCtx ctx)
	{
		bool bool1 = snapshot.CompareInt(instance.m_Drone)
			&& snapshot.CompareVector(instance.m_vDroneTransform1)
			&& snapshot.CompareVector(instance.m_vDroneTransform2)
			&& snapshot.CompareVector(instance.m_vDroneTransform3)
			&& snapshot.CompareVector(instance.m_vDroneTransform4)
			&& snapshot.CompareFloat(instance.m_aRotorRPMs1)
			&& snapshot.CompareFloat(instance.m_aRotorRPMs2)
			&& snapshot.CompareFloat(instance.m_aRotorRPMs3)
			&& snapshot.CompareFloat(instance.m_aRotorRPMs4)
			&& snapshot.CompareInt(instance.m_aRotors1);
		
		bool bool2 = snapshot.CompareInt(instance.m_aRotors2)
			&& snapshot.CompareInt(instance.m_aRotors3)
			&& snapshot.CompareInt(instance.m_aRotors4)
			&& snapshot.CompareInt(instance.m_fTimeSlice)
			&& snapshot.CompareBool(instance.m_bIsTrigger)
			&& snapshot.CompareString(instance.m_sExplosion)
			&& snapshot.CompareBool(instance.m_bIsArmed)
			&& snapshot.CompareFloat(instance.m_fbatteryLevel)
			&& snapshot.CompareFloat(instance.m_fHealth);
		return bool1 && bool2;
			
	}
}