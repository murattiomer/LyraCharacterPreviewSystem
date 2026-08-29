// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Cosmetics/LyraCosmeticAnimationTypes.h"
#include "Components/ActorComponent.h"
#include "LyraCharacterPreviewComponent.generated.h"

#define UE_API LYRAGAME_API

class APawn;
class UActorComponent;
class USceneCaptureComponent2D;
class UTextureRenderTarget2D;
class ALyraCharacterPreview;
class UAnimInstance;
class IPreviewVisualsProvider;

/**
 * Manages a client-only character preview session.
 */
UCLASS(Blueprintable, meta=(BlueprintSpawnableComponent))
class ULyraCharacterPreviewComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	/** Default constructor. */
	UE_API ULyraCharacterPreviewComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** Called every frame to update the capture component's transform and refresh visuals if requested. */
	UE_API virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	/** Called when the component is registered; spawns the preview actor and render target. */
	UE_API virtual void BeginPlay() override;
	
	/** Called when the component is unregistered; destroys the preview actor and render target. */
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	/** Finds the character preview component on the specified actor, if any. */
	UFUNCTION(BlueprintPure)
	static ULyraCharacterPreviewComponent* FindCharacterPreviewComponent(const AActor* Actor);

	/** Spawns the preview actor and binds to the pawn's visuals provider. */
	UE_API void InitPreview(APawn* SourcePawn);

	/** Destroys the preview actor and unbinds from the provider. */
	UE_API void DestroyPreview();

	/** Enables or disables the SceneCapture, pausing rendering when the preview widget is hidden. */
	UFUNCTION(BlueprintCallable, Category = "Preview")
	UE_API void SetCaptureEnabled(bool bEnabled);

	/** Adjusts the capture distance by Delta steps and repositions the capture component. */
	UFUNCTION(BlueprintCallable, Category = "Preview")
	UE_API void AddZoomDelta(float Delta);

	/** Rotates the preview mesh around the vertical axis by Delta steps. */
	UFUNCTION(BlueprintCallable, Category = "Preview")
	UE_API void AddRotationDelta(float Delta);

	/** Returns true if the preview actor has been successfully spawned. */
	UFUNCTION(BlueprintPure, Category = "Preview")
	UE_API bool IsPreviewReady() const;

	/** Returns the render target written to by the SceneCaptureComponent2D. */
	UFUNCTION(BlueprintPure, Category = "Preview")
	UE_API UTextureRenderTarget2D* GetRenderTarget() const { return RenderTarget; }

	/** Returns the pawn whose appearance is being previewed. */
	UFUNCTION(BlueprintPure, Category = "Preview")
	UE_API APawn* GetSourcePawn() const { return SourcePawnRef; }

protected:

	/** Blueprint class to spawn as the preview actor. */
	UPROPERTY(EditDefaultsOnly, Category = "Preview|Setup")
	TSubclassOf<ALyraCharacterPreview> PreviewActorClass;

	/** Fallback anim selection used when the provider returns none. Picked by cosmetic tags. */
	UPROPERTY(EditDefaultsOnly, Category = "Preview|Setup")
	FLyraAnimLayerSelectionSet PreviewAnimSelection;

	/** World location where the preview actor is spawned, well below the playable area. */
	UPROPERTY(EditDefaultsOnly, Category = "Preview|Setup")
	FVector PreviewSpawnLocation = FVector(0.f, 0.f, -5000.f);

	/** Resolution of the render target used for the character portrait. */
	UPROPERTY(EditDefaultsOnly, Category = "Preview|Setup")
	FIntPoint RenderTargetSize = FIntPoint(1024, 1024);

	/** Relative offset of the capture component from the preview actor root. */
	UPROPERTY(EditDefaultsOnly, Category = "Preview|Setup")
	FVector CaptureOffset = FVector(600.f, 0.f, 120.f);

	/** Rotation of the capture component relative to the preview actor root. */
	UPROPERTY(EditDefaultsOnly, Category = "Preview|Setup")
	FRotator CaptureRotation = FRotator(0.f, -180.f, 0.f);

	/** Field of view of the capture component; narrow values produce a portrait-like result. */
	UPROPERTY(EditDefaultsOnly, Category = "Preview|Setup")
	float CaptureFOV = 15.f;

	/** Minimum zoom distance, clamped to ZoomMin/ZoomMax. */
	UPROPERTY(EditDefaultsOnly, Category = "Preview|Zoom")
	float ZoomMin = 400.f;

	/** Maximum zoom distance, clamped to ZoomMin/ZoomMax. */
	UPROPERTY(EditDefaultsOnly, Category = "Preview|Zoom")
	float ZoomMax = 900.f;

	/** Step size for each zoom delta. */
	UPROPERTY(EditDefaultsOnly, Category = "Preview|Zoom")
	float ZoomStep = 10.f;

	/** Current zoom distance, clamped to ZoomMin/ZoomMax. */
	UPROPERTY()
	float CurrentZoom = 600.f;

	/** Minimum rotation angle, clamped to RotationMin/RotationMax. */
	UPROPERTY(EditDefaultsOnly, Category = "Preview|Rotation")
	float RotationMin = -180.f;
	
	/** Maximum rotation angle, clamped to RotationMin/RotationMax. */
	UPROPERTY(EditDefaultsOnly, Category = "Preview|Rotation")
	float RotationMax = 180.f;

	/** Step size for each rotation delta. */
	UPROPERTY(EditDefaultsOnly, Category = "Preview|Rotation")
	float RotationStep = 0.3f;

	/** Current rotation angle, clamped to RotationMin/RotationMax. */
	UPROPERTY()
	float CurrentRotation = 0.f;

private:

	/** Spawns the preview actor, creates the render target and attaches the SceneCaptureComponent2D. */
	UE_API void SpawnPreviewActor();

	/** Asks the provider for resolved visuals and pushes them onto the preview actor. */
	UE_API void RefreshCharacterPreview();

	/** Queues a refresh on next tick. */
	UE_API void RequestRefresh();

	/** Finds the component on the source pawn that implements IPreviewVisualsProvider. */
	UE_API UActorComponent* FindProviderComponent() const;

	/** Called when the bound provider signals its visuals changed. */
	UE_API void OnProviderVisualsChanged();
	
	/** Called when the bound provider is destroyed. */
	UFUNCTION()
	void OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn);

	/** The preview actor spawned for this component. */
	UPROPERTY()
	TObjectPtr<ALyraCharacterPreview> PreviewActor;

	/** The SceneCaptureComponent2D used to render the preview actor into a render target. */
	UPROPERTY()
	TObjectPtr<USceneCaptureComponent2D> CaptureComponent;

	/** The render target used to hold the preview actor's image. */
	UPROPERTY()
	TObjectPtr<UTextureRenderTarget2D> RenderTarget;

	/** The pawn whose appearance is being previewed. */
	UPROPERTY()
	TObjectPtr<APawn> SourcePawnRef;

	/** The provider component we're bound to (as UObject, cast to interface on use). */
	TWeakObjectPtr<UActorComponent> BoundProviderComponent;

	/** Handle for our subscription to the provider's change delegate. */
	FDelegateHandle ProviderChangedHandle;
};

#undef UE_API