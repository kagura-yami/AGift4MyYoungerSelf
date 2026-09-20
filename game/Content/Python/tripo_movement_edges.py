"""Collision edges on the possessed PIE character."""
import json
import time
from pathlib import Path
import unreal
_handle=None
def run(report_path):
    global _handle
    w=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
    p=unreal.GameplayStatics.get_player_character(w,0)
    assert p
    a=p.abilities
    for ability in (unreal.TripoAbility.UP_DASH,unreal.TripoAbility.WALL_JUMP): a.grant_level_floor(ability,1)
    s={'phase':0,'at':time.monotonic(),'checks':[]}
    def check(v,label):
        assert v,label
        s['checks'].append(label)
    def advance(): s.update(phase=s['phase']+1,at=time.monotonic())
    def move(x,y,z):
        p.character_movement.stop_movement_immediately(); p.set_actor_location(unreal.Vector(x,y,z),False,True)
    def tick(delta):
        global _handle
        try:
            age=time.monotonic()-s['at']; phase=s['phase']
            if phase==0:
                move(-500,800,100); advance()
            elif phase==1 and age>2.2:
                check(a.try_activate(unreal.TripoAbility.UP_DASH,None)[0]==unreal.TripoAbilityFailure.NONE,'Up dash activates below ceiling'); advance()
            elif phase==2 and age>.3:
                check(p.get_actor_location().z<224,'Ceiling sweep prevents upward penetration')
                move(5630,700,250); p.look_for_test(0); p.set_actor_rotation(unreal.Rotator(yaw=0),False)
                p.character_movement.set_editor_property('gravity_scale',0.)
                advance()
            elif phase==3 and age>.1:
                check(a.try_activate(unreal.TripoAbility.WALL_JUMP,None)[0]==unreal.TripoAbilityFailure.NONE,'Marked vertical wall supports wall jump')
                check(p.get_velocity().z>600 and p.get_velocity().x<0,'Wall jump propels away and upward'); advance()
            elif phase==4 and age>.25:
                move(5630,700,250)
                check(a.try_activate(unreal.TripoAbility.WALL_JUMP,None)[0]==unreal.TripoAbilityFailure.AIR_USE_SPENT,'Same wall cannot reset repeated contact')
                p.character_movement.set_editor_property('gravity_scale',1.5)
                Path(report_path).write_text(json.dumps({'ok':True,**s},indent=2))
                unreal.unregister_slate_post_tick_callback(_handle); _handle=None
        except Exception as exc:
            p.character_movement.set_editor_property('gravity_scale',1.5)
            Path(report_path).write_text(json.dumps({'ok':False,'error':str(exc),**s},indent=2))
            unreal.unregister_slate_post_tick_callback(_handle); _handle=None
    _handle=unreal.register_slate_post_tick_callback(tick)
