import unreal,json,shutil,time
from pathlib import Path
root=Path(unreal.Paths.project_dir()).resolve().parent
assert unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world().get_name()=='lv4'
assert not unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
backup=root/'tools/evaluation/focus-backup';backup.mkdir(exist_ok=True)
shutil.copy2(root/'game/Content/Maps/lv4.umap',backup/('lv4-before-solidify-'+time.strftime('%Y%m%d-%H%M%S')+'.umap'))
actors=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
by={a.get_actor_label():a for a in actors.get_all_level_actors()}
count=0
for a in list(by.values()):
 if not a.get_actor_label().startswith('SM_Bld_Wall_'):continue
 for c in a.get_components_by_class(unreal.SkeletalMeshComponent):
  mesh=unreal.TripoCollisionTools.create_rigid_collision_mesh(c.skeletal_mesh_asset,'/Game/Blueprint/Lv4/Collision/'+c.skeletal_mesh_asset.get_name()+'_Rigid')
  assert mesh,a.get_actor_label()
  unreal.EditorAssetLibrary.save_loaded_asset(mesh)
  label='LV4_Solid_'+a.get_name()
  solid=by.get(label) or actors.spawn_actor_from_class(unreal.StaticMeshActor,a.get_actor_location(),a.get_actor_rotation())
  solid.modify();solid.set_actor_label(label);solid.set_folder_path('LV4_Mechanisms/WallCollision')
  solid.set_actor_transform(a.get_actor_transform(),False,True)
  solid.static_mesh_component.set_static_mesh(mesh)
  solid.static_mesh_component.set_collision_profile_name('BlockAll');solid.set_actor_hidden_in_game(True);solid.static_mesh_component.set_visibility(False)
  c.modify();c.set_enable_per_poly_collision(False)
  count+=1
assert unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
print('RIGID WALL COLLISION',count)
