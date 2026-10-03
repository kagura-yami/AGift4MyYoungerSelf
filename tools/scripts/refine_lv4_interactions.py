"""LV4 focused elevator controls + exact collision. Back up map before executing. Idempotent labels."""
import unreal,shutil,time,json
from pathlib import Path
root=Path(unreal.Paths.project_dir()).resolve().parent
levels=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
actors=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
assert not unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
assert levels.load_level('/Game/Maps/lv4')
backup=root/'tools/evaluation/focus-backup';backup.mkdir(exist_ok=True)
shutil.copy2(root/'game/Content/Maps/lv4.umap',backup/('lv4-'+time.strftime('%Y%m%d-%H%M%S')+'.umap'))
by={a.get_actor_label():a for a in actors.get_all_level_actors()}
e=by['LV4_WestElevator'];e.modify()
assets=unreal.AssetToolsHelpers.get_asset_tools();mel=unreal.MaterialEditingLibrary
mat=unreal.load_asset('/Game/Materials/Whitebox/M_InteractionFocus')
if not mat:
 mat=assets.create_asset('M_InteractionFocus','/Game/Materials/Whitebox',unreal.Material,unreal.MaterialFactoryNew())
 mat.set_editor_property('blend_mode',unreal.BlendMode.BLEND_TRANSLUCENT)
 mat.set_editor_property('shading_model',unreal.MaterialShadingModel.MSM_UNLIT)
 col=mel.create_material_expression(mat,unreal.MaterialExpressionConstant3Vector)
 col.set_editor_property('constant',unreal.LinearColor(.26,.65,.95,1))
 mel.connect_material_property(col,'',unreal.MaterialProperty.MP_EMISSIVE_COLOR)
 opacity=mel.create_material_expression(mat,unreal.MaterialExpressionConstant);opacity.set_editor_property('r',.24)
 mel.connect_material_property(opacity,'',unreal.MaterialProperty.MP_OPACITY)
 mel.recompile_material(mat);unreal.EditorAssetLibrary.save_loaded_asset(mat)
e.set_editor_property('manual_doors',True)
e.set_editor_property('upper_door_offset',unreal.Vector(0,0,52.390167))
e.set_editor_property('door_travel',124)
e.set_editor_property('landing_door_offset',unreal.Vector(5,0,0))
# Only landing panels are visible; eliminate the doubled surfaces in the reveal.
for c in [e.left_door,e.right_door]:c.modify();c.set_visibility(False);c.set_hidden_in_game(True)
# Keep the landing panels flush behind the original jambs, with matching metallic material.
for c in e.landing_doors:c.modify();c.set_visibility(True);c.set_hidden_in_game(False)
for i,(label,z) in enumerate([('SM_Prop_Elevator_Button_2',212),('SM_Prop_Elevator_Button_01',653)]):
 a=by[label];pos=a.get_actor_location();control=e.landing_controls[i];mesh=control.get_attach_parent()
 mesh.modify();mesh.set_world_location(unreal.Vector(pos.x+12,pos.y,z),False,True);mesh.set_world_scale3d(unreal.Vector(.035,.105,.09))
 control.modify();control.set_world_scale3d(unreal.Vector(1,1,1));control.set_box_extent(unreal.Vector(4,6,5.5));control.set_editor_property('highlight_material',mat)
 control.set_editor_property('reach',210)
 clear=unreal.load_asset('/Game/Materials/Whitebox/M_ControlClear')
 if clear:mesh.set_material(0,clear);mesh.set_editor_property('cast_shadow',False)
e.cabin_control.set_editor_property('highlight_material',mat)
# Collision copies keep art assets and all other map instances unchanged.
changed=[]
for a in list(by.values()):
 label=a.get_actor_label()
 if not label.startswith(('SM_Bld_Wall_','SM_Bld_FeatureWall_')) and label!='SM_Bld_Elevator_01':continue
 for c in a.get_components_by_class(unreal.MeshComponent):
  a.modify();c.modify()
  if isinstance(c,unreal.SkeletalMeshComponent):
   # Rigid collision companions are created by solidify_lv4_walls.py.
   c.set_enable_per_poly_collision(False)
  elif isinstance(c,unreal.StaticMeshComponent):
   original=c.static_mesh
   path=original.get_path_name().split('.')[0] if original.get_path_name().startswith('/Game/Blueprint/Lv4/Collision/') else '/Game/Blueprint/Lv4/Collision/'+original.get_name()+'_Solid'
   copy=unreal.load_asset(path)
   if not copy:copy=unreal.EditorAssetLibrary.duplicate_asset(original.get_path_name().split('.')[0],path)
   body=copy.get_editor_property('body_setup');body.set_editor_property('collision_trace_flag',unreal.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE);body.set_editor_property('double_sided_geometry',True)
   unreal.EditorAssetLibrary.save_loaded_asset(copy);c.set_static_mesh(copy)
  else:continue
  a.set_actor_enable_collision(True);c.set_collision_profile_name('BlockAll');c.set_collision_enabled(unreal.CollisionEnabled.QUERY_AND_PHYSICS)
  changed.append(label)
# Actual cabin shell now provides exact side/ceiling collision, instead of boxes intruding through trim.
for c in e.get_components_by_class(unreal.StaticMeshComponent):
 if c.get_name() in ['BackWall','SideWallLeft','SideWallRight']:
  c.modify();c.set_visibility(False);c.set_hidden_in_game(True);c.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
# One-way, heading-constrained upper door crossing. Upper cabin must be present.
for label,xyz,entry in [('LV4_UpperDoorDashEntry',(-1485,565,610),True),('LV4_UpperDoorDashExit',(-1720,565,610),False)]:
 a=by.get(label) or actors.spawn_actor_from_class(unreal.TripoTeleportPoint,unreal.Vector(*xyz),unreal.Rotator(pitch=0,yaw=180,roll=0))
 a.modify();a.set_actor_label(label);a.set_folder_path('LV4_Mechanisms/WestElevator');a.set_actor_location_and_rotation(unreal.Vector(*xyz),unreal.Rotator(pitch=0,yaw=180,roll=0),False,True)
 a.set_editor_property('link_id','LV4_UpperElevatorDoor');a.set_editor_property('entry_enabled',entry);a.set_editor_property('require_forward_direction',True);a.set_editor_property('trigger_radius',85);a.set_editor_property('destination_offset',unreal.Vector());a.set_editor_property('required_elevator',e)
# Mark the special threshold without a blocking volume.
label='LV4_UpperDashHint'
a=by.get(label) or actors.spawn_actor_from_class(unreal.TextRenderActor,unreal.Vector(-1555,565,725),unreal.Rotator(pitch=0,yaw=0,roll=0))
a.set_actor_label(label);a.set_folder_path('LV4_Mechanisms/WestElevator');a.text_render.set_text('SHIFT');a.text_render.set_world_size(13);a.text_render.set_horizontal_alignment(unreal.HorizTextAligment.EHTA_CENTER)
e.restore_floor(0)
assert levels.save_current_level()
(backup/'collision-changed.json').write_text(json.dumps(changed,indent=2),encoding='utf8')
print('UPDATED',len(changed),'wall/cabin components; manual controls; upper dash gate')
