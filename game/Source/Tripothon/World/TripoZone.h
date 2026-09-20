#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TripoZone.generated.h"
class UBoxComponent;
class UPrimitiveComponent;
class UTextRenderComponent;
class UTripoIdentityComponent;
class UTripoChallengeDefinition;
UENUM(BlueprintType)
enum class ETripoZoneKind : uint8 { Checkpoint, Hazard, Start, Finish, BuildAllowed, BuildForbidden, Safe, NPC };
UCLASS()
class TRIPOTHON_API ATripoZone : public AActor
{
    GENERATED_BODY()
public:
    ATripoZone();
    virtual void BeginPlay() override;
    virtual void OnConstruction(const FTransform& Transform) override;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UBoxComponent> Volume;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UTextRenderComponent> Label;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UTripoIdentityComponent> Identity;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) ETripoZoneKind Kind = ETripoZoneKind::Checkpoint;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FString Hint = TEXT("Checkpoint");
    UPROPERTY(EditAnywhere, Instanced, BlueprintReadWrite) TObjectPtr<UTripoChallengeDefinition> Challenge;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName ChallengeId;
    UPROPERTY(EditAnywhere) FVector SafeOffset = FVector(0, 0, 100);
    UPROPERTY(EditAnywhere) TArray<int32> ProtectedLevels;
    bool Contains(const FVector& Point) const;
    static bool Inside(UWorld* World, ETripoZoneKind ZoneKind, const FVector& Point);
private:
    UFUNCTION() void Enter(UPrimitiveComponent* Component, AActor* Other, UPrimitiveComponent* OtherComponent, int32 BodyIndex, bool bSweep, const FHitResult& Hit);
};
