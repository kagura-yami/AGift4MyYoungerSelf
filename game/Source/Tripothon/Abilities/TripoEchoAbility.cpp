#include "Abilities/TripoEchoAbility.h"
#include "Abilities/TripoAbilityComponent.h"
#include "Player/TripoCharacter.h"
#include "Time/TripoEchoActor.h"
#include "Engine/World.h"
#include "EngineUtils.h"
static ATripoCharacter* EchoPlayer(const UObject* Instance)
{ auto* C = Cast<UTripoAbilityComponent>(Instance->GetOuter()); return C ? Cast<ATripoCharacter>(C->GetOwner()) : nullptr; }
ETripoAbilityFailure UTripoEchoAbility::Validate(AActor*, const FTripoAbilityParameters& P) const
{
    auto* Player = EchoPlayer(this); if (!Player) return ETripoAbilityFailure::InvalidContext;
    const auto Clip = Player->History->Clip(P.Duration);
    if (Clip.Num() < 2 || Clip.Last().Time - Clip[0].Time < .1) return ETripoAbilityFailure::NoTarget;
    FCollisionQueryParams Query(SCENE_QUERY_STAT(TripoEchoSpawn), false, Player);
    if (Player->GetWorld()->OverlapBlockingTestByChannel(Clip[0].Transform.GetLocation(), FQuat::Identity, ECC_Pawn, FCollisionShape::MakeCapsule(34,88), Query)) return ETripoAbilityFailure::Blocked;
    return ETripoAbilityFailure::None;
}
ETripoAbilityFailure UTripoEchoAbility::BeginEffect(AActor* Target, const FTripoAbilityParameters& P)
{
    const auto Failure = Validate(Target, P); if (Failure != ETripoAbilityFailure::None) return Failure;
    auto* Player = EchoPlayer(this); const auto Clip = Player->History->Clip(P.Duration);
    for (TActorIterator<ATripoEchoActor> It(Player->GetWorld()); It; ++It) if (It->GetOwner() == Player) It->Destroy();
    auto* NewEcho = Player->GetWorld()->SpawnActorDeferred<ATripoEchoActor>(ATripoEchoActor::StaticClass(), Clip[0].Transform, Player, Player, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
    if (!NewEcho) return ETripoAbilityFailure::Blocked;
    NewEcho->Frames = Clip; NewEcho->Events = Player->History->ClipEvents(Clip[0].Time, Clip.Last().Time);
    NewEcho->FinishSpawning(Clip[0].Transform); Echo = NewEcho; return ETripoAbilityFailure::None;
}
void UTripoEchoAbility::UpdateEffect(double) { if (!Echo.IsValid()) RequestCompletion(); }
void UTripoEchoAbility::EndEffect(bool) { if (Echo.IsValid()) Echo->Destroy(); Echo.Reset(); }
