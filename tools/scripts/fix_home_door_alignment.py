import unreal,json,shutil
from pathlib import Path
root=Path('C:/Dev/Projects/UE/Tripothon')
backup=root/'tools/evaluation/home-door-backup';backup.mkdir(exist_ok=True)
results=[]
for suffix in ['01','02']:
 package='/Game/Maps/HomeVertical/Maps/L_Home_Vertical_'+suffix
 file=root/('game/Content/Maps/HomeVertical/Maps/L_Home_Vertical_'+suffix+'.umap')
 dest=backup/file.name
 if not dest.exists():shutil.copy2(file,dest)
 assert unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).load_level(package)
 world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
 for actor in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors():
  if actor.get_class().get_name() not in ['BP_CanOpenDoor_C','BP_NotCanOpenDoor_C']:continue
  # Ground-floor doorway: the opening is 112.6 x 223.5 cm, not the
  # 84.2 x 165.2 cm produced by the old non-uniform 0.75 actor scale.
  if actor.get_actor_label()=='BP_NotCanOpenDoor2' and abs(actor.get_actor_location().x+453.483755)<1:
   world.modify();actor.modify()
   actor.set_actor_scale3d(unreal.Vector(1,1.025,1.025))
  for c in actor.get_components_by_class(unreal.StaticMeshComponent):
   if c.get_name()!='rusted_green_door_3d_model':continue
   if c.static_mesh and c.static_mesh.get_name()=='SM_GreenDoor_HingePivot':continue
   old=c.get_editor_property('relative_location')
   if abs(old.z-147.961041)>.05 or abs(old.y-68.332072)>.05:continue
   before=[old.x,old.y,old.z]
   world.modify();actor.modify();c.modify()
   c.set_editor_property('relative_location',unreal.Vector(-5.229419,50.166752,108.008867))
   results.append({'map':package,'actor':actor.get_actor_label(),'before':before,'after':[-5.229419,50.166752,108.008867]})
 assert unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
(root/'tools/evaluation/home-door-fix.json').write_text(json.dumps(results,ensure_ascii=False,indent=2),encoding='utf-8')
print('FIXED',len(results))
