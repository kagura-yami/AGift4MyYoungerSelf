"""Add the book/card/drawer puzzle without moving existing room geometry."""
import unreal,shutil
from pathlib import Path
root=Path(unreal.Paths.project_dir()).resolve().parent
es=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
aa=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
assert es.get_editor_world() and es.get_editor_world().get_name()=='lv4'
backup=root/'tools/evaluation/lv4-before-office-cipher.umap'
if not backup.exists():shutil.copy2(root/'game/Content/Maps/lv4.umap',backup)
by={a.get_actor_label():a for a in aa.get_all_level_actors()}
folder='LV4_Mechanisms/CompanyCipher'
def actor(cls,name,pos):
    a=by.get(name) or aa.spawn_actor_from_class(cls,unreal.Vector(*pos))
    a.modify();a.set_actor_label(name);a.set_folder_path(folder);a.set_actor_location(unreal.Vector(*pos),False,True)
    return a
def material(name,color,metal=0):
    p='/Game/UI/OfficeCipher'
    m=unreal.load_asset(p+'/'+name) or unreal.AssetToolsHelpers.get_asset_tools().create_asset(name,p,unreal.Material,unreal.MaterialFactoryNew())
    e=unreal.MaterialEditingLibrary;e.delete_all_material_expressions(m)
    c=e.create_material_expression(m,unreal.MaterialExpressionConstant3Vector);c.set_editor_property('constant',unreal.LinearColor(*color))
    e.connect_material_property(c,'',unreal.MaterialProperty.MP_BASE_COLOR)
    for value,pin in [(.68,unreal.MaterialProperty.MP_ROUGHNESS),(metal,unreal.MaterialProperty.MP_METALLIC)]:
        v=e.create_material_expression(m,unreal.MaterialExpressionConstant);v.set_editor_property('r',value);e.connect_material_property(v,'',pin)
    e.recompile_material(m);unreal.EditorAssetLibrary.save_loaded_asset(m);return m
dark=material('M_DrawerEnamel',(.09,.105,.10,1),.35)
brass=material('M_DrawerHandle',(.39,.28,.12,1),.7)
inside=material('M_DrawerLining',(.026,.022,.018,1))
cube=unreal.load_asset('/Engine/BasicShapes/Cube')
p=actor(unreal.TripoOfficeCipher,'LV4_OfficeCipher',(0,0,0))
p.set_editor_property('card_ui',unreal.load_asset('/Game/UI/OfficeCipher/M_CipherCard_UI'))
p.set_editor_property('drawer_travel',unreal.Vector(0,-42,0))
# A dedicated whiteboard sits beside the workstation; existing furniture is untouched.
board=actor(unreal.StaticMeshActor,'LV4_CipherWhiteboard',(-20,-1450,561.1))
board.static_mesh_component.set_static_mesh(unreal.load_asset('/Game/Models/lv4/SM_Prop_Whiteboard_02_Combined'))
for i,m in enumerate(by['SM_Prop_Whiteboard_02_Combined'].static_mesh_component.get_materials()):board.static_mesh_component.set_material(i,m)
board.set_actor_rotation(unreal.Rotator(yaw=180),False)
p.card.set_static_mesh(unreal.load_asset('/Engine/BasicShapes/Plane'))
p.card.set_material(0,unreal.load_asset('/Game/UI/OfficeCipher/M_CipherCard_World'))
p.card.set_world_location(unreal.Vector(-20,-1449.1,712),False,True)
p.card.set_world_rotation(unreal.Rotator(roll=90),False,True)
p.card.set_world_scale3d(unreal.Vector(.225,.4,1))
p.card_target.set_world_scale3d(unreal.Vector(1,1,1));p.card_target.set_box_extent(unreal.Vector(13,22,3))
source=by['SM_Prop_Book_Magazine_3']
p.book.set_static_mesh(source.static_mesh_component.static_mesh)
for i,m in enumerate(source.static_mesh_component.get_materials()):p.book.set_material(i,m)
p.book.set_world_transform(source.get_actor_transform(),False,True)
p.book_target.set_world_scale3d(unreal.Vector(1,1,1));p.book_target.set_box_extent(unreal.Vector(18,22,6))
source.modify();source.set_actor_hidden_in_game(True);source.set_is_temporarily_hidden_in_editor(True);source.set_actor_enable_collision(False)
# A small lockable pedestal beside the desk, with an actual hollow sliding drawer.
p.drawer.set_static_mesh(cube);p.drawer.set_material(0,dark)
p.drawer.set_world_location(unreal.Vector(-40,-1170,598),False,True);p.drawer.set_world_scale3d(unreal.Vector(.56,.025,.28))
p.drawer_target.set_world_scale3d(unreal.Vector(1,1,1));p.drawer_target.set_box_extent(unreal.Vector(29,5,16))
def block(name,pos,size,mat,moving=False):
    a=actor(unreal.StaticMeshActor,'LV4_Cipher_'+name,pos)
    c=a.static_mesh_component;c.set_mobility(unreal.ComponentMobility.MOVABLE);c.set_static_mesh(cube);c.set_material(0,mat)
    a.set_actor_scale3d(unreal.Vector(*(v/100 for v in size)))
    if moving:a.attach_to_component(p.drawer,'None',unreal.AttachmentRule.KEEP_WORLD,unreal.AttachmentRule.KEEP_WORLD,unreal.AttachmentRule.KEEP_WORLD,False)
    return a
block('CabinetBottom',(-40,-1140,563),(60,64,4),dark)
block('CabinetTop',(-40,-1140,624),(60,64,3),dark)
block('CabinetLeft',(-69,-1140,593),(2,64,60),dark)
block('CabinetRight',(-11,-1140,593),(2,64,60),dark)
block('CabinetBack',(-40,-1109,593),(56,2,60),dark)
block('CabinetLower',(-40,-1171,574),(56,2,18),dark)
block('DrawerBase',(-40,-1140,585),(54,58,2),inside,True)
block('DrawerLeft',(-66,-1140,593),(2,58,16),dark,True)
block('DrawerRight',(-14,-1140,593),(2,58,16),dark,True)
block('DrawerBack',(-40,-1112,593),(52,2,16),dark,True)
block('Handle',(-40,-1174,601),(19,3,2),brass,True)
for i in range(4):block('LockDial'+str(i),(-49+i*6,-1172,591),(4,1,5),brass,True)
arrival=actor(unreal.TargetPoint,'LV4_FinalBossArrival',(0,500,1085))
arrival.set_actor_rotation(unreal.Rotator(yaw=-90),False)
g=actor(unreal.TripoChapterGift,'LV4_CipherChapterGift',(-40,-1140,586))
g.set_editor_property('gift_id','LV4.Company.Cipher.Level2')
g.set_editor_property('completion_event','Story.Company.CipherComplete')
g.set_editor_property('chapter_caption','第三章  /  公司')
g.set_editor_property('exit_hint','收下这份礼物，继续向前。')
g.set_editor_property('bFadeToRoom',True);g.set_editor_property('arrival_point',arrival);g.set_editor_property('bEnabled',False)
for c,path in [(g.box_mesh,'/Game/Blueprint/Gift/SM_HomeGiftBody'),(g.lid_mesh,'/Game/Blueprint/Gift/SM_HomeGiftLid')]:
    c.set_static_mesh(unreal.load_asset(path));c.set_relative_location(unreal.Vector(),False,True);c.set_relative_scale3d(unreal.Vector(.42,.42,.42))
g.prompt.set_hidden_in_game(True)
for t in g.get_components_by_class(unreal.TripoInteractionTarget):
    if t!=g.portal_interaction:t.set_relative_location(unreal.Vector(0,0,10),False,True);t.set_box_extent(unreal.Vector(13,13,12));t.set_editor_property('prompt','打开礼物 · 选择能力')
p.set_editor_property('reward',g)
unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
print('OFFICE_CIPHER_PLACED',p.get_path_name())
