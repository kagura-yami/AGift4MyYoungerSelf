"""Third-floor suit NPC and the safe-code arrival point. Run in lv4, outside PIE."""
import unreal, shutil, time
from pathlib import Path
es=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
aa=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
ls=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
assert not es.get_game_world() and es.get_editor_world().get_name()=='lv4'
root=Path(unreal.Paths.project_dir()).resolve().parent
out=root/'tools/evaluation/third-floor-npc';out.mkdir(parents=True,exist_ok=True)
ls.save_current_level()
shutil.copy2(root/'game/Content/Maps/lv4.umap',out/('before-'+time.strftime('%Y%m%d-%H%M%S')+'.umap'))
by={a.get_actor_label():a for a in aa.get_all_level_actors()}
source=by['SK_Character_BusinessMan_Suit3'].skeletal_mesh_component
path='/Game/Blueprint/Lv4/BP_ThirdFloorSuitNPC'
bp=unreal.load_asset(path)
if not bp:
    factory=unreal.BlueprintFactory();factory.set_editor_property('parent_class',unreal.TripoDialogueNPC)
    bp=unreal.AssetToolsHelpers.get_asset_tools().create_asset('BP_ThirdFloorSuitNPC','/Game/Blueprint/Lv4',unreal.Blueprint,factory)
cls=unreal.EditorAssetLibrary.load_blueprint_class(path)
cdo=unreal.get_default_object(cls)
cdo.modify();cdo.visual.set_skeletal_mesh_asset(source.get_skinned_asset())
cdo.visual.set_editor_property('override_materials',list(source.get_materials()))
cdo.visual.set_relative_rotation(unreal.Rotator(),False,True)
cdo.set_editor_property('idle_animation',unreal.load_asset('/Game/Models/lv4/A_Merchant_Idle'))
cdo.visual.set_editor_property('animation_data',source.get_editor_property('animation_data'))
catalog_path='/Game/Blueprint/Lv4/DA_ThirdFloorSuitDialogue'
catalog=unreal.load_asset(catalog_path)
if not catalog:
    factory=unreal.DataAssetFactory();factory.set_editor_property('data_asset_class',unreal.TripoStoryCatalog)
    catalog=unreal.AssetToolsHelpers.get_asset_tools().create_asset('DA_ThirdFloorSuitDialogue','/Game/Blueprint/Lv4',unreal.TripoStoryCatalog,factory)
    event=unreal.TripoStoryEvent()
    event.id='Story.Company.ThirdFloorSuit'
    event.title=unreal.Text('西装男')
    event.text=unreal.Text('你找到这里了。我们聊聊吧。')
    event.grant_floors=[0]*8
    catalog.set_editor_property('events',[event])
cdo.set_editor_property('dialogue_catalog',catalog)
cdo.set_editor_property('dialogue_event','Story.Company.ThirdFloorSuit')
unreal.EditorAssetLibrary.save_loaded_asset(catalog,False)
unreal.EditorAssetLibrary.save_loaded_asset(bp,False)
npc=by.get('LV4_ThirdFloorSuitNPC') or aa.spawn_actor_from_class(cls,unreal.Vector(0,200,990.34),unreal.Rotator(yaw=90))
npc.modify();npc.set_actor_label('LV4_ThirdFloorSuitNPC');npc.set_folder_path('Gameplay/ThirdFloor')
npc.set_actor_location_and_rotation(unreal.Vector(0,200,990.34),unreal.Rotator(yaw=90),False,True)
# Explicit instance configuration also supports rerunning after an earlier spawn.
npc.visual.set_skeletal_mesh_asset(source.get_skinned_asset())
npc.visual.set_editor_property('override_materials',list(source.get_materials()))
npc.visual.set_relative_rotation(unreal.Rotator(),False,True)
npc.visual.set_editor_property('animation_data',source.get_editor_property('animation_data'))
npc.set_editor_property('idle_animation',cdo.idle_animation)
npc.set_editor_property('dialogue_catalog',catalog);npc.set_editor_property('dialogue_event',cdo.dialogue_event)
arrival=by['LV4_CipherArrival'];arrival.modify()
arrival.set_actor_location_and_rotation(unreal.Vector(0,500,1085),unreal.Rotator(yaw=-90),False,True)
by['LV4_OfficeCipher'].modify();by['LV4_OfficeCipher'].set_editor_property('code_destination',arrival)
ls.save_current_level();aa.set_selected_level_actors([npc,arrival])
print('NPC',npc.get_actor_location(),'arrival',arrival.get_actor_location(),'separation 300 cm')
