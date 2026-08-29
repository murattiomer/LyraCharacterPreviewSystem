// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"
#include "UObject/Interface.h"
#include "IPreviewVisualsProvider.generated.h"

class USkeletalMesh;
class UStaticMesh;
class UAnimInstance;

/** Fired by the provider when the loadout changed and the preview should re-gather. */
DECLARE_MULTICAST_DELEGATE(FOnPreviewVisualsChanged);

/** One resolved attachment: a ready mesh + where to attach it. No item/equipment types. */
USTRUCT()
struct FPreviewAttachmentSpec
{
	GENERATED_BODY()

	UPROPERTY() TObjectPtr<USkeletalMesh> SkeletalMesh = nullptr;
	UPROPERTY() TObjectPtr<UStaticMesh>   StaticMesh   = nullptr;
	UPROPERTY() FName AttachSocket = NAME_None;
	UPROPERTY() FTransform AttachTransform = FTransform::Identity;
};

/** Everything the preview needs to display, fully resolved by the provider. */
USTRUCT()
struct FCharacterPreviewVisuals
{
	GENERATED_BODY()

	UPROPERTY() TSubclassOf<UAnimInstance> AnimClass = nullptr;
	UPROPERTY() TArray<FPreviewAttachmentSpec> Attachments;
};

UINTERFACE(MinimalAPI)
class UPreviewVisualsProvider : public UInterface
{
	GENERATED_BODY()
};

/**
 * Implemented by whatever knows how to turn a pawn's loadout into displayable meshes.
 * The preview never sees equipment/inventory types — it receives resolved meshes through
 * GatherPreviewVisuals and refreshes when OnPreviewVisualsChanged fires.
 */
class IPreviewVisualsProvider
{
	GENERATED_BODY()
public:
	/** Fills Out with the anim class and resolved attachment meshes. */
	virtual void GatherPreviewVisuals(FCharacterPreviewVisuals& Out) const = 0;

	/** Delegate the preview subscribes to; broadcast when visuals become stale. */
	virtual FOnPreviewVisualsChanged& OnPreviewVisualsChanged() = 0;
};