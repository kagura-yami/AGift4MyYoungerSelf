"""Two independently focusable cabin controls: open doors and travel."""
import unreal,shutil,time
from pathlib import Path
root=Path(unreal.Paths.project_dir()).resolve().parent
levels=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
assert not unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
assert levels.save_current_level()
out=root/'tools/evaluation/elevator-rework'
shutil.copy2(root/'game/Content/Maps/lv4.umap',out/('lv4-before-split-controls-'+time.strftime('%Y%m%d-%H%M%S')+'.umap'))
aes=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
by={a.get_actor_label():a for a in aes.get_all_level_actors()}
lift=by['LV4_WestElevator'];lift.modify()
cube=unreal.load_asset('/Engine/BasicShapes/Cube')
def material(name,color):
 path='/Game/Materials/Lv4/'+name
 m=unreal.load_asset(path)
 if not m:
  m=unreal.AssetToolsHelpers.get_asset_tools().create_asset(name,'/Game/Materials/Lv4',unreal.Material,unreal.MaterialFactoryNew())
  mel=unreal.MaterialEditingLibrary
  c=mel.create_material_expression(m,unreal.MaterialExpressionConstant3Vector)
  c.set_editor_property('constant',unreal.LinearColor(*color,1))
  mel.connect_material_property(c,'',unreal.MaterialProperty.MP_BASE_COLOR)
  mel.recompile_material(m);unreal.EditorAssetLibrary.save_loaded_asset(m)
 return m
for mesh,target,z,name,col,glyph in [(lift.door_button,lift.door_control,151,'Open',(.045,.24,.19),'< >'),(lift.button,lift.cabin_control,117,'Travel',(.26,.15,.045),'^ v')]:
 mesh.modify();target.modify()
 mesh.set_static_mesh(cube);mesh.set_material(0,material('M_Control'+name,col))
 mesh.set_relative_location(unreal.Vector(-55,128,z),False,True)
 mesh.set_relative_rotation(unreal.Rotator(),False,True)
 mesh.set_relative_scale3d(unreal.Vector(.22,.045,.24))
 mesh.set_collision_profile_name('BlockAllDynamic')
 target.set_relative_location(unreal.Vector(0,-70,0),False,True)
 target.set_world_scale3d(unreal.Vector(1,1,1))
 target.set_box_extent(unreal.Vector(12,4,12))
 target.set_editor_property('reach',240);target.set_editor_property('highlight_mesh',mesh)
 target.set_editor_property('prompt',unreal.Text('开门 / 延长开门' if name=='Open' else '关门并前往另一层'))
 label='LV4_ControlIcon_'+name
 a=by.get(label) or aes.spawn_actor_from_class(unreal.TextRenderActor,unreal.Vector(),unreal.Rotator())
 a.modify();a.set_actor_label(label);a.set_folder_path('LV4_Mechanisms/WestElevator/CabinControls')
 a.text_render.set_mobility(unreal.ComponentMobility.MOVABLE)
 a.attach_to_component(lift.cabin,'',unreal.AttachmentRule.KEEP_RELATIVE,unreal.AttachmentRule.KEEP_RELATIVE,unreal.AttachmentRule.KEEP_RELATIVE,False)
 a.text_render.set_relative_location(unreal.Vector(-55,125.5,z-3),False,True)
 a.text_render.set_relative_rotation(unreal.Rotator(pitch=0,yaw=-90,roll=0),False,True)
 a.text_render.set_text(glyph);a.text_render.set_world_size(9)
 a.text_render.set_horizontal_alignment(unreal.HorizTextAligment.EHTA_CENTER)
 a.text_render.set_text_render_color(unreal.Color(230,245,245,255))
lift.set_editor_property('auto_close_delay',5)
lift.restore_floor(0)
assert levels.save_current_level()
print('Saved separate cabin open and travel controls')
