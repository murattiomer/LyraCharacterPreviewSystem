// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "IPreviewVisualsProvider.h"
#include "Components/GameFrameworkComponent.h"
#include "LyraEquipmentPreviewProvider.generated.h"

#define UE_API LYRAGAME_API

struct FLyraEquipmentVisibilityMessage;
struct FLyraQuickBarActiveIndexChangedMessage;
class ULyraPawnComponent_CharacterParts;

/**
 * Reads the owning pawn's equipment and cosmetic tags and exposes them to the preview system.
 */
UCLASS(meta=(BlueprintSpawnableComponent))
class ULyraEquipmentPreviewProvider : public UGameFrameworkComponent, public IPreviewVisualsProvider
{
	GENERATED_BODY()

public:
	
	/** Default constructor. */
	UE_API ULyraEquipmentPreviewProvider(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~ IPreviewVisualsProvider
	UE_API virtual void GatherPreviewVisuals(FCharacterPreviewVisuals& Out) const override;
	UE_API virtual FOnPreviewVisualsChanged& OnPreviewVisualsChanged() override { return VisualsChangedDelegate; }
	//~ End IPreviewVisualsProvider

protected:
	
	/** Called when the component is registered and the game starts. */
	UE_API virtual void BeginPlay() override;
	
	/** Called when the component is unregistered or the game ends. */
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	/** Finds the equipment preview provider component on the specified actor, if any. */
	UFUNCTION(BlueprintPure)
	static ULyraEquipmentPreviewProvider* FindProviderComponent(const AActor* Actor)
	{
		return (Actor ? Actor->FindComponentByClass<ULyraEquipmentPreviewProvider>() : nullptr);
	}

private:
	/** Registers the component as a listener for relevant gameplay messages. */
	UE_API void RegisterListeners();
	
	/** Unregisters the component as a listener for relevant gameplay messages. */
	UE_API void UnregisterListeners();
	
	/** Called when the quickbar active index changes. */
	UE_API void OnQuickbarChanged(FGameplayTag Channel, const FLyraQuickBarActiveIndexChangedMessage& Message);
	
	/** Called when the visibility of an equipment instance changes. */
	UE_API void OnEquipmentChanged(FGameplayTag Channel, const FLyraEquipmentVisibilityMessage& Message);

	/** Delegate that is broadcast when the preview visuals change. */
	FOnPreviewVisualsChanged VisualsChangedDelegate;
	
	/** Handle for the gameplay message listener for quickbar changes. */
	FGameplayMessageListenerHandle QuickBarListenerHandle;
	
	/** Handle for the gameplay message listener for equipment changes. */
	FGameplayMessageListenerHandle EquipmentListenerHandle;
};

#undef UE_API