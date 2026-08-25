// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "IPreviewVisualsProvider.h"
#include "LyraEquipmentPreviewProvider.generated.h"

#define UE_API LYRAGAME_API

struct FLyraEquipmentVisibilityMessage;
struct FLyraQuickBarActiveIndexChangedMessage;

/**
 * Reads the owning pawn's equipment and exposes it to the preview system as resolved meshes.
 * Lives on the pawn (added by the preview game feature). Equipment core knows nothing about it;
 * this component depends on equipment, not the other way around.
 */
UCLASS(meta=(BlueprintSpawnableComponent))
class ULyraEquipmentPreviewProvider : public UActorComponent, public IPreviewVisualsProvider
{
	GENERATED_BODY()

public:
	UE_API ULyraEquipmentPreviewProvider(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~ IPreviewVisualsProvider
	UE_API virtual void GatherPreviewVisuals(FCharacterPreviewVisuals& Out) const override;
	UE_API virtual FOnPreviewVisualsChanged& OnPreviewVisualsChanged() override { return VisualsChangedDelegate; }
	//~ End IPreviewVisualsProvider

protected:
	UE_API virtual void BeginPlay() override;
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UE_API void RegisterListeners();
	UE_API void UnregisterListeners();

	UE_API void OnQuickbarChanged(FGameplayTag Channel, const FLyraQuickBarActiveIndexChangedMessage& Message);
	UE_API void OnEquipmentChanged(FGameplayTag Channel, const FLyraEquipmentVisibilityMessage& Message);

	FOnPreviewVisualsChanged VisualsChangedDelegate;

	FGameplayMessageListenerHandle QuickBarListenerHandle;
	FGameplayMessageListenerHandle EquipmentListenerHandle;
};

#undef UE_API