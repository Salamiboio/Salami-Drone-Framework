//Made by Salami._.
class SAL_DistanceActionAntenna : ScriptedUserAction
{
	IEntity m_ePlayer;
	IEntity m_eObject;
	SAL_DroneAntennaComponent m_AntennaComp;
	SAL_DroneConnectionManager m_DroneManager;
	string m_outName;
	
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
	{
		m_ePlayer = pUserEntity;
		m_eObject = pOwnerEntity;
	}
	
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		m_eObject = pOwnerEntity;
		m_DroneManager = SAL_DroneConnectionManager.GetInstance();
		m_AntennaComp = SAL_DroneAntennaComponent.Cast(m_eObject.FindComponent(SAL_DroneAntennaComponent));
	}
	
	override bool CanBePerformedScript(IEntity user)
	{
		return false;
	}
	
	override bool CanBeShownScript(IEntity user)
	{
		if (!SCR_PlayerController.GetLocalControlledEntity())
			return false;

		if (m_DroneManager.GetPlayersAntenna(SCR_PlayerController.GetLocalPlayerId()) == m_eObject)
			return true;
		
		return false;;
	}
	
	override bool GetActionNameScript(out string outName)
	{
		float distance = vector.Distance(m_eObject.GetOrigin(), SCR_PlayerController.GetLocalControlledEntity().GetOrigin());
		int finalDistance = m_AntennaComp.m_iAntennaDistance - Math.Round(distance);
		outName = "Distance to Disconnect: " + finalDistance.ToString() + "m";
		return true;
	}
	
}