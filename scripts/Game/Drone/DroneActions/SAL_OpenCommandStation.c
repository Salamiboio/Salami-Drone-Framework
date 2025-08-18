class SAL_OpenCommandStation : ScriptedUserAction
{
	//! Signal manager to pass signals into procedural animation
	private SignalsManagerComponent m_SignalsManager;
	private int m_iSignalIndex = -1;

	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		m_SignalsManager = SignalsManagerComponent.Cast(pOwnerEntity.FindComponent(SignalsManagerComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		return CanBePerformedScript(user);
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		if(!m_SignalsManager) // Do nothing if there is no signal manager
			return false;

		if (m_iSignalIndex < 0) // Check if signal index is valid
		{
			m_iSignalIndex = m_SignalsManager.AddOrFindSignal("CommandStationOpen");
			return false;
		}

		return true;
	}
	
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		float targetValue = Math.AbsFloat((m_SignalsManager.GetSignalValue(m_iSignalIndex)) - 1);

		m_SignalsManager.SetSignalValue(m_iSignalIndex, 1);
	}
	
	override bool GetActionNameScript(out string outName)
	{
		if((m_SignalsManager.GetSignalValue(m_iSignalIndex)) == 0)
		{
			outName = "Open";
		} else
		{
			outName = "Close";
		}
		return true;
	}
}