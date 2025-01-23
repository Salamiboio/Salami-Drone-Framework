modded class SCR_HelicopterControllerComponent
{
	override void OnEngineStart()
	{
		super.OnEngineStart();
		if(GetOwner().FindComponent(SAL_DroneComponent))
		{
			IEntity rotor1;
			IEntity rotor2;
			IEntity rotor3;
			IEntity rotor4;
			ref array<IEntity> rotors = {};
			if(SlotManagerComponent.Cast(GetOwner().FindComponent(SlotManagerComponent)).GetSlotByName("rotor1"))
			{
				rotor1 = SlotManagerComponent.Cast(GetOwner().FindComponent(SlotManagerComponent)).GetSlotByName("rotor1").GetAttachedEntity();
				rotors.Insert(rotor1);
			}
			if(SlotManagerComponent.Cast(GetOwner().FindComponent(SlotManagerComponent)).GetSlotByName("rotor2"))
			{
				rotor2 = SlotManagerComponent.Cast(GetOwner().FindComponent(SlotManagerComponent)).GetSlotByName("rotor2").GetAttachedEntity();
				rotors.Insert(rotor2);
			}
			if(SlotManagerComponent.Cast(GetOwner().FindComponent(SlotManagerComponent)).GetSlotByName("rotor3"))
			{
				rotor3 = SlotManagerComponent.Cast(GetOwner().FindComponent(SlotManagerComponent)).GetSlotByName("rotor3").GetAttachedEntity();
				rotors.Insert(rotor3);
			}
			if(SlotManagerComponent.Cast(GetOwner().FindComponent(SlotManagerComponent)).GetSlotByName("rotor4"))
			{
				rotor4 = SlotManagerComponent.Cast(GetOwner().FindComponent(SlotManagerComponent)).GetSlotByName("rotor4").GetAttachedEntity();
				rotors.Insert(rotor4);
			}
			GetGame().GetCallqueue().CallLater(RotateRotor, 1, true, rotors);
		}
	}
	
	void RotateRotor(array<IEntity> rotors)
	{
		if(!rotors)
			return;
		if(rotors.Count() == 0)
			return;
		vector oldAngles = rotors[0].GetAngles();
		oldAngles[1] = oldAngles[1] + 100;
		foreach(IEntity rotor: rotors)
		{
			rotor.SetAngles(oldAngles);
		}
	}
	
	override void OnEngineStop()
	{
		if(GetOwner().FindComponent(SAL_DroneComponent))
			GetGame().GetCallqueue().Remove(RotateRotor);
	}
}