class SAL_DroneSoundComponentClass: SoundComponentClass
{
}

class SAL_DroneSoundComponent: SoundComponent
{
	float m_fAverageRotorRPM = 0.0;
    int m_iAverageRotorRPMIndex;
	
	SignalsManagerComponent m_SignalsManager;
	
	override void OnPostInit(IEntity owner)
	{	
		super.OnPostInit(owner);
		m_SignalsManager = SignalsManagerComponent.Cast(owner.FindComponent(SignalsManagerComponent));
		if (!m_SignalsManager)
			return;
			
		m_iAverageRotorRPMIndex = m_SignalsManager.AddOrFindSignal("AverageDroneRotorRPM");
	}
	
	override void UpdateSoundJob(IEntity owner, float timeSlice)
	{
		m_SignalsManager.SetSignalValue(m_iAverageRotorRPMIndex, m_fAverageRotorRPM);
	}
	
	bool IsEngineOn()
	{
		return IsPlaying();
	}
	
	void StartEngine()
	{
		SoundEvent("DRONE_ROTOR");
	}
	
	void ShutOffEngine()
	{
		TerminateAll();
	}
}