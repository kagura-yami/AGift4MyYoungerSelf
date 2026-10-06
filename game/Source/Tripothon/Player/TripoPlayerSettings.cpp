#include "Player/TripoPlayerSettings.h"
#include "Engine/World.h"
#include "AudioDevice.h"
void UTripoPlayerSettings::ApplyAudio(UObject* Context)
{
    if (Context && Context->GetWorld())
        if (auto Device=Context->GetWorld()->GetAudioDevice()) Device->SetTransientPrimaryVolume(FMath::Clamp(MasterVolume,0.f,1.f));
}
void UTripoPlayerSettings::SetVolume(float Value, UObject* Context)
{ MasterVolume=FMath::Clamp(Value,0.f,1.f); ApplyAudio(Context); SaveConfig(); }
void UTripoPlayerSettings::SetSensitivity(float Value)
{ MouseSensitivity=FMath::Clamp(Value,.25f,3.f); SaveConfig(); }
void UTripoPlayerSettings::SetInvertY(bool Value)
{ bInvertLookY=Value; SaveConfig(); }
