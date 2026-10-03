import unreal,json
from pathlib import Path
root=Path(unreal.Paths.project_dir()).resolve().parent;out=root/'tools/evaluation/door-pivot'
# Gather original-mesh instances before changing Blueprint defaults. Reruns skip
# components already migrated, so compensation is never applied twice.
import shutil
out.mkdir(exist_ok=True)
source='/Game/TripoModels/rusted_green_door_3d_model/rusted_green_door_3d_model'
target='/Game/Blueprint/Door/SM_GreenDoor_HingePivot'
m=unreal.load_asset(target)
assert m, 'Import the checked-in SM_GreenDoor_HingePivot asset first'
h=unreal.Vector(-4.7363,-1.44176,0)
bps=['/Game/Blueprint/Door/BP_CanOpenDoor','/Game/Blueprint/Door/BP_NotCanOpenDoor','/Game/Maps/School/Blueprint/BP_TheLockedDoor']
maps=['/Game/Maps/HomeVertical/Maps/L_Home_Vertical_01','/Game/Maps/HomeVertical/Maps/L_Home_Vertical_02','/Game/Maps/School/School_v2']
classes=['BP_CanOpenDoor_C','BP_NotCanOpenDoor_C','BP_TheLockedDoor_C']
for package in maps+bps:
 src=root/('game/Content/'+package[6:]+('.umap' if package in maps else '.uasset'))
 if not (out/src.name).exists():shutil.copy2(src,out/src.name)
rows=[]
for package in maps:
 assert unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).load_level(package)
 for a in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors():
  if a.get_class().get_name() not in classes:continue
  for c in a.get_components_by_class(unreal.StaticMeshComponent):
   if not c.static_mesh or c.static_mesh.get_path_name().split('.')[0]!=source:continue
   v=c.get_editor_property('relative_location');sc=c.get_editor_property('relative_scale3d');r=c.get_editor_property('relative_rotation')
   rows.append(dict(map=package,actor=a.get_name(),label=a.get_actor_label(),location=[v.x,v.y,v.z],scale=[sc.x,sc.y,sc.z],rotation=[r.pitch,r.yaw,r.roll]))
(out/'latest-migration.json').write_text(json.dumps(rows,indent=2),encoding='utf-8')
s=unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem);f=unreal.SubobjectDataBlueprintFunctionLibrary
for path in bps:
 bp=unreal.load_asset(path);seen=set()
 for handle in s.k2_gather_subobject_data_for_blueprint(bp):
  c=f.get_object_for_blueprint(f.get_data(handle),bp)
  if not isinstance(c,unreal.StaticMeshComponent) or c.get_name()!='rusted_green_door_3d_model_GEN_VARIABLE' or c.get_path_name() in seen:continue
  seen.add(c.get_path_name())
  if c.static_mesh==m:continue
  v=c.get_editor_property('relative_location');sc=c.get_editor_property('relative_scale3d');new=unreal.Vector(v.x+h.x*sc.x,v.y+h.y*sc.y,v.z)
  bp.modify();c.modify();c.set_editor_property('static_mesh',m);c.set_editor_property('relative_location',new)
 unreal.BlueprintEditorLibrary.compile_blueprint(bp);assert unreal.EditorAssetLibrary.save_loaded_asset(bp,False)
for package in dict.fromkeys(r['map'] for r in rows):
 assert unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).load_level(package)
 world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world();actors={a.get_name():a for a in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()}
 for r in [r for r in rows if r['map']==package]:
  a=actors[r['actor']];c=next(c for c in a.get_components_by_class(unreal.StaticMeshComponent) if c.get_name()=='rusted_green_door_3d_model')
  world.modify();a.modify();c.modify();v=r['location'];sc=r['scale']
  c.set_editor_property('static_mesh',m)
  c.set_editor_property('relative_location',unreal.Vector(v[0]+h.x*sc[0],v[1]+h.y*sc[1],v[2]))
  rot=r['rotation'];c.set_editor_property('relative_rotation',unreal.Rotator(pitch=rot[0],yaw=rot[1],roll=rot[2]))
 assert unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
 print('FIXED',package)
print('HINGE_PIVOT_INSTANCES',len(rows))
