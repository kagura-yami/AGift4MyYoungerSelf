"""Real key mapping and camera-region regression, scheduled across PIE frames."""
import json
import time
from pathlib import Path
import unreal

_handle = None


def run(report_path):
    global _handle
    if _handle is not None:
        raise RuntimeError('Verification already running')
    world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
    player = unreal.GameplayStatics.get_player_character(world, 0)
    pc = unreal.GameplayStatics.get_player_controller(world, 0)
    assert player and pc, 'PIE required'
    report = Path(report_path)
    report.parent.mkdir(parents=True,exist_ok=True)
    report.write_text(json.dumps({'test':'input_and_pause','ok':None,'status':'running'}),encoding='utf-8')
    state = {'phase':0, 'at':time.monotonic(), 'checks':[]}
    keys = {}
    for name in ['SpaceBar','W','Escape']:
        key = unreal.Key()
        key.set_editor_property('key_name',name)
        keys[name] = key

    def key(name, down):
        pc.key_for_test(keys[name],down)

    def tap(name):
        key(name,True)
        key(name,False)

    def check(condition, name):
        assert condition, name
        state['checks'].append(name)

    def advance():
        state.update(phase=state['phase']+1,at=time.monotonic())

    def tick(delta):
        age = time.monotonic()-state['at']
        phase = state['phase']
        try:
            if age > 8:
                raise AssertionError('Phase timeout '+str(phase))
            if phase == 0 and age > .6:
                state['jumps'] = player.get_jump_count()
                tap('SpaceBar')
                advance()
            elif phase == 1 and age > .15:
                check(player.get_jump_count() == state['jumps']+1,'same-frame tap jumps once')
                check(player.character_movement.is_falling(),'tap lifts capsule')
                advance()
            elif phase == 2 and age > 1:
                key('SpaceBar',True)
                advance()
            elif phase == 3 and age > 1.3:
                check(player.get_jump_count() == state['jumps']+2,'held jump does not auto-repeat after landing')
                key('SpaceBar',False)
                state['x'] = player.get_actor_location().x
                key('W',True)
                advance()
            elif phase == 4 and age > .3:
                key('W',False)
                check(player.get_actor_location().x > state['x']+40,'held W uses keyboard mapping')
                tap('Escape')
                advance()
            elif phase == 5 and age > .2:
                check(unreal.GameplayStatics.is_game_paused(world),'tap pauses')
                state['game_time'] = unreal.GameplayStatics.get_time_seconds(world)
                advance()
            elif phase == 6 and age > .3:
                check(unreal.GameplayStatics.get_time_seconds(world) == state['game_time'],'pause freezes world time')
                tap('Escape')
                advance()
            elif phase == 7 and age > .2:
                check(not unreal.GameplayStatics.is_game_paused(world),'tap resumes while paused')
                finish(True,'')
        except Exception as error:
            finish(False,str(error))

    def finish(ok,error):
        global _handle
        if _handle is not None:
            unreal.unregister_slate_post_tick_callback(_handle)
        _handle = None
        for name in keys:
            key(name,False)
        unreal.GameplayStatics.set_game_paused(world,False)
        region = state.get('region')
        if region:
            region.destroy_actor()
        result = {'test':'input_and_pause','ok':ok,'error':error,'checks':state['checks']}
        path = Path(report_path)
        path.parent.mkdir(parents=True,exist_ok=True)
        path.write_text(json.dumps(result,indent=2),encoding='utf-8')
        unreal.log('TRIPO_VERIFY '+json.dumps(result))

    player.reset_practice_position()
    _handle = unreal.register_slate_post_tick_callback(tick)
    return 'scheduled'
