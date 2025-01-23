modded class SCR_PlayerController
{
	override protected void UpdateLocalPlayerController()
	{
		super.UpdateLocalPlayerController();
		
		m_bIsLocalPlayerController = this == GetGame().GetPlayerController();
		if (!m_bIsLocalPlayerController)
			return;

		s_pLocalPlayerController = this;
		InputManager inputManager = GetGame().GetInputManager();
		if (!inputManager)
			return;
		GetGame().GetInputManager().AddActionListener("EnterDrone", EActionTrigger.PRESSED, EnterDrone);
        GetGame().GetInputManager().AddActionListener("DisconnectDrone", EActionTrigger.PRESSED, DisconnectDroneAction);
	}
	
	protected void EnterDrone(float value, EActionTrigger reason)
	{
		Print("Entering Drone");
		SAL_DroneClientComponent.GetInstance().SwitchToDrone();
	}
	
	protected void DisconnectDroneAction(float value, EActionTrigger reason)
	{
		RplId droneID = RplId.Invalid();
		if(SAL_DroneClientComponent.GetInstance().GetConnectedDrone())
			droneID = RplComponent.Cast(SAL_DroneClientComponent.GetInstance().GetConnectedDrone().FindComponent(RplComponent)).Id();
		if(droneID == RplId.Invalid())
			return;
		SAL_DroneClientComponent.GetInstance().ExitDrone();
		SAL_DroneClientComponent.GetInstance().SetConnectedDrone(droneID, false);
	}
}