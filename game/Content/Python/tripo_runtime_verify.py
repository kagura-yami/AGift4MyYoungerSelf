"""Nonblocking T02 checks in a fresh PIE world. Do not interact during the run."""
import json
import time
from pathlib import Path
import unreal

_handle = None

def run(report_path, identity_baseline_path):
    global _handle
    assert _handle is None, 'Already running'
    world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
    runtime = unreal.TripoRuntimeSubsystem.get_runtime(world)
    registry = unreal.TripoIdentityRegistry.get_registry(world)
    assert runtime and registry, 'PIE required'
    report = Path(report_path)
    report.parent.mkdir(parents=True,exist_ok=True)
    report.write_text(json.dumps({'ok':None,'status':'running'}))
    state = {'phase':0,'at':time.monotonic(),'checks':[],'samples':[]}
    original = set(json.loads(Path(identity_baseline_path).read_text())['ids'])
    menu, loading = unreal.TripoPauseReason.MENU, unreal.TripoPauseReason.LOADING

    def check(condition,name):
        assert condition,name
        state['checks'].append(name)

    def advance():
        state.update(phase=state['phase']+1,at=time.monotonic())

    def tick(delta):
        age = time.monotonic()-state['at']
        try:
            if state['phase'] == 0:
                regions = unreal.GameplayStatics.get_all_actors_of_class(world,unreal.TripoCameraVolume)
                check(len(original) == len(regions) == 2,'Two unique region identities')
                for actor in regions:
                    identity = actor.get_component_by_class(unreal.TripoIdentityComponent)
                    check(identity.get_stable_id().to_string() in original,'PIE preserves '+actor.get_name())
                    check(registry.resolve(identity.get_stable_id()) == actor,'ID resolves '+actor.get_name())
                state['actor'] = regions[0]
                state['id'] = regions[0].get_component_by_class(unreal.TripoIdentityComponent).get_stable_id()
                runtime.set_pause_reason(menu,True)
                runtime.set_pause_reason(loading,True)
                state['clock'] = runtime.get_action_seconds()
                advance()
            elif state['phase'] == 1 and age > .3:
                runtime.set_pause_reason(menu,False)
                check(unreal.GameplayStatics.is_game_paused(world),'Loading still pauses after closing menu')
                check(runtime.get_action_seconds() == state['clock'],'Nested pause clock is frozen')
                advance()
            elif state['phase'] == 2 and age > .3:
                runtime.set_pause_reason(loading,False)
                state['clock'] = runtime.get_action_seconds()
                advance()
            elif state['phase'] == 3 and age > .3:
                elapsed = runtime.get_action_seconds()-state['clock']
                check(abs(elapsed-age)<.08,'Resume has no paused catch-up')
                unreal.GameplayStatics.set_global_time_dilation(world,.1)
                state['clock'] = runtime.get_action_seconds()
                advance()
            elif state['phase'] == 4 and age > .4:
                elapsed = runtime.get_action_seconds()-state['clock']
                state['samples'].append({'real':age,'action':elapsed,'dilation':.1})
                check(abs(elapsed-age)<.08,'World slowdown does not slow ActionClock')
                unreal.GameplayStatics.set_global_time_dilation(world,1.)
                phases = unreal.TripoRestorePhase
                check(not runtime.advance_restore(phases.PLAYER_RESTORED),'Skip recovery steps rejected')
                epoch = runtime.get_epoch()
                for phase in [phases.LOCKED,phases.ABILITIES_CLEARED,phases.WORLD_RESTORED,phases.PLAYER_RESTORED,phases.HISTORY_CLEARED,phases.OVERLAPS_REFRESHED,phases.RUNNING]:
                    check(runtime.advance_restore(phase),'Recovery step '+str(phase))
                check(runtime.get_epoch() == epoch+1,'Epoch advances once at history barrier')
                state['actor'].destroy_actor()
                advance()
            elif state['phase'] == 5 and age > .1:
                check(registry.resolve(state['id']) is None,'Destroyed actor is unregistered')
                finish(True,'')
        except Exception as error:
            finish(False,str(error))

    def finish(ok,error):
        global _handle
        if _handle is not None:
            unreal.unregister_slate_post_tick_callback(_handle)
            _handle = None
        runtime.set_pause_reason(menu,False)
        runtime.set_pause_reason(loading,False)
        unreal.GameplayStatics.set_global_time_dilation(world,1.)
        result = {'ok':ok,'error':error,'checks':state['checks'],'samples':state['samples']}
        report.write_text(json.dumps(result,indent=2),encoding='utf-8')
        unreal.log('TRIPO_RUNTIME_VERIFY '+json.dumps(result))

    _handle = unreal.register_slate_post_tick_callback(tick)
    return 'scheduled'
