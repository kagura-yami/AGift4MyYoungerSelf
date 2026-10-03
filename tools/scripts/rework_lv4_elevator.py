"""Separate fixed landing architecture from the cabin. Retain originals and back up the current map."""
import unreal,json,shutil,time
from pathlib import Path
root=Path(unreal.Paths.project_dir()).resolve().parent
folder=root/'tools/evaluation/elevator-rework';folder.mkdir(exist_ok=True)
ls=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem);actors=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
assert not unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
assert unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world().get_name()=='lv4'
shutil.copy2(root/'game/Content/Maps/lv4.umap',folder/('lv4-before-rework-'+time.strftime('%Y%m%d-%H%M%S')+'.umap'))
by={a.get_actor_label():a for a in actors.get_all_level_actors()};e=by['LV4_WestElevator'];shell=by['SM_Bld_Elevator_01'];e.modify();shell.modify()
source=unreal.load_asset('/Game/Models/lv4/SM_Bld_Elevator_01');md=source.get_static_mesh_description(0)
# Identify the connected front jamb island; all front-facing planar fascia stay with the building too.
rows=[];parent=list(range(md.get_polygon_count()));seen={}
def find(i):
 while parent[i]!=i:parent[i]=parent[parent[i]];i=parent[i]
 return i
for i in range(md.get_polygon_count()):
 pts=[md.get_vertex_position(v) for v in md.get_polygon_vertices(unreal.PolygonID(i))];rows.append(pts)
 for p in pts:
  key=(round(p.x,2),round(p.y,2),round(p.z,2))
  if key in seen:parent[find(i)]=find(seen[key])
  else:seen[key]=i
groups={}
for i,pts in enumerate(rows):groups.setdefault(find(i),[]).extend(pts)
jambs={g for g,pts in groups.items() if min(p.y for p in pts)>-12 and max(p.z for p in pts)>280 and min(p.z for p in pts)<1 and min(p.x for p in pts)>45 and max(p.x for p in pts)<205}
assert len(jambs)==1
frame={i for i,pts in enumerate(rows) if find(i) in jambs or min(p.y for p in pts)>=-7.3}
parts={}
for name,is_frame in [('SM_Lv4FixedFrame',True),('SM_Lv4MovingCabin',False)]:
 path='/Game/Blueprint/Lv4/'+name;mesh=unreal.load_asset(path)
 if not mesh:
  mesh=unreal.EditorAssetLibrary.duplicate_asset(source.get_path_name().split('.')[0],path)
  description=mesh.get_static_mesh_description(0)
  for i in range(len(rows)):
   if (i in frame)!=is_frame:description.delete_polygon(unreal.PolygonID(i))
  mesh.build_from_static_mesh_descriptions([description],False,False)
  body=mesh.get_editor_property('body_setup');body.set_editor_property('collision_trace_flag',unreal.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE);body.set_editor_property('double_sided_geometry',True)
  unreal.EditorAssetLibrary.save_loaded_asset(mesh)
 parts[name]=mesh
# Actual floor traces: 60 and 523.619cm. Preserve the actor's designer placement.
base=e.get_actor_location().z;e.set_editor_property('stop0',unreal.Vector(0,0,60-base));e.set_editor_property('stop1',unreal.Vector(0,0,523.619-base))
e.set_editor_property('auto_close_delay',3);e.set_editor_property('travel_speed',160);e.set_editor_property('travel_acceleration',140)
e.set_editor_property('upper_door_offset',unreal.Vector());e.set_editor_property('landing_door_offset',unreal.Vector(5,0,0))
shell.static_mesh_component.set_static_mesh(parts['SM_Lv4MovingCabin'])
# Frames remain independent of the cabin, at each landing.
for i,z in enumerate([57.335462,520.954462]):
 label='LV4_FixedFrame_'+str(i);a=by.get(label) or actors.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector(-1549.404681,743.991914,z),unreal.Rotator(pitch=0,yaw=-90,roll=0))
 a.modify();a.set_actor_label(label);a.set_folder_path('LV4_Mechanisms/WestElevator/FixedLandings')
 a.set_actor_location(unreal.Vector(-1549.404681,743.991914,z),False,True);a.set_actor_scale3d(unreal.Vector(1.381722,1.347714,1.498588))
 a.static_mesh_component.set_static_mesh(parts['SM_Lv4FixedFrame']);a.static_mesh_component.set_collision_profile_name('BlockAll')
# Cabin doors must actually conceal the passing shaft while travelling.
for c in [e.left_door,e.right_door]:c.modify();c.set_visibility(True);c.set_hidden_in_game(False)
for c in e.landing_doors:c.modify();c.set_relative_scale3d(e.left_door.get_editor_property('relative_scale3d'))
e.door_sensor.modify();e.door_sensor.set_relative_location(unreal.Vector(-123.696,0,176.845),False,True);e.door_sensor.set_box_extent(unreal.Vector(18,112,177))
e.cabin_volume.modify();e.cabin_volume.set_relative_location(unreal.Vector(0,0,176),False,True);e.cabin_volume.set_box_extent(unreal.Vector(120,130,176))
e.set_editor_property('landing_button_actors',[by['SM_Prop_Elevator_Button_2'],by['SM_Prop_Elevator_Button_01']])
e.set_editor_property('landing_button_offsets',[unreal.Vector(0,12.8,24),unreal.Vector(0,12.8,8)])
for i,c in enumerate(e.landing_controls):
 c.modify();c.get_attach_parent().modify();anchor=e.landing_button_actors[i]
 c.get_attach_parent().set_world_location(unreal.MathLibrary.transform_location(anchor.get_actor_transform(),e.landing_button_offsets[i]),False,True)
 c.get_attach_parent().set_world_scale3d(unreal.Vector(.008,.105,.09));c.set_world_scale3d(unreal.Vector(1,1,1));c.set_box_extent(unreal.Vector(2,7,7));c.set_editor_property('reach',240)
# Move the cabin control onto the interior side near the doorway, away from the handrail.
e.button.set_relative_location(unreal.Vector(-55,135,205),False,True);e.button.set_relative_scale3d(unreal.Vector(.16,.06,.24))
e.set_editor_property('interaction_distance',240)
e.cabin_control.set_world_scale3d(unreal.Vector(1,1,1));e.cabin_control.set_box_extent(unreal.Vector(11,6,15))
# Remove redundant floating E/SHIFT text; the focus UI provides the current prompt.
for label in ['LV4_CabinButtonHint','LV4_UpperDashHint']:
 a=by.get(label)
 if a:a.modify();a.set_actor_hidden_in_game(True);a.set_actor_enable_collision(False)
for label in ['LV4_UpperDoorDashEntry','LV4_UpperDoorDashExit']:
 a=by[label];a.modify();v=a.get_actor_location();v.z=613.619;a.set_actor_location(v,False,True)
e.restore_floor(0)
assert ls.save_current_level()
print('REWORKED',len(frame),'fixed polygons; anchored floor buttons; floors',e.stop0,e.stop1)
