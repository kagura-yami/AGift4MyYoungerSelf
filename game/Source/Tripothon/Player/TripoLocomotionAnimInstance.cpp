#include "Player/TripoLocomotionAnimInstance.h"
#include "Animation/AnimInstanceProxy.h"
#include "Animation/AnimNode_SequencePlayer.h"
#include "Animation/AnimSequence.h"
#include "AnimationRuntime.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UObject/ConstructorHelpers.h"
#if WITH_EDITOR
#include "Animation/AnimData/IAnimationDataController.h"
#endif

namespace
{
struct FTripoMotionNode : FAnimNode_Base
{
    FAnimNode_SequencePlayer_Standalone Players[4];
    float Weights[4]={1,0,0,0};
    int32 State=0,PreviousState=0;
    float Speed=0;
    virtual void Initialize_AnyThread(const FAnimationInitializeContext& Context) override
    {
        for(auto& Player:Players)Player.Initialize_AnyThread(Context);
        Weights[0]=1;for(int32 i=1;i<4;++i)Weights[i]=0;
    }
    virtual void CacheBones_AnyThread(const FAnimationCacheBonesContext& Context) override
    {for(auto& Player:Players)Player.CacheBones_AnyThread(Context);}
    virtual void Update_AnyThread(const FAnimationUpdateContext& Context) override
    {
        if(State!=PreviousState && State>=2)Players[State].SetAccumulatedTime(0);
        PreviousState=State;
        Players[1].SetPlayRate(FMath::Clamp(Speed/300.f,.65f,1.8f));
        const float Alpha=1.f-FMath::Exp(-Context.GetDeltaTime()*18.f);
        for(int32 i=0;i<4;++i)
        {
            Weights[i]=FMath::Lerp(Weights[i],i==State?1.f:0.f,Alpha);
            Players[i].Update_AnyThread(Context.FractionalWeight(Weights[i]));
        }
    }
    virtual void Evaluate_AnyThread(FPoseContext& Output) override
    {
        Players[0].Evaluate_AnyThread(Output);float Sum=Weights[0];
        for(int32 i=1;i<4;++i)
        {
            if(Weights[i]<SMALL_NUMBER)continue;
            FPoseContext Next(Output),Blended(Output);Players[i].Evaluate_AnyThread(Next);
            FAnimationPoseData A(Output),B(Next),Result(Blended);
            FAnimationRuntime::BlendTwoPosesTogether(A,B,Sum/(Sum+Weights[i]),Result);
            Output=MoveTemp(Blended);Sum+=Weights[i];
        }
    }
};
struct FTripoMotionProxy : FAnimInstanceProxy
{
    FTripoMotionNode Root;
    FTripoMotionProxy(UAnimInstance* Instance):FAnimInstanceProxy(Instance){}
    virtual void Initialize(UAnimInstance* Instance) override
    {
        auto* Anim=CastChecked<UTripoLocomotionAnimInstance>(Instance);
        for(int32 i=0;i<4;++i){Root.Players[i].SetSequence(Anim->Clips.IsValidIndex(i)?Anim->Clips[i].Get():nullptr);Root.Players[i].SetLoopAnimation(i<2||i==3);}
        FAnimInstanceProxy::Initialize(Instance);
    }
    virtual FAnimNode_Base* GetCustomRootNode() override{return &Root;}
    virtual void PreUpdate(UAnimInstance* Instance,float DeltaSeconds) override
    {
        FAnimInstanceProxy::PreUpdate(Instance,DeltaSeconds);
        if(auto* Character=Cast<ACharacter>(Instance->TryGetPawnOwner()))
        {
            Root.Speed=Character->GetVelocity().Size2D();
            const bool Air=Character->GetCharacterMovement()->IsFalling();
            Root.State=Air?(Character->GetVelocity().Z>20.f?2:3):(Root.Speed>(Root.State==1?5.f:12.f)?1:0);
            auto* Anim=CastChecked<UTripoLocomotionAnimInstance>(Instance);Anim->GroundSpeed=Root.Speed;Anim->MotionState=Root.State;
        }
    }
};
}
UTripoLocomotionAnimInstance::UTripoLocomotionAnimInstance()
{
    static ConstructorHelpers::FObjectFinder<UAnimSequence> Idle(TEXT("/Game/Characters/Animations/A_Tripo_Idle"));
    static ConstructorHelpers::FObjectFinder<UAnimSequence> Move(TEXT("/Game/Characters/Animations/A_Tripo_Move"));
    static ConstructorHelpers::FObjectFinder<UAnimSequence> Jump(TEXT("/Game/Characters/Animations/A_Tripo_Jump"));
    static ConstructorHelpers::FObjectFinder<UAnimSequence> Fall(TEXT("/Game/Characters/Animations/A_Tripo_Fall"));
    Clips={Idle.Object,Move.Object,Jump.Object,Fall.Object};
}
FAnimInstanceProxy* UTripoLocomotionAnimInstance::CreateAnimInstanceProxy(){return new FTripoMotionProxy(this);}
void UTripoLocomotionAnimInstance::DestroyAnimInstanceProxy(FAnimInstanceProxy* Proxy){delete Proxy;}
void UTripoLocomotionAnimInstance::FinalizeAuthoredClip(UAnimSequence* Clip)
{
#if WITH_EDITOR
    if(Clip){Clip->GetController().NotifyPopulated();Clip->MarkPackageDirty();}
#endif
}
