import unreal
es=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
aa=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
assert es.get_editor_world().get_name()=='L_Home_Vertical_02' and not es.get_game_world()
by={a.get_actor_label():a for a in aa.get_all_level_actors()}
old=by['KitchenWater']; loc=old.get_actor_location(); _,ext=old.get_actor_bounds(False)
surface_z=loc.z+ext.z
water=aa.spawn_actor_from_class(unreal.TripoWaterHazard,unreal.Vector(loc.x,loc.y,surface_z-100))
water.set_actor_label('KitchenWater');water.set_folder_path('Home/04_Kitchen/Water')
water.water_volume.set_box_extent(unreal.Vector(ext.x,ext.y,100))
water.surface.set_static_mesh(unreal.load_asset('/Engine/BasicShapes/Plane'))
water.surface.set_material(0,unreal.load_asset('/Game/Materials/Water/M_KitchenWater_Surface'))
water.surface.set_relative_location(unreal.Vector(0,0,100),False,True)
water.surface.set_relative_scale3d(unreal.Vector(ext.x/50,ext.y/50,1))
assert aa.destroy_actor(old)
# Retain the authored art, yaw and footprint. A single movable deck provides stable footing.
for label in ['FKScene_FloatingBook','BP_Rotation']:
    old=by[label]; source=old.get_component_by_class(unreal.StaticMeshComponent)
    mesh=source.static_mesh; tr=source.get_world_transform(); b=mesh.get_bounding_box(); center=(b.min+b.max)*.5
    # Static mesh local centre transformed through the component's full scale/rotation.
    origin=unreal.MathLibrary.transform_location(tr,center)
    scale=source.get_world_scale(); local_extent=(b.max-b.min)*.5
    ex=abs(local_extent.x*scale.x);ey=abs(local_extent.y*scale.y);ez=abs(local_extent.z*scale.z)
    # Lift only the thin book if necessary to keep the playable deck above wave height.
    origin.z=max(origin.z,surface_z+14-ez)
    g=aa.spawn_actor_from_class(unreal.TripoFloatingPlatform,origin,old.get_actor_rotation())
    g.set_actor_label(label);g.set_folder_path('Home/04_Kitchen/Water')
    g.deck.set_box_extent(unreal.Vector(ex,ey,max(ez,3)))
    g.visual.set_static_mesh(mesh)
    visual_loc=source.get_world_location(); visual_loc.z+=origin.z-unreal.MathLibrary.transform_location(tr,center).z
    g.visual.set_world_location(visual_loc,False,True);g.visual.set_world_rotation(source.get_world_rotation(),False,True);g.visual.set_world_scale3d(scale)
    g.visual.set_editor_property('override_materials',source.get_editor_property('override_materials'))
    g.set_editor_property('max_tilt',2.5);g.set_editor_property('sink_depth',2.5)
    print('FLOAT',label,origin,'extent',ex,ey,ez)
    assert aa.destroy_actor(old)
# The old slab hazard is replaced by exact water-foot contact, not an overlapping broad box.
if 'Hazard_Home.Route1_401' in by: aa.destroy_actor(by['Hazard_Home.Route1_401'])
for a in aa.get_all_level_actors():
    if isinstance(a,unreal.TripoZone) and (a.get_actor_label().startswith('SafeFinish_') or a.get_actor_label()=='Finish_Home.Route1_Gifts' or a.kind==unreal.TripoZoneKind.START):
        a.modify();a.set_editor_property('bAutoCheckpoint',True)
# Kitchen route exits onto the next start landing, which now silently records a stable checkpoint.
assert unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
print('WATER_READY',surface_z)
