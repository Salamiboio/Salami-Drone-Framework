class SAL_DroneRangeFinderComponentClass: ScriptComponentClass
{
}

class SAL_DroneRangeFinderComponent: ScriptComponent
{
	[Attribute("1000.0")] float m_fMaxRange;
	int m_iGridX;
	int m_iGridY;
	float m_fHitDistance;
	
	bool m_bHasFired = false;
	
	InputManager m_InputManager;
	SAL_DroneConnectionManager m_DroneManager;
	
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT | EntityEvent.FRAME);
	}
	
	override void EOnInit(IEntity owner)
	{
		m_InputManager = GetGame().GetInputManager();
		m_DroneManager = SAL_DroneConnectionManager.GetInstance();
	}
	
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		if (!m_DroneManager.IsDronePlayers(owner))
			return;
		
		if (m_InputManager.GetActionValue("DroneLaser") > 0)
			GetGrid();
	}
	
	//Inspired from the RHS RangeFinder
	void GetGrid()
	{
		if (!m_bHasFired)
			m_bHasFired = true;
		
		vector transform[4];
		GetGame().GetWorld().GetCurrentCamera(transform);
		vector targetPos = transform[3] + (transform[2].Normalized() * m_fMaxRange);
		ref array<IEntity> excludeEntities = {};
		excludeEntities.Insert(GetOwner());
		m_fHitDistance = RayCastLine(transform[3], targetPos, EPhysicsLayerDefs.Projectile, excludeEntities) * m_fMaxRange;
		vector hitPos
		if (m_fHitDistance > 0)
			hitPos = transform[3] + (transform[2].Normalized() * m_fHitDistance);
		int gridX;
		int gridY;
		SCR_MapEntity.GetGridPos(hitPos, gridX, gridY, 0, 4);
		m_iGridX = gridX;
		m_iGridY = gridY;
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
			return 0;

		
		return hit;
	}
}