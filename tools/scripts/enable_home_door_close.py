import unreal
bp=unreal.load_asset('/Game/Blueprint/Door/BP_CanOpenDoor')
s=unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
f=unreal.SubobjectDataBlueprintFunctionLibrary
for h in s.k2_gather_subobject_data_for_blueprint(bp):
    o=f.get_object_for_blueprint(f.get_data(h),bp)
    if isinstance(o,unreal.TripoInteractionTarget):
        o.set_editor_property('bToggleDoorTimeline',True)
        o.set_editor_property('single_use',False)
        o.set_editor_property('bEnabled',True)
        o.set_editor_property('prompt','开门')
unreal.BlueprintEditorLibrary.compile_blueprint(bp)
unreal.EditorAssetLibrary.save_loaded_asset(bp)
print('DOOR_TOGGLE_SAVED')
