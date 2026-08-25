// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraCharacterPreview.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/MeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"

ALyraCharacterPreview::ALyraCharacterPreview()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("PreviewRoot"));
	SetRootComponent(Root);

	BodyMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BodyMesh"));
	BodyMesh->SetupAttachment(Root);
	BodyMesh->SetSimulatePhysics(false);
	BodyMesh->SetEnableGravity(false);
	BodyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BodyMesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
	BodyMesh->SetLightingChannels(false, false, true);
	BodyMesh->SetRelativeRotation(FRotator(0.f, BaseYawOffset, 0.f));

	KeyLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("KeyLight"));
	KeyLight->SetupAttachment(Root);
	KeyLight->SetRelativeLocation(FVector(200.f, 0.f, 200.f));
	KeyLight->SetRelativeRotation(FRotator(-30.f, 180.f, 0.f));
	KeyLight->Intensity = 10000.f;
	KeyLight->OuterConeAngle = 45.f;
	KeyLight->InnerConeAngle = 30.f;
	KeyLight->LightColor = FColor::White;
	KeyLight->SetLightingChannels(false, false, true);

	FillLight = CreateDefaultSubobject<URectLightComponent>(TEXT("FillLight"));
	FillLight->SetupAttachment(Root);
	FillLight->SetRelativeLocation(FVector(100.f, -300.f, 100.f));
	FillLight->SetRelativeRotation(FRotator(0.f, 90.f, 0.f));
	FillLight->Intensity = 5000.f;
	FillLight->LightColor = FColor(200, 220, 255);
	FillLight->SetLightingChannels(false, false, true);
}

void ALyraCharacterPreview::SetBodyMesh(USkeletalMesh* InMesh)
{
	if (BodyMesh)
	{
		BodyMesh->SetSkeletalMeshAsset(InMesh);
	}
}

void ALyraCharacterPreview::SetBodyAnimClass(TSubclassOf<UAnimInstance> InAnimClass)
{
	if (BodyMesh)
	{
		BodyMesh->SetAnimInstanceClass(InAnimClass);
	}
}

void ALyraCharacterPreview::SetPreviewYaw(float Yaw)
{
	if (BodyMesh)
	{
		BodyMesh->SetRelativeRotation(FRotator(0.f, -Yaw + BaseYawOffset, 0.f));
	}
}

void ALyraCharacterPreview::SetupAttachmentComponent(UMeshComponent* Comp, FName AttachSocket, const FTransform& RelativeTransform)
{
	Comp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Comp->SetLightingChannels(false, false, true);
	Comp->RegisterComponent();
	Comp->AttachToComponent(BodyMesh, FAttachmentTransformRules::KeepRelativeTransform, AttachSocket);
	Comp->SetRelativeTransform(RelativeTransform);
	Attachments.Add(Comp);
}

void ALyraCharacterPreview::AddSkeletalAttachment(USkeletalMesh* Mesh, FName AttachSocket, const FTransform& RelativeTransform)
{
	if (!Mesh) return;

	USkeletalMeshComponent* Comp = NewObject<USkeletalMeshComponent>(this);
	Comp->SetSkeletalMeshAsset(Mesh);
	SetupAttachmentComponent(Comp, AttachSocket, RelativeTransform);
}

void ALyraCharacterPreview::AddStaticAttachment(UStaticMesh* Mesh, FName AttachSocket, const FTransform& RelativeTransform)
{
	if (!Mesh) return;

	UStaticMeshComponent* Comp = NewObject<UStaticMeshComponent>(this);
	Comp->SetStaticMesh(Mesh);
	SetupAttachmentComponent(Comp, AttachSocket, RelativeTransform);
}

void ALyraCharacterPreview::ClearAttachments()
{
	for (UMeshComponent* Comp : Attachments)
	{
		if (IsValid(Comp))
		{
			Comp->DestroyComponent();
		}
	}
	Attachments.Empty();
}