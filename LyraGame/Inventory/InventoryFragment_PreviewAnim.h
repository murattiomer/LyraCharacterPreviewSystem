// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Cosmetics/LyraCosmeticAnimationTypes.h"
#include "Inventory/LyraInventoryItemDefinition.h"

#include "InventoryFragment_PreviewAnim.generated.h"

/**
 * Preview-only animation selection for an item. Used by the character preview system
 * to pick an anim BP for the body mesh based on cosmetic tags (e.g. male/female).
 * Independent from gameplay anim layers (see ULyraWeaponInstance's EquippedAnimSet).
 */
UCLASS()
class LYRAGAME_API UInventoryFragment_PreviewAnim : public ULyraInventoryItemFragment
{
	GENERATED_BODY()

public:
	/** Layer rules by cosmetic tag; falls back to DefaultLayer if no match. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Preview)
	FLyraAnimLayerSelectionSet PreviewAnimSelection;
};