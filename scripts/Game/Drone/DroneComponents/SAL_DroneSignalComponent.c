class SAL_DroneSignalComponentClass: ScriptComponentClass
{
}

class SAL_DroneSignalComponent: ScriptComponent
{
	[Attribute("2000.0")] float m_fDroneRange;
	
	float m_fSignalStrength = 255.0;
	int m_iGroundBuffer = 0;
	float m_fRSSI = 0.0;
	float m_fLQ = 0;
	
	SAL_DroneConnectionManager m_DroneManager;
	
	float m_fObstructionPenaltySmoothed = 0.0;
	
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.FIXEDFRAME | EntityEvent.INIT);
	}
	
	override void EOnInit(IEntity owner)
	{
		m_DroneManager = SAL_DroneConnectionManager.GetInstance();
	}
	
	float m_fTimer = 0;
	override void EOnFixedFrame(IEntity owner, float timeSlice)
	{
		if (!m_DroneManager.IsDronePlayers(owner))
			return;
		
		if (m_fTimer < 0.01)
		{
			m_fTimer += timeSlice;
			return;
		}
		else
			m_fTimer = 0;
		
		vector playerOrigin = SCR_PlayerController.GetLocalControlledEntity().GetOrigin();
		vector droneOrigin = owner.GetOrigin();
		
		int distanceFrom = vector.Distance(playerOrigin, droneOrigin);
		float distancePercentage = distanceFrom/m_fDroneRange;
		float signalLoss = Math.Pow(distancePercentage, 2.2);  // tuned exponent
		
		vector droneSignal = droneOrigin;
		droneSignal[1] = droneSignal[1] + 0.5;
		droneSignal[2] = droneSignal[2] - 0.5;
		
		vector playerSignal = playerOrigin;
		playerSignal[1] = playerSignal[1] + 2;
		
		ref array<int> playerIDs = {};
		GetGame().GetPlayerManager().GetAllPlayers(playerIDs);
		
		ref array<IEntity> allEntities = {};
		foreach(int playerID: playerIDs)
		{
			allEntities.Insert(GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID));
		}
		ref array<AIAgent> allAgents = {};
		if(GetGame().GetAIWorld())
		{
			GetGame().GetAIWorld().GetAIAgents(allAgents);
			foreach(AIAgent agent: allAgents)
			{
				allEntities.Insert(agent.GetControlledEntity());
			}
		}
		allEntities.Insert(owner);
		float losFraction = RayCastLine(droneSignal, playerSignal, EPhysicsLayerDefs.Terrain, allEntities);
		float obstructionBase = 1.0 - losFraction;
		float distWeight = Math.Pow(distancePercentage, 1.50);
		float proximityWeight = Math.Lerp(0.50, 1.60, losFraction);          
		float targetObstructionPenalty = Math.Clamp(Math.Pow(obstructionBase, 0.50) * distWeight * proximityWeight, 0.0, 1.0);
		
		m_fObstructionPenaltySmoothed = Math.Lerp(m_fObstructionPenaltySmoothed, targetObstructionPenalty, timeSlice * 2.5);
		
		float finalSignal = Math.Clamp(signalLoss + m_fObstructionPenaltySmoothed, 0.0, 1.0);
		
		
		ref array<IEntity> jammers = {};
		//Find Jammers In Range
		foreach (RplId jammerId: m_DroneManager.m_aJammers)
		{	
			if (!Replication.FindItem(jammerId))
				continue;
			IEntity jammer = RplComponent.Cast(Replication.FindItem(jammerId)).GetEntity();
			if (!jammer)
				continue;
			SAL_DroneJammerComponent jammerComp = SAL_DroneJammerComponent.Cast(jammer.FindComponent(SAL_DroneJammerComponent));
			float maxRange = jammerComp.m_fJammingRange;
			
			if (vector.Distance(droneOrigin, jammer.GetOrigin()) > maxRange)
				continue;
			
			jammers.Insert(jammer);
		}
		
		IEntity closestJammer;
		//Find closest Jammer
		if (jammers.GetSizeOf() > 0)
		{
			foreach (IEntity jammer: jammers)
			{
				if (!closestJammer)
				{
					closestJammer = jammer;
					continue;
				}
					
				
				if (vector.Distance(droneOrigin, jammer.GetOrigin()) < vector.Distance(droneOrigin, closestJammer.GetOrigin()))
					closestJammer = jammer;
			}
		
			if (closestJammer)
			{
				SAL_DroneJammerComponent jammerComp = SAL_DroneJammerComponent.Cast(closestJammer.FindComponent(SAL_DroneJammerComponent));
				float jammerDist = vector.Distance(droneOrigin, closestJammer.GetOrigin());
				float jammerNorm = Math.Clamp(jammerDist / jammerComp.m_fJammingRange, 0.0, 1.0);
				float jammerLoss = Math.Pow(1 - jammerNorm, 3.0);
		
				// Combine using probability union: L_eff = 1 - (1 - Lp)(1 - Lj)
				finalSignal = finalSignal + jammerLoss - (finalSignal * jammerLoss); // L_eff
			}
		}
		m_fSignalStrength = Math.Round(finalSignal * 255);
		m_fRSSI = Math.Clamp(signalLoss + m_fObstructionPenaltySmoothed, 0.0, 1.0);
		m_fLQ = finalSignal;
	}
	
	
	
	//Written by an RHS Dev
	private static float RayCastLine(vector start, vector end, EPhysicsLayerDefs layer, array<IEntity> entityArray = null)
	{
		autoptr TraceParam p = new TraceParam;
		ChimeraCharacter ce = ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
		
		p.Exclude = ce;
		p.Flags = TraceFlags.DEFAULT | TraceFlags.ANY_CONTACT;
		p.LayerMask = layer;
		p.Start = start;
		p.End = end;
		if(entityArray)
			p.ExcludeArray = entityArray;
		
		return TraceMoveHit(p);
	}
	
	//Written by an RHS Dev
	private static float TraceMoveHit(TraceParam p)
	{
		float hit = GetGame().GetWorld().TraceMove(p, null);
		
		if (!p.TraceEnt)
			return 1;

		
		return hit;
	}
}