"""Cross-frame gameplay validation. PIE only; never writes save slots or editor assets."""
import unreal, time, json
from pathlib import Path
w=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
assert w and 'Lv4MechanismWhitebox' in w.get_name()
p=unreal.GameplayStatics.get_player_character(w,0)
e=unreal.GameplayStatics.get_all_actors_of_class(w,unreal.TripoElevator)[0]
n=unreal.GameplayStatics.get_all_actors_of_class(w,unreal.TripoChaser)[0]
r=unreal.TripoRuntimeSubsystem.get_runtime(p)
ws=unreal.TripoWorldSubsystem.get(p)
checks=[]; phase=0; at=time.monotonic(); marker=None; echo=None
out=Path(unreal.Paths.project_dir()).resolve().parent/'tools/evaluation/lv4-pie.json'
def check(name,result):
 checks.append(dict(name=name,passed=bool(result)))
 if not result: raise AssertionError(name)
def place(a,xyz,yaw=0):
 a.set_actor_location_and_rotation(unreal.Vector(*xyz),unreal.Rotator(pitch=0,yaw=yaw,roll=0),False,True);a.character_movement.stop_movement_immediately()
def next_phase():
 global phase,at
 phase+=1;at=time.monotonic()
def done(error=None):
 unreal.unregister_slate_post_tick_callback(cb)
 r.set_pause_reason(unreal.TripoPauseReason.MENU,False);r.set_pause_reason(unreal.TripoPauseReason.REWARD,False)
 out.write_text(json.dumps(dict(passed=error is None,error=error,checks=checks),ensure_ascii=False,indent=2),encoding='utf-8')
 unreal.log('LV4_PIE_RESULT '+str(error))
n.stop_chase();e.restore_floor(0);place(p,(-600,0,92))
check('Checkpoint on safe ground',ws.set_checkpoint(p,p.get_actor_transform(),False))
place(p,(-270,0,92))
def tick(dt):
 global phase,at,marker,echo
 elapsed=time.monotonic()-at
 try:
  if phase==0 and elapsed>1.2:
   check('Approach leaves manual doors closed',e.door_alpha==0)
   place(p,(-600,0,92));next_phase()
  elif phase==1 and elapsed>2.6:
   check('Leaving closes after delay',e.door_alpha<.01)
   place(p,(20,0,92));next_phase()
  elif phase==2 and elapsed>.4:
   check('Cabin button within range and sight',e.can_interact(p))
   check('E interaction starts request',e.interact(p))
   check('Second input cannot retrigger',not e.interact(p))
   check('Checkpoint rejected during travel request',not ws.set_checkpoint(p,p.get_actor_transform(),False))
   next_phase()
  elif phase==3 and e.phase==unreal.TripoElevatorPhase.MOVING and e.cabin.get_world_location().z>80:
   check('Character rides movement base',abs(p.get_actor_location().z-e.cabin.get_world_location().z-90)<8)
   r.set_pause_reason(unreal.TripoPauseReason.MENU,True);marker=e.cabin.get_world_location();next_phase()
  elif phase==4 and elapsed>.5:
   check('Menu freezes elevator',e.cabin.get_world_location().distance(marker)<.1)
   r.set_pause_reason(unreal.TripoPauseReason.MENU,False);r.set_pause_reason(unreal.TripoPauseReason.REWARD,True);next_phase()
  elif phase==5 and elapsed>.5:
   check('Reward modal freezes elevator',e.cabin.get_world_location().distance(marker)<.1)
   r.set_pause_reason(unreal.TripoPauseReason.REWARD,False);next_phase()
  elif phase==6 and e.current_floor==1 and e.phase==unreal.TripoElevatorPhase.DOCKED and e.door_alpha>.95:
   check('Arrives and opens upper landing',abs(p.get_actor_location().z-690)<8)
   check('Failure restore succeeds',ws.restore_player(p,False))
   check('Checkpoint restores original floor and closed doors',e.current_floor==0 and e.door_alpha==0 and e.phase==unreal.TripoElevatorPhase.DOCKED)
   n.stop_chase();place(p,(0,0,92));p.abilities.grant_level_floor(unreal.TripoAbility.ECHO,1);next_phase()
  elif phase==7 and elapsed>.4:
   print('ECHO_ACTIVATE',p.abilities.try_activate(unreal.TripoAbility.ECHO,None))
   next_phase()
  elif phase==8 and elapsed>.4:
   echoes=unreal.GameplayStatics.get_all_actors_of_class(w,unreal.TripoEchoActor)
   check('Created persistent echo',len(echoes)==1);echo=echoes[0]
   place(echo,(-170,0,92));place(p,(30,0,92));check('Request with doorway occupied',e.interact(p));next_phase()
  elif phase==9 and elapsed>1.2:
   check('Echo in doorway reopens and prevents departure',e.door_alpha>.95 and e.phase==unreal.TripoElevatorPhase.CLOSING_FOR_TRAVEL)
   place(echo,(30,-75,92));next_phase()
  elif phase==10 and e.current_floor==1:
   check('Unpossessed echo rides cabin',abs(echo.get_actor_location().z-690)<10)
   check('Player rides with echo',abs(p.get_actor_location().z-690)<10)
   check('Return button accepted',e.interact(p));next_phase()
  elif phase==11 and e.current_floor==0 and e.phase==unreal.TripoElevatorPhase.DOCKED:
   check('Downward travel carries player',abs(p.get_actor_location().z-90)<10)
   echo.destroy_actor();place(p,(-600,0,92));n.reset_after_restore();n.set_editor_property('sight_radius',0);marker=n.get_actor_location();next_phase()
  elif phase==12 and elapsed>2:
   check('Patrol moves via navmesh',n.get_actor_location().distance(marker)>100 and not n.navigation_blocked)
   n.set_editor_property('sight_radius',900);n.set_editor_property('close_detection_radius',100);n.character_movement.disable_movement()
   place(n,(-1000,-800,90),0);place(p,(-1500,-800,92));next_phase()
  elif phase==13 and elapsed>.4:
   check('Back-facing distant player does not trigger',n.state==unreal.TripoChaseState.WANDERING)
   place(p,(-500,-800,92));next_phase()
  elif phase==14 and elapsed>.4:
   check('Front-facing visible player triggers pursuit',n.state==unreal.TripoChaseState.CHASING)
   place(p,(-1300,-1170,92));n.set_editor_property('spawn_grace_seconds',0);n.start_chase(p);place(n,(-1300,-880,90),-90);n.character_movement.set_movement_mode(unreal.MovementMode.MOVE_WALKING,0);next_phase()
  elif phase==15 and elapsed>2.2:
   check('Safe zone clears aggro at 45 per second',n.aggro==0)
   check('Safe player never caught',p.get_actor_location().y<-1000)
   zones=unreal.GameplayStatics.get_all_actors_of_class(w,unreal.TripoChaseHideZone)
   check('Monster remains outside safe room',not any(z.contains_point(n.get_actor_location()) for z in zones))
   check('Cleared aggro returns to patrol',n.state==unreal.TripoChaseState.WANDERING)
   place(n,(-1000,-800,90),0);place(p,(-500,-800,92));next_phase()
  elif phase==16 and elapsed>.4:
   check('Leaving safety allows redetection',n.state==unreal.TripoChaseState.CHASING)
   check('Checkpoint recovery resets monster',ws.restore_player(p,False))
   check('Monster aggro cleared on recovery',n.aggro==0)
   check('Monster returns to initial route point',n.get_actor_location().distance(unreal.Vector(-1900,-800,90))<10)
   done()
  elif elapsed>15: raise TimeoutError('phase '+str(phase)+' '+str(e.phase))
 except Exception as exc: done(str(exc))
cb=unreal.register_slate_post_tick_callback(tick)
