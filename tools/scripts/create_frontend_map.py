import unreal
ls=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
assert not unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
path='/Game/Maps/L_MainMenu'
if unreal.EditorAssetLibrary.does_asset_exist(path):
 assert ls.load_level(path)
else:
 assert ls.new_level(path)
w=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
w.get_world_settings().set_editor_property('default_game_mode',unreal.TripoFrontEndMode)
assert ls.save_current_level()
print('Created independent 2D front-end map',w.get_name())
ls.editor_request_begin_play()
