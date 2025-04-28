modded class SCR_NameTagData
{
	override protected void InitData(SCR_NameTagConfig config)
	{
		// configurables
		if (m_NTDisplay)
		{
			SCR_NameTagRulesetBase ruleset = config.m_aVisibilityRuleset;
			
			m_fDeadEntitiesCleanup = ruleset.m_fDeadEntitiesCleanup;
			
			if (ruleset.m_fTagFadeTime == 0)
				m_fTagFadeSpeed = 0;
			else
				m_fTagFadeSpeed = 1/ruleset.m_fTagFadeTime; // convert multiplier to seconds
		}
		
		UpdateEntityType();
		
		if (ChimeraCharacter.Cast(m_Entity))
		{				
			// Vehicle enter/leave event
			SCR_CompartmentAccessComponent accessComp = SCR_CompartmentAccessComponent.Cast( m_Entity.FindComponent(SCR_CompartmentAccessComponent) );
			if (accessComp)
			{
				accessComp.GetOnCompartmentEntered().Insert(OnVehicleEntered);
				accessComp.GetOnCompartmentLeft().Insert(OnVehicleLeft);
			}
		}

		if (m_Entity)
		{
			if (m_Entity.GetAnimation())
			{
				m_iSpineBone = m_Entity.GetAnimation().GetBoneIndex(SPINE_BONE);
				m_iHeadBone = m_Entity.GetAnimation().GetBoneIndex(HEAD_BONE);
			}
		}
		
		GetName(m_sName, m_aNameParams);
	}
}