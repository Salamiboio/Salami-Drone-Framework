//Made by Salami._.
[EntityEditorProps(category: "GameScripted", description: "Stores Values to use", color: "")]
class SAL_DroneJammerComponentClass: ScriptComponentClass
{
}

class SAL_DroneJammerComponent : ScriptComponent
{	
	[Attribute("200", desc: "Range of jammer, AKA when it cuts off drone feed")]
	protected int m_iJammerRange;
	
	[Attribute("3", desc: "How far the range can be felt using a multiplier, AKA a range of 200m with a multiplier of 4 will have static start at 800m from it. CANNOT BE 1")]
	protected int m_iJammerMultiplier;
	
	[Attribute("", desc: "What Faction should this jammer not effect.")]
	protected string m_sFactionKey;
	
	[Attribute("false", desc: "Does that back spawn powered?")]
	protected bool m_bStartPowered;
	
	[Attribute("", desc: "Bag that spawns when destroyed")]
	protected ResourceName m_rDestroyedBag;
	
	override void EOnInit(IEntity owner)
	{
		GetGame().GetCallqueue().CallLater(UpdateJammerBags, 100, false, owner);
	}
	
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT);
	}
	
	void UpdateJammerBags(IEntity owner)
	{
		if(Replication.IsServer())
			if(owner.GetParent())
			{
				if(SAL_DroneGamemodeComponent.GetInstance())
				{
					if(m_sFactionKey == "")
						SAL_DroneGamemodeComponent.GetInstance().UpdateJammers(owner, SCR_FactionManager.SGetPlayerFaction(GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(owner.GetParent())).GetFactionKey(), m_bStartPowered);
					else
						SAL_DroneGamemodeComponent.GetInstance().UpdateJammers(owner, m_sFactionKey, m_bStartPowered);
				}
			}
			else
			{	
				SAL_DroneGamemodeComponent.GetInstance().UpdateJammers(owner, m_sFactionKey, m_bStartPowered);
			}
	}
	
	ResourceName GetDestroyedBag()
	{
		return m_rDestroyedBag;
	}
	
	string GetFactionKey()
	{
		return m_sFactionKey;
	}
	
	int GetJammerRange()
	{
		return m_iJammerRange;
	}
	
	int GetJammerMultiplier()
	{
		return m_iJammerMultiplier;
	}
}