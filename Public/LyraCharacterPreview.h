// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/RectLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "LyraCharacterPreview.generated.h"

#define UE_API LYRAGAME_API

class USkeletalMeshComponent;
class USkeletalMesh;
class UStaticMesh;
class UMeshComponent;
class UAnimInstance;

/**
 * Client-only display surface: shows a body skeletal mesh plus resolved attachment meshes.
 * Knows nothing about items, equipment, inventory, or replication — it renders what it is handed.
 */
UCLASS()
class ALyraCharacterPreview : public AActor
{
	GENERATED_BODY()

public:
	ALyraCharacterPreview();

	/** Sets the body skeletal mesh. */
	UE_API void SetBodyMesh(USkeletalMesh* InMesh);

	/** Sets the anim instance class driving the body mesh. */
	UE_API void SetBodyAnimClass(TSubclassOf<UAnimInstance> InAnimClass);

	/** Rotates the body mesh to an absolute yaw (degrees). */
	UE_API void SetPreviewYaw(float Yaw);

	/** Attaches a resolved skeletal mesh at the given socket/transform. */
	UE_API void AddSkeletalAttachment(USkeletalMesh* Mesh, FName AttachSocket, const FTransform& RelativeTransform);

	/** Attaches a resolved static mesh at the given socket/transform. */
	UE_API void AddStaticAttachment(UStaticMesh* Mesh, FName AttachSocket, const FTransform& RelativeTransform);

	/** Destroys all spawned attachment components. */
	UE_API void ClearAttachments();

	/** Returns the body mesh component. */
	UE_API USkeletalMeshComponent* GetBodyMesh() const { return BodyMesh; }

protected:
	/** Primary spot light aimed at the character front. */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USpotLightComponent> KeyLight;

	/** Soft fill light from the character's left side. */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<URectLightComponent> FillLight;

private:
	/** Applies preview-only render settings to a freshly created attachment component. */
	void SetupAttachmentComponent(UMeshComponent* Comp, FName AttachSocket, const FTransform& RelativeTransform);

	/** Skeletal mesh component that displays the body. */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USkeletalMeshComponent> BodyMesh;

	/** Base yaw offset so the mesh faces the capture (the old -90). */
	static constexpr float BaseYawOffset = -90.f;

	/** Currently spawned attachment mesh components. */
	UPROPERTY()
	TArray<TObjectPtr<UMeshComponent>> Attachments;
};

#undef UE_API