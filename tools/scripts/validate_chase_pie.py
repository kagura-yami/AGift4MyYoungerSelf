"""Cross-frame integration checks in an already-running L_ChaseWhitebox PIE world.

Only mutates PIE actors. Writes evidence to tools/evaluation/chase-pie.json.
"""
import json
import time
from pathlib import Path
import unreal

world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
assert world and 'ChaseWhitebox' in world.get_name(), 'Start PIE in L_ChaseWhitebox first'
player = unreal.GameplayStatics.get_player_character(world, 0)
trigger = unreal.GameplayStatics.get_all_actors_of_class(world, unreal.TripoChaseTrigger)[0]
runtime = unreal.TripoRuntimeSubsystem.get_runtime(player)
report = Path(unreal.Paths.project_dir()).resolve().parent / 'tools/evaluation/chase-pie.json'
checks = []
samples = []
phase = 0
phase_at = time.monotonic()
npc = None
marker = None

def check(name, result):
    checks.append({'name': name, 'passed': bool(result)})
    if not result:
        raise AssertionError(name)

def place(actor, xyz, yaw=0):
    actor.set_actor_location_and_rotation(unreal.Vector(*xyz), unreal.Rotator(pitch=0, yaw=yaw, roll=0), False, True)
    actor.character_movement.stop_movement_immediately()

def finish(error=None):
    global callback
    runtime.set_pause_reason(unreal.TripoPauseReason.MENU, False)
    unreal.unregister_slate_post_tick_callback(callback)
    report.write_text(json.dumps({'passed': error is None, 'error': error, 'checks': checks, 'route_samples': samples}, ensure_ascii=False, indent=2), encoding='utf-8')
    unreal.log('CHASE_PIE_RESULT ' + str(report) + ' success=' + str(error is None))

place(player, (400, 0, 100))

def tick(dt):
    global phase, phase_at, npc, marker
    elapsed = time.monotonic() - phase_at
    try:
        if phase == 0 and elapsed > .7:
            npc = trigger.active_chaser
            check('Entering the trigger spawns exactly one chaser', npc and len(unreal.GameplayStatics.get_all_actors_of_class(world, unreal.TripoChaser)) == 1)
            check('Visible player starts pursuit', npc.state == unreal.TripoChaseState.CHASING)
            check('NPC actually moves with navigation', npc.get_actor_location().x > -620)
            check('Valid floor produces a complete path', not npc.navigation_blocked)
            place(player, (1800, 0, 100))
            npc.set_editor_property('catch_hold_seconds', 100)
            npc.set_editor_property('lost_aggro_per_second', 0)
            npc.start_chase(player)
            phase += 1
            phase_at = time.monotonic()
        elif phase == 1:
            v = npc.get_actor_location()
            samples.append([round(v.x, 1), round(v.y, 1)])
            if elapsed > 10:
                check('Navigation routes around the wall', max(abs(v[1]) for v in samples) > 500)
                check('NPC reaches the far side rather than clipping the wall', v.x > 1200)
                place(player, (2330, 750, 100))
                place(npc, (1750, 750, 100))
                npc.set_editor_property('lost_aggro_per_second', 12)
                npc.start_chase(player)
                phase += 1
                phase_at = time.monotonic()
        elif phase == 2 and elapsed > 2.5:
            check('Hiding drains aggro to zero', npc.aggro == 0)
            check('Zero aggro transitions to wandering', npc.state == unreal.TripoChaseState.WANDERING)
            marker = npc.get_actor_location()
            phase += 1
            phase_at = time.monotonic()
        elif phase == 3 and elapsed > 3:
            check('NPC keeps moving after aggro clears', npc.get_actor_location().distance(marker) > 40)
            place(player, (2400, -950, 100))
            place(npc, (2300, -950, 100))
            npc.set_editor_property('catch_hold_seconds', .15)
            npc.set_editor_property('spawn_grace_seconds', 0)
            npc.start_chase(player)
            phase += 1
            phase_at = time.monotonic()
        elif phase == 4 and elapsed > .3:
            check('Instant hide clears aggro in a frame', npc.aggro == 0)
            check('Hidden player cannot be caught', npc.state != unreal.TripoChaseState.CAUGHT)
            place(player, (1800, -950, 100))
            place(npc, (2400, -950, 100), 180)
            phase += 1
            phase_at = time.monotonic()
        elif phase == 5 and elapsed > .4:
            check('Leaving hiding reacquires target', npc.state == unreal.TripoChaseState.CHASING and npc.aggro > 0)
            runtime.set_pause_reason(unreal.TripoPauseReason.MENU, True)
            phase += 1
            phase_at = time.monotonic()
        elif phase == 6 and elapsed > .3:
            marker = (npc.get_actor_location(), npc.aggro)
            phase += 1
            phase_at = time.monotonic()
        elif phase == 7 and elapsed > .6:
            check('Menu pause freezes NPC movement', npc.get_actor_location().distance(marker[0]) < 1)
            check('Menu pause freezes aggro', npc.aggro == marker[1])
            runtime.set_pause_reason(unreal.TripoPauseReason.MENU, False)
            place(player, (1200, 0, 100))
            place(npc, (1000, 0, 100))
            npc.set_editor_property('catch_surface_distance', 200)
            npc.set_editor_property('chase_speed_ratio', 0)
            npc.set_editor_property('full_aggro_speed_bonus', 0)
            npc.set_editor_property('search_speed_ratio', 0)
            npc.character_movement.disable_movement()
            phase += 1
            phase_at = time.monotonic()
        elif phase == 8 and elapsed > .4:
            check('No catch through solid wall', npc.state != unreal.TripoChaseState.CAUGHT)
            place(player, (1600, -700, 350))
            player.character_movement.disable_movement()
            place(npc, (1600, -700, 100))
            npc.set_editor_property('catch_surface_distance', 12)
            phase += 1
            phase_at = time.monotonic()
        elif phase == 9 and elapsed > .4:
            check('Jumping above NPC does not count as contact', npc.state != unreal.TripoChaseState.CAUGHT)
            player.character_movement.set_movement_mode(unreal.MovementMode.MOVE_WALKING)
            place(player, (1672, -700, 100))
            marker = runtime.get_epoch()
            phase += 1
            phase_at = time.monotonic()
        elif phase == 10 and elapsed > .5:
            check('Sustained contact restores a checkpoint', runtime.get_epoch() > marker)
            check('Player returns to safe spawn', player.get_actor_location().distance(unreal.Vector(0, 0, 100)) < 30)
            check('Restore removes the old NPC', not trigger.active_chaser)
            place(player, (400, 0, 100))
            phase += 1
            phase_at = time.monotonic()
        elif phase == 11 and elapsed > .4:
            check('Trigger rearms for retry', bool(trigger.active_chaser))
            trigger.end_chase(False)
            check('Explicit finish removes the NPC', not trigger.active_chaser)
            finish()
    except Exception as exc:
        finish(str(exc))

callback = unreal.register_slate_post_tick_callback(tick)
print('CHASE_PIE_STARTED')
