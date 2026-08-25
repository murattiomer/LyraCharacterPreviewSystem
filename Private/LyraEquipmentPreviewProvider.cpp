// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraEquipmentPreviewProvider.h"

#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "LyraGameplayTags.h"

#include "Equipment/LyraEquipmentManagerComponent.h"
#include "Equipment/LyraEquipmentInstance.h"
#include "Equipment/LyraQuickBarComponent.h"
#include "Inventory/LyraInventoryItemDefinition.h"
#include "Inventory/LyraInventoryItemInstance.h"
#include "Inventory/LyraItemFunctionLibrary.h"
#include "Weapons/LyraWeaponInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraEquipmentPreviewProvider)

ULyraEquipmentPreviewProvider::ULyraEquipmentPreviewProvider(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(false);
}

void ULyraEquipmentPreviewProvider::BeginPlay()
{
	Super::BeginPlay();

	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	RegisterListeners();
}

void ULyraEquipmentPreviewProvider::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnregisterListeners();
	Super::EndPlay(EndPlayReason);
}

void ULyraEquipmentPreviewProvider::GatherPreviewVisuals(FCharacterPreviewVisuals& Out) const
{
	const APawn* Pawn = Cast<APawn>(GetOwner());
	if (!Pawn) return;

	ULyraEquipmentManagerComponent* EquipManager =
		const_cast<ULyraEquipmentManagerComponent*>(
			Pawn->FindComponentByClass<ULyraEquipmentManagerComponent>());
	if (!EquipManager) return;

	// Resolve every equipped instance's spawn info down to meshes.
	const TArray<ULyraEquipmentInstance*> EquipInstances =
		EquipManager->GetEquipmentInstancesOfType(ULyraEquipmentInstance::StaticClass());

	for (ULyraEquipmentInstance* EInstance : EquipInstances)
	{
		if (!EInstance) continue;

		ULyraInventoryItemInstance* ItemInstance =
			Cast<ULyraInventoryItemInstance>(EInstance->GetInstigator());
		if (!ItemInstance) continue;

		const TSubclassOf<ULyraInventoryItemDefinition> ItemDef =
			ULyraItemFunctionLibrary::GetItemDefinition(ItemInstance);
		if (!ItemDef) continue;

		TArray<FLyraEquipmentActorToSpawn> ActorsToSpawn;
		EquipManager->GetCurrentActorsToSpawnForInstance(EInstance, ActorsToSpawn);

		for (const FLyraEquipmentActorToSpawn& SpawnInfo : ActorsToSpawn)
		{
			// We ignore custom actor classes on purpose: preview is mesh-only.
			FPreviewAttachmentSpec Spec;
			Spec.AttachSocket = SpawnInfo.AttachSocket;
			Spec.AttachTransform = SpawnInfo.AttachTransform;

			if (SpawnInfo.MeshType == ELyraEquipmentMeshType::Skeletal)
			{
				Spec.SkeletalMesh = ULyraItemFunctionLibrary::GetItemSkeletalMesh(ItemDef, SpawnInfo.MeshKey);
			}
			else
			{
				Spec.StaticMesh = ULyraItemFunctionLibrary::GetItemStaticMesh(ItemDef, SpawnInfo.MeshKey);
			}

			if (Spec.SkeletalMesh || Spec.StaticMesh)
			{
				Out.Attachments.Add(Spec);
			}
		}
	}

	// Anim class comes from the held weapon, if any.
	if (const ULyraWeaponInstance* WeaponInstance = EquipManager->GetFirstHeldInstanceOfType<ULyraWeaponInstance>())
	{
		if (const ULyraInventoryItemInstance* WeaponItem =
				Cast<ULyraInventoryItemInstance>(WeaponInstance->GetInstigator()))
		{
			Out.AnimClass = ULyraItemFunctionLibrary::GetItemPreviewAnimClass(
				WeaponItem->GetItemDef(), FGameplayTagContainer());
		}
	}
}

void ULyraEquipmentPreviewProvider::RegisterListeners()
{
	UWorld* World = GetWorld();
	if (!World) return;

	UGameplayMessageSubsystem& Msg = UGameplayMessageSubsystem::Get(World);

	QuickBarListenerHandle = Msg.RegisterListener<FLyraQuickBarActiveIndexChangedMessage>(
		LyraGameplayTags::Lyra_QuickBar_Message_ActiveIndexChanged,
		this, &ULyraEquipmentPreviewProvider::OnQuickbarChanged);

	EquipmentListenerHandle = Msg.RegisterListener<FLyraEquipmentVisibilityMessage>(
		LyraGameplayTags::Lyra_Equipment_Message_VisibilityChanged,
		this, &ULyraEquipmentPreviewProvider::OnEquipmentChanged);
}

void ULyraEquipmentPreviewProvider::UnregisterListeners()
{
	QuickBarListenerHandle.Unregister();
	EquipmentListenerHandle.Unregister();
}

void ULyraEquipmentPreviewProvider::OnQuickbarChanged(FGameplayTag Channel, const FLyraQuickBarActiveIndexChangedMessage& Message)
{
	if (Message.Owner != GetOwner()) return;
	VisualsChangedDelegate.Broadcast();
}

void ULyraEquipmentPreviewProvider::OnEquipmentChanged(FGameplayTag Channel, const FLyraEquipmentVisibilityMessage& Message)
{
	ULyraInventoryItemInstance* Item = Cast<ULyraInventoryItemInstance>(Message.Instigator);
	if (!Item) return;

	const ULyraEquipmentManagerComponent* EquipManager =
		GetOwner()->FindComponentByClass<ULyraEquipmentManagerComponent>();
	if (!EquipManager || EquipManager->GetInstanceForItem(Item) == nullptr)
	{
		return;
	}

	VisualsChangedDelegate.Broadcast();
}