// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraCharacterPreviewComponent.h"

#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/World.h"
#include "LyraCharacterPreview.h"
#include "IPreviewVisualsProvider.h"
#include "TimerManager.h"
#include "Animation/AnimInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraCharacterPreviewComponent)

ULyraCharacterPreviewComponent::ULyraCharacterPreviewComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	SetIsReplicatedByDefault(false);
}

void ULyraCharacterPreviewComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (CaptureComponent)
	{
		CaptureComponent->CaptureScene();
	}
}

void ULyraCharacterPreviewComponent::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PC = Cast<APlayerController>(GetOwner()))
	{
		PC->OnPossessedPawnChanged.AddDynamic(this, &ULyraCharacterPreviewComponent::OnPossessedPawnChanged);

		if (APawn* Pawn = PC->GetPawn())
		{
			InitPreview(Pawn);
		}
	}
}

void ULyraCharacterPreviewComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (APlayerController* PC = Cast<APlayerController>(GetOwner()))
	{
		PC->OnPossessedPawnChanged.RemoveDynamic(this, &ULyraCharacterPreviewComponent::OnPossessedPawnChanged);
	}

	DestroyPreview();
	Super::EndPlay(EndPlayReason);
}

ULyraCharacterPreviewComponent* ULyraCharacterPreviewComponent::FindCharacterPreviewComponent(const AActor* Actor)
{	
	if (!Actor) return nullptr;
	return Actor->FindComponentByClass<ULyraCharacterPreviewComponent>();
}

void ULyraCharacterPreviewComponent::InitPreview(APawn* SourcePawn)
{
	if (!SourcePawn) return;

	DestroyPreview();
	SourcePawnRef = SourcePawn;

	SpawnPreviewActor();
	if (!PreviewActor) return;
	
	if (const USkeletalMeshComponent* PawnMesh = SourcePawn->FindComponentByClass<USkeletalMeshComponent>())
	{
		PreviewActor->SetBodyMesh(PawnMesh->GetSkeletalMeshAsset());
	}
	
	if (UActorComponent* ProviderComp = FindProviderComponent())
	{
		BoundProviderComponent = ProviderComp;
		if (IPreviewVisualsProvider* Provider = Cast<IPreviewVisualsProvider>(ProviderComp))
		{
			ProviderChangedHandle = Provider->OnPreviewVisualsChanged().AddUObject(
				this, &ULyraCharacterPreviewComponent::OnProviderVisualsChanged);
		}
	}

	RequestRefresh();
}

void ULyraCharacterPreviewComponent::DestroyPreview()
{
	if (UActorComponent* ProviderComp = BoundProviderComponent.Get())
	{
		if (IPreviewVisualsProvider* Provider = Cast<IPreviewVisualsProvider>(ProviderComp))
		{
			Provider->OnPreviewVisualsChanged().Remove(ProviderChangedHandle);
		}
	}
	BoundProviderComponent = nullptr;
	ProviderChangedHandle.Reset();

	if (IsValid(PreviewActor))
	{
		PreviewActor->Destroy();
	}
	PreviewActor = nullptr;
	CaptureComponent = nullptr;
	RenderTarget = nullptr;
	SourcePawnRef = nullptr;
}

void ULyraCharacterPreviewComponent::OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	if (NewPawn && NewPawn != SourcePawnRef)
	{
		InitPreview(NewPawn);
	}
	else if (!NewPawn)
	{
		DestroyPreview();
	}
}

void ULyraCharacterPreviewComponent::SetCaptureEnabled(const bool bEnabled)
{
	SetComponentTickEnabled(bEnabled);

	if (bEnabled)
	{
		RequestRefresh();
	}
}

// ----------------------------------------------------------------------
// Zoom & Rotation
// ----------------------------------------------------------------------

void ULyraCharacterPreviewComponent::AddZoomDelta(float Delta)
{
	CurrentZoom = FMath::Clamp(CurrentZoom - (Delta * ZoomStep), ZoomMin, ZoomMax);

	if (CaptureComponent)
	{
		FVector NewOffset = CaptureOffset;
		NewOffset.X = CurrentZoom;
		CaptureComponent->SetRelativeLocation(NewOffset);
	}
}

void ULyraCharacterPreviewComponent::AddRotationDelta(float Delta)
{
	CurrentRotation = FMath::Clamp(CurrentRotation + (Delta * RotationStep), RotationMin, RotationMax);

	if (PreviewActor)
	{
		PreviewActor->SetPreviewYaw(CurrentRotation);
	}
}

bool ULyraCharacterPreviewComponent::IsPreviewReady() const
{
	return IsValid(PreviewActor);
}

// ----------------------------------------------------------------------
// Spawn / Refresh
// ----------------------------------------------------------------------

void ULyraCharacterPreviewComponent::SpawnPreviewActor()
{
	UWorld* World = GetWorld();
	if (!World) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.ObjectFlags = RF_Transient;

	PreviewActor = World->SpawnActor<ALyraCharacterPreview>(
		PreviewActorClass, PreviewSpawnLocation, FRotator::ZeroRotator, SpawnParams);

	if (!PreviewActor) return;

	RenderTarget = NewObject<UTextureRenderTarget2D>(this);
	RenderTarget->RenderTargetFormat = ETextureRenderTargetFormat::RTF_RGBA16f;
	RenderTarget->ClearColor = FLinearColor::Transparent;
	RenderTarget->bAutoGenerateMips = false;
	RenderTarget->InitAutoFormat(RenderTargetSize.X, RenderTargetSize.Y);
	RenderTarget->UpdateResourceImmediate(true);

	CaptureComponent = NewObject<USceneCaptureComponent2D>(PreviewActor, TEXT("PreviewCapture"));
	CaptureComponent->RegisterComponentWithWorld(World);
	CaptureComponent->AttachToComponent(PreviewActor->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	CaptureComponent->SetRelativeLocation(CaptureOffset);
	CaptureComponent->SetRelativeRotation(CaptureRotation);
	CaptureComponent->FOVAngle = CaptureFOV;
	CaptureComponent->TextureTarget = RenderTarget;
	CaptureComponent->CaptureSource = SCS_SceneColorHDR;
	CaptureComponent->bCaptureEveryFrame = false;
	CaptureComponent->bCaptureOnMovement = false;
	CaptureComponent->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
	CaptureComponent->ShowOnlyActors.Add(PreviewActor);
	CaptureComponent->ShowFlags.SetAtmosphere(false);
	CaptureComponent->ShowFlags.SetFog(false);
	CaptureComponent->ShowFlags.SetSkyLighting(false);
	CaptureComponent->ShowFlags.SetVisualizeSkyAtmosphere(false);
}

void ULyraCharacterPreviewComponent::RequestRefresh()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimerForNextTick(this, &ULyraCharacterPreviewComponent::RefreshCharacterPreview);
	}
}

UActorComponent* ULyraCharacterPreviewComponent::FindProviderComponent() const
{
	if (!SourcePawnRef) return nullptr;
	return SourcePawnRef->FindComponentByInterface(UPreviewVisualsProvider::StaticClass());
}

void ULyraCharacterPreviewComponent::OnProviderVisualsChanged()
{
	RequestRefresh();
}

void ULyraCharacterPreviewComponent::RefreshCharacterPreview()
{
	if (!PreviewActor || !CaptureComponent) return;

	PreviewActor->ClearAttachments();

	FCharacterPreviewVisuals Visuals;
	if (UActorComponent* ProviderComp = BoundProviderComponent.Get())
	{
		if (IPreviewVisualsProvider* Provider = Cast<IPreviewVisualsProvider>(ProviderComp))
		{
			Provider->GatherPreviewVisuals(Visuals);
		}
	}

	for (const FPreviewAttachmentSpec& Spec : Visuals.Attachments)
	{
		if (Spec.SkeletalMesh)
		{
			PreviewActor->AddSkeletalAttachment(Spec.SkeletalMesh, Spec.AttachSocket, Spec.AttachTransform);
		}
		else if (Spec.StaticMesh)
		{
			PreviewActor->AddStaticAttachment(Spec.StaticMesh, Spec.AttachSocket, Spec.AttachTransform);
		}
	}

	TSubclassOf<UAnimInstance> AnimClassToUse = Visuals.AnimClass;
	if (!AnimClassToUse)
	{
		AnimClassToUse = PreviewAnimSelection.SelectBestLayer(Visuals.CosmeticTags);
	}
	PreviewActor->SetBodyAnimClass(AnimClassToUse);
}