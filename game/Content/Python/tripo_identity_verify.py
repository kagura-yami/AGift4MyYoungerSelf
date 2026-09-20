"""Editor-only Lab identity save/reload/duplicate check. Run with PIE stopped."""
import json
from pathlib import Path
import unreal

def run(report_path):
    editor = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    def regions():
        return [a for a in editor.get_all_level_actors() if isinstance(a,unreal.TripoCameraVolume)]
    def ids():
        return sorted(a.get_component_by_class(unreal.TripoIdentityComponent).get_stable_id().to_string() for a in regions())
    before = ids()
    assert len(before) == len(set(before)) == 2
    duplicate = editor.duplicate_actor(regions()[0])
    duplicate_id = duplicate.get_component_by_class(unreal.TripoIdentityComponent).get_stable_id().to_string()
    assert duplicate_id not in before
    editor.destroy_actor(duplicate)
    world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
    assert unreal.EditorLoadingAndSavingUtils.save_map(world,'/Game/Maps/L_LogicLab')
    unreal.EditorLoadingAndSavingUtils.load_map('/Game/Maps/L_LogicLab')
    after = ids()
    result = {'ok':before==after,'ids':before,'reloaded':after,'duplicate':duplicate_id,'reload_preserved':before==after}
    Path(report_path).write_text(json.dumps(result,indent=2),encoding='utf-8')
    assert result['ok'], result
    return result
