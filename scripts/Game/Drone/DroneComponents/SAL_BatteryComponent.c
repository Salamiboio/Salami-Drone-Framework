class SAL_BatteryComponentClass: ScriptComponentClass
{
}

class SAL_BatteryComponent: ScriptComponent
{
	[Attribute("1800")] int m_fBatteryStorage;
	[Attribute("0", UIWidgets.ComboBox, "Battery Type", "", ParamEnumArray.FromEnum(SAL_HUDType))] SAL_BatteryType m_eBatteryType;
}