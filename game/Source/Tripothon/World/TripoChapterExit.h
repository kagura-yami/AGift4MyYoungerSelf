#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TripoChapterExit.generated.h"
class UBoxComponent;

/** Walk-in chapter exit. Uses the existing progress-aware travel path. */
UCLASS(Blueprintable)
class TRIPOTHON_API ATripoChapterExit : public AActor
{
    GENERATED_BODY()
public:
    ATripoChapterExit();
    virtual void Tick(float DeltaSeconds) override;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Exit") TObjectPtr<UBoxComponent> Volume;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Exit") FName Destination;
private:
    bool bTravelling=false;
};
