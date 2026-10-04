import unreal,shutil,time
from pathlib import Path
root=Path(unreal.Paths.project_dir()).resolve().parent
backup=root/'tools/evaluation/elevator-rework';backup.mkdir(exist_ok=True)
ls=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
assert not unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
assert ls.save_current_level()
shutil.copy2(root/'game/Content/Maps/lv4.umap',backup/('lv4-before-seal-'+time.strftime('%Y%m%d-%H%M%S')+'.umap'))
aes=unreal.get_editor_subsystem(unreal.EditorActorSubsystem);by={a.get_actor_label():a for a in aes.get_all_level_actors()};e=by['LV4_WestElevator']
def material(name,color):
 path='/Game/Materials/Whitebox/'+name
 mat=unreal.load_asset(path)
 if not mat:
  mat=unreal.AssetToolsHelpers.get_asset_tools().create_asset(name,'/Game/Materials/Whitebox',unreal.Material,unreal.MaterialFactoryNew())
  mel=unreal.MaterialEditingLibrary
  c=mel.create_material_expression(mat,unreal.MaterialExpressionConstant3Vector);c.set_editor_property('constant',unreal.LinearColor(*color,1));mel.connect_material_property(c,'',unreal.MaterialProperty.MP_BASE_COLOR)
  for prop,value in [(unreal.MaterialProperty.MP_METALLIC,.35),(unreal.MaterialProperty.MP_ROUGHNESS,.68),(unreal.MaterialProperty.MP_SPECULAR,.25)]:
   n=mel.create_material_expression(mat,unreal.MaterialExpressionConstant);n.set_editor_property('r',value);mel.connect_material_property(n,'',prop)
  mel.recompile_material(mat);unreal.EditorAssetLibrary.save_loaded_asset(mat)
 return mat
metal=material('M_Lv4ElevatorMetal',(.16,.19,.20));trim=material('M_Lv4ElevatorTrim',(.045,.055,.06))
for c in [e.left_door,e.right_door]+list(e.landing_doors):c.modify();c.set_material(0,metal);scale=c.get_editor_property('relative_scale3d');scale.y=1.16364;c.set_relative_scale3d(scale)
e.modify()
for prop in ['left_closed_position','right_closed_position']:
 v=e.get_editor_property(prop);v.x=-90;e.set_editor_property(prop,v)
e.set_editor_property('landing_door_offset',unreal.Vector(-38.696,0,0))
e.restore_floor(0)
# Door leaves span z=0..354; cabin roof is above 400. Overlapping inner surround seals the shaft view.
cube=unreal.load_asset('/Engine/BasicShapes/Cube')
for name,pos,size in [('Header',(-80,0,399),(16,320,110)),('LeftJamb',(-80,-140,177),(16,48,358)),('RightJamb',(-80,140,177),(16,48,358)),('Threshold',(-80,0,1),(16,320,4))]:
 label='LV4_CabinSeal_'+name;a=by.get(label) or aes.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector(),unreal.Rotator())
 a.modify();a.set_actor_label(label);a.set_folder_path('LV4_Mechanisms/WestElevator/CabinTrim')
 c=a.static_mesh_component;c.set_mobility(unreal.ComponentMobility.MOVABLE);c.set_static_mesh(cube);c.set_material(0,trim);c.set_collision_profile_name('BlockAll')
 a.attach_to_component(e.cabin,'',unreal.AttachmentRule.KEEP_RELATIVE,unreal.AttachmentRule.KEEP_RELATIVE,unreal.AttachmentRule.KEEP_RELATIVE,False)
 c.set_relative_location(unreal.Vector(*pos),False,True);c.set_relative_rotation(unreal.Rotator(),False,True);c.set_relative_scale3d(unreal.Vector(*(v/100 for v in size)))
assert ls.save_current_level()
for i,parent in enumerate([e.left_door,e.landing_doors[0],e.landing_doors[2]]):
 label='LV4_DoorMeetingStrip_'+str(i);a=by.get(label) or aes.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector(),unreal.Rotator())
 a.modify();a.set_actor_label(label);a.set_folder_path('LV4_Mechanisms/WestElevator/DoorSeals')
 c=a.static_mesh_component;c.set_mobility(unreal.ComponentMobility.MOVABLE);c.set_static_mesh(cube);c.set_material(0,trim);c.set_collision_profile_name('NoCollision')
 a.attach_to_component(parent,'',unreal.AttachmentRule.KEEP_RELATIVE,unreal.AttachmentRule.KEEP_RELATIVE,unreal.AttachmentRule.KEEP_RELATIVE,False)
 c.set_relative_rotation(unreal.Rotator(),False,True);c.set_relative_location(unreal.Vector(0,50.34,0),False,True);c.set_world_scale3d(unreal.Vector(.084,.018,3.54))
for i,stop in enumerate([e.stop0,e.stop1]):
 label='LV4_LandingHeader_'+str(i);a=by.get(label) or aes.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector(),unreal.Rotator())
 a.modify();a.set_actor_label(label);a.set_folder_path('LV4_Mechanisms/WestElevator/FixedLandings')
 c=a.static_mesh_component;c.set_mobility(unreal.ComponentMobility.MOVABLE);c.set_static_mesh(cube);c.set_material(0,trim);c.set_collision_profile_name('BlockAll')
 a.attach_to_component(e.root_component,'',unreal.AttachmentRule.KEEP_RELATIVE,unreal.AttachmentRule.KEEP_RELATIVE,unreal.AttachmentRule.KEEP_RELATIVE,False)
 c.set_relative_rotation(unreal.Rotator(),False,True);c.set_relative_location(stop+unreal.Vector(-145,0,400),False,True);c.set_relative_scale3d(unreal.Vector(.3,3.2,1.12))
assert ls.save_current_level()
print('Sealed cabin and landing headers; restored 8 mm dark meeting seam')
