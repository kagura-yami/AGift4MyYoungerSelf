"""Continuous tutorial traversal using Enhanced Input injection, with no teleportation."""
import json
import time
from pathlib import Path
import unreal
_handle=None

def run(report_path):
    global _handle
    w=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
    p=unreal.GameplayStatics.get_player_character(w,0)
    assert p and p.get_actor_location().x<100, 'Start a fresh tutorial at PlayerStart'
    progress=unreal.TripoProgressSubsystem.get(w)
    story=unreal.TripoStorySubsystem.get(w)
    hazards=[z.get_actor_location().x for z in unreal.GameplayStatics.get_all_actors_of_class(w,unreal.TripoZone) if z.kind==unreal.TripoZoneKind.HAZARD]
    s={'started':time.monotonic(),'events':[],'rewards':[],'jumps':0,'max_x':0,'recoveries':0,'input':'Enhanced Input forward + ordinary jump only; no ability shortcut or teleport'}
    path=Path(report_path)
    last_jump=-10
    def tick(delta):
        nonlocal last_jump
        global _handle
        try:
            now=time.monotonic(); x=p.get_actor_location().x
            if now-s['started']>180: raise AssertionError('Traversal timeout at '+str(x))
            if x<s['max_x']-300: s['recoveries']+=1; s['max_x']=x
            s['max_x']=max(s['max_x'],x)
            if story.has_dialogue():
                eid=str(story.get_current_id())
                if eid not in s['events']: s['events'].append(eid)
                if eid=='Story.Finale.SendReply': unreal.GameplayStatics.get_player_controller(w,0).get_hud().handle_action('reply.0')
                else: story.close_event(False)
            elif progress.get_phase()==unreal.TripoChallengePhase.PENDING_REWARD:
                cid=str(progress.get_challenge_id())
                assert progress.commit_reward(p,0), 'Commit route reward '+cid
                s['rewards'].append(cid)
            else:
                jump=now-last_jump>.7 and p.character_movement.is_moving_on_ground() and any(125 < h-x < 185 for h in hazards)
                if jump: last_jump=now; s['jumps']+=1
                p.look_for_test(0)
                p.drive_for_test(.12,1.,0.,jump)
            if 'Story.Finale.FreeRoam' in s['events']:
                assert len(s['events'])==34, 'All 34 dialogues visited'
                assert len(s['rewards'])==8, 'All 8 challenges traversed'
                p.drive_for_test(0,0,0,False); p.character_movement.stop_movement_immediately()
                s['elapsed']=now-s['started']
                path.write_text(json.dumps({'ok':True,**s},indent=2))
                unreal.unregister_slate_post_tick_callback(_handle); _handle=None
            else: path.write_text(json.dumps({'ok':None,**s},indent=2))
        except Exception as exc:
            p.drive_for_test(0,0,0,False); p.character_movement.stop_movement_immediately()
            path.write_text(json.dumps({'ok':False,'error':str(exc),**s},indent=2))
            unreal.unregister_slate_post_tick_callback(_handle); _handle=None
    _handle=unreal.register_slate_post_tick_callback(tick)
