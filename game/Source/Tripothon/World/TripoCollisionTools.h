#pragma once
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "TripoCollisionTools.generated.h"
class USkeletalMesh;
class UStaticMesh;
/** Editor-only asset conversion for imported rigid architecture that has no physics asset. */
UCLASS()
class TRIPOTHON_API UTripoCollisionTools : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category="Tripo|Editor")
    static UStaticMesh* CreateRigidCollisionMesh(USkeletalMesh* Source, const FString& PackagePath);
};
