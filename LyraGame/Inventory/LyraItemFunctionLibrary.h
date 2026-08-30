// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Templates/SubclassOf.h"

#include "LyraItemFunctionLibrary.generated.h"

struct FGameplayTagContainer;
class UAnimInstance;
class ULyraInventoryItemDefinition;
class ULyraInventoryItemInstance;
class USkeletalMesh;
class UStaticMesh;

/**
 * Blueprint-callable helpers for querying data off an inventory item definition.
 * Kept as a library (not on the definition itself) so callers can look up mesh entries
 * without holding a live item instance.
 */
UCLASS()
class LYRAGAME_API ULyraItemFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Definition class of the given item instance, or nullptr. */
	UFUNCTION(BlueprintPure, Category = "Item Library|Property")
	static TSubclassOf<ULyraInventoryItemDefinition> GetItemDefinition(ULyraInventoryItemInstance* ItemInstance);

	/** Static mesh registered under MeshName on the item's UInventoryFragment_ItemMeshes, or nullptr. */
	UFUNCTION(BlueprintPure, Category = "Item Library|Property")
	static UStaticMesh* GetItemStaticMesh(const TSubclassOf<ULyraInventoryItemDefinition>& ItemDefinition, FName MeshName);

	/** Skeletal mesh registered under MeshName on the item's UInventoryFragment_ItemMeshes, or nullptr. */
	UFUNCTION(BlueprintPure, Category = "Item Library|Property")
	static USkeletalMesh* GetItemSkeletalMesh(const TSubclassOf<ULyraInventoryItemDefinition>& ItemDefinition, FName MeshName);
	
	/** Preview anim class from the item's UInventoryFragment_PreviewAnim, matched against CosmeticTags. Nullptr if fragment missing. */
	UFUNCTION(BlueprintPure, Category = "Item Library|Property")
	static TSubclassOf<UAnimInstance> GetItemPreviewAnimClass(const TSubclassOf<ULyraInventoryItemDefinition>& ItemDefinition, const FGameplayTagContainer& CosmeticTags);
};