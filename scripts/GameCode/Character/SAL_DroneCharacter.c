//Made by Salami._.
modded class SCR_ChimeraCharacter: ChimeraCharacter
{
	
	//Based on GME's SetVisibility
	[RplProp(onRplName: "UpdateVisibility")]
	protected bool m_bInput = true;
	
	void SetVisibility(bool input)
	{		
		m_bInput = input;
		Replication.BumpMe();
		this.UpdateVisibility();
	}
	
	
	void UpdateVisibility()
	{
		if (m_bInput)
			this.SetFlags(EntityFlags.VISIBLE|EntityFlags.TRACEABLE, m_bInput);
		else
			this.ClearFlags(EntityFlags.VISIBLE|EntityFlags.TRACEABLE);
	}
}