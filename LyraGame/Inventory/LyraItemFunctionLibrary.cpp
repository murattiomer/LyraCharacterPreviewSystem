// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraItemFunctionLibrary.h"

#include "InventoryFragment_ItemMeshes.h"
#include "InventoryFragment_PreviewAnim.h"
#include "Inventory/LyraInventoryItemDefinition.h"
#include "Inventory/LyraInventoryItemInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraItemFunctionLibrary)

TSubclassOf<ULyraInventoryItemDefinition> ULyraItemFunctionLibrary::GetItemDefinition(ULyraInventoryItemInstance* ItemInstance)
{
	if (ItemInstance == nullptr)
	{
		return nullptr;
	}
	return ItemInstance->GetItemDef();
}

UStaticMesh* ULyraItemFunctionLibrary::GetItemStaticMesh(const TSubclassOf<ULyraInventoryItemDefinition>& ItemDefinition, FName MeshName)
{
	if (!ItemDefinition)
	{
		return nullptr;
	}

	const ULyraInventoryItemDefinition* CDO = GetDefault<ULyraInventoryItemDefinition>(ItemDefinition);
	if (CDO == nullptr)
	{
		return nullptr;
	}

	const UInventoryFragment_ItemMeshes* Fragment = Cast<UInventoryFragment_ItemMeshes>(CDO->FindFragmentByClass(UInventoryFragment_ItemMeshes::StaticClass()));
	if (Fragment == nullptr)
	{
		return nullptr;
	}

	if (const FLyraItemStaticMeshEntry* Entry = Fragment->StaticMeshes.Find(MeshName))
	{
		return Entry->StaticMesh;
	}
	return nullptr;
}

USkeletalMesh* ULyraItemFunctionLibrary::GetItemSkeletalMesh(const TSubclassOf<ULyraInventoryItemDefinition>& ItemDefinition, FName MeshName)
{
	if (!ItemDefinition)
	{
		return nullptr;
	}

	const ULyraInventoryItemDefinition* CDO = GetDefault<ULyraInventoryItemDefinition>(ItemDefinition);
	if (CDO == nullptr)
	{
		return nullptr;
	}

	const UInventoryFragment_ItemMeshes* Fragment = Cast<UInventoryFragment_ItemMeshes>(CDO->FindFragmentByClass(UInventoryFragment_ItemMeshes::StaticClass()));
	if (Fragment == nullptr)
	{
		return nullptr;
	}

	if (const FLyraItemSkeletalMeshEntry* Entry = Fragment->SkeletalMeshes.Find(MeshName))
	{
		return Entry->SkeletalMesh;
	}
	return nullptr;
}

TSubclassOf<UAnimInstance> ULyraItemFunctionLibrary::GetItemPreviewAnimClass(const TSubclassOf<ULyraInventoryItemDefinition>& ItemDefinition, const FGameplayTagContainer& CosmeticTags)
{
	if (!ItemDefinition)
	{
		return nullptr;
	}

	const ULyraInventoryItemDefinition* CDO = GetDefault<ULyraInventoryItemDefinition>(ItemDefinition);
	if (CDO == nullptr)
	{
		return nullptr;
	}

	const UInventoryFragment_PreviewAnim* Fragment = Cast<UInventoryFragment_PreviewAnim>(CDO->FindFragmentByClass(UInventoryFragment_PreviewAnim::StaticClass()));
	if (Fragment == nullptr)
	{
		return nullptr;
	}

	return Fragment->PreviewAnimSelection.SelectBestLayer(CosmeticTags);
}
