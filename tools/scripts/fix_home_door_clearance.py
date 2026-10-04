import unreal,shutil,json
from pathlib import Path
root=Path(unreal.Paths.project_dir()).resolve(); backup=root.parent/'tools/evaluation/home-door-clearance';backup.mkdir(exist_ok=True)
ls=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem);es=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
report=[]
for name in ['L_Home_Vertical_01','L_Home_Vertical_02']:
 src=root/'Content/Maps/HomeVertical/Maps'/f'{name}.umap'
 if not (backup/src.name).exists():shutil.copy2(src,backup/src.name)
 ls.load_level('/Game/Maps/HomeVertical/Maps/'+name)
 actors=es.get_all_level_actors()
 for a in actors:
  if 'CanOpenDoor' not in a.get_class().get_name():continue
  frame=next((c for c in a.get_components_by_class(unreal.StaticMeshComponent) if 'frame' in c.get_name()),None)
  if not frame:continue
  mn,mx=frame.get_local_bounds();sc=frame.get_world_scale();loc=frame.get_world_location()
  oldtop=loc.z+mx.z*sc.z
  height=(mx.z-mn.z)*sc.z
  if height>=200:continue
  factor=231/height;scale=a.get_actor_scale3d()
  newtop=loc.z+(oldtop-loc.z)*factor
  # Only lift the lower edge of the lintel immediately above this doorway.
  for wall in actors:
   if not isinstance(wall,unreal.StaticMeshActor):continue
   m=wall.static_mesh_component.static_mesh
   if not m or m.get_path_name()!='/Engine/BasicShapes/Cube.Cube':continue
   center,ext=wall.get_actor_bounds(False);bottom=center.z-ext.z;top=center.z+ext.z
   if abs(bottom-oldtop)>15 or abs(center.x-loc.x)>ext.x+12 or abs(center.y-loc.y)>ext.y+12:continue
   if min(ext.x,ext.y)>45 or top<=newtop+20:continue
   wall.modify();ws=wall.get_actor_scale3d();ws.z*= (top-newtop)/(2*ext.z)
   wl=wall.get_actor_location();wl.z+=(newtop-bottom)/2
   wall.set_actor_scale3d(ws);wall.set_actor_location(wl,False,True)
   report.append([name,wall.get_actor_label(),'lintel',bottom,newtop])
  a.modify();scale.z*=factor;a.set_actor_scale3d(scale)
  report.append([name,a.get_actor_label(),'door height',height,231])
 ls.save_current_level()
print(report)
(backup/'changes.json').write_text(json.dumps(report,ensure_ascii=False,indent=2),encoding='utf-8')
