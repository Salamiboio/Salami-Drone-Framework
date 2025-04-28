//Made by Salami._.
[EntityEditorProps(category: "GameScripted", description: "Stores Values to use", color: "")]
class SAL_DroneComponentClass: ScriptComponentClass
{
}

[BaseContainerProps(), SCR_BaseContainerCustomTitleFields({"m_rHUDLayout"}, "%1")]
class SAL_DroneHUDConfig
{
	[Attribute("", desc: "Layout Name")]
	ResourceName m_rHUDLayout;
	
	[Attribute("false", desc: "Does the HUD have a moving compass?")]
	bool m_bMoveCompass;
	
	[Attribute("", desc: "Name of compass widget")]
	string m_sCompassWidget;
	
	[Attribute("false", desc: "Does the HUD have a bearing text to be updated?")]
	bool m_bBearing;
	
	[Attribute("", desc: "Name of bearing text widget")]
	string m_sBearingWidget;
	
	[Attribute("false", desc: "Does the HUD have a altitude text to be updated?")]
	bool m_bAltitude;
	
	[Attribute("", desc: "Name of Altitude widget")]
	string m_sAltitudeWidget;
	
	[Attribute("false", desc: "Does the HUD have a speed text to be updated?")]
	bool m_bSpeed;
	
	[Attribute("", desc: "Name of speed widget")]
	string m_sSpeedWidget;
	
	[Attribute("false", desc: "Does the HUD have a moving roll element?")]
	bool m_bMoveRoll;
	
	[Attribute("", desc: "Name of roll widget")]
	string m_sRollWidget;
	
	[Attribute("false", desc: "Does this have a zoom level marker?")]
	bool m_bZoomLevel;
	
	[Attribute("", desc: "Name of zoom widget")]
	string m_sZoomWidget;
	
	[Attribute("false", desc: "Does this have a record icon in the top left?")]
	bool m_bRecordSymbol;
	
	[Attribute("", desc: "Name of recording widget")]
	string m_sRecordWidget;
}

class SAL_DroneComponent : ScriptComponent
{	
	[Attribute("10000", desc: "Range of drone in meters.")]
	protected int m_iDroneRange;
	
	[Attribute("", desc: "Controller used to control the drone.", params: "et")]
	protected ResourceName m_rControllerPrefab;
	
	[Attribute("", desc: "Defines what item will spawn in the players inventory when the drone is picked up.", params: "et")]
	protected ResourceName m_rGroundItemPrefab;
	
	[Attribute("", desc: "What ammo is used to reload the drone from the player.", params: "et")]
	protected ResourceName m_rAmmo;
	
	[Attribute("", desc: "What ammo is used to reload the drone weapon.", params: "et")]
	protected ResourceName m_rAmmoMagazine;
	
	[Attribute("", desc: "Layout the drone will use for its hud")]
	protected ref SAL_DroneHUDConfig m_rHUDLayout;
	
	[Attribute("", desc: "Should the player be able to move the camera")]
	protected bool m_bCanMoveCamera;
	
	[Attribute("false", desc: "Can the drone pull grids?")]
	protected bool m_bCanTarget;
	
	[Attribute("1000", desc: "Max range the drone can pull a grid")]
	protected int m_iMaxRange;
	
	[Attribute("true", desc: "Does the drone use Line of Sight signals")]
	protected bool m_bLOS;
	
	[Attribute("", desc: "Max the camera can go up")]
	protected vector m_vMaxUp;
	
	[Attribute("", desc: "Max the camera can go down")]
	protected vector m_vMaxDown;
	
	protected float m_fFOVMax;
	protected CameraManager m_CameraManager;
	protected float m_fMinZoomFOV = 30;
	
	override void EOnInit(IEntity owner)
	{
		if(Replication.IsServer())
			GetGame().GetCallqueue().CallLater(SpawnController, 100, false, owner);
	}
	
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT);
		SetEventMask(owner, EntityEvent.CONTACT);
	}
	
	void SpawnController(IEntity entity)
	{
		SAL_DroneGamemodeComponent.GetInstance().SpawnDroneController(entity);
	}
	
	bool GetLOS()
	{
		return m_bLOS;
	}
	
	int GetMaxRange()
	{
		return m_iMaxRange;
	}
	
	bool GetCanTarget()
	{
		return m_bCanTarget;
	}
	
	ResourceName GetAmmo()
	{
		return m_rAmmo;
	}
	
	ResourceName GetAmmoMagazine()
	{
		return m_rAmmoMagazine;
	}
	
	SAL_DroneHUDConfig GetHud()
	{
		return m_rHUDLayout;
	}
	
	int GetDroneRange()
	{
		return m_iDroneRange;
	}
	
	ResourceName GetControllerPrefab()
	{
		return m_rControllerPrefab;
	}
	
	ResourceName GetGroundItem()
	{
		return m_rGroundItemPrefab;
	}
	
	bool GetCanMoveCamera()
	{
		return m_bCanMoveCamera;
	}
	
	vector GetMaxUp()
	{
		return m_vMaxUp;
	}
	
	vector GetMaxDown()
	{
		return m_vMaxDown;
	}
	
	override void EOnContact(IEntity owner, IEntity other, Contact contact)
	{
		if (SAL_DroneClientComponent.GetInstance().GetConnectedDrone() != owner)
			return;
		
		if (SAL_DroneClientComponent.GetInstance().m_bIsArmed)
		{
			Print("explod");
		}
	}
}