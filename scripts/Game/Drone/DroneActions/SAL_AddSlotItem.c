//Made by Salami._.
class SAL_AddSlotItem : SCR_InventoryAction
{
	[Attribute(desc: "Item in slot", params: "et")] ResourceName m_sItem;
	[Attribute(desc: "Slot name in Slot Manager Comp")] string m_sSlotName;
	EntitySlotInfo m_EntitySlot;
	IEntity m_eCase;
	IEntity m_ePlayer;
	SAL_DroneConnectionManager m_DroneManager;
	string m_outName;
	
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
	{
		m_ePlayer = pUserEntity;
		m_eCase = pOwnerEntity;
		
		SCR_InventoryStorageManagerComponent storageComp = SCR_InventoryStorageManagerComponent.Cast(pUserEntity.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!storageComp)
			return;
		
		if (!m_EntitySlot.GetAttachedEntity())
		{
			ref array<IEntity> items = {};
			storageComp.GetItems(items);
			IEntity itemSelected;
			
			foreach (IEntity item: items)
			{
				if (item.GetPrefabData().GetPrefabName() == m_sItem)
				{
					itemSelected = item;
					break;
				}
			}
	
			SCR_PlayerController.Cast(GetGame().GetPlayerController()).AddOrRemoveItem(SCR_PlayerController.GetRplId(m_eCase), SCR_PlayerController.GetRplId(itemSelected), m_sSlotName);
		}
		else
			SCR_PlayerController.Cast(GetGame().GetPlayerController()).AddOrRemoveItem(SCR_PlayerController.GetRplId(m_eCase), SCR_PlayerController.GetRplId(m_EntitySlot.GetAttachedEntity()), m_sSlotName);
	
	}
	
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		m_eCase = pOwnerEntity;
		m_EntitySlot = SlotManagerComponent.Cast(m_eCase.FindComponent(SlotManagerComponent)).GetSlotByName(m_sSlotName);
		m_DroneManager = SAL_DroneConnectionManager.GetInstance();
	}
	
	override bool CanBePerformedScript(IEntity user)
	{
		if (!SCR_PlayerController.GetLocalControlledEntity())
			return false;
		
		SCR_InventoryStorageManagerComponent storageComp = SCR_InventoryStorageManagerComponent.Cast(user.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!storageComp)
			return false;
		
		if (m_EntitySlot.GetAttachedEntity())
			if (!storageComp.CanInsertItem(m_EntitySlot.GetAttachedEntity()))
				return false;
			
		if (m_EntitySlot.GetAttachedEntity())
			m_outName = "#Grab_Item";
		else
			m_outName = "#Place_Item";
		
		return true;
	}
	
	override bool CanBeShownScript(IEntity user)
	{
		if (!SCR_PlayerController.GetLocalControlledEntity())
			return false;
		
		if (m_EntitySlot.GetAttachedEntity())
			return true;
		
		SCR_InventoryStorageManagerComponent storageComp = SCR_InventoryStorageManagerComponent.Cast(user.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!storageComp)
			return false;
		
		ref array<IEntity> items = {};
		storageComp.GetItems(items);
		
		foreach (IEntity item: items)
		{
			if (item.GetPrefabData().GetPrefabName() == m_sItem)
				return true;
		}
		
		return false;
	}
	
	override bool GetActionNameScript(out string outName)
	{
		outName = m_outName;
		return true;
	}
	
}