"""Replace cabin checker material and author restrained metal control hardware."""
import unreal,shutil
from pathlib import Path
aa=unreal.get_editor_subsystem(unreal.EditorActorSubsystem);ls=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
assert unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world().get_name()=='lv4'
root=Path(unreal.Paths.project_dir()).resolve();backup=root.parent/'tools/evaluation/lv4-before-metal-controls.umap'
if not backup.exists():shutil.copy2(root/'Content/Maps/lv4.umap',backup)
by={a.get_actor_label():a for a in aa.get_all_level_actors()};e=by['LV4_WestElevator']
mel=unreal.MaterialEditingLibrary
def material(name,color,metallic,roughness,glow=0,grain=False):
    m=unreal.load_asset('/Game/Materials/Lv4/'+name)
    if m:return m
    m=unreal.AssetToolsHelpers.get_asset_tools().create_asset(name,'/Game/Materials/Lv4',unreal.Material,unreal.MaterialFactoryNew())
    def constant(value,prop):
        n=mel.create_material_expression(m,unreal.MaterialExpressionConstant);n.set_editor_property('r',value);mel.connect_material_property(n,'',prop);return n
    c=mel.create_material_expression(m,unreal.MaterialExpressionConstant3Vector);c.set_editor_property('constant',unreal.LinearColor(*color,1));mel.connect_material_property(c,'',unreal.MaterialProperty.MP_BASE_COLOR)
    constant(metallic,unreal.MaterialProperty.MP_METALLIC);r=constant(roughness,unreal.MaterialProperty.MP_ROUGHNESS)
    if glow:
        mul=mel.create_material_expression(m,unreal.MaterialExpressionMultiply);mul.set_editor_property('const_b',glow);mel.connect_material_expressions(c,'',mul,'A');mel.connect_material_property(mul,'',unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    if grain:
        uv=mel.create_material_expression(m,unreal.MaterialExpressionTextureCoordinate);uv.set_editor_property('u_tiling',180);uv.set_editor_property('v_tiling',2)
        noise=mel.create_material_expression(m,unreal.MaterialExpressionNoise);noise.set_editor_property('scale',1);noise.set_editor_property('levels',1)
        mel.connect_material_expressions(uv,'',noise,'Position')
        mul=mel.create_material_expression(m,unreal.MaterialExpressionMultiply);mul.set_editor_property('const_b',.018);mel.connect_material_expressions(noise,'',mul,'A')
        add=mel.create_material_expression(m,unreal.MaterialExpressionAdd);mel.connect_material_expressions(mul,'',add,'A');mel.connect_material_expressions(r,'',add,'B');mel.connect_material_property(add,'',unreal.MaterialProperty.MP_ROUGHNESS)
    mel.recompile_material(m);unreal.EditorAssetLibrary.save_loaded_asset(m);return m
steel=material('M_CabinBrushedSteel',(.22,.25,.27),.82,.44,grain=True)
dark=material('M_CabinPanelGraphite',(.026,.033,.04),.65,.32)
silver=material('M_CabinButtonSilver',(.36,.4,.43),.9,.24)
face=material('M_CabinButtonFace',(.06,.075,.08),.55,.4)
ivory=material('M_CabinButtonEngraving',(.7,.78,.73),.2,.5,.65)
led=material('M_CabinButtonIndicator',(.15,.44,.32),.1,.4,1.4)
by['SM_Bld_Elevator_01'].static_mesh_component.set_material(0,steel)
for c in e.get_components_by_class(unreal.StaticMeshComponent):
    if c.get_name() in ['BackWall','SideWallLeft','SideWallRight'] or 'Door' in c.get_name():c.modify();c.set_material(0,steel)
e.floor.set_material(0,dark)
cube=unreal.load_asset('/Engine/BasicShapes/Cube');cylinder=unreal.load_asset('/Engine/BasicShapes/Cylinder')
def part(name,loc,size,mat,mesh=cube,rot=unreal.Rotator()):
    label='LV4_ControlDesign_'+name;a=by.get(label)
    if not a:a=aa.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector())
    a.set_actor_label(label);a.set_folder_path('LV4_Mechanisms/WestElevator/CabinControls')
    c=a.static_mesh_component;c.set_mobility(unreal.ComponentMobility.MOVABLE);c.set_static_mesh(mesh);c.set_material(0,mat);c.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
    a.attach_to_component(e.cabin,'',unreal.AttachmentRule.KEEP_RELATIVE,unreal.AttachmentRule.KEEP_RELATIVE,unreal.AttachmentRule.KEEP_RELATIVE,False)
    c.set_relative_location(unreal.Vector(*loc),False,True);c.set_relative_rotation(rot,False,True);c.set_relative_scale3d(unreal.Vector(*(v/100 for v in size)))
    return a
part('Frame',(-55,148.5,134),(33,3,74),silver)
part('Plate',(-55,146.5,134),(30,2,71),dark)
for mesh,target,z,name in [(e.door_button,e.door_control,151,'Open'),(e.button,e.cabin_control,117,'Travel')]:
    part(name+'Rim',(-55,144.3,z),(20,20,2),silver,cylinder,unreal.Rotator(roll=90))
    mesh.set_static_mesh(cylinder);mesh.set_material(0,face)
    mesh.set_relative_location(unreal.Vector(-55,142.9,z),False,True)
    mesh.set_relative_rotation(unreal.Rotator(roll=90),False,True)
    mesh.set_relative_scale3d(unreal.Vector(.17,.17,.025))
    # Keep authored independent actions; reset rotated parent-relative target axes.
    target.set_world_location(mesh.get_world_location(),False,True)
    target.set_world_rotation(e.get_actor_rotation(),False,True)
    target.set_world_scale3d(unreal.Vector(1,1,1));target.set_box_extent(unreal.Vector(11,5,11))
    target.set_editor_property('highlight_material',unreal.load_asset('/Game/Materials/BookDecal/M_BookFocus'))
    if 'LV4_ControlIcon_'+name in by:by['LV4_ControlIcon_'+name].set_actor_hidden_in_game(True)
    # Four narrow strokes form outward chevrons (open) or up/down chevrons (travel).
    coords=[(-3,2,-45),(-3,-2,45),(3,2,45),(3,-2,-45)] if name=='Open' else [(-2,3,-45),(2,3,45),(-2,-3,45),(2,-3,-45)]
    for i,(x,dz,angle) in enumerate(coords):part(name+'Glyph'+str(i),(-55+x,141.45,z+dz),(1.0,.4,5.7),ivory,rot=unreal.Rotator(pitch=angle))
    part(name+'Light',(-55,145.2,z+13),(6,.8,1.2),led)
for x in [-67,-43]:
    for z in [102,166]:part('Screw'+str(x)+'_'+str(z),(x,145.3,z),(1.5,1.5,.5),silver,cylinder,unreal.Rotator(roll=90))
assert ls.save_current_level()
print('Cabin materials and moving control hardware saved')

