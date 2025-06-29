[EntityEditorProps(category: "GameScripted", description: "Acro-style Drone Controller", color: "")]
class SAL_DroneControllerComponentClass: ScriptComponentClass {}

[BaseContainerProps()]
class SAL_DroneStablizerProfile
{
	[Attribute("8.0")] float m_fMaxHorizontalSpeed;   // m/s
	[Attribute("6.0")] float m_fBrakeStrength;        // N per m/s too fast
	[Attribute("20.0")] float m_fMaxTiltDegrees;	
	[Attribute("0.5")] float m_fVelocityDampeningStrength;	
	[Attribute("0.5")] float m_fAngleCorrectionRate;	
}

class SAL_DroneControllerComponent: ScriptComponent
{
	SAL_DroneConnectionManager m_DroneManager;
	InputManager m_InputManager;
	SAL_DroneSignalComponent m_SignalComponent;
	SAL_DroneBatteryComponent m_BatteryComponent;
	
	[Attribute("35000")] int m_iMaxThrustRPM;
	[Attribute("0")] bool m_bStabilized;
	[Attribute("1")] float m_fPitchSensitivity;
	[Attribute("1")] float m_fYawSensitivity;
	[Attribute("1")] float m_fRollSensitivity;
	[Attribute("")] ref SAL_DroneStablizerProfile m_DroneStablizerProfile;
	
	RplId m_DroneId;

	float m_aRotorRPM[4] = { 0.0, 0.0, 0.0, 0.0 };
	IEntity m_aRotors[4];
	RplId m_aRotorsRplId[4];

	float m_iThrottle = 0.0;
	float m_iPitch = 0.0;
	float m_iRoll = 0.0;
	float m_iYaw = 0.0;
	bool m_bIsArmed = false;

	float m_fHoverForce = 0.0;
	float m_fMaxAdditionalThrust = 0.0;
	float m_fSyncTimer = 0.0;
	
	bool m_bIsTriggered = false;
	
	int rotorSpinDir[4] = { 1, -1, -1, 1 };

	void SpinRotors(float timeSlice)
	{
//		if (m_aRotors[].GetSizeOf() < 4)
//			InitializeRotors();

		for (int i = 0; i < 4; i++)
		{
			IEntity rotor = m_aRotors[i];
			if (!rotor) continue;

			float degPerSecond = m_aRotorRPM[i] * 6.0;
			float rotationAmount = rotorSpinDir[i] * degPerSecond * timeSlice;
			vector oldAngles = rotor.GetLocalAngles();
			oldAngles[1] = oldAngles[1] + rotationAmount;
			rotor.SetAngles(oldAngles);
		}
	}
	
	float m_fRPMSmoothFactor = 5.0; // Smoothing speed, tweak this
	
	void UpdateSimulatedRPMs(float timeSlice)
	{
		float baseRPM = m_iThrottle * m_iMaxThrustRPM;
	
		float targetRPMs[4];
		targetRPMs[0] = baseRPM + (-m_iPitch + m_iRoll + m_iYaw) * 1000;
		targetRPMs[1] = baseRPM + (-m_iPitch - m_iRoll - m_iYaw) * 1000;
		targetRPMs[2] = baseRPM + ( m_iPitch + m_iRoll - m_iYaw) * 1000;
		targetRPMs[3] = baseRPM + ( m_iPitch - m_iRoll + m_iYaw) * 1000;
	
		for (int i = 0; i < 4; i++)
		{
			targetRPMs[i] = Math.Clamp(targetRPMs[i], 0.0, m_iMaxThrustRPM);
			m_aRotorRPM[i] = Math.Lerp(m_aRotorRPM[i], targetRPMs[i], timeSlice * m_fRPMSmoothFactor);
		}
	}
	
	void DampenCrossInput(float pitchIn, float rollIn, out float pitchOut, out float rollOut, float cutoffRatio = 0.5)
	{
		float pitchAbs = Math.AbsFloat(pitchIn);
		float rollAbs = Math.AbsFloat(rollIn);
	
		if (pitchAbs > rollAbs * (1.0 + cutoffRatio))
		{
			// Pitch is dominant — suppress roll
			rollOut = 0.0;
			pitchOut = pitchIn;
		}
		else if (rollAbs > pitchAbs * (1.0 + cutoffRatio))
		{
			// Roll is dominant — suppress pitch
			pitchOut = 0.0;
			rollOut = rollIn;
		}
		else
		{
			// Allow both, but scale proportionally to reduce unintended influence
			float blendFactor = 0.5;
			pitchOut = pitchIn * blendFactor;
			rollOut = rollIn * blendFactor;
		}
	}

	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		SetEventMask(owner, EntityEvent.INIT | EntityEvent.SIMULATE | EntityEvent.FRAME);
	}

	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		if (!owner.GetPhysics())
			return;
		m_DroneManager = SAL_DroneConnectionManager.GetInstance();
		m_InputManager = GetGame().GetInputManager();
		m_SignalComponent = SAL_DroneSignalComponent.Cast(owner.FindComponent(SAL_DroneSignalComponent));
		m_BatteryComponent = SAL_DroneBatteryComponent.Cast(owner.FindComponent(SAL_DroneBatteryComponent));
		
		owner.GetPhysics().SetActive(true);
		float mass = owner.GetPhysics().GetMass();
		m_fHoverForce = mass * 9.81;
		m_fMaxAdditionalThrust = m_fHoverForce * 1.5;
		m_DroneId = RplComponent.Cast(owner.FindComponent(RplComponent)).Id();
		InitializeRotors();
	}

	void InitializeRotors()
	{
		SlotManagerComponent sm = SlotManagerComponent.Cast(GetOwner().FindComponent(SlotManagerComponent));
		if (!sm) return;

		m_aRotors[0] = sm.GetSlotByName("Rotor1").GetAttachedEntity();
		m_aRotors[1] = sm.GetSlotByName("Rotor2").GetAttachedEntity();
		m_aRotors[2] = sm.GetSlotByName("Rotor3").GetAttachedEntity();
		m_aRotors[3] = sm.GetSlotByName("Rotor4").GetAttachedEntity();
		
		m_aRotorsRplId[0] = SCR_PlayerController.GetRplId(sm.GetSlotByName("Rotor1").GetAttachedEntity());
		m_aRotorsRplId[1] = SCR_PlayerController.GetRplId(sm.GetSlotByName("Rotor2").GetAttachedEntity());
		m_aRotorsRplId[2] = SCR_PlayerController.GetRplId(sm.GetSlotByName("Rotor2").GetAttachedEntity());
		m_aRotorsRplId[3] = SCR_PlayerController.GetRplId(sm.GetSlotByName("Rotor3").GetAttachedEntity());
	}
	
	void ArmDrone()
	{
		if (m_DroneManager.GetPlayersDrone(SCR_PlayerController.GetLocalPlayerId()) == null)
			return;
		
		m_bIsArmed = !m_bIsArmed;
		
		if (m_bIsArmed)
		{
			SAL_DroneSoundComponent.Cast(m_DroneManager.GetPlayersDrone(SCR_PlayerController.GetLocalPlayerId()).FindComponent(SAL_DroneSoundComponent)).StartEngine();
			SAL_DroneNetworkPacket packet = new SAL_DroneNetworkPacket;
			packet.SetDrone(SCR_PlayerController.GetRplId(GetOwner()));
			packet.SetIsArmed(m_bIsArmed);	
			SCR_PlayerController.Cast(GetGame().GetPlayerController()).DisarmDrone(packet);
		}
		else
		{
			SAL_DroneSoundComponent.Cast(m_DroneManager.GetPlayersDrone(SCR_PlayerController.GetLocalPlayerId()).FindComponent(SAL_DroneSoundComponent)).ShutOffEngine();
			SAL_DroneNetworkPacket packet = new SAL_DroneNetworkPacket;
			packet.SetDrone(SCR_PlayerController.GetRplId(GetOwner()));
			packet.SetIsArmed(m_bIsArmed);	
			SCR_PlayerController.Cast(GetGame().GetPlayerController()).DisarmDrone(packet);
		}
			
	}

	override void EOnFrame(IEntity owner, float timeSlice)
	{
		super.EOnFrame(owner, timeSlice);
		//Why havent we found it
		if (!m_DroneManager)
			m_DroneManager = SAL_DroneConnectionManager.GetInstance();
		
		//Ohh hell nah
		if (!m_DroneManager)
			return;
		
		//Disables the physics for anyone not controlling the drone
		if (!m_DroneManager.IsDronePlayers(owner))
		{
			if (m_bIsArmed && owner.GetPhysics().IsActive())
				owner.GetPhysics().SetActive(0);
			else if (!m_bIsArmed && !owner.GetPhysics().IsActive())
				owner.GetPhysics().SetActive(1);
		}
			
			
		//Enables the physics for the person controlling the drone
		if (m_DroneManager.IsDronePlayers(owner) && !owner.GetPhysics().IsActive())
			owner.GetPhysics().SetActive(1);
		
		//Rest of the code non drone controllers don't need to worry about
		if (!m_DroneManager.IsDronePlayers(owner))
			return;
		
		if (!SCR_PlayerController.GetLocalControlledEntity())
				return;
			
		BaseWeaponManagerComponent weaponMan = BaseWeaponManagerComponent.Cast(ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity()).FindComponent(BaseWeaponManagerComponent));
		if (!weaponMan)
			return;

		if (!weaponMan.GetCurrentWeapon())
			return;
		
		if (weaponMan.GetCurrentWeapon().GetOwner().GetPrefabData().GetPrefabName() != "{E2434ED1318D8476}Prefabs/Characters/Items/DroneController.et")
			return;
		
		if (m_InputManager.GetActionValue("ArmDrone") > 0)
				ArmDrone();

		//If the drone controller arms it this is what starts listening for inputs
		if (m_bIsArmed)
		{
			float lq = 1 - m_SignalComponent.m_fLQ;
			
			if (lq <= 0)
			{
				ArmDrone();
				SCR_PlayerController.Cast(GetGame().GetPlayerController()).DisconnectDrone();
			}
				
			if (lq < 0.2)
				return;
			
			if (lq < 0.4)
			{
				float dropChance = Math.Clamp(1.0 - ((lq - 0.4) / (0.2 - 0.4)), 0.0, 1.0);
				if (Math.RandomFloat01() < dropChance)
					return;
			}
			
			float rawInput = Math.Clamp(m_InputManager.GetActionValue("DroneUp"), -1.0, 1.0);
			m_iThrottle = (rawInput + 1.0) * 0.5;

			m_iPitch = -m_InputManager.GetActionValue("DroneForward") * m_fPitchSensitivity;
			m_iRoll  = -m_InputManager.GetActionValue("DroneLeft") * m_fYawSensitivity;
			m_iYaw    = m_InputManager.GetActionValue("DroneYaw") * m_fRollSensitivity;

			if (Math.AbsFloat(m_iYaw) < 0.1)
				m_iYaw = 0;
			
			float averageRPM = (m_aRotorRPM[0] + m_aRotorRPM[1] + m_aRotorRPM[2] + m_aRotorRPM[3]) / 4;
			
			SAL_DroneSoundComponent soundComp = SAL_DroneSoundComponent.Cast(owner.FindComponent(SAL_DroneSoundComponent));
			soundComp.m_fAverageRotorRPM = averageRPM;
		}
	}
	
	void SendPacket(IEntity owner, float timeSlice)
	{
			
		m_fSyncTimer += timeSlice;
		if (m_fSyncTimer > 0.03)
		{
			m_fSyncTimer = 0;
			vector transform[4];
			owner.GetTransform(transform);
					
			SAL_DroneNetworkPacket packet = new SAL_DroneNetworkPacket;
			packet.SetDrone(SCR_PlayerController.GetRplId(owner));
			packet.SetRotors(m_aRotorsRplId);
			packet.SetRotorRPMs(m_aRotorRPM);
			packet.SetTransform(transform);
			packet.SetTimeSlice(timeSlice);
			packet.SetIsTriggerd(m_bIsTriggered);
			packet.SetIsArmed(m_bIsArmed);
			packet.SetBatteryLevel(m_BatteryComponent.m_fCurrentBattery);
			if (m_bIsTriggered)
			{
				packet.SetExplosion(SAL_DroneExplosionComponent.Cast(owner.FindComponent(SAL_DroneExplosionComponent)).m_sExplosionEffect);
				SCR_PlayerController.Cast(GetGame().GetPlayerController()).ExplodeDrone(packet);
			}
			else
				SCR_PlayerController.Cast(GetGame().GetPlayerController()).SendTransformToServer(packet);
		}
	}
	
	bool IsOnGround(IEntity owner)
	{
		vector origin = owner.GetOrigin();
		vector end = origin + Vector(0, -1, 0) * 1000.0; // cast 1000 meters down
			
		TraceParam trace = new TraceParam();
		trace.Start = origin;
		trace.End = end;
		trace.Exclude = owner; // prevent hitting self
		trace.Flags = TraceFlags.WORLD;
			
		int height = Math.Round(GetGame().GetWorld().TraceMove(trace, null) * 10000);
		return height <= 0;
	}

	override void EOnSimulate(IEntity owner, float timeSlice)
	{
		super.EOnSimulate(owner, timeSlice);
		Physics physics = owner.GetPhysics();
		if (!physics) return;
		
		if (!m_DroneManager) 
			m_DroneManager = SAL_DroneConnectionManager.GetInstance();
		
		//Server checks rq
		if (RplSession.Mode() == RplMode.Dedicated)
		{
			//Is the drone disarmed and not connected to a player, if so takes over the network replication
			if (!m_bIsArmed && m_DroneManager.GetConnectedDrones().Find(RplComponent.Cast(owner.FindComponent(RplComponent)).Id()))
				if (!IsOnGround(owner))
					SendPacket(owner, timeSlice);
		}
		
		//Same as above just checks to see if the person running this is the drones controller
		if (!m_DroneManager || !m_DroneManager.IsDronePlayers(owner))
			return;

		//Variables needed for apply force and torque
		vector thrustForce;
		vector inputTorque;
		vector localAngVel;
		
		// Needed for when the drone is not armed and in the air so everyone can still track where its at
		if (!m_bIsArmed)
		{
			if (!IsOnGround(owner))
				SendPacket(owner, timeSlice);
			return;
		}

		//If the drone is stabalized duh
		if (m_bStabilized)
		{
			//Get them angles
			vector up = owner.GetTransformAxis(1);
			vector right = owner.GetTransformAxis(0);
			vector forward = owner.GetTransformAxis(2);
			vector worldUp = vector.Up;
			
			DampenCrossInput(m_iPitch, m_iRoll, m_iPitch, m_iRoll, 0.4); // 0.2 = tighter stick discipline
		
			float thrustOffset = (m_iThrottle - 0.5) * 2.0;
		
			//God bless vibe coding sometimes
			//Compensation for tilt: divide by cosine of tilt angle
			float verticalLiftEfficiency = Math.Max(vector.Dot(up, worldUp), 0.1);
			float hoverThrust = (m_fHoverForce * 1.02) / verticalLiftEfficiency;
		
			//Add pilot throttle input to go above/below hover
			float userThrust = thrustOffset * m_fMaxAdditionalThrust;
			float totalThrust = hoverThrust + userThrust;
		
			//Cap to max safe thrust
			float maxThrust = m_fHoverForce + m_fMaxAdditionalThrust * 1.5;
			totalThrust = Math.Clamp(totalThrust, 0, maxThrust);
		
			thrustForce = up * totalThrust * timeSlice;
		
			//Only apply vertical velocity damping when throttle is near hover (user not actively climbing/descending)
			if (Math.AbsFloat(m_iThrottle) < 0.05)
			{
				float verticalVelocity = vector.Dot(physics.GetVelocity(), worldUp);
				thrustForce -= worldUp * verticalVelocity * 2.0 * timeSlice;
			}
		
			//Angular velocity
			vector worldAngVel = physics.GetAngularVelocity();
			localAngVel[0] = vector.Dot(worldAngVel, right);
			localAngVel[1] = vector.Dot(worldAngVel, up);
			localAngVel[2] = vector.Dot(worldAngVel, forward);
		
			bool isRollNeutral = Math.AbsFloat(m_iRoll) < 0.1;
			bool isPitchNeutral = Math.AbsFloat(m_iPitch) < 0.1;
		
			//Constants
			float maxTiltAngleDeg = m_DroneStablizerProfile.m_fMaxTiltDegrees; // 15–20 deg max tilt
			
			//Compute desired tilt angles based on stick
			float targetPitchDeg = -m_iPitch * maxTiltAngleDeg;  // forward/back
			float targetRollDeg  = -m_iRoll  * maxTiltAngleDeg;  // left/right
			
			//Measure current drone tilt from level (90 = level)
			float pitchDot = vector.Dot(forward, worldUp);
			float rollDot  = vector.Dot(right, worldUp);
			
			float currentPitchDeg = Math.RAD2DEG * Math.Acos(Math.Clamp(pitchDot, -1.0, 1.0)) - 90.0;
			float currentRollDeg  = Math.RAD2DEG * Math.Acos(Math.Clamp(rollDot,  -1.0, 1.0)) - 90.0;
			
			//Compute angle error (desired - actual)
			float pitchError = targetPitchDeg - currentPitchDeg;
			float rollError  = targetRollDeg  - currentRollDeg;
			
			//Proportional gain
			float angleCorrectionStrength = m_DroneStablizerProfile.m_fAngleCorrectionRate; // tune: smaller = slower, bigger = twitchy
			
			inputTorque[0] = Math.Clamp(pitchError * angleCorrectionStrength, -0.5, 0.5); // X = pitch
			inputTorque[2] = Math.Clamp(rollError  * -angleCorrectionStrength, -0.5, 0.5); // Z = roll
			inputTorque[1] = m_iYaw;
		
			//Counter drift
			vector velocityWorld = physics.GetVelocity();
			vector localVel;
			localVel[0] = vector.Dot(velocityWorld, forward);
			localVel[1] = vector.Dot(velocityWorld, up);
			localVel[2] = -vector.Dot(velocityWorld, right);
		
			float velocityDampStrength = m_DroneStablizerProfile.m_fVelocityDampeningStrength;
			if (isPitchNeutral)
				inputTorque[0] = inputTorque[0] - Math.Clamp(localVel[0] * velocityDampStrength, -0.5, 0.5);
			if (isRollNeutral)
				inputTorque[2] = inputTorque[2] - Math.Clamp(localVel[2] * velocityDampStrength, -0.5, 0.5);
			
			// --- horizontal (XZ) speed limiter ---------------------------------
			vector horizVel = velocityWorld;
			horizVel[1] = 0;                                   // ignore vertical
			
			float horizSpeed = horizVel.Length();
			if (horizSpeed > m_DroneStablizerProfile.m_fMaxHorizontalSpeed)
			{
			    // brake force opposite to travel direction
			    vector brakeDir = -horizVel.Normalized();
			    float excess    = horizSpeed - m_DroneStablizerProfile.m_fMaxHorizontalSpeed;
			    vector brakeForce = brakeDir * (excess * m_DroneStablizerProfile.m_fBrakeStrength);
			
			    // add to thrustForce so it is applied every frame
			    thrustForce += brakeForce;
			}
		}
		else
		{
			//Get upward direction of drone body
			vector thrustDir = owner.GetTransformAxis(1);
			
			//How far throttle is from neutral (0.5)
			float thrustOffset = (m_iThrottle - 0.5) * 2.0;
			
			//Optionally add hover margin to compensate for simulation inefficiencies
			float hoverMargin = 1.9; // 5% extra force to overcome rounding, drag, etc.
			
			//Calculate thrust with margin
			float totalThrust = (m_fHoverForce * hoverMargin) + (thrustOffset * m_fMaxAdditionalThrust * 4);
			
			//Cap total thrust to a safety ceiling
			float maxThrust = m_fHoverForce + m_fMaxAdditionalThrust * 1.5;
			totalThrust = Math.Clamp(totalThrust, 0, maxThrust);
			
			//Apply thrust in the drone's up direction
			thrustForce = thrustDir * totalThrust * timeSlice;
				
			vector worldAngVel = physics.GetAngularVelocity();
			localAngVel[0] = vector.Dot(worldAngVel, owner.GetTransformAxis(0));
			localAngVel[1] = vector.Dot(worldAngVel, owner.GetTransformAxis(1));
			localAngVel[2] = vector.Dot(worldAngVel, owner.GetTransformAxis(2));
			
			inputTorque[0] = -m_iPitch;
			inputTorque[1] = m_iYaw;
			inputTorque[2] = m_iRoll;
		}

		//Helps stabalize the drones tilt
		vector dampingTorque = -localAngVel * 4.0;
		vector controlTorque = (inputTorque * 6.0) + dampingTorque;

		//Summarize that torque
		vector worldTorque =
			(owner.GetTransformAxis(0) * controlTorque[0]) +
			(owner.GetTransformAxis(1) * controlTorque[1]) +
			(owner.GetTransformAxis(2) * controlTorque[2]);

		//Get the rotors spinning and the drone in the sky
		UpdateSimulatedRPMs(timeSlice);
		SpinRotors(timeSlice);
		physics.ApplyImpulse(thrustForce);
		physics.ApplyTorque(worldTorque);

		//Sends data to other players every 30ms
		SendPacket(owner, timeSlice);
	}
}
