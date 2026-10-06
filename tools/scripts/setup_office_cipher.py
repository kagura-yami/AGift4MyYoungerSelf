"""Configure the existing office puzzle and its editable Blueprint destination."""
import unreal, shutil, time
from pathlib import Path
es=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
aa=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
ls=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
assert not es.get_game_world() and es.get_editor_world().get_name()=='lv4'
root=Path(unreal.Paths.project_dir()).resolve().parent
out=root/'tools/evaluation/office-cipher';out.mkdir(parents=True,exist_ok=True)
ls.save_current_level()
shutil.copy2(root/'game/Content/Maps/lv4.umap',out/('before-'+time.strftime('%Y%m%d-%H%M%S')+'.umap'))
by={a.get_actor_label():a for a in aa.get_all_level_actors()}
p=by['LV4_OfficeCipher'];p.modify()
path='/Game/Blueprint/Lv4/BP_CipherArrival'
bp=unreal.load_asset(path)
if not bp:
    factory=unreal.BlueprintFactory();factory.set_editor_property('parent_class',unreal.TargetPoint)
    bp=unreal.AssetToolsHelpers.get_asset_tools().create_asset('BP_CipherArrival','/Game/Blueprint/Lv4',unreal.Blueprint,factory)
unreal.EditorAssetLibrary.save_loaded_asset(bp,False)
target=by.get('LV4_CipherArrival')
if not target:
    target=aa.spawn_actor_from_class(unreal.EditorAssetLibrary.load_blueprint_class(path),unreal.Vector(0,620,660),unreal.Rotator(0,0,0))
target.set_actor_label('LV4_CipherArrival');target.set_folder_path('Gameplay/OfficeCipher')
p.set_editor_property('code_destination',target)
p.card_target.modify()
# Keep physical/camera collision but let focus rays reach the visible panel.
# Enclosing the entire hull would catch rays from a camera behind the player
# even when the player looks away from the board.
board=by['LV4_CipherWhiteboard'].static_mesh_component
board.modify()
board.set_collision_profile_name('Custom')
board.set_collision_response_to_channel(unreal.CollisionChannel.ECC_VISIBILITY,unreal.CollisionResponseType.ECR_IGNORE)
p.card_target.set_world_location_and_rotation(unreal.Vector(-20,-1449,705),unreal.Rotator(),False,True)
p.card_target.set_world_scale3d(unreal.Vector(1,1,1))
p.card_target.set_box_extent(unreal.Vector(80,5,50))
p.card_target.set_editor_property('highlight_mesh',by['LV4_CipherWhiteboard'].static_mesh_component)
p.card_target.set_editor_property('prompt',unreal.Text('点击画板 · 获得打孔卡'))
p.drawer_target.set_editor_property('prompt',unreal.Text('输入保险箱密码'))
# Wrap the complete cabinet, including handle/dials, so these separate actors
# do not intercept focus or the character-to-target reachability trace.
p.drawer_target.modify()
p.drawer_target.set_world_location_and_rotation(unreal.Vector(-40,-1142,595),unreal.Rotator(),False,True)
p.drawer_target.set_world_scale3d(unreal.Vector(1,1,1))
p.drawer_target.set_box_extent(unreal.Vector(35,40,35))
ls.save_current_level()
aa.set_selected_level_actors([target])
print('Cipher destination:',target.get_path_name(),target.get_actor_location())
