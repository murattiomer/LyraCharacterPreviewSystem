# LyraCharacterPreview

A Lyra-based, client-only character preview system. Displays the player's current pawn and loadout (equipment) in the UI via a render target — used in inventory screens, character cards, death screens, and similar places.

## Purpose

In one sentence: **Manages a client-side preview session; gathers the pawn's loadout as resolved meshes, attaches them to a hidden preview actor, and produces a render target the UI can consume via SceneCapture.**

## System flow

![System flow](https://raw.githubusercontent.com/murattiomer/LyraCharacterPreviewSystem/main/Images/Screenshot_1.png)

## Components

| Class | Lives on | Single-sentence responsibility |
|---|---|---|
| `ALyraCharacterPreview` | World (spawned at a hidden location) | Displays the given body mesh and attachment meshes. |
| `ULyraCharacterPreviewComponent` | PlayerController | Manages the preview session: spawns the actor, drives the SceneCapture, binds to the provider. |
| `IPreviewVisualsProvider` | — (interface) | The contract between the loadout source and the preview. |
| `ULyraEquipmentPreviewProvider` | Pawn (added via GFA) | Adapts the Lyra equipment system to the `IPreviewVisualsProvider` contract. |
| `FCharacterPreviewVisuals` | — (struct) | The preview's language: anim class + attachment mesh list. |
| `FPreviewAttachmentSpec` | — (struct) | One attachment: mesh + socket + relative transform. |

## Dependency directions

![Dependency directions](https://raw.githubusercontent.com/murattiomer/LyraCharacterPreviewSystem/main/Images/Screenshot_2.png)

The rule: **arrows are one-way.**

- No information flow from Preview to Equipment. The preview component does not include `Equipment/*.h`.
- No information flow from Equipment to Preview. The equipment manager is unaware the preview exists.
- The provider is the single **knowledge concentration** point: it knows equipment and implements the preview contract. It's the buffer that absorbs the information asymmetry.

## How it works — detailed flow

### 1. Setup (when the PC spawns)

In `ULyraCharacterPreviewComponent::BeginPlay`:
- Subscribes to `OnPossessedPawnChanged`.
- If a pawn already exists, calls `InitPreview(Pawn)`.

`InitPreview`:
1. Spawns `ALyraCharacterPreview` at a hidden location (default `Z=-5000`).
2. Creates a `UTextureRenderTarget2D` and a `USceneCaptureComponent2D`; the capture is limited to the preview actor via `ShowOnlyActors`.
3. Sets the preview's body mesh to the pawn's own skeletal mesh.
4. Finds the component implementing `IPreviewVisualsProvider` on the pawn via `FindComponentByInterface`.
5. Subscribes to the provider's `OnPreviewVisualsChanged` delegate.
6. Queues the first `RefreshCharacterPreview`.

### 2. Refresh cycle

The provider listens to two gameplay messages:
- `Lyra.QuickBar.Message.ActiveIndexChanged` — active weapon changed.
- `Lyra.Equipment.Message.VisibilityChanged` — visibility changed.

On either message, the provider calls `VisualsChangedDelegate.Broadcast()`. The preview component queues `RefreshCharacterPreview` for the next tick. Refresh:
1. Destroys all existing attachments on the preview actor.
2. Asks the provider via `GatherPreviewVisuals(Out)`.
3. The provider reads the equipment manager, iterates each `ULyraEquipmentInstance`, resolves `FLyraEquipmentActorToSpawn` info, pulls the mesh assets, fills the `FPreviewAttachmentSpec` list; if a weapon is held, it also pulls the preview anim class from the item def.
4. Iterates the returned spec list and calls `AddSkeletalAttachment` / `AddStaticAttachment`.
5. Applies the anim class to the body mesh.

### 3. Render

While the widget is open, the preview component's tick is enabled and `CaptureScene()` is called each frame. The render target is shown as a texture in the UI. When the widget closes, `SetCaptureEnabled(false)` stops the tick and frees the GPU.

### 4. Pawn changes (death/respawn)

`OnPossessedPawnChanged` fires → the current preview is cleaned up via `DestroyPreview` → `InitPreview` runs again for the new pawn.

## How to use

1. **Add the preview component to the PC.** Attach `ULyraCharacterPreviewComponent` to your PlayerController (e.g. `ALyraPlayerController`) via a client-only Game Feature Action. Target the base class, not a subclass, to avoid cross-GFP coupling.
2. **Add the provider to the pawn.** Attach `ULyraEquipmentPreviewProvider` to `ALyraCharacter` via the same Game Feature Action.
3. **Show the render target in your UI.** Grab the texture via `GetRenderTarget()` from your widget and feed it to an `Image` brush.
4. **Toggle capture with widget lifecycle.** Call `SetCaptureEnabled(true)` in the widget's construct, `SetCaptureEnabled(false)` in destruct.
5. **UI interactions:** wire `AddZoomDelta(Delta)` and `AddRotationDelta(Delta)` to mouse/touch input.

## Extension

To add a new preview source:

1. Write a new component that implements `IPreviewVisualsProvider`.
2. In `GatherPreviewVisuals`, translate your source into `FCharacterPreviewVisuals`.
3. Listen to your source's change events and call `VisualsChangedDelegate.Broadcast()` when they fire.
4. Add it to the target pawn class via a GFA.

The preview component and preview actor **do not change**. Example extension scenarios: cosmetic-only provider (skin/clothing), NPC preview provider, mount preview provider.

## What is not known — table

| Class | Doesn't know about |
|---|---|
| `ALyraCharacterPreview` | items, equipment, inventory, replication, ASC |
| `ULyraCharacterPreviewComponent` | equipment, inventory, gameplay message types, item defs |
| `IPreviewVisualsProvider` | any concrete source system |
| `ULyraEquipmentManagerComponent` | the preview's existence, the provider's existence |

## Notes

- **Client-only.** The system is added client-side only via a Game Feature; `ALyraCharacterPreview` already has `bReplicates = false` and only spawns on clients.
- **No ASC/GAS.** The preview is purely visual; it doesn't need an ASC, GameplayEffects, or attribute sets. The lightest tool for the job, per cost discipline.
- **The name "provider" may mislead.** "Provider" suggests pull-only, but this class does both pull (`GatherPreviewVisuals`) and push (`OnPreviewVisualsChanged`). The pattern is really **Adapter + Observable**.
- **Refresh does a full teardown each time.** If attachment counts grow, diff-based refresh could be considered; there's no measured problem today, so premature optimization was avoided.