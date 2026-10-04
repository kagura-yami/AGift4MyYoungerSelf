"""Keep the moving cabin header behind the fixed landing door plane."""
import json
import shutil
import time
from pathlib import Path
import unreal

root = Path(unreal.Paths.project_dir()).resolve().parent
out = root / 'tools/evaluation/elevator-rework'
out.mkdir(exist_ok=True)
levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
worlds = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
assert worlds.get_game_world() is None, 'Stop PIE before editing'
assert worlds.get_editor_world().get_name() == 'lv4'
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
by = {a.get_actor_label(): a for a in actors.get_all_level_actors()}
lift = by['LV4_WestElevator']
header = by['LV4_CabinSeal_Header'].static_mesh_component
assert header.get_attach_parent() == lift.cabin
assert levels.save_current_level()
shutil.copy2(root / 'game/Content/Maps/lv4.umap', out / ('lv4-before-header-clearance-' + time.strftime('%Y%m%d-%H%M%S') + '.umap'))
before = str(header.get_relative_transform())
header.modify()
location = header.get_editor_property('relative_location')
scale = header.get_editor_property('relative_scale3d')
location.x = -80
scale.x = .16
header.set_relative_location(location, False, True)
header.set_relative_scale3d(scale)
# Local X increases toward the cabin interior. Preserve a positive gap
# between the header's front face and the landing door's inner face.
door = lift.landing_doors[0]
door_center = door.get_editor_property('relative_location').x
door_half = door.get_editor_property('relative_scale3d').x * 50
gap = location.x - scale.x * 50 - (door_center + door_half)
assert gap > 30, gap
assert levels.save_current_level()
(out / 'header-clearance-fix.json').write_text(json.dumps(dict(before=before, after=str(header.get_relative_transform()), clearance_cm=gap), indent=2), encoding='utf-8')
print('Saved lv4: moving header clearance', gap, 'cm')
