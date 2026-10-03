#include "Player/TripoPlayerController.h"
#include "InputKeyEventArgs.h"
#include "Lab/TripoHUD.h"
#include "Player/TripoCharacter.h"
#include "Time/TripoEchoActor.h"
#include "Abilities/TripoAbilityComponent.h"
#include "Core/TripoRuntimeSubsystem.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"

void ATripoPlayerController::KeyForTest(FKey Key, bool bPressed)
{
#if !UE_BUILD_SHIPPING
    InputKey(FInputKeyEventArgs::CreateSimulated(Key, bPressed ? IE_Pressed : IE_Released, bPressed ? 1.f : 0.f));
#endif
}
bool ATripoPlayerController::CanUseEchoInput() const
{
    const auto* HUD=Cast<ATripoHUD>(GetHUD());
    const auto* R=UTripoRuntimeSubsystem::GetRuntime(this);
    return GetPawn() && !IsPaused() && (!HUD || !HUD->IsGameplayBlocked()) &&
        (!R || (!R->IsActionPaused() && R->GetRestorePhase()==ETripoRestorePhase::Running));
}
bool ATripoPlayerController::InputKey(const FInputKeyEventArgs& Params)
{
    if (Params.Key==EKeys::C)
    {
        if (Params.Event==IE_Pressed && !bEchoKeyHeld && !bReclaimKeyHeld && !bEchoWheelOpen && CanUseEchoInput())
        { bEchoKeyHeld=true; EchoPressedAt=GetWorld()->GetRealTimeSeconds(); }
        else if (Params.Event==IE_Released)
        {
            const bool bTap=bEchoKeyHeld && !bEchoWheelOpen;
            bEchoKeyHeld=false;
            if (bEchoWheelOpen && !bReclaimWheel) CloseEchoWheel(CanUseEchoInput());
            else if (bTap && CanUseEchoInput()) TapEcho();
        }
        return true;
    }
    if (bEchoWheelOpen && (Params.Key==EKeys::MouseX || Params.Key==EKeys::MouseY))
    {
        MoveEchoWheel(Params.Key==EKeys::MouseX ? FVector2D(Params.AmountDepressed*2.5f,0) : FVector2D(0,-Params.AmountDepressed*2.5f));
        return true;
    }
    if (Params.Key==EKeys::X)
    {
        if (Params.Event==IE_Pressed && !bReclaimKeyHeld && !bEchoKeyHeld && !bEchoWheelOpen && CanUseEchoInput())
        { bReclaimKeyHeld=true; ReclaimPressedAt=GetWorld()->GetRealTimeSeconds(); }
        else if (Params.Event==IE_Released)
        {
            const bool bTap=bReclaimKeyHeld && !bEchoWheelOpen;
            bReclaimKeyHeld=false;
            if (bEchoWheelOpen && bReclaimWheel) CloseEchoWheel(CanUseEchoInput());
            else if (bTap && CanUseEchoInput()) ReclaimEcho();
        }
        return true;
    }
    if (bEchoWheelOpen && Params.Event==IE_Pressed && (Params.Key==EKeys::Escape || Params.Key==EKeys::RightMouseButton))
    { CloseEchoWheel(false); return true; }
    if (Params.Event==IE_Pressed && (Params.Key==EKeys::P || Params.Key==EKeys::F2)) CloseEchoWheel(false);
    if (Params.Key==EKeys::F2)
    {
        if (Params.Event==IE_Pressed) if (auto* HUD=Cast<ATripoHUD>(GetHUD())) HUD->HandleAction(TEXT("abilities.toggle"));
        return true;
    }
    return Super::InputKey(Params);
}
void ATripoPlayerController::FlushPressedKeys()
{
    CloseEchoWheel(false);
    Super::FlushPressedKeys();
}
void ATripoPlayerController::PlayerTick(float DeltaTime)
{
    Super::PlayerTick(DeltaTime);
    if (!CanUseEchoInput()) { CloseEchoWheel(false); return; }
    if (bEchoWheelOpen)
    {
        const auto* R=UTripoRuntimeSubsystem::GetRuntime(this);
        bool bInvalid=!WheelSource.IsValid() || GetPawn()!=WheelSource.Get() || (R && R->GetEpoch()!=WheelEpoch);
        for (auto* Body: GetWheelBodies()) bInvalid |= !IsValid(Body);
        if (bInvalid) CloseEchoWheel(false);
    }
    else if (bEchoKeyHeld && GetWorld()->GetRealTimeSeconds()-EchoPressedAt>=EchoHoldSeconds)
    {
        if (!OpenEchoWheel(false)) bEchoKeyHeld=false;
    }
    else if (bReclaimKeyHeld && GetWorld()->GetRealTimeSeconds()-ReclaimPressedAt>=EchoHoldSeconds)
    {
        if (!OpenEchoWheel(true)) bReclaimKeyHeld=false;
    }
}
int32 ATripoPlayerController::GetEchoCount() const
{ int32 Count=0; for (const auto& Body:EchoBodies) if (IsValid(Body)) ++Count; return Count; }
bool ATripoPlayerController::HasEcho() const { return IsValid(OriginalBody) && GetEchoCount()>0; }
int32 ATripoPlayerController::GetEchoCapacity() const
{
    const auto* Source=Cast<ATripoCharacter>(GetPawn()); FTripoAbilityParameters P;
    return Source && Source->Abilities->GetParameters(ETripoAbility::Echo,P) ? FMath::Max(0,P.Capacity) : 0;
}
TArray<ATripoCharacter*> ATripoPlayerController::GetEchoBodies() const
{
    TArray<ATripoCharacter*> Result;
    if (IsValid(OriginalBody)) Result.Add(OriginalBody);
    else if (auto* Body=Cast<ATripoCharacter>(GetPawn())) Result.Add(Body);
    for (const auto& Body:EchoBodies) if (IsValid(Body)) Result.Add(Body);
    return Result;
}
TArray<ATripoCharacter*> ATripoPlayerController::GetWheelBodies() const
{ TArray<ATripoCharacter*> Result; for (const auto& Body:WheelBodies) Result.Add(Body); return Result; }
bool ATripoPlayerController::CreateEcho()
{
    auto* Source=Cast<ATripoCharacter>(GetPawn());
    if (!Source || bEchoWheelOpen || !CanUseEchoInput() || GetEchoCount()>=GetEchoCapacity()) return false;
    EchoBodies.RemoveAll([](const auto& Body){ return !IsValid(Body); });
    if (!IsValid(OriginalBody)) OriginalBody=Source;
    auto* NewEcho=GetWorld()->SpawnActorDeferred<ATripoEchoActor>(ATripoEchoActor::StaticClass(),Source->GetActorTransform(),OriginalBody,Source,ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
    if (!NewEcho) return false;
    for (auto* Body:GetEchoBodies())
    {
        NewEcho->GetCapsuleComponent()->IgnoreActorWhenMoving(Body,true);
        Body->GetCapsuleComponent()->IgnoreActorWhenMoving(NewEcho,true);
    }
    // Preserve authored ability overrides before component BeginPlay initializes its catalog.
    NewEcho->Abilities->Definitions=Source->Abilities->Definitions;
    NewEcho->EchoNumber=NextEchoNumber++;
    NewEcho->SavedViewRotation=GetControlRotation();
    NewEcho->FinishSpawning(Source->GetActorTransform());
    NewEcho->Abilities->InitializeDefinitions();
    NewEcho->Abilities->ImportLevels(Source->Abilities->ExportLevels());
    NewEcho->Abilities->ImportCooldowns(Source->Abilities->ExportCooldowns());
    NewEcho->GetCharacterMovement()->Velocity=Source->GetVelocity();
    EchoBodies.Add(NewEcho);
    return true;
}
void ATripoPlayerController::TapEcho()
{
    if (auto* Source=Cast<ATripoCharacter>(GetPawn()))
    {
        FGuid Handle;
        if (Source->Abilities->TryActivate(ETripoAbility::Echo,nullptr,Handle)==ETripoAbilityFailure::None && !EchoBodies.IsEmpty())
            PossessEchoBody(EchoBodies.Last());
    }
}
bool ATripoPlayerController::SwitchEcho()
{
    const auto Bodies=GetEchoBodies();
    if (Bodies.Num()<2) return false;
    const int32 Index=Bodies.IndexOfByKey(GetPawn());
    return Index!=INDEX_NONE && PossessEchoBody(Bodies[(Index+1)%Bodies.Num()]);
}
bool ATripoPlayerController::PossessEchoBody(ATripoCharacter* Destination)
{
    auto* Source=Cast<ATripoCharacter>(GetPawn());
    if (!Source || !IsValid(Destination) || !GetEchoBodies().Contains(Destination) || !CanUseEchoInput()) return false;
    if (Source==Destination) { SetViewTarget(Source); return true; }
    const auto Levels=Source->Abilities->ExportLevels();
    auto Cooldowns=Source->Abilities->ExportCooldowns();
    for (auto* Body:GetEchoBodies())
    {
        const auto Other=Body->Abilities->ExportCooldowns();
        for (int32 I=0; I<Cooldowns.Num(); ++I) Cooldowns[I]=FMath::Max(Cooldowns[I],Other[I]);
    }
    for (auto* Body:GetEchoBodies())
    { Body->Abilities->ImportLevels(Levels); Body->Abilities->ImportCooldowns(Cooldowns); }
    const FRotator View=Destination->SavedViewRotation;
    Possess(Destination);
    SetControlRotation(View);
    SetViewTarget(Destination);
    return GetPawn()==Destination;
}
bool ATripoPlayerController::ReclaimEcho(ATripoEchoActor* Body)
{
    if (!CanUseEchoInput()) return false;
    if (!Body) for (int32 I=EchoBodies.Num()-1; I>=0; --I) if (IsValid(EchoBodies[I])) { Body=EchoBodies[I]; break; }
    if (!IsValid(Body) || !EchoBodies.Contains(Body)) return false;
    if (GetPawn()==Body && !PossessEchoBody(OriginalBody)) return false;
    // A view target must be detached before destroying its actor.
    if (GetViewTarget()==Body) SetViewTarget(GetPawn());
    for (auto* Other:GetEchoBodies()) if (Other!=Body) Other->GetCapsuleComponent()->IgnoreActorWhenMoving(Body,false);
    EchoBodies.Remove(Body);
    Body->Destroy();
    return true;
}
bool ATripoPlayerController::OpenEchoWheel(bool bReclaim)
{
    if (bEchoWheelOpen) return true;
    if (!CanUseEchoInput() || (!HasEcho() && GetEchoCapacity()<1)) return false;
    if (bReclaim && !HasEcho()) return false;
    bReclaimWheel=bReclaim;
    WheelBodies.Empty(); for (auto* Body:GetEchoBodies()) if (!bReclaim || Body!=OriginalBody) WheelBodies.Add(Body);
    WheelSource=Cast<ATripoCharacter>(GetPawn());
    WheelView=GetControlRotation();
    WheelSource->SavedViewRotation=WheelView;
    WheelPointer=FVector2D::ZeroVector; WheelSelection=INDEX_NONE;
    if (auto* R=UTripoRuntimeSubsystem::GetRuntime(this)) WheelEpoch=R->GetEpoch();
    bEchoWheelOpen=true;
    return true;
}
void ATripoPlayerController::MoveEchoWheel(FVector2D Delta)
{
    if (!bEchoWheelOpen || Delta.ContainsNaN()) return;
    WheelPointer=(WheelPointer+Delta).GetClampedToMaxSize(180.f);
    int32 Index=INDEX_NONE;
    if (WheelPointer.Size()>45.f && WheelBodies.Num()>0)
    {
        const double Step=2*PI/WheelBodies.Num();
        const double Angle=FMath::Atan2(WheelPointer.Y,WheelPointer.X)+PI/2;
        Index=FMath::FloorToInt(FMath::Fmod(Angle+2*PI+Step/2,2*PI)/Step)%WheelBodies.Num();
    }
    if (Index==WheelSelection) return;
    WheelSelection=Index;
    if (WheelBodies.IsValidIndex(Index) && IsValid(WheelBodies[Index])) SetViewTargetWithBlend(WheelBodies[Index],.12f);
    else if (WheelSource.IsValid()) SetViewTargetWithBlend(WheelSource.Get(),.12f);
}
void ATripoPlayerController::CloseEchoWheel(bool bCommit)
{
    bEchoKeyHeld=false; bReclaimKeyHeld=false;
    if (!bEchoWheelOpen) return;
    auto* Destination=WheelBodies.IsValidIndex(WheelSelection) ? WheelBodies[WheelSelection].Get() : nullptr;
    bEchoWheelOpen=false;
    if (bReclaimWheel)
    {
        SetViewTarget(GetPawn());
        if (bCommit && IsValid(Destination)) ReclaimEcho(Cast<ATripoEchoActor>(Destination));
    }
    else if (!bCommit || !IsValid(Destination) || !PossessEchoBody(Destination))
    { SetViewTarget(GetPawn()); if (GetPawn()==WheelSource.Get()) SetControlRotation(WheelView); }
    WheelBodies.Empty(); WheelSource.Reset(); WheelSelection=INDEX_NONE; WheelPointer=FVector2D::ZeroVector;
}
