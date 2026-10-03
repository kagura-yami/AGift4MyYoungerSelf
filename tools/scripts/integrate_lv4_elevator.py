"""Integrate only the unambiguous western LV4 elevator. Backup first; never rerun over designer edits."""
import unreal,json,shutil,time
from pathlib import Path
root=Path(unreal.Paths.project_dir()).resolve().parent
levels=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
actors=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
assert not unreal.EditorLoadingAndSavingUtils.get_dirty_map_packages()
assert levels.load_level('/Game/Maps/lv4')
world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
by={a.get_actor_label():a for a in actors.get_all_level_actors()}
assert 'LV4_WestElevator' not in by, 'Already integrated: inspect rather than overwrite'
required=['SM_Bld_Elevator_01','Cube','Cube2','Cube5','Cube6','SM_Prop_Elevator_Button_01','SM_Prop_Elevator_Button_2']
assert all(k in by for k in required)
backup=root/'tools/evaluation/lv4-backup';backup.mkdir(exist_ok=True)
shutil.copy2(root/'game/Content/Maps/lv4.umap',backup/('lv4-before-integration-'+time.strftime('%Y%m%d-%H%M%S')+'.umap'))
records=[dict(label=k,name=by[k].get_name(),transform=str(by[k].get_actor_transform())) for k in required]
(backup/'west-elevator-original.json').write_text(json.dumps(records,ensure_ascii=False,indent=2),encoding='utf-8')
# Original lower door bottom=69.8; upper carpet top checked at Z=520.
world.modify()
cls=unreal.EditorAssetLibrary.load_blueprint_class('/Game/Blueprint/Lv4/BP_Lv4Elevator')
lift=actors.spawn_actor_from_class(cls,unreal.Vector(-1720,565.024,70),unreal.Rotator(pitch=0,yaw=180,roll=0))
lift.set_actor_label('LV4_WestElevator');lift.set_folder_path('LV4_Mechanisms/WestElevator')
lift.set_editor_property('stop1',unreal.Vector(0,0,450))
lift.set_editor_property('left_closed_position',unreal.Vector(-123.696,-58.582,176.845))
lift.set_editor_property('right_closed_position',unreal.Vector(-123.696,58.582,176.845))
lift.set_editor_property('landing_door_offset',unreal.Vector(-10,0,0));lift.set_editor_property('door_travel',120)
# Match all original panel materials and sizes. Originals retained, hidden and nonblocking for reversibility.
for component,label in [(lift.left_door,'Cube2'),(lift.right_door,'Cube'),(lift.landing_doors[0],'Cube2'),(lift.landing_doors[1],'Cube'),(lift.landing_doors[2],'Cube5'),(lift.landing_doors[3],'Cube6')]:
 src=by[label].static_mesh_component
 component.set_static_mesh(src.static_mesh);component.set_relative_scale3d(by[label].get_actor_scale3d());component.set_material(0,src.get_material(0))
for label in ['Cube','Cube2','Cube5','Cube6']:
 a=by[label];a.modify();a.set_actor_hidden_in_game(True);a.set_actor_enable_collision(False);a.static_mesh_component.set_visibility(False);a.set_folder_path('LV4_Mechanisms/WestElevator/OriginalPanels')
lift.door_sensor.set_relative_location(unreal.Vector(-130,0,180),False,True);lift.door_sensor.set_box_extent(unreal.Vector(80,135,190))
for b in lift.landing_barriers:b.set_box_extent(unreal.Vector(10,120,220))
# Keep the existing cabin art and its exact world transform; native simple geometry supplies movement collision.
shell=by['SM_Bld_Elevator_01'];shell.modify();shell.static_mesh_component.set_mobility(unreal.ComponentMobility.MOVABLE);shell.set_actor_enable_collision(False)
shell.attach_to_component(lift.cabin,'',unreal.AttachmentRule.KEEP_WORLD,unreal.AttachmentRule.KEEP_WORLD,unreal.AttachmentRule.KEEP_WORLD,False)
shell.set_folder_path('LV4_Mechanisms/WestElevator')
for c in lift.get_components_by_class(unreal.StaticMeshComponent):
 if c.get_name() in ['Floor','BackWall','SideWallLeft','SideWallRight']:
  c.set_visibility(False)
  if c.get_name()!='Floor':
   v=c.get_editor_property('relative_scale3d');v.z=4.2;c.set_relative_scale3d(v)
   loc=c.get_editor_property('relative_location');loc.z=210;c.set_relative_location(loc,False,True)
# Moving lighting and an unobtrusive button hint.
light=actors.spawn_actor_from_class(unreal.PointLight,unreal.Vector(-1720,565,440));light.set_actor_label('LV4_CabinLight');light.point_light_component.set_mobility(unreal.ComponentMobility.MOVABLE);light.point_light_component.set_editor_property('intensity',35);light.point_light_component.set_editor_property('attenuation_radius',500);light.point_light_component.set_editor_property('cast_shadows',False)
light.attach_to_component(lift.cabin,'',unreal.AttachmentRule.KEEP_WORLD,unreal.AttachmentRule.KEEP_WORLD,unreal.AttachmentRule.KEEP_WORLD,False);light.set_folder_path('LV4_Mechanisms/WestElevator')
hint=actors.spawn_actor_from_class(unreal.TextRenderActor,lift.button.get_world_location()+unreal.Vector(0,0,35),unreal.Rotator(pitch=0,yaw=0,roll=0));hint.set_actor_label('LV4_CabinButtonHint');hint.text_render.set_text('[ E ]');hint.text_render.set_world_size(16);hint.attach_to_component(lift.cabin,'',unreal.AttachmentRule.KEEP_WORLD,unreal.AttachmentRule.KEEP_WORLD,unreal.AttachmentRule.KEEP_WORLD,False);hint.set_folder_path('LV4_Mechanisms/WestElevator')
lift.restore_floor(0)
assert levels.save_current_level()
print('INTEGRATED',lift.get_path_name(),'Original decorative second elevator left untouched')
