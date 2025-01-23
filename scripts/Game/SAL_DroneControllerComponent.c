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
