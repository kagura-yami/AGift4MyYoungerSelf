import unreal,shutil,time
from pathlib import Path
root=Path(unreal.Paths.project_dir()).resolve().parent
ls=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
assert not unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
assert ls.save_current_level()
assert ls.load_level('/Game/Maps/lv4')
shutil.copy2(root/'game/Content/Maps/lv4.umap',root/'tools/evaluation/elevator-rework'/('lv4-before-arrow-'+time.strftime('%Y%m%d-%H%M%S')+'.umap'))
e=next(a for a in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors() if a.get_actor_label()=='LV4_WestElevator')
source=unreal.TripoCollisionTools.create_rigid_collision_mesh(unreal.load_asset('/Game/Models/lv4/SM_Prop_Elevator_Button_01'),'/Game/Blueprint/Lv4/Collision/SM_ButtonSource')
unreal.EditorAssetLibrary.save_loaded_asset(source)
e.modify(); offsets=[]
for i,target in enumerate(e.landing_controls):
 center=22.385 if i==0 else 8.125
 offset=unreal.Vector(0,7.65,center);offsets.append(offset)
 path='/Game/Blueprint/Lv4/Collision/SM_FocusArrowSurface'+str(i)
 mesh=unreal.load_asset(path)
 if not mesh:
  mesh=unreal.EditorAssetLibrary.duplicate_asset(source.get_path_name().split('.')[0],path)
  d=mesh.get_static_mesh_description(0);vertices=set()
  for j in range(d.get_polygon_count()):
   vv=d.get_polygon_vertices(unreal.PolygonID(j));pts=[d.get_vertex_position(v) for v in vv]
   keep=all(p.y>7.5 and (p.z>18 if i==0 else p.z<12) for p in pts)
   if not keep:d.delete_polygon(unreal.PolygonID(j))
   else:
    for v in vv:
     vertices.add(v.to_tuple()[0])
  for vid in vertices:
   v=unreal.VertexID(vid);p=d.get_vertex_position(v);p.y-=7.58;p.z-=center;d.set_vertex_position(v,p)
  mesh.build_from_static_mesh_descriptions([d],False,False);unreal.EditorAssetLibrary.save_loaded_asset(mesh)
 target.modify();mount=target.get_attach_parent();mount.modify()
 original=e.landing_button_actors[i].get_component_by_class(unreal.SkeletalMeshComponent);original.set_overlay_material(None)
 mount.set_static_mesh(mesh);mount.set_material(0,unreal.load_asset('/Game/Materials/Whitebox/M_ControlClear'));mount.set_visibility(True)
 mount.set_world_scale3d(unreal.Vector(1,1,1));mount.set_world_rotation(e.landing_button_actors[i].get_actor_rotation(),False,True)
 mount.set_world_location(unreal.MathLibrary.transform_location(e.landing_button_actors[i].get_actor_transform(),offset),False,True)
 target.set_editor_property('highlight_mesh',mount);target.set_world_rotation(unreal.Rotator(pitch=0,yaw=180,roll=0),False,True);target.set_world_scale3d(unreal.Vector(1,1,1));target.set_box_extent(unreal.Vector(6,4.1,4))
e.set_editor_property('landing_button_offsets',offsets)
assert ls.save_current_level()
print('Arrow-only exact front polygons applied')
ls.editor_request_begin_play()
