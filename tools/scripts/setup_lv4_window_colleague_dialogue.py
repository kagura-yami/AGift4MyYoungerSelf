"""Give the colleague beside the office doorway a repeatable, aim-selected hint."""
import unreal, shutil
from pathlib import Path

es = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
assert es.get_editor_world().get_name() == 'lv4'
aa = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
npc = next(a for a in aa.get_all_level_actors() if a.get_actor_label() == 'SK_Character_BusinessMan_Suit')
root = Path(unreal.Paths.project_dir()).resolve()
backup = root.parent / 'tools/evaluation/lv4-before-window-colleague.umap'
if not backup.exists():
    shutil.copy2(root / 'Content/Maps/lv4.umap', backup)

path = '/Game/Story/DA_OfficeWindowColleague'
catalog = unreal.load_asset(path)
if not catalog:
    factory = unreal.DataAssetFactory()
    factory.set_editor_property('data_asset_class', unreal.TripoStoryCatalog)
    catalog = unreal.AssetToolsHelpers.get_asset_tools().create_asset('DA_OfficeWindowColleague', '/Game/Story', unreal.TripoStoryCatalog, factory)
event = unreal.TripoStoryEvent()
event.set_editor_property('id', 'Story.Company.ColleagueWindowHint')
event.set_editor_property('title', '同事')
event.set_editor_property('text', '同事：你要是能穿过这个窗户，就能跳槽出去了。\n同事：那边待遇比咱们这儿高多了，还不用加班。')
event.set_editor_property('grant_floors', [0] * 8)
catalog.set_editor_property('events', [event])
assert unreal.EditorAssetLibrary.save_loaded_asset(catalog)

mesh = npc.get_component_by_class(unreal.SkeletalMeshComponent)
focus = npc.get_component_by_class(unreal.TripoInteractionTarget)
if not focus:
    s = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
    f = unreal.SubobjectDataBlueprintFunctionLibrary
    parent = next(h for h in s.k2_gather_subobject_data_for_instance(npc) if f.get_associated_object(f.get_data(h)) == mesh)
    handle, reason = s.add_new_subobject(unreal.AddNewSubobjectParams(parent_handle=parent, new_class=unreal.TripoInteractionTarget))
    assert not str(reason), reason
    s.rename_subobject(handle, 'ColleagueConversation')
    focus = f.get_associated_object(f.get_data(handle))
focus.set_editor_property('story_catalog', catalog)
focus.set_editor_property('story_event', event.get_editor_property('id'))
focus.set_editor_property('prompt', '和同事交谈')
focus.set_editor_property('reach', 260)
focus.set_editor_property('single_use', False)
focus.set_editor_property('bNPCConversation', True)
focus.set_editor_property('highlight_mesh', mesh)
focus.set_editor_property('highlight_material', None)
focus.set_relative_location(unreal.Vector(0, 0, 95), False, True)
focus.set_box_extent(unreal.Vector(38, 38, 85))
assert unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
print('Colleague dialogue saved:', npc.get_actor_label(), focus.get_path_name())



