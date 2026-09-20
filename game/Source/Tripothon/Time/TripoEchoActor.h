#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Time/TripoHistoryComponent.h"
#include "TripoEchoActor.generated.h"
class UCapsuleComponent;
class UStaticMeshComponent;
class UTripoInteractorComponent;
class UTripoIdentityComponent;
UCLASS()
class TRIPOTHON_API ATripoEchoActor : public AActor
{
    GENERATED_BODY()
public:
    ATripoEchoActor();
    virtual void BeginPlay() override;
    virtual void Tick(float Delta) override;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UCapsuleComponent> Capsule;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> Body;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UTripoInteractorComponent> Interactor;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UTripoIdentityComponent> Identity;
    UPROPERTY() TArray<FTripoHistoryFrame> Frames;
    UPROPERTY() TArray<FTripoHistoryEvent> Events;
private:
    TSet<FGuid> Played;
    double StartedAt = 0;
    double Cursor = 0;
    int64 Epoch = 0;
};
