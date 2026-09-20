#pragma once
#include "CoreMinimal.h"

// Pure rules shared by the playable character and boundary tests.
namespace TripoMovement
{
    inline bool CanUseBufferedJump(double Now, double RequestedAt, double LastGroundedAt,
        bool bGrounded, bool bSpent, double Buffer, double Coyote)
    {
        return !bSpent && Now >= RequestedAt && Now - RequestedAt <= Buffer
            && (bGrounded || (Now >= LastGroundedAt && Now - LastGroundedAt <= Coyote));
    }

    inline float ClampYaw(float Desired, float Anchor, float Limit)
    {
        return FRotator::NormalizeAxis(Anchor + FMath::Clamp(FMath::FindDeltaAngleDegrees(Anchor, Desired), -Limit, Limit));
    }
}
