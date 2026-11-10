// Fill out your copyright notice in the Description page of Project Settings.

#include "InventoryListContainer.h"
#include "InventoryItemDefinition.h"
#include "InventorySlotData.h"
#include "Processors/InventoryProcessor_Stackable.h"

/////////////////////////
// FInventoryItemEntry //
/////////////////////////

FInventoryItemEntry::FInventoryItemEntry(UObject* OuterObject, UInventoryItemDefinition* InItemDefinition,
	const FItemSlotFilter& InItemSlotFilter)
{
	ItemDefinition = InItemDefinition;
	if (ItemDefinition)
	{
		if (const auto CDO = ItemDefinition->GetSlotDataDefinitionCDO())
		{
			// 修复：实例型物品需要有效 Outer 才能创建其 UObject。
			const auto SlotData = CDO->GetDefaultSlotData(InItemDefinition, OuterObject);
			ItemSlotData = SlotData;
		}
	}
	ItemSlotFilter = InItemSlotFilter;
}

void FInventoryItemEntry::CopyFrom(const FInventoryItemEntry& Other, const bool bCopyFilter)
{
	ItemDefinition = Other.ItemDefinition;
	ItemSlotData = Other.ItemSlotData;

	if (bCopyFilter)
	{
		ItemSlotFilter = Other.ItemSlotFilter;
	}
}

void FInventoryItemEntry::SetItemSlotFilter(const FItemSlotFilter& Filter)
{
	ItemSlotFilter = Filter;
}

bool FInventoryItemEntry::IsItemAllowed(const UInventoryItemDefinition* ItemDef) const
{
	return ItemSlotFilter.IsItemAllowed(ItemDef);
}

bool FInventoryItemEntry::IsSlotEmpty() const
{
	return ItemDefinition == nullptr && ItemSlotData == FInstancedStruct();
}

void FInventoryItemEntry::EmptySlot(const bool& bCallEmptyLogic, const bool& bClearFilter)
{
	if (bClearFilter)
	{
		ItemSlotFilter = FItemSlotFilter();
	}
	
	// 修复：先从原槽移交数据，再调用清理钩子，防止重入或转移后的实例被强制销毁。
	FInventoryItemEntry Released;
	Released.ItemDefinition = ItemDefinition;
	Released.ItemSlotFilter = ItemSlotFilter;
	Released.ItemSlotData = MoveTemp(ItemSlotData);
	ItemDefinition = nullptr;
	if (bCallEmptyLogic && Released.ItemDefinition)
	{
		if (const auto CDO = Released.ItemDefinition->GetSlotDataDefinitionCDO())
		{
			CDO->EmptySlotData(Released);
		}
	}
}

int FInventoryItemEntry::GetStackAmount() const
{
	if (ItemDefinition)
	{
		if (const auto CDO = ItemDefinition->GetSlotDataDefinitionCDO())
		{
			return CDO->GetStackAmount(*this);
		}
		
		return 1;
	}

	return 0;
}

void FInventoryItemEntry::SetStackAmount(const int& InStackAmount)
{
	if (ItemDefinition)
	{
		if (const auto CDO = ItemDefinition->GetSlotDataDefinitionCDO())
		{
			return CDO->SetStackAmount(*this, InStackAmount);
		}
	}
}

int FInventoryItemEntry::GetMaxStackAmount() const
{
	if (ItemSlotFilter.bOnlyOne)
	{
		return 1;
	}
	
	if (ItemDefinition)
	{
		if (const auto CDO = ItemDefinition->GetSlotDataDefinitionCDO())
		{
			return CDO->GetMaxStackAmount(*this);
		}
	}

	return 1;
}

bool FInventoryItemEntry::CanEntryStack(const FInventoryItemEntry& OtherItemEntry) const
{
	if (ItemDefinition)
	{
		if (const auto CDO = ItemDefinition->GetSlotDataDefinitionCDO())
		{
			return CDO->CanStack(*this, OtherItemEntry);
		}
	}

	return false;
}

bool FInventoryItemEntry::StackEntry(FInventoryItemEntry& OtherEntry, int SpecificAmount)
{
	if (ItemDefinition)
	{
		if (const auto CDO = ItemDefinition->GetSlotDataDefinitionCDO())
		{
			return CDO->AddStackAmount(*this, OtherEntry, SpecificAmount);
		}
	}
	
	return false;
}

void FInventoryItemEntry::RemoveStack(int SpecificAmount)
{
	if (IsSlotEmpty() || SpecificAmount <= 0)
	{
		return;
	}

	if (ItemDefinition)
	{
		if (const auto CDO = ItemDefinition->GetSlotDataDefinitionCDO())
		{
			CDO->RemoveStackAmount(*this, SpecificAmount);
		}
	}
}

TObjectPtr<UInventoryItemInstance> FInventoryItemEntry::TryGetItemInstance() const
{
	if (const auto Instance = ItemSlotData.GetPtr<FItemSlotData_ItemInstance>())
	{
		return Instance->ItemInstance;
	}
	
	return nullptr;
}

////////////////////////
// FInventoryItemList //
////////////////////////

void FInventoryItemList::EmptySlotByIndex(TArray<int32> Indices, bool bDirty)
{
	for (const int Index : Indices)
	{
		if (ItemList.IsValidIndex(Index))
		{
			ItemList[Index].EmptySlot();
			if (bDirty)
			{
				MarkIndexChange(Index);
			}
		}
	}
}

void FInventoryItemList::Clear()
{
	TArray<int32> RemovedIndices;
	for (int i = 0; i < ItemList.Num(); ++i)
	{
		ItemList[i].EmptySlot();
		RemovedIndices.Add(i);
	}

	OnItemListRemove.Broadcast(RemovedIndices);
	ItemList.Empty();
	MarkArrayDirty();
}

void FInventoryItemList::AddEmptySlots(const int32 AddAmount)
{
	TArray<int32> AddIndices;
	for (int i = 0; i < AddAmount; ++i)
	{
		const auto OutIndex = ItemList.Add(FInventoryItemEntry());
		AddIndices.Add(OutIndex);
		MarkItemDirty(ItemList[OutIndex]);
	}

	OnItemListAdd.Broadcast(AddIndices);
}

void FInventoryItemList::RemoveSlots(TArray<int32> Indices)
{
	TArray<int32> RemovedIndices;
	for (const auto Index : Indices)
	{
		if (ItemList.IsValidIndex(Index))
		{
			RemovedIndices.AddUnique(Index);
		}
	}

	OnItemListRemove.Broadcast(RemovedIndices);

	// 修复：从后往前删除原索引，重复索引或数组收缩不会导致越界。
	RemovedIndices.Sort(TGreater<int32>());
	for (const auto Index : RemovedIndices)
	{
		if (ItemList.IsValidIndex(Index)) ItemList.RemoveAt(Index, EAllowShrinking::No);
	}
	ItemList.Shrink();
	
	MarkArrayDirty();
}

void FInventoryItemList::MarkIndexAdd(int32 Index)
{
	if (ItemList.IsValidIndex(Index))
	{
		MarkItemDirty(ItemList[Index]);
		OnItemListAdd.Broadcast({Index});
	}
}

void FInventoryItemList::MarkIndexAdd(TArray<int32> Indices)
{
	TArray<int32> ChangedIndices;
	for (const auto Index : Indices)
	{
		if (ItemList.IsValidIndex(Index))
		{
			ChangedIndices.Add(Index);
			MarkItemDirty(ItemList[Index]);
		}
	}

	OnItemListAdd.Broadcast(ChangedIndices);
}

void FInventoryItemList::MarkIndexRemove(int32 Index)
{
	if (ItemList.IsValidIndex(Index))
	{
		MarkArrayDirty();
		OnItemListRemove.Broadcast({Index});
	}
}

void FInventoryItemList::MarkIndexRemove(TArray<int32> Indices)
{
	if (Indices.Num() > 0)
	{
		MarkArrayDirty();
		OnItemListRemove.Broadcast(Indices);
	}
}

void FInventoryItemList::MarkIndexChange(int32 Index)
{
	if (ItemList.IsValidIndex(Index))
	{
		MarkItemDirty(ItemList[Index]);
		OnItemListChange.Broadcast({Index});
	}
}

void FInventoryItemList::MarkIndexChange(TArray<int32> Indices)
{
	TArray<int32> ChangedIndices;
	for (const auto Index : Indices)
	{
		if (ItemList.IsValidIndex(Index))
		{
			ChangedIndices.Add(Index);
			MarkItemDirty(ItemList[Index]);
		}
	}

	OnItemListChange.Broadcast(ChangedIndices);
}

bool FInventoryItemList::IsSlotEmpty(int Index, bool& Empty) const
{
	if (ItemList.IsValidIndex(Index))
	{
		Empty = ItemList[Index].IsSlotEmpty();
		return true;
	}
	
	return false;
}

int FInventoryItemList::FindFirstEmptySlot(const UInventoryItemDefinition* ItemDefinition) const
{
	for (int i = 0; i < ItemList.Num(); ++i)
	{
		if (ItemList[i].IsSlotEmpty())
		{
			if (ItemDefinition)
			{
				if (ItemList[i].IsItemAllowed(ItemDefinition))
				{
					return i;
				}
			}
			else
			{
				return i;
			}
		}
	}
	
	return INDEX_NONE;
}

int FInventoryItemList::FindFirstStackableSlot(const FInventoryItemEntry& OtherItemEntry) const
{
	for (int i = 0; i < ItemList.Num(); ++i)
    {
    	const auto ItrEntry = ItemList[i];
    	if (ItrEntry.CanEntryStack(OtherItemEntry))
    	{
    		return i;
    	}
    }
    
    return INDEX_NONE;
}

int FInventoryItemList::GetItemTotalAmountByDefinition(
	const TSoftObjectPtr<UInventoryItemDefinition>& ItemDefinitionSoft) const
{
	int Result = 0;
	
	for (auto Itr : ItemList)
	{
		if (!Itr.IsSlotEmpty() && TSoftObjectPtr<>(Itr.ItemDefinition) == ItemDefinitionSoft)
		{
			Result += Itr.GetStackAmount();
		}
	}
	
	return Result;
}

int FInventoryItemList::GetItemTotalAmountByDefinition(const UInventoryItemDefinition* ItemDef) const
{
	int Result = 0;
	
	for (auto Itr : ItemList)
	{
		if (!Itr.IsSlotEmpty() && Itr.ItemDefinition == ItemDef)
		{
			Result += Itr.GetStackAmount();
		}
	}
	
	return Result;
}

UInventoryItemDefinition* FInventoryItemList::GetItemDefinition(const int& Index) const
{
	if (ItemList.IsValidIndex(Index))
	{
		return ItemList[Index].ItemDefinition;
	}

	return nullptr;
}

UInventoryItemInstance* FInventoryItemList::GetItemInstance(const int& Index) const
{
	if (ItemList.IsValidIndex(Index))
	{
		return ItemList[Index].TryGetItemInstance();
	}

	return nullptr;
}

bool FInventoryItemList::CanSlotSplit(const int& SlotIndex) const
{
	if (ItemList.IsValidIndex(SlotIndex))
	{
		return ItemList[SlotIndex].GetStackAmount() > 1;
	}
	
	return false;
}

FItemStackMapping FInventoryItemList::GetItemMappings() const
{
	TMap<TSoftObjectPtr<UInventoryItemDefinition>, int> Result;

	for (const auto ItemSlot : ItemList)
	{
		if (!ItemSlot.IsSlotEmpty())
		{
			if (const auto Found = Result.Find(ItemSlot.ItemDefinition))
			{
				Result.Add(ItemSlot.ItemDefinition, *Found + ItemSlot.GetStackAmount());
			}
			else
			{
				Result.Add(ItemSlot.ItemDefinition, ItemSlot.GetStackAmount());
			}
		}
	}

	return FItemStackMapping(Result);
}

bool FInventoryItemList::CanRemoveItems(const FItemStackMapping& InItems) const
{
	// Not empty.
	if (InItems.Items.IsEmpty())
	{
		return false;
	}
	
	for (const auto Map : InItems.Items)
	{
		if (GetItemTotalAmountByDefinition(Map.Key) < Map.Value)
		{
			return false;
		}
	}

	return true;
}

bool FInventoryItemList::CanAddItems(const FItemStackMapping& InItems) const
{
	// Input invalid.
	if (InItems.Items.Num() == 0)
	{
		return false;
	}
	
	// Copy list for calculation.
	// 修复：容量试算只操作副本，避免注册临时复制对象或调用真实库存的监听器。
	FInventoryItemList CopiedList;
	CopiedList.ItemList = ItemList;
	for (auto Item : InItems.Items)
	{
		if (const auto LoadedItem = Item.Key.LoadSynchronous())
		{
			if (!CopiedList.AddItemByDefinition(LoadedItem, Item.Value))
			{
				return false;
			}
		}
	}

	return true;
}

bool FInventoryItemList::CanExchangeItems(const FItemStackMapping& RemoveItems, const FItemStackMapping& AddItems) const
{
	// 修复：容量试算只操作副本，避免注册临时复制对象或调用真实库存的监听器。
	FInventoryItemList CopiedList;
	CopiedList.ItemList = ItemList;
	if (CopiedList.CanRemoveItems(RemoveItems))
	{
		for (const auto Itr : RemoveItems.Items)
		{
			const auto ItemDef = Itr.Key.LoadSynchronous();
			CopiedList.RemoveItemByDefinition(ItemDef, Itr.Value);
		}
		
		if (CopiedList.CanAddItems(AddItems))
		{
			return true;
		}
	}

	return false;
}

bool FInventoryItemList::ExchangeItems(const FItemStackMapping& RemoveItems, const FItemStackMapping& AddItems)
{
	if (CanExchangeItems(RemoveItems, AddItems))
	{
		for (const auto Itr : RemoveItems.Items)
		{
			const auto ItemDef = Itr.Key.LoadSynchronous();
			RemoveItemByDefinition(ItemDef, Itr.Value);
		}
		
		for (const auto Itr : AddItems.Items)
		{
			const auto ItemDef = Itr.Key.LoadSynchronous();
			AddItemByDefinition(ItemDef, Itr.Value);
		}

		return true;
	}

	return false;
}

bool FInventoryItemList::RemoveStackAtIndex(const int& Index, const int& Amount, const bool& bForceRemove)
{
	if (!ItemList.IsValidIndex(Index)) return false;
	
	// Valid check.
	if (!bForceRemove)
	{
		if (ItemList[Index].GetStackAmount() < Amount)
		{
			return false;
		}
	}

	ItemList[Index].RemoveStack(Amount);
	MarkIndexChange(Index);
	return true;
}

int FInventoryItemList::RemoveItemByDefinition(const UInventoryItemDefinition* ItemDef, const int& Amount,
                                               const bool& bForceRemove)
{
	int LocalAmount = Amount;
	// Valid check.
	if (!bForceRemove)
	{
		if (GetItemTotalAmountByDefinition(ItemDef) < Amount)
		{
			return Amount;
		}
	}

	TArray<int> EmptyIndex;
	TArray<int> DirtyIndex;
	for (int i = ItemList.Num() - 1; i >= 0; --i)
	{
		if (ItemList.IsValidIndex(i) && !ItemList[i].IsSlotEmpty() && ItemList[i].ItemDefinition == ItemDef && LocalAmount > 0)
		{
			if (ItemList[i].GetStackAmount() > LocalAmount)
			{
				ItemList[i].RemoveStack(LocalAmount);
				LocalAmount = 0;
				MarkIndexChange(i);
				break;
			}
			
			if (ItemList[i].GetStackAmount() == LocalAmount)
			{
				LocalAmount = 0;
				EmptySlotByIndex({i});
				break;
			}
			
			// 修复：清空操作可能广播并改变数组，先读取本槽数量。
			LocalAmount -= ItemList[i].GetStackAmount();
			EmptySlotByIndex({i});
		}
	}
	
	return LocalAmount;
}

bool FInventoryItemList::AddItemByDefinition(UInventoryItemDefinition* ItemDef, const int& Amount)
{
	auto NewEntry = FInventoryItemEntry(OuterObject, ItemDef);
	NewEntry.SetStackAmount(Amount);
	return AddItem(NewEntry);
}

bool FInventoryItemList::AddItem(FInventoryItemEntry& ItemEntry)
{
	// 修复：旧存档可留下槽数据却没有定义，不能继续堆叠。
	if (!IsValid(ItemEntry.ItemDefinition)) return false;

	// 1. Stack
	int StackIndex = FindFirstStackableSlot(ItemEntry);
	while (StackIndex >= 0)
	{
		// 修复：自定义堆叠逻辑未消耗物品时结束尝试，避免无限循环。
		const int32 BeforeStack = ItemEntry.GetStackAmount();
		const auto StackResult = ItemList[StackIndex].StackEntry(ItemEntry);
		MarkIndexChange(StackIndex);
		if (StackResult)
		{
			return true;
		}

		if (ItemEntry.GetStackAmount() >= BeforeStack) break;

		// Find stack again.
		StackIndex = FindFirstStackableSlot(ItemEntry);
	}

	// 2. Empty
	int EmptyIndex = FindFirstEmptySlot(ItemEntry.ItemDefinition);
	while (EmptyIndex >= 0)
	{
		const auto AmountToMax = ItemEntry.GetMaxStackAmount();

		// End
		if (AmountToMax >= ItemEntry.GetStackAmount())
		{
			ItemList[EmptyIndex].CopyFrom(ItemEntry);
			if (const auto ItemInstance = ItemEntry.TryGetItemInstance())
			{
				ItemInstance->ChangeOuter(OuterObject);
			}
			// 修复：先结束原槽转移，再通知可能调整库存数组的监听器。
			ItemEntry.EmptySlot(false);
			MarkIndexChange(EmptyIndex);
			return true;
		}

		// Special Empty Loop (Amount over max stack.)
		ItemList[EmptyIndex].ItemDefinition = ItemEntry.ItemDefinition;
		if (const auto LogicCDO = ItemEntry.ItemDefinition->GetSlotDataDefinitionCDO())
		{
			const auto DefaultSlotData = LogicCDO->GetDefaultSlotData(ItemEntry.ItemDefinition, OuterObject);
			ItemList[EmptyIndex].ItemSlotData = DefaultSlotData;
			if (LogicCDO->AddStackAmount(ItemList[EmptyIndex], ItemEntry))
			{
				return true;
			}
		}
		
		// Instance
		MarkIndexChange(EmptyIndex);
		
		// Find empty again.
		EmptyIndex = FindFirstEmptySlot(ItemEntry.ItemDefinition);
	}
	
	return false;
}

bool FInventoryItemList::DragDropItem(int DragIndex, FInventoryItemList& DropItemList, int DropIndex)
{
	// Not valid when drag slot is empty!!!
	
	if (!DropItemList.ItemList.IsValidIndex(DropIndex) ||
		!ItemList.IsValidIndex(DragIndex) ||
		ItemList[DragIndex].IsSlotEmpty()) return false;

	// Slot filter.
	if (DropItemList.ItemList[DropIndex].IsSlotEmpty())
	{
		if (!DropItemList.ItemList[DropIndex].IsItemAllowed(ItemList[DragIndex].ItemDefinition))
		{
			return false;
		}
	}
	
	const auto DragItemDef = ItemList[DragIndex].ItemDefinition;
	const auto DropItemDef = DropItemList.ItemList[DropIndex].ItemDefinition;
	
	const auto DragCDO = DragItemDef ? DragItemDef->GetSlotDataDefinitionCDO() : nullptr;
	const auto DropCDO = DropItemDef ? DropItemDef->GetSlotDataDefinitionCDO() : nullptr;
	
	const auto DragItemSlotData = ItemList[DragIndex].ItemSlotData;
	const auto DropItemSlotData = DropItemList.ItemList[DropIndex].ItemSlotData;
	
	const auto DragItemStackAmount = ItemList[DragIndex].GetStackAmount();
	const auto DropItemStackAmount = DropItemList.ItemList[DropIndex].GetStackAmount();

	const auto ItemMaxStackAmount = ItemList[DragIndex].GetMaxStackAmount();
	const bool bIsSame = DragItemDef == DropItemDef;
	const bool bSameExchange = bIsSame && (ItemMaxStackAmount == DragItemStackAmount || ItemMaxStackAmount == DropItemStackAmount);
	
	// 1. Not the same definition or Max stack exchange
	if (!bIsSame || bSameExchange)
	{
		ItemList[DragIndex].ItemDefinition = DropItemDef;
		ItemList[DragIndex].ItemSlotData = DropItemSlotData;
		DropItemList.ItemList[DropIndex].ItemDefinition = DragItemDef;
		DropItemList.ItemList[DropIndex].ItemSlotData = DragItemSlotData;

		// Update post begin play logic for item instances or other things.
		if (DropCDO)
		{
			DropCDO->PostBeginPlay(ItemList[DragIndex], OuterObject);
		}
		if (DragCDO)
		{
			DragCDO->PostBeginPlay(DropItemList.ItemList[DropIndex], DropItemList.OuterObject);
		}
		
		MarkIndexChange(DragIndex);
		DropItemList.MarkIndexChange(DropIndex);
		return true;
	}

	// 2. Is the same definition
	DropItemList.ItemList[DropIndex].StackEntry(ItemList[DragIndex]);
	MarkIndexChange(DragIndex);
	DropItemList.MarkIndexChange(DropIndex);
	return true;
}

bool FInventoryItemList::QuickMoveItem(int FromIndex, FInventoryItemList& ToItemList)
{
	// 修复：同一列表快速移动可能把物品堆叠到自身。
	if (this != &ToItemList && ItemList.IsValidIndex(FromIndex))
	{
		if (ToItemList.AddItem(ItemList[FromIndex]))
		{
			MarkIndexChange(FromIndex);
			return true;
		}
	}

	return false;
}

bool FInventoryItemList::SplitItem(int SplitIndex, int SplitAmount, FInventoryItemList& ContainerToSplit)
{
	if (!ItemList.IsValidIndex(SplitIndex))
	{
		return false;
	}
	if (ItemList[SplitIndex].IsSlotEmpty())
	{
		return false;
	}
	if (!CanSlotSplit(SplitIndex) || SplitAmount < 1)
	{
		return false;
	}

	const auto FoundEmptyIndex = ContainerToSplit.FindFirstEmptySlot(ItemList[SplitIndex].ItemDefinition);
	if (FoundEmptyIndex >= 0)
	{
		const auto Def = ItemList[SplitIndex].ItemDefinition;
		ContainerToSplit.ItemList[FoundEmptyIndex].ItemDefinition = Def;
		if (Def)
		{
			if (const auto CDO = Def->GetSlotDataDefinitionCDO())
			{
				const auto ReturnEntry = CDO->RemoveStackAmount(ItemList[SplitIndex], SplitAmount);
				ContainerToSplit.ItemList[FoundEmptyIndex].ItemSlotData = ReturnEntry.ItemSlotData;
				CDO->PostBeginPlay(ContainerToSplit.ItemList[FoundEmptyIndex], ContainerToSplit.OuterObject);
			}
		}
			
		MarkIndexChange(SplitIndex);
		ContainerToSplit.MarkIndexChange(FoundEmptyIndex);
		return true;
	}
	
	return false;
}

void FInventoryItemList::PreReplicatedRemove(const TArrayView<int32>& RemovedIndices, int32 FinalSize)
{
	TArray<int> Indices;
	Indices.Append(RemovedIndices);
	OnItemListRemove.Broadcast(Indices);
}

void FInventoryItemList::PostReplicatedAdd(const TArrayView<int32>& AddedIndices, int32 FinalSize)
{
	TArray<int> Indices;
	Indices.Append(AddedIndices);
	OnItemListAdd.Broadcast(Indices);
}

void FInventoryItemList::PostReplicatedChange(const TArrayView<int32>& ChangedIndices, int32 FinalSize)
{
	TArray<int> Indices;
	Indices.Append(ChangedIndices);
	OnItemListChange.Broadcast(Indices);
}

bool FInventoryItemList::NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
{
	return FFastArraySerializer::FastArrayDeltaSerialize<FInventoryItemEntry, FInventoryItemList>(ItemList, DeltaParams, *this);
}