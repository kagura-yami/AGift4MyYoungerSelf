import unreal
es=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
ls=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
assert not es.get_game_world()
if es.get_editor_world().get_name()!='lv4': ls.load_level('/Game/Maps/lv4')
task=unreal.AssetImportTask()
task.filename='C:/Dev/Projects/UE/Tripothon/game/Content/Characters/Materials/T_Son_Palette.png'
task.destination_path='/Game/Characters/Materials'
task.automated=True;task.replace_existing=True;task.save=True
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
tex=unreal.load_asset('/Game/Characters/Materials/T_Son_Palette')
tex.set_editor_property('filter',unreal.TextureFilter.TF_NEAREST)
mat=unreal.load_asset('/Game/Characters/Materials/M_Son_Colored')
if not mat:
 mat=unreal.AssetToolsHelpers.get_asset_tools().create_asset('M_Son_Colored','/Game/Characters/Materials',unreal.Material,unreal.MaterialFactoryNew())
 sample=unreal.MaterialEditingLibrary.create_material_expression(mat,unreal.MaterialExpressionTextureSample,0,0)
 sample.texture=tex
 unreal.MaterialEditingLibrary.connect_material_property(sample,'RGB',unreal.MaterialProperty.MP_BASE_COLOR)
 rough=unreal.MaterialEditingLibrary.create_material_expression(mat,unreal.MaterialExpressionConstant,0,200)
 rough.r=.8
 unreal.MaterialEditingLibrary.connect_material_property(rough,'',unreal.MaterialProperty.MP_ROUGHNESS)
 unreal.MaterialEditingLibrary.recompile_material(mat)
son=unreal.load_asset('/Game/Models/juese/SK_MiniCharacter_Son_01')
unreal.MaterialEditingLibrary.set_material_usage(mat,unreal.MaterialUsage.MATUSAGE_SKELETAL_MESH)
unreal.MaterialEditingLibrary.recompile_material(mat)
slots=list(son.materials);slots[0].material_interface=mat
son.set_editor_property('materials',slots)
for a in [tex,mat,son]:
 a.modify();assert unreal.EditorAssetLibrary.save_loaded_asset(a,False)
by={a.get_actor_label():a for a in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()}
trigger=by['LV4_OfficeDoor_ChaseTrigger'];trigger.modify()
trigger.set_editor_property('exit_zones',[a for a in by.values() if isinstance(a,unreal.TripoChaseHideZone)])
patrol=by['LV4_CorridorPatrol'];patrol.modify()
patrol.set_editor_property('search_speed_ratio',.42)
assert ls.save_current_level()
print('PLAYER_CHASE_CONFIGURED',len(trigger.exit_zones),patrol.search_speed_ratio)
