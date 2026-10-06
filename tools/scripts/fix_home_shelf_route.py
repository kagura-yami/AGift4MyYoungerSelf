import unreal
aa=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
ls=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
assert unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world().get_path_name().startswith('/Game/Maps/HomeVertical/Maps/L_Home_Vertical_02.')
by={a.get_actor_label():a for a in aa.get_all_level_actors()}
names=['rustic_wooden_shelf_3d_model6','rustic_wooden_shelf_3d_model','BedroomDrawer_7','rustic_wooden_shelf_3d_model2','rustic_wooden_shelf_3d_model7']
source=by[names[0]].static_mesh_component.static_mesh
path='/Game/Maps/HomeVertical/SM_BedroomShelf_PreciseCollision'
mesh=unreal.load_asset(path) or unreal.EditorAssetLibrary.duplicate_asset(source.get_path_name(),path)
mesh.modify()
mesh.get_editor_property('body_setup').set_editor_property('collision_trace_flag',unreal.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE)
unreal.EditorAssetLibrary.save_loaded_asset(mesh,False)
previous=unreal.Vector(627.95,-11.42,166.48)
for i,name in enumerate(names):
    a=by[name];a.modify();a.static_mesh_component.set_static_mesh(mesh)
    pos=a.get_actor_location();pos.x=620;pos.y=[-35,-270,0,270,40][i]
    a.set_actor_location(pos,False,True);a.set_actor_scale3d(unreal.Vector(1,0.5,1))
    c,e=a.get_actor_bounds(False)
    target=unreal.Vector(c.x,c.y,c.z+e.z)
    label='Bedroom_Shelf_Transition_%02d'%(i+1)
    loc=unreal.Vector([470,470,350,470,350][i],[40,-160,-130,140,160][i],(previous.z+target.z)/2-64.355469)
    step=by.get(label) or aa.spawn_actor_from_class(unreal.StaticMeshActor,loc)
    step.modify();step.set_actor_label(label);step.set_folder_path('Gameplay/BedroomShelfRoute')
    step.set_actor_location(loc,False,True)
    step.set_actor_scale3d(unreal.Vector(1,0.5,1))
    step.static_mesh_component.set_static_mesh(mesh)
    step.static_mesh_component.set_collision_profile_name('BlockAll')
    print(label,'top',loc.z+64.355469,'half rise',(target.z-previous.z)/2)
    previous=target
for a in by.values():
    if a.get_actor_label() in ['vintage_desk_lamp_3d_model','vintage_globe_3d_model','gold_trophy_3d_model','cardboard_box_3d_model','rusty_boombox_3d_model']:
        a.modify();a.set_actor_enable_collision(False)
for label,x,y in [('vintage_globe_3d_model',638,-290),('gold_trophy_3d_model',640,-245),('cardboard_box_3d_model',638,10)]:
    a=by[label];a.modify();pos=a.get_actor_location();pos.x=x;pos.y=y;a.set_actor_location(pos,False,True)
ls.save_current_level()
