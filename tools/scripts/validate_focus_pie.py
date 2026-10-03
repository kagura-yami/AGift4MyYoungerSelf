# Focus regression: map/PIE only, no disk save slots.
import unreal,json,time
from pathlib import Path
w=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
assert w
p=unreal.GameplayStatics.get_player_character(w,0);pc=unreal.GameplayStatics.get_player_controller(w,0)
e=unreal.GameplayStatics.get_all_actors_of_class(w,unreal.TripoElevator)[0]
r=unreal.TripoRuntimeSubsystem.get_runtime(p)
checks=[];phase=0;at=time.monotonic()
out=Path(unreal.Paths.project_dir()).resolve().parent/'tools/evaluation/focus-pie.json'
def check(name,ok):
 checks.append(dict(name=name,passed=bool(ok)))
 if not ok:raise AssertionError(name)
def place(v):
 p.set_actor_location(unreal.Vector(*v),False,True);p.character_movement.stop_movement_immediately();p.character_movement.disable_movement()
def step():
 global phase,at
 phase+=1;at=time.monotonic()
def finish(error=None):
 unreal.unregister_slate_post_tick_callback(cb)
 out.write_text(json.dumps(dict(checks=checks,error=error),indent=2),encoding='utf8')
 print('FOCUS TEST',len(checks),error)
def tick(dt):
 global phase,at,marker
 try:
  elapsed=time.monotonic()-at
  if elapsed>20:raise AssertionError('Timeout '+str(phase))
  if phase==0:
   e.restore_floor(0);place((-1470,565,170));step()
  elif phase==1 and elapsed>1:
   check('Approach does not open door',e.door_alpha==0)
   place((-1430,772,180));check('Ground up control reachable',e.landing_controls[0].can_interact(p))
   check('Other floor control out of reach',not e.landing_controls[1].can_interact(p))
   check('Explicit call accepted',e.landing_controls[0].try_interact(p));step()
  elif phase==2 and elapsed>1:
   check('Explicit call opens doors',e.door_alpha>.95)
   place((-1590,565,170));step()
  elif phase==3 and elapsed>2:
   check('Doorway occupancy holds open',e.door_alpha>.95)
   place((-1300,800,170));step()
  elif phase==4 and elapsed>3:
   check('Leaving closes doors',e.door_alpha==0)
   place((-1430,772,180));r.set_pause_reason(unreal.TripoPauseReason.MENU,True)
   check('Paused input rejected',not e.landing_controls[0].try_interact(p));r.set_pause_reason(unreal.TripoPauseReason.MENU,False)
   wall=unreal.SystemLibrary.line_trace_single(w,unreal.Vector(-50,-50,250),unreal.Vector(-50,-250,250),unreal.TraceTypeQuery.TRACE_TYPE_QUERY1,False,[p],unreal.DrawDebugTrace.NONE,True)
   check('Previously intangible wall now blocks trace',wall is not None)
   place((-50,-70,250));p.set_actor_location(unreal.Vector(-50,-250,250),True,False)
   check('Capsule cannot cross wall',p.get_actor_location().y>-160)
   portal=next(a for a in unreal.GameplayStatics.get_all_actors_of_class(w,unreal.TripoTeleportPoint) if a.entry_enabled and str(a.link_id)=='LV4_UpperElevatorDoor')
   place((-1485,565,610));p.set_actor_rotation(unreal.Rotator(pitch=0,yaw=180,roll=0),True)
   check('Empty shaft portal refused',not portal.try_teleport(p))
   e.restore_floor(1);p.set_actor_rotation(unreal.Rotator(pitch=0,yaw=0,roll=0),True)
   check('Backwards portal refused',not portal.try_teleport(p))
   p.set_actor_rotation(unreal.Rotator(pitch=0,yaw=180,roll=0),True)
   check('Upper doorway coordinate transfer',portal.try_teleport(p))
   check('Exact landing coordinate',p.get_actor_location().distance(unreal.Vector(-1720,565,610))<.1)
   check('Portal is one-way',not next(a for a in unreal.GameplayStatics.get_all_actors_of_class(w,unreal.TripoTeleportPoint) if not a.entry_enabled).try_teleport(p))
   e.restore_floor(0);place((-1430,772,180));finish()
 except Exception as error:finish(str(error))
cb=unreal.register_slate_post_tick_callback(tick)
