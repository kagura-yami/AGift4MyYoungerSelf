import unreal
ls=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
aa=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
ls.load_level('/Game/Maps/HomeVertical/Maps/L_Home_Vertical_02')
actors=aa.get_all_level_actors()
by={a.get_actor_label():a for a in actors}
door=next(a for a in actors if a.get_name()=='BP_NotCanOpenDoor_C_5')
assert (door.get_actor_location()-unreal.Vector(2503.299,200.759,2100)).length()<5
door.modify();door.set_actor_enable_collision(False)
folder='/Game/Blueprint/Door';name='BP_HomeSchoolExit'
bp=unreal.load_asset(folder+'/'+name)
if not bp:
    factory=unreal.BlueprintFactory();factory.set_editor_property('parent_class',unreal.TripoChapterExit)
    bp=unreal.AssetToolsHelpers.get_asset_tools().create_asset(name,folder,unreal.Blueprint,factory)
cdo=unreal.get_default_object(bp.generated_class());cdo.modify()
cdo.set_editor_property('destination','/Game/Maps/School/School_v2')
unreal.EditorAssetLibrary.save_loaded_asset(bp,False)
loc=unreal.Vector(2480,201,2220)
exit=by.get('Home_Door_ToSchool') or aa.spawn_actor_from_class(bp.generated_class(),loc)
exit.modify();exit.set_actor_label('Home_Door_ToSchool');exit.set_folder_path('Gameplay/ChapterExit')
exit.set_actor_location(loc,False,True);exit.volume.set_box_extent(unreal.Vector(35,65,110))
exit.set_editor_property('destination','/Game/Maps/School/School_v2')
safe=by.get('Home_Door_ExitSafe') or aa.spawn_actor_from_class(unreal.TripoZone,loc)
safe.modify();safe.set_actor_label('Home_Door_ExitSafe');safe.set_folder_path('Gameplay/ChapterExit')
safe.set_editor_property('kind',unreal.TripoZoneKind.SAFE);safe.set_editor_property('hint','')
safe.get_component_by_class(unreal.TextRenderComponent).set_visibility(False);safe.volume.set_box_extent(unreal.Vector(110,100,130))
ls.save_current_level();aa.set_selected_level_actors([exit])
print('EXIT',exit.get_actor_location(),exit.destination)
