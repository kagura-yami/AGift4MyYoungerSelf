"""PIE camera pitch and yaw-only ground movement regression."""
import json
import time
from pathlib import Path
import unreal

_handle = None

def run(report_path):
    global _handle
    assert _handle is None
    world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
    pawn = unreal.GameplayStatics.get_player_character(world, 0)
    pc = unreal.GameplayStatics.get_player_controller(world, 0)
    camera = unreal.GameplayStatics.get_player_camera_manager(world, 0)
    subsystem = next(o for o in unreal.ObjectIterator(unreal.EnhancedInputLocalPlayerSubsystem) if 'LocalPlayer_0.' in o.get_path_name())
    actions = {o.get_name(): o for o in unreal.ObjectIterator(unreal.InputAction) if o.get_outer() == pawn}
    settings = unreal.get_default_object(unreal.load_class(None, '/Script/UnrealEd.EditorPerformanceSettings'))
    settings.set_editor_property('bThrottleCPUWhenNotForeground', False)
    report = Path(report_path)
    state = {'phase': 0, 'at': time.monotonic(), 'checks': [], 'samples': []}
    report.write_text(json.dumps({'ok': None}), encoding='utf-8')
    def check(ok, name):
        assert ok, name
        state['checks'].append(name)
    def angle(value): return (value + 180) % 360 - 180
    def inject(name, value): subsystem.inject_input_vector_for_action(actions[name], unreal.Vector(value, 0, 0), [], [])
    def advance(): state.update(phase=state['phase']+1, at=time.monotonic())
    def finish(ok, error=''):
        global _handle
        unreal.unregister_slate_post_tick_callback(_handle)
        _handle = None
        report.write_text(json.dumps({'ok': ok, 'error': error, 'checks': state['checks'], 'samples': state['samples']}, indent=2), encoding='utf-8')
        pawn.reset_practice_position()
    def tick(dt):
        try:
            phase = state['phase']
            age = time.monotonic()-state['at']
            if phase == 0 and age > .5:
                inject('LookPitch', 1000)
                advance()
            elif phase == 1 and age > .2:
                check(abs(angle(camera.get_camera_rotation().pitch)-60)<.1, 'Upward action reaches +60 degree camera limit')
                inject('Look', 30)
                advance()
            elif phase == 2 and age > .2:
                check(abs(angle(camera.get_camera_rotation().pitch)-60)<.1, 'Horizontal look preserves pitch')
                check(abs(angle(pc.get_control_rotation().yaw)-45)<.1, 'Horizontal look changes yaw')
                inject('LookPitch', -1000)
                advance()
            elif phase == 3 and age > .2:
                check(abs(angle(camera.get_camera_rotation().pitch)+75)<.1, 'Downward action reaches -75 degree camera limit')
                check(abs(angle(pc.get_control_rotation().yaw)-45)<.1, 'Vertical look preserves yaw')
                advance()
            elif phase in (4, 6, 8) and age > .2:
                pitch = [-75, 0, 60][(phase-4)//2]
                pawn.reset_practice_position()
                pawn.set_actor_location(unreal.Vector(-500, -500, 90), False, False)
                pc.set_control_rotation(unreal.Rotator(pitch=pitch, yaw=0, roll=0))
                pawn.drive_for_test(1., 1., 0., False)
                advance()
            elif phase in (5, 7, 9) and age > .6:
                v = pawn.get_velocity()
                p = pawn.get_actor_location()
                r = pawn.get_actor_rotation()
                state['samples'].append({'pitch': angle(pc.get_control_rotation().pitch), 'vx': v.x, 'vy': v.y, 'vz': v.z, 'z': p.z})
                check(abs(v.x-450)<1 and abs(v.y)<.1 and abs(v.z)<.1, 'Pitch sample %s keeps horizontal forward speed 450' % phase)
                check(abs(r.pitch)<.1 and abs(r.roll)<.1, 'Pitch sample %s leaves body upright' % phase)
                if phase == 9:
                    check(max(s['z'] for s in state['samples'])-min(s['z'] for s in state['samples'])<.1, 'All pitch samples keep identical ground height')
                    finish(True)
                else:
                    advance()
        except Exception as error:
            finish(False, str(error))
    pawn.reset_practice_position()
    _handle = unreal.register_slate_post_tick_callback(tick)
    return 'scheduled'
