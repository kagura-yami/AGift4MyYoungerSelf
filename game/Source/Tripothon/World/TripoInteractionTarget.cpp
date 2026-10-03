#include "World/TripoInteractionTarget.h"
#include "World/TripoElevator.h"
#include "World/TripoGiftBox.h"
#include "World/TripoInteractorComponent.h"
#include "Core/TripoRuntimeSubsystem.h"
#include "Player/TripoCharacter.h"
#include "Components/MeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"
#include "World/TripoMechanism.h"
UTripoInteractionTarget::UTripoInteractionTarget()
{
    InitBoxExtent(FVector(10,12,12));
    BodyInstance.SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    BodyInstance.SetResponseToAllChannels(ECR_Ignore);
    BodyInstance.SetResponseToChannel(ECC_Visibility,ECR_Block);
    SetGenerateOverlapEvents(false); SetCanEverAffectNavigation(false);
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> Glow(TEXT("/Game/Materials/Whitebox/M_InteractionFocus.M_InteractionFocus"));
    HighlightMaterial=Glow.Object;
    Prompt=FText::FromString(TEXT("交互"));
}
bool UTripoInteractionTarget::CanInteract(ATripoCharacter* P) const
{
    const auto* R=UTripoRuntimeSubsystem::GetRuntime(this);
    if(!bEnabled || !IsValid(P) || !P->IsPlayerControlled() || P->Interactor->bSuppressed || !R || R->IsActionPaused() ||
        R->GetRestorePhase()!=ETripoRestorePhase::Running || FVector::DistSquared(P->GetActorLocation(),GetComponentLocation())>FMath::Square(Reach)) return false;
    if(const auto* Lift=Cast<ATripoElevator>(GetOwner()); Lift && !Lift->CanUseTarget(this,P)) return false;
    if(const auto* Gift=Cast<ATripoGiftBox>(GetOwner()); Gift && (!Gift->bEnabled || Gift->bOpened || !Gift->IsInReach(P))) return false;
    if(const auto* Mechanism=Cast<ATripoMechanism>(GetOwner()); Mechanism && Mechanism->Kind!=ETripoMechanismKind::Switch) return false;
    FHitResult Hit; FCollisionQueryParams Q(SCENE_QUERY_STAT(FocusReach),false,P);
    return !GetWorld()->LineTraceSingleByChannel(Hit,P->GetActorLocation(),GetComponentLocation(),ECC_Visibility,Q) || Hit.GetComponent()==this || Hit.GetComponent()==HighlightMesh;
}
bool UTripoInteractionTarget::TryInteract(ATripoCharacter* P)
{
    if(!GetOwner()->HasAuthority() || !CanInteract(P)) return false;
    if(auto* Lift=Cast<ATripoElevator>(GetOwner())) return Lift->UseTarget(this,P);
    if(auto* Gift=Cast<ATripoGiftBox>(GetOwner())) return Gift->TryOpen(P);
    if(auto* Mechanism=Cast<ATripoMechanism>(GetOwner())) return Mechanism->Interact(P);
    OnInteract.Broadcast(P); return true;
}
void UTripoInteractionTarget::SetFocused(bool bFocused)
{
    if(bFocused==bHighlighted) return;
    bHighlighted=bFocused;
    if(!IsValid(HighlightMesh) || !HighlightMaterial) return;
    if(bFocused) { PreviousOverlay=HighlightMesh->GetOverlayMaterial(); HighlightMesh->SetOverlayMaterial(HighlightMaterial); }
    else { HighlightMesh->SetOverlayMaterial(PreviousOverlay); PreviousOverlay=nullptr; }
}
