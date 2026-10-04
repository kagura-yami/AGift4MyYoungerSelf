import unreal
ls=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem);es=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
for name in ['L_Home_Vertical_01','L_Home_Vertical_02']:
 ls.load_level('/Game/Maps/HomeVertical/Maps/'+name);actors=es.get_all_level_actors()
 for a in actors:
  if 'CanOpenDoor' not in a.get_class().get_name() or a.get_actor_scale3d().y>=.9:continue
  loc=a.get_actor_location();scale=a.get_actor_scale3d()
  # These authored small doors are aligned to X, with lateral opening along Y.
  assert abs(a.get_actor_forward_vector().x)>.99
  for wall in actors:
   if not isinstance(wall,unreal.StaticMeshActor) or not wall.static_mesh_component.static_mesh:continue
   if wall.static_mesh_component.static_mesh.get_path_name()!='/Engine/BasicShapes/Cube.Cube':continue
   c,e=wall.get_actor_bounds(False)
   if abs(c.x-loc.x)>e.x+12 or e.x>45 or not c.z-e.z<loc.z+100<c.z+e.z:continue
   side=1 if c.y>loc.y else -1
   inner=c.y-side*e.y
   if not 25<side*(inner-loc.y)<75:continue
   new=loc.y+side*58;delta=side*(new-inner)
   if delta<=0 or 2*e.y<=delta:continue
   sc=wall.get_actor_scale3d();sc.y*=(2*e.y-delta)/(2*e.y)
   pos=wall.get_actor_location();pos.y+=side*delta/2
   wall.modify();wall.set_actor_scale3d(sc);wall.set_actor_location(pos,False,True)
   print('WIDEN WALL',name,wall.get_actor_label())
  a.modify();scale.y=1.;a.set_actor_scale3d(scale);print('WIDEN DOOR',name,a.get_actor_label())
 ls.save_current_level()
