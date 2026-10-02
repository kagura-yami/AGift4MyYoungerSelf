#pragma once
#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "TripoLocomotionAnimInstance.generated.h"

class UAnimSequence;
UCLASS(Transient, Blueprintable)
class TRIPOTHON_API UTripoLocomotionAnimInstance : public UAnimInstance
{
    GENERATED_BODY()
public:
    UTripoLocomotionAnimInstance();
    UPROPERTY(EditDefaultsOnly, Category="Locomotion") TArray<TObjectPtr<UAnimSequence>> Clips;
    UPROPERTY(Transient, BlueprintReadOnly, Category="Locomotion") float GroundSpeed=0;
    UPROPERTY(Transient, BlueprintReadOnly, Category="Locomotion") int32 MotionState=0;
    UFUNCTION(BlueprintCallable, Category="Locomotion|Authoring") static void FinalizeAuthoredClip(UAnimSequence* Clip);
protected:
    virtual FAnimInstanceProxy* CreateAnimInstanceProxy() override;
    virtual void DestroyAnimInstanceProxy(FAnimInstanceProxy* Proxy) override;
};
