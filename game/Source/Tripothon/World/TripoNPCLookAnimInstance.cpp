#include "World/TripoNPCLookAnimInstance.h"
#include "Animation/AnimSingleNodeInstanceProxy.h"
#include "BonePose.h"

namespace
{
struct FTripoNPCLookProxy final : FAnimSingleNodeInstanceProxy
{
    explicit FTripoNPCLookProxy(UAnimInstance* Instance):FAnimSingleNodeInstanceProxy(Instance) {}
    float Yaw=0,Pitch=0,Torso=0;
    virtual void PreUpdate(UAnimInstance* Instance,float Dt) override
    {
        FAnimSingleNodeInstanceProxy::PreUpdate(Instance,Dt);
        const auto* Look=CastChecked<UTripoNPCLookAnimInstance>(Instance);
        Yaw=Look->GazeYaw; Pitch=Look->GazePitch; Torso=Look->TorsoYaw;
    }
    virtual bool Evaluate(FPoseContext& Output) override
    {
        FAnimSingleNodeInstanceProxy::Evaluate(Output);
        FCSPose<FCompactPose> Pose;
        Pose.InitPose(Output.Pose);
        const auto Rotate=[&](FName Name,float Y,float P)
        {
            FBoneReference Bone; Bone.BoneName=Name; Bone.Initialize(Output.Pose.GetBoneContainer());
            if(!Bone.IsValidToEvaluate(Output.Pose.GetBoneContainer())) return;
            const auto Index=Bone.GetCompactPoseIndex(Output.Pose.GetBoneContainer());
            FTransform Transform=Pose.GetComponentSpaceTransform(Index);
            // These meshes face local +Y. Positive pitch about +X looks up.
            const FQuat Offset=FQuat(FVector::UpVector,FMath::DegreesToRadians(Y))*
                FQuat(FVector::ForwardVector,FMath::DegreesToRadians(P));
            Transform.SetRotation((Offset*Transform.GetRotation()).GetNormalized());
            const FBoneTransform Adjustment(Index,Transform);
            Pose.LocalBlendCSBoneTransforms(MakeArrayView(&Adjustment,1),1.f);
        };
        Rotate(TEXT("spine_03"),Torso,Pitch*.1f);
        Rotate(TEXT("neck_01"),(Yaw-Torso)*.4f,Pitch*.35f);
        Rotate(TEXT("head"),(Yaw-Torso)*.6f,Pitch*.55f);
        FCSPose<FCompactPose>::ConvertComponentPosesToLocalPoses(Pose,Output.Pose);
        return true;
    }
};
}
FAnimInstanceProxy* UTripoNPCLookAnimInstance::CreateAnimInstanceProxy() { return new FTripoNPCLookProxy(this); }
