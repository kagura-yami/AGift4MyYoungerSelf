#include "World/TripoCollisionTools.h"
#if WITH_EDITOR
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "PhysicsEngine/BodySetup.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Misc/PackageName.h"
#include "MeshDescription.h"
#endif
UStaticMesh* UTripoCollisionTools::CreateRigidCollisionMesh(USkeletalMesh* Source,const FString& PackagePath)
{
#if WITH_EDITOR
    if(!Source || !PackagePath.StartsWith(TEXT("/Game/Blueprint/Lv4/Collision/"))) return nullptr;
    if(auto* Existing=LoadObject<UStaticMesh>(nullptr,*PackagePath)) return Existing;
    const auto* Description=Source->GetMeshDescription(0);
    if(!Description || Description->Vertices().Num()==0) return nullptr;
    auto* Mesh=NewObject<UStaticMesh>(CreatePackage(*PackagePath),*FPackageName::GetShortName(PackagePath),RF_Public|RF_Standalone);
    for(const auto& Material:Source->GetMaterials()) Mesh->GetStaticMaterials().Add(FStaticMaterial(Material.MaterialInterface,Material.MaterialSlotName,Material.ImportedMaterialSlotName));
    Mesh->CreateBodySetup(); Mesh->GetBodySetup()->CollisionTraceFlag=CTF_UseComplexAsSimple;
    Mesh->GetBodySetup()->bDoubleSidedGeometry=true;
    UStaticMesh::FBuildMeshDescriptionsParams Params; Params.bFastBuild=false; Params.bBuildSimpleCollision=false;
    if(!Mesh->BuildFromMeshDescriptions({Description},Params)) return nullptr;
    FAssetRegistryModule::AssetCreated(Mesh); Mesh->MarkPackageDirty(); return Mesh;
#else
    return nullptr;
#endif
}
