"""Runtime checks on an explicitly spawned SIE character; not a player-input test."""
import json
import time
from pathlib import Path
import unreal

_handle = None

def run(report_path):
    global _handle
    assert _handle is None
    w = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
    p = unreal.GameplayStatics.get_all_actors_of_class(w, unreal.TripoCharacter)[0]
    a = p.abilities
    possessed = unreal.GameplayStatics.get_player_character(w,0) == p
    if not possessed: p.character_movement.set_editor_property('run_physics_with_no_controller', True)
    ability_ids = [unreal.TripoAbility.DASH, unreal.TripoAbility.UP_DASH, unreal.TripoAbility.WALL_JUMP, unreal.TripoAbility.STEP_STONE, unreal.TripoAbility.SLOW, unreal.TripoAbility.REWIND, unreal.TripoAbility.ECHO, unreal.TripoAbility.BONUS_TIME]
    r = unreal.TripoRuntimeSubsystem.get_runtime(w)
    recovery = unreal.TripoWorldSubsystem.get(w)
    platform = next(x for x in unreal.GameplayStatics.get_all_actors_of_class(w, unreal.TripoMechanism) if x.kind == unreal.TripoMechanismKind.PLATFORM)
    for ability in [unreal.TripoAbility.REWIND, unreal.TripoAbility.ECHO, unreal.TripoAbility.STEP_STONE]:
        a.grant_level_floor(ability, 1)
    s = {'phase': 0, 'at': time.monotonic(), 'checks': [], 'fixture': 'PIE possessed TripoCharacter' if possessed else 'SIE spawned TripoCharacter'}
    path = Path(report_path)
    def check(value, label):
        assert value, label
        s['checks'].append(label)
    def advance():
        s.update(phase=s['phase']+1, at=time.monotonic())
    def move(x,y,z):
        p.character_movement.stop_movement_immediately()
        p.set_actor_location(unreal.Vector(x,y,z),False,True)
    def actors(cls):
        return unreal.GameplayStatics.get_all_actors_of_class(w,cls)
    def tick(delta):
        global _handle
        try:
            age=time.monotonic()-s['at']
            phase=s['phase']
            if phase == 0:
                move(-500,-600,100)
                p.history.clear()
                advance()
            elif phase == 1 and age > .5:
                check(p.history.get_sample_count()>3,'History records real character frames')
                s['rewind_start']=p.get_actor_location().x
                move(-200,-600,100)
                advance()
            elif phase == 2 and age > .5:
                s['levels']=[a.get_level(i) for i in ability_ids]
                check(a.try_activate(unreal.TripoAbility.REWIND,None)[0]==unreal.TripoAbilityFailure.NONE,'Self rewind activates from recorded history')
                check(p.interactor.get_editor_property('suppressed'),'Self rewind suppresses interaction')
                advance()
            elif phase == 3 and age > 1.4:
                check(abs(p.get_actor_location().x-s['rewind_start'])<10,'Self rewind restores earlier position through swept playback')
                check(not p.interactor.get_editor_property('suppressed'),'Self rewind releases interaction')
                check([a.get_level(i) for i in ability_ids]==s['levels'],'Self rewind leaves ability levels unchanged')
                check(a.get_cooldown_remaining(unreal.TripoAbility.REWIND)>0,'Self rewind does not refund cooldown')
                p.history.clear()
                advance()
            elif phase == 4 and age > .5:
                check(a.try_activate(unreal.TripoAbility.ECHO,None)[0]==unreal.TripoAbilityFailure.NONE,'Echo accepts recorded clip')
                check(len(actors(unreal.TripoEchoActor))==1,'One owned echo spawned')
                advance()
            elif phase == 5 and age > .15:
                check(len(actors(unreal.TripoEchoActor))==1,'Echo survives initial playback frames')
                advance()
            elif phase == 6 and age > .7:
                check(len(actors(unreal.TripoEchoActor))==0,'Echo disappears when frozen clip ends')
                s['slow1']=platform.add_slow(p,.5,3).to_string()
                s['slow2']=platform.add_slow(p,.25,.5).to_string()
                check(abs(platform.get_local_rate()-.25)<.01,'Overlapping slow sources choose minimum rate')
                advance()
            elif phase == 7 and age > .7:
                check(abs(platform.get_local_rate()-.5)<.01,'Expired strongest slow reveals remaining source')
                advance()
            elif phase == 8 and age > 2.5:
                check(abs(platform.get_local_rate()-1)<.01,'All expired slow sources restore normal rate')
                move(4700,0,240)
                p.set_actor_rotation(unreal.Rotator(yaw=0),False)
                check(a.try_activate(unreal.TripoAbility.STEP_STONE,None)[0]==unreal.TripoAbilityFailure.NONE,'Stone placement inside allowed volume succeeds')
                stones=actors(unreal.TripoStone)
                check(len(stones)==1,'Stone creates one collision actor')
                s['stone_scale']=str(stones[0].get_actor_scale3d())
                move(4600,-500,100)
                advance()
            elif phase == 9 and age > .5:
                check(unreal.TripoProgressSubsystem.get(w).save_safe(p),'Safe NPC area writes and verifies rotating save slot')
                s['epoch']=r.get_epoch()
                check(recovery.restore_player(p,False),'Recovery succeeds with active temporary stone')
                check(len(actors(unreal.TripoStone))==0,'Recovery removes temporary stones')
                check(r.get_epoch()==s['epoch']+1,'Recovery advances history epoch')
                check(p.history.get_sample_count()==0,'Recovery clears history before resuming')
                path.write_text(json.dumps({'ok':True,**s},indent=2))
                unreal.unregister_slate_post_tick_callback(_handle)
                _handle=None
            if _handle is not None:
                path.write_text(json.dumps({'ok':None,**s},indent=2))
        except Exception as exc:
            path.write_text(json.dumps({'ok':False,'error':str(exc),**s},indent=2))
            unreal.unregister_slate_post_tick_callback(_handle)
            _handle=None
    _handle=unreal.register_slate_post_tick_callback(tick)
