#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TripoHistoryComponent.generated.h"
USTRUCT()
struct FTripoHistoryFrame
{
    GENERATED_BODY()
    UPROPERTY() double Time = 0;
    UPROPERTY() FTransform Transform;
    UPROPERTY() FVector Velocity = FVector::ZeroVector;
    UPROPERTY() FGuid BaseId;
    UPROPERTY() FTransform Relative;
    UPROPERTY() double Phase = 0;
};
USTRUCT()
struct FTripoHistoryEvent
{
    GENERATED_BODY()
    UPROPERTY() double Time = 0;
    UPROPERTY() FGuid EventId;
    UPROPERTY() FGuid TargetId;
};
UCLASS(ClassGroup=(Tripo), meta=(BlueprintSpawnableComponent))
class TRIPOTHON_API UTripoHistoryComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UTripoHistoryComponent();
    virtual void TickComponent(float Delta, ELevelTick TickType, FActorComponentTickFunction* Function) override;
    TArray<FTripoHistoryFrame> Clip(double Seconds) const;
    TArray<FTripoHistoryEvent> ClipEvents(double Start, double End) const;
    static bool Sample(const TArray<FTripoHistoryFrame>& Clip, double Time, FTripoHistoryFrame& Frame);
    bool Resolve(FTripoHistoryFrame& Frame) const;
    void RecordInteraction(FGuid TargetId);
    void PruneAfter(double Time);
    UFUNCTION(BlueprintCallable) void Clear();
    UFUNCTION(BlueprintPure) int32 GetSampleCount() const { return Frames.Num(); }
    bool bReplaying = false;
private:
    UPROPERTY() TArray<FTripoHistoryFrame> Frames;
    UPROPERTY() TArray<FTripoHistoryEvent> Events;
    int64 Epoch = -1;
    double LastSample = -1;
};
