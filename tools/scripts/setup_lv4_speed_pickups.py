import unreal,json,shutil,time
from pathlib import Path
out=Path(unreal.Paths.project_dir()).resolve().parent/'tools/evaluation/speed-pickups'
out.mkdir(parents=True,exist_ok=True)
shutil.copy2(Path(unreal.Paths.project_content_dir())/'Maps/lv4.umap',out/('lv4-before-'+str(int(time.time()))+'.umap'))
s=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
actors=s.get_all_level_actors(); rows=[]
for a in actors:
 if not a.get_class().get_name().startswith(('BP_SpeedUpBubble_Office','BP_SlowDownBubble_Office')):continue
 origin,extent=a.get_actor_bounds(False)
 label='LV4_Pickup_'+a.get_actor_label()
 pickup=next((x for x in actors if x.get_actor_label()==label),None)
 if not pickup:pickup=s.spawn_actor_from_class(unreal.TripoSpeedPickup,origin)
 pickup.set_actor_label(label)
 pickup.set_editor_property('visual_actor',a)
 pickup.set_editor_property('multiplier',.65 if 'SlowDown' in a.get_class().get_name() else 1.5)
 pickup.set_editor_property('duration',6.)
 pickup.volume.set_sphere_radius(max(45.,min(65.,extent.x+15.)))
 rows.append(dict(actor=a.get_actor_label(),location=str(origin),extent=str(extent),multiplier=pickup.multiplier))
assert len(rows)==6,rows
unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True,True)
(out/'setup.json').write_text(json.dumps(rows,indent=2),encoding='utf-8')
print(rows)
