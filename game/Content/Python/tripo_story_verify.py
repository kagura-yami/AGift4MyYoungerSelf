"""Full story state-machine integration. Teleports set fixtures; not route traversal proof."""
import json
import time
from pathlib import Path
import unreal
_handle=None

def run(report_path):
    global _handle
    assert _handle is None
    w=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
    p=unreal.GameplayStatics.get_player_character(w,0)
    assert p and 'L_Tutorial' in w.get_path_name()
    progress=unreal.TripoProgressSubsystem.get(w)
    story=unreal.TripoStorySubsystem.get(w)
    catalog=unreal.load_asset('/Game/Data/DA_Story')
    rows=list(catalog.get_editor_property('events'))
    triggers={str(x.event_id):x for x in unreal.GameplayStatics.get_all_actors_of_class(w,unreal.TripoStoryTrigger)}
    zones=unreal.GameplayStatics.get_all_actors_of_class(w,unreal.TripoZone)
    starts={str(x.challenge.get_editor_property('challenge_id')):x for x in zones if x.kind==unreal.TripoZoneKind.START}
    s={'event':0,'phase':'move','checks':[],'at':time.monotonic(),'fixture':'PIE real player; explicit location setup'}
    path=Path(report_path)
    def check(value,label):
        assert value,label
        s['checks'].append(label)
    def move(v):
        p.character_movement.stop_movement_immediately()
        p.set_actor_location(v,False,True)
    def tick(delta):
        global _handle
        try:
            row=rows[s['event']]
            eid=str(row.id)
            phase=s['phase']
            if phase=='move':
                missing=[str(cid) for cid in row.required_challenges if not progress.has_completed(cid)]
                if missing:
                    z=starts[missing[0]]
                    move(z.get_actor_location()+unreal.Vector(0,0,100))
                    s.update(phase='challenge',cid=missing[0],at=time.monotonic())
                else:
                    move(triggers[eid].get_actor_location()+unreal.Vector(0,0,100))
                    s.update(phase='event',at=time.monotonic())
            elif phase=='challenge' and time.monotonic()-s['at']>.4:
                if progress.get_phase()!=unreal.TripoChallengePhase.RUNNING:
                    check(progress.start(p,starts[s['cid']].challenge),'Start '+s['cid'])
                check(progress.finish(p,s['cid']),'Finish '+s['cid'])
                check(progress.commit_reward(p,0),'Reward '+s['cid'])
                check(not progress.commit_reward(p,0),'No duplicate reward '+s['cid'])
                s['phase']='move'
            elif phase=='event' and time.monotonic()-s['at']>.4:
                if not story.has_dialogue():
                    check(story.open_event(p,catalog,row.id),'Open '+eid)
                check(str(story.get_current_id())==eid,'Correct dialogue '+eid)
                check(progress.has_applied(row.id),'Applied '+eid)
                if eid=='Story.Finale.SendReply':
                    unreal.GameplayStatics.get_player_controller(w,0).get_hud().handle_action('reply.1')
                    check(progress.get_reply_choice()==1,'Reply choice committed')
                else:
                    story.close_event(eid=='Story.Home.GrantDash')
                if eid=='Story.Home.GrantDash':
                    check(not progress.has_viewed(row.id),'Skip preserves separate viewed state')
                    check(p.abilities.get_level(unreal.TripoAbility.DASH)>=1,'Skip preserves mandatory ability grant')
                check(not story.has_dialogue(),'Dialogue closes '+eid)
                s['event']+=1
                if s['event']==len(rows):
                    check(progress.save_safe(p),'Finale saves safe progress')
                    check(progress.get_reply_choice()==1,'Reply retained after ending')
                    path.write_text(json.dumps({'ok':True,**s},ensure_ascii=False,indent=2))
                    unreal.unregister_slate_post_tick_callback(_handle); _handle=None
                else: s['phase']='move'
            if _handle is not None: path.write_text(json.dumps({'ok':None,**s},ensure_ascii=False,indent=2))
        except Exception as exc:
            path.write_text(json.dumps({'ok':False,'error':str(exc),**s},ensure_ascii=False,indent=2))
            unreal.unregister_slate_post_tick_callback(_handle); _handle=None
    _handle=unreal.register_slate_post_tick_callback(tick)
