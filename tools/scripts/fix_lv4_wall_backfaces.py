"""Give LV4's open-backed wall modules a dedicated two-sided material."""
import unreal,json,shutil,time
from pathlib import Path
root=Path(unreal.Paths.project_dir()).resolve().parent
ls=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
assert unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world() is None
assert unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world().get_name()=='lv4'
out=root/'tools/evaluation/surface-audit';out.mkdir(exist_ok=True)
shutil.copy2(root/'game/Content/Maps/lv4.umap',out/('lv4-before-wall-batch-'+time.strftime('%Y%m%d-%H%M%S')+'.umap'))
source='/Game/Models/lv4/caizhi1_shili'
path='/Game/Materials/Lv4/MI_Wall_DoubleSided'
material=unreal.load_asset(path) or unreal.EditorAssetLibrary.duplicate_asset(source,path)
overrides=material.get_editor_property('base_property_overrides')
overrides.set_editor_property('override_two_sided',True)
overrides.set_editor_property('two_sided',True)
material.set_editor_property('base_property_overrides',overrides)
unreal.MaterialEditingLibrary.update_material_instance(material)
assert unreal.EditorAssetLibrary.save_loaded_asset(material)
meshes={'/Game/Models/lv4/SM_Bld_Wall_Baseboard_01.SM_Bld_Wall_Baseboard_01','/Game/Models/lv4/SM_Bld_Wall_Baseboard_Door_01.SM_Bld_Wall_Baseboard_Door_01'}
changed=[]
for actor in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors():
 for c in actor.get_components_by_class(unreal.MeshComponent):
  mesh=c.static_mesh if isinstance(c,unreal.StaticMeshComponent) else (c.skeletal_mesh_asset if isinstance(c,unreal.SkeletalMeshComponent) else None)
  if not mesh or mesh.get_path_name() not in meshes:continue
  for i in range(c.get_num_materials()):
   old=c.get_material(i)
   if old and old.get_path_name().split('.')[0] in {source,path}:
    actor.modify();c.modify();c.set_material(i,material)
    changed.append(dict(actor=actor.get_actor_label(),mesh=mesh.get_path_name(),slot=i))
assert ls.save_current_level()
(out/'wall-backface-fix.json').write_text(json.dumps(changed,ensure_ascii=False,indent=2),encoding='utf-8')
print('Wall material slots repaired:',len(changed))
