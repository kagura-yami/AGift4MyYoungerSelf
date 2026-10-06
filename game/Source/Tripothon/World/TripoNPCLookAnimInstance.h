#pragma once
#include "CoreMinimal.h"
#include "Animation/AnimSingleNodeInstance.h"
#include "TripoNPCLookAnimInstance.generated.h"

/** Preserves the authored idle while layering a limited head/neck/torso gaze. */
UCLASS(Transient)
class TRIPOTHON_API UTripoNPCLookAnimInstance : public UAnimSingleNodeInstance
{
    GENERATED_BODY()
public:
    float GazeYaw=0.f;
    float GazePitch=0.f;
    float TorsoYaw=0.f;
protected:
    virtual FAnimInstanceProxy* CreateAnimInstanceProxy() override;
};
