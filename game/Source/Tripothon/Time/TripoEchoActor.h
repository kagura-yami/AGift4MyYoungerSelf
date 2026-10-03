#pragma once
#include "Player/TripoCharacter.h"
#include "TripoEchoActor.generated.h"
UCLASS()
class TRIPOTHON_API ATripoEchoActor : public ATripoCharacter
{
    GENERATED_BODY()
public:
    ATripoEchoActor(const FObjectInitializer& Initializer = FObjectInitializer::Get());
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Tripo|Echo") int32 EchoNumber = 0;
    virtual void BeginPlay() override;
};
