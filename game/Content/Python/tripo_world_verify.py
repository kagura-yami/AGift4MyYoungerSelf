"""Real PIE overlap, recovery, and reward-transaction regression. No input during run."""
import json
import time
from pathlib import Path
import unreal

_handle = None


def run(report_path):
    global _handle
    assert _handle is None
    world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
    player = unreal.GameplayStatics.get_player_character(world, 0)
    if player is None:
        pawns = unreal.GameplayStatics.get_all_actors_of_class(world, unreal.TripoCharacter)
        player = pawns[0] if len(pawns) == 1 else None
    assert player, 'One real TripoCharacter is required (PIE or explicitly spawned simulation fixture)'
    runtime = unreal.TripoRuntimeSubsystem.get_runtime(world)
    recovery = unreal.TripoWorldSubsystem.get(world)
    progress = unreal.TripoProgressSubsystem.get(world)
    devices = unreal.GameplayStatics.get_all_actors_of_class(world, unreal.TripoMechanism)
    plate = next(a for a in devices if a.kind == unreal.TripoMechanismKind.PLATE)
    switch = next(a for a in devices if a.kind == unreal.TripoMechanismKind.SWITCH)
    gate = next(a for a in devices if a.kind == unreal.TripoMechanismKind.GATE)
    definition = unreal.load_asset('/Game/Data/DA_LabChallenge')
    report = Path(report_path)
    state = {'phase': 0, 'at': time.monotonic(), 'checks': []}
    report.write_text(json.dumps({'ok': None}))

    def check(value, name):
        assert value, name
        state['checks'].append(name)

    def move(x, y, z):
        player.character_movement.stop_movement_immediately()
        player.set_actor_location(unreal.Vector(x, y, z), False, True)

    def advance():
        state.update(phase=state['phase'] + 1, at=time.monotonic())

    def tick(delta):
        global _handle
        try:
            age = time.monotonic() - state['at']
            if state['phase'] == 0:
                move(2700, 0, 115)
                advance()
            elif state['phase'] == 1 and age > .3:
                check(plate.is_powered(), 'Actual pawn overlap powers plate')
                check(gate.is_powered(), 'Plate opens OR gate')
                move(2800, 250, 100)
                advance()
            elif state['phase'] == 2 and age > .3:
                check(not plate.is_powered(), 'Leaving plate clears occupancy')
                check(switch.interact(player), 'Nearby player can use switch')
                check(gate.is_powered(), 'Switch independently powers OR gate')
                move(2200, -500, 100)
                advance()
            elif state['phase'] == 3 and age > .3:
                check(progress.start(player, definition), 'Start captures safe checkpoint')
                state['epoch'] = runtime.get_epoch()
                state['start'] = progress.get_elapsed()
                advance()
            elif state['phase'] == 4 and age > .3:
                move(2200, -500, -950)
                advance()
            elif state['phase'] == 5 and age > .4:
                check(player.get_actor_location().z > 0, 'Fall automatically restores safe position')
                check(runtime.get_epoch() == state['epoch'] + 1, 'Recovery changes history epoch once')
                check(progress.get_elapsed() > state['start'] + .3, 'Ordinary death does not reset challenge time')
                check(runtime.get_restore_phase() == unreal.TripoRestorePhase.RUNNING, 'Recovery releases lock')
                check(progress.restart(player), 'Explicit restart succeeds')
                check(progress.get_elapsed() < .1, 'Explicit restart resets elapsed time')
                check(progress.finish(player, 'Lab.FirstRoute'), 'Finish freezes result')
                state['elapsed'] = progress.get_elapsed()
                state['level'] = player.abilities.get_level(unreal.TripoAbility.DASH)
                advance()
            elif state['phase'] == 6 and age > .3:
                check(progress.get_elapsed() == state['elapsed'], 'Reward screen cannot change finish time')
                check(not progress.finish(player, 'Lab.FirstRoute'), 'Repeated finish cannot recreate candidate snapshot')
                check(progress.commit_reward(player, 0), 'Reward transaction commits')
                check(not progress.commit_reward(player, 0), 'Repeated reward cannot commit twice')
                check(player.abilities.get_level(unreal.TripoAbility.DASH) == state['level'] + 1, 'Exactly one upgrade granted')
                check(progress.has_completed('Lab.FirstRoute'), 'Completion ledger recorded')
                report.write_text(json.dumps({'ok': True, **state}, indent=2))
                unreal.unregister_slate_post_tick_callback(_handle)
                _handle = None
        except Exception as exc:
            report.write_text(json.dumps({'ok': False, 'error': str(exc), **state}, indent=2))
            unreal.unregister_slate_post_tick_callback(_handle)
            _handle = None

    _handle = unreal.register_slate_post_tick_callback(tick)
    return {'running': True}
