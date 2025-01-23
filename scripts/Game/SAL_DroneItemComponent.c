//Made by Salami._.
[EntityEditorProps(category: "GameScripted", description: "Stores Values to use", color: "")]
class SAL_DroneItemComponentClass: ScriptComponentClass
{
}

class SAL_DroneItemComponent : ScriptComponent
{	
	[Attribute("", desc: "Drone that's spawned when drone item is put on the ground.", params: "et")]
	protected ResourceName m_rDroneToSpawn;
	
	ResourceName GetDroneToSpawn()
	{
		return m_rDroneToSpawn;
	}
}