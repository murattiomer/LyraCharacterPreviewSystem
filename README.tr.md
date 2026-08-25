# LyraCharacterPreview

Lyra tabanlı, client-only bir karakter önizleme (character preview) sistemi. Oyuncunun mevcut pawn'ını ve loadout'unu (equipment) UI'da render target üzerinden gösterir — envanter ekranları, karakter kartları, ölüm ekranı gibi yerlerde kullanılır.

## Amaç

Bir cümle: **Client tarafında bir preview session'ı yönetir, pawn'ın loadout'unu resolved mesh'ler olarak toplar, gizli bir preview actor'a bağlar ve SceneCapture üzerinden UI'ın kullanabileceği bir render target üretir.**

## Sistem akışı

![Sistem akışı](images/screenshot_1.png)

## Bileşenler

| Sınıf | Nerede yaşar | Tek cümle sorumluluğu |
|---|---|---|
| `ALyraCharacterPreview` | World (gizli konumda spawn) | Verilen body mesh + attachment mesh'leri gösterir. |
| `ULyraCharacterPreviewComponent` | PlayerController | Preview session'ını yönetir: actor'ı spawn eder, SceneCapture'ı sürer, provider'a bağlanır. |
| `IPreviewVisualsProvider` | — (interface) | Loadout kaynağı ile preview arasındaki kontrat. |
| `ULyraEquipmentPreviewProvider` | Pawn (GFA ile eklenir) | Lyra equipment sistemini `IPreviewVisualsProvider` kontratına adapte eder. |
| `FCharacterPreviewVisuals` | — (struct) | Preview'ın anladığı dil: anim class + attachment mesh listesi. |
| `FPreviewAttachmentSpec` | — (struct) | Tek bir attachment: mesh + socket + relative transform. |

## Bağımlılık yönleri

![Bağımlılık yönleri](images/screenshot_2.png)

Kural: **oklar tek yönlüdür.**

- Preview → Equipment yönünde bilgi akışı **yok**. Preview component `Equipment/*.h` include etmez.
- Equipment → Preview yönünde bilgi akışı **yok**. Equipment manager preview'ın varlığından habersizdir.
- Provider tek "bilgi yoğunlaşma" (knowledge concentration) noktasıdır: equipment'ı bilir, preview kontratını implement eder. Bilgi asimetrisini emen tampondur.

## Nasıl çalışır — detaylı akış

### 1. Kurulum (PC spawn olduğunda)

`ULyraCharacterPreviewComponent::BeginPlay` içinde:
- `OnPossessedPawnChanged`'e abone olunur.
- Mevcut pawn varsa `InitPreview(Pawn)` çağrılır.

`InitPreview`:
1. `ALyraCharacterPreview` gizli bir konumda (varsayılan `Z=-5000`) spawn edilir.
2. `UTextureRenderTarget2D` ve `USceneCaptureComponent2D` yaratılır; capture sadece preview actor'ı gösterecek şekilde `ShowOnlyActors`'a eklenir.
3. Pawn'ın kendi skeletal mesh'i preview'ın body mesh'ine set edilir.
4. Pawn üzerinde `IPreviewVisualsProvider` implement eden component `FindComponentByInterface` ile bulunur.
5. Provider'ın `OnPreviewVisualsChanged` delegate'ine subscribe olunur.
6. İlk `RefreshCharacterPreview` sıraya konur.

### 2. Refresh döngüsü

Provider iki gameplay message'a abonedir:
- `Lyra.QuickBar.Message.ActiveIndexChanged` — silah değişti.
- `Lyra.Equipment.Message.VisibilityChanged` — görünürlük değişti.

Herhangi biri geldiğinde provider `VisualsChangedDelegate.Broadcast()` yapar. Preview component sonraki tick için `RefreshCharacterPreview`'ı sıraya koyar. Refresh:
1. Preview actor'daki tüm eski attachment'ları destroy eder.
2. Provider'a `GatherPreviewVisuals(Out)` sorar.
3. Provider equipment manager'ı okur, `ULyraEquipmentInstance` listesindeki her item için `FLyraEquipmentActorToSpawn` bilgisini çözer, mesh asset'lerini alır, `FPreviewAttachmentSpec` listesini doldurur; elde silah varsa item def'inden preview anim class'ını okur.
4. Dönen spec listesini gezip `AddSkeletalAttachment` / `AddStaticAttachment` çağırır.
5. Anim class'ı body mesh'e uygular.

### 3. Render

Widget açıkken preview component'in tick'i açılır ve her frame `CaptureScene()` çağrılır. Render target UI'da texture olarak gösterilir. Widget kapanınca `SetCaptureEnabled(false)` → tick durur, GPU boşta kalır.

### 4. Pawn değişimi (ölüm/respawn)

`OnPossessedPawnChanged` tetiklenir → mevcut preview `DestroyPreview` ile temizlenir → yeni pawn için `InitPreview` yeniden çalışır.

## Nasıl kullanılır

1. **Preview component'i PC'ye ekle.** `ULyraCharacterPreviewComponent`'i client-only Game Feature Action ile hedef PlayerController'a (örn. `ALyraPlayerController`) ekle. GFA target'ı olarak subclass değil, base class kullan — cross-GFP coupling'i önler.
2. **Provider'ı pawn'a ekle.** `ULyraEquipmentPreviewProvider`'ı aynı Game Feature Action ile `ALyraCharacter`'a ekle.
3. **UI'da render target'ı göster.** Widget içinden `GetRenderTarget()` ile texture'ı al, bir `Image` brush'ına ver.
4. **Widget lifecycle'ında capture'ı aç/kapat.** Widget construct'ta `SetCaptureEnabled(true)`, destruct'ta `SetCaptureEnabled(false)`.
5. **UI etkileşimleri:** `AddZoomDelta(Delta)` ve `AddRotationDelta(Delta)` fonksiyonlarını mouse/touch input'una bağla.

## Genişletme

Yeni bir preview kaynağı eklemek için:

1. `IPreviewVisualsProvider` implement eden yeni bir component yaz.
2. `GatherPreviewVisuals` içinde kendi kaynağını `FCharacterPreviewVisuals`'a çevir.
3. Kaynağın değişiklik olaylarını dinle, değişince `VisualsChangedDelegate.Broadcast()` çağır.
4. GFA ile ilgili pawn tipine ekle.

Preview component ve preview actor **hiç değişmez**. Örnek genişleme senaryoları: cosmetic-only provider (skin/kıyafet), NPC preview provider, mount preview provider.

## Ne bilinmez — tablo

| Sınıf | Bilmez |
|---|---|
| `ALyraCharacterPreview` | item, equipment, inventory, replication, ASC |
| `ULyraCharacterPreviewComponent` | equipment, inventory, gameplay message tipleri, item def'leri |
| `IPreviewVisualsProvider` | herhangi bir concrete kaynak sistemi |
| `ULyraEquipmentManagerComponent` | preview'ın varlığı, provider'ın varlığı |

## Notlar

- **Client-only.** Sistem Game Feature üzerinden yalnızca client'a eklenir; `ALyraCharacterPreview` zaten `bReplicates = false` ve client'ta spawn olur.
- **ASC/GAS kullanılmaz.** Preview salt görseldir; ASC, GameplayEffect veya attribute set gerektirmez. Cost discipline gereği en hafif yol seçilmiştir.
- **Provider'ın adı yanıltıcı olabilir.** "Provider" yalnızca pull çağrışımı yapar; bu sınıf hem pull (`GatherPreviewVisuals`) hem push (`OnPreviewVisualsChanged`) yapar. Pattern olarak **Adapter + Observable**.
- **Refresh her seferinde tam teardown yapar.** Attachment sayısı büyürse diff-based refresh düşünülebilir; şu an ölçülmüş bir problem yok, premature optimization'dan kaçınıldı.