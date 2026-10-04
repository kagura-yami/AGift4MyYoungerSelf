"""Remove obsolete narrow portal trim and keep shaft sill outside cabin doors."""
import unreal,shutil,time,json
from pathlib import Path
root=Path(unreal.Paths.project_dir()).resolve().parent
out=root/'tools/evaluation/elevator-rework'
levels=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
assert not unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
aes=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
by={a.get_actor_label():a for a in aes.get_all_level_actors()}
lift=by['LV4_WestElevator']
# Restore temporary diagnostic poses before saving or backing up.
lift.restore_floor(0)
by['LV4_FixedFrame_0'].static_mesh_component.set_visibility(True)
for a in list(by.values()):
 if a.get_actor_label()=='TEMP_InteriorQA':aes.destroy_actor(a)
assert levels.save_current_level()
shutil.copy2(root/'game/Content/Maps/lv4.umap',out/('lv4-before-landing-clearance-'+time.strftime('%Y%m%d-%H%M%S')+'.umap'))
sill=by['LV4_UpperShaftSill'];sill.modify()
sill.set_actor_location(unreal.Vector(-1590,565,507),False,True)
sill.set_actor_scale3d(unreal.Vector(.5,3.54,.32))
assert sill.get_actor_bounds(False)[0].x-sill.get_actor_bounds(False)[1].x > -1630+10
cube=unreal.load_asset('/Engine/BasicShapes/Cube')
trim=unreal.load_asset('/Game/Materials/Whitebox/M_Lv4ElevatorTrim')
for i,stop in enumerate([lift.stop0,lift.stop1]):
 frame=by['LV4_FixedFrame_'+str(i)];frame.modify()
 frame.static_mesh_component.set_visibility(False);frame.set_actor_hidden_in_game(True);frame.set_actor_enable_collision(False)
 for side,y in [('Left',-137),('Right',137)]:
  label='LV4_AlignedLandingJamb_'+str(i)+'_'+side
  a=by.get(label) or aes.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector(),unreal.Rotator())
  a.modify();a.set_actor_label(label);a.set_folder_path('LV4_Mechanisms/WestElevator/FixedLandings')
  c=a.static_mesh_component;c.set_mobility(unreal.ComponentMobility.MOVABLE)
  c.set_static_mesh(cube);c.set_material(0,trim);c.set_collision_profile_name('BlockAll')
  a.attach_to_component(lift.root_component,'',unreal.AttachmentRule.KEEP_RELATIVE,unreal.AttachmentRule.KEEP_RELATIVE,unreal.AttachmentRule.KEEP_RELATIVE,False)
  c.set_relative_location(stop+unreal.Vector(-145,y,177),False,True)
  c.set_relative_rotation(unreal.Rotator(),False,True);c.set_relative_scale3d(unreal.Vector(.28,.4,3.54))
assert levels.save_current_level()
print('Saved: sill behind cabin door; two landing portals clear width 234 cm')
