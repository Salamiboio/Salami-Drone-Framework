//Made by Salami._.
class SAL_DistanceActionCommandStation : ScriptedUserAction
{
	IEntity m_ePlayer;
	IEntity m_eObject;
	SAL_DroneCommandStationComponent m_CommandStation;
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
		m_CommandStation = SAL_DroneCommandStationComponent.Cast(m_eObject.FindComponent(SAL_DroneCommandStationComponent));
	}
	
	override bool CanBePerformedScript(IEntity user)
	{
		return false;
	}
	
	override bool CanBeShownScript(IEntity user)
	{
		if (!SCR_PlayerController.GetLocalControlledEntity())
			return false;

		if (m_DroneManager.GetPlayersCommandStation(SCR_PlayerController.GetLocalPlayerId()) == m_eObject)
			return true;
		
		return false;;
	}
	
	override bool GetActionNameScript(out string outName)
	{
		float distance = vector.Distance(m_eObject.GetOrigin(), SCR_PlayerController.GetLocalControlledEntity().GetOrigin());
		int finalDistance = m_CommandStation.m_iCommandStationDistance - Math.Round(distance);
		outName = "Distance to Disconnect: " + finalDistance.ToString() + "m";
		return true;
	}
	
}