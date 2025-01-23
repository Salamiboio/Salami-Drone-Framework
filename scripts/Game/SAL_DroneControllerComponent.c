//Made by Salami._.
[EntityEditorProps(category: "GameScripted/Gadgets", description: "Drone Controller gadget", color: "0 0 255 255")]
class SAL_DroneControllerComponentClass : SCR_GadgetComponentClass
{
}

class SAL_DroneControllerComponent : SCR_GadgetComponent
{
	protected IEntity m_eController;

	//------------------------------------------------------------------------------------------------
	override void ModeSwitch(EGadgetMode mode, IEntity charOwner)
	{
		super.ModeSwitch(mode, charOwner); 
	}
	
	//------------------------------------------------------------------------------------------------
	override void ModeClear(EGadgetMode mode)
	{
		super.ModeClear(mode);
	}
	
	override void OnModeChanged(EGadgetMode mode, IEntity charOwner)
	{
		if(mode == EGadgetMode.IN_HAND)
		{
			GetGame().GetInputManager().ActivateContext("DroneContext", 99999);
            GetGame().GetInputManager().AddActionListener("EnterDrone", EActionTrigger.PRESSED, EnterDrone);
            GetGame().GetInputManager().AddActionListener("DisconnectDrone", EActionTrigger.PRESSED, DisconnectDroneAction);
		}
		else
		{
			GetGame().GetInputManager().ActivateContext("DroneContext");
			GetGame().GetInputManager().RemoveActionListener("EnterDrone", EActionTrigger.PRESSED, EnterDrone);
            GetGame().GetInputManager().RemoveActionListener("DisconnectDrone", EActionTrigger.PRESSED, DisconnectDroneAction);
		}
	}
	
	protected void EnterDrone(float value, EActionTrigger reason)
	{
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
	
	//------------------------------------------------------------------------------------------------
	override EGadgetType GetType()
	{
		return EGadgetType.DETONATOR;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool IsVisibleEquipped()
	{
		return false;
	}
}
