"""PIE movement/collision and ability lifetime checks using the actual character."""
import json
import time
from pathlib import Path
import unreal
_handle = None


def run(report_path):
    global _handle
    assert _handle is None
    world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
    p = unreal.GameplayStatics.get_player_character(world, 0)
    assert p
    a = p.abilities
    for ability in [unreal.TripoAbility.DASH, unreal.TripoAbility.UP_DASH, unreal.TripoAbility.STEP_STONE, unreal.TripoAbility.SLOW]:
        a.grant_level_floor(ability, 1)
    report = Path(report_path)
    report.write_text(json.dumps({'ok': None}))
    state = {'phase': 0, 'at': time.monotonic(), 'checks': [], 'samples': []}

    def check(value, name):
        assert value, name
        state['checks'].append(name)

    def advance():
        state.update(phase=state['phase']+1, at=time.monotonic())

    def move(x, y, z):
        p.character_movement.stop_movement_immediately()
        p.set_actor_location(unreal.Vector(x,y,z), False, True)

    def tick(delta):
        global _handle
        try:
            age = time.monotonic()-state['at']
            if state['phase'] == 0:
                p.look_for_test(0)
                move(-500,0,100)
                advance()
            elif state['phase'] == 1 and age > 1.6:
                state['start'] = p.get_actor_location().x
                check(a.try_activate(unreal.TripoAbility.DASH,None)[0] == unreal.TripoAbilityFailure.NONE, 'Ground dash activates')
                advance()
            elif state['phase'] == 2 and age > .3:
                distance = p.get_actor_location().x-state['start']
                state['samples'].append({'dash_distance':distance})
                check(440 < distance < 510, 'Dash travels configured distance with swept movement')
                move(850,0,100)
                advance()
            elif state['phase'] == 3 and age > 1.6:
                check(a.try_activate(unreal.TripoAbility.DASH,None)[0] == unreal.TripoAbilityFailure.NONE, 'Next grounded dash rearmed')
                advance()
            elif state['phase'] == 4 and age > .3:
                state['samples'].append({'wall_stop_x':p.get_actor_location().x})
                check(p.get_actor_location().x < 1042, 'Dash cannot cross collision wall')
                move(-500,0,100)
                advance()
            elif state['phase'] == 5 and age > .3:
                check(a.try_activate(unreal.TripoAbility.UP_DASH,None)[0] == unreal.TripoAbilityFailure.NONE, 'Up dash activates')
                advance()
            elif state['phase'] == 6 and age > .15:
                check(p.get_actor_location().z > 150, 'Up dash moves actual capsule upward')
                check(a.try_activate(unreal.TripoAbility.SLOW,None)[0] == unreal.TripoAbilityFailure.NO_TARGET, 'Slow without target fails')
                check(a.get_cooldown_remaining(unreal.TripoAbility.SLOW) == 0, 'Missing slow target does not start cooldown')
                check(a.try_activate(unreal.TripoAbility.STEP_STONE,None)[0] == unreal.TripoAbilityFailure.BLOCKED, 'Stone outside allowed area fails')
                check(a.get_cooldown_remaining(unreal.TripoAbility.STEP_STONE) == 0, 'Invalid placement does not start cooldown')
                report.write_text(json.dumps({'ok':True,**state},indent=2))
                unreal.unregister_slate_post_tick_callback(_handle)
                _handle = None
        except Exception as exc:
            report.write_text(json.dumps({'ok':False,'error':str(exc),**state},indent=2))
            unreal.unregister_slate_post_tick_callback(_handle)
            _handle = None
    _handle = unreal.register_slate_post_tick_callback(tick)
