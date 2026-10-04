import unreal,time,json
from pathlib import Path
w=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
p=unreal.GameplayStatics.get_player_character(w,0)
pc=unreal.GameplayStatics.get_player_controller(w,0)
e=unreal.GameplayStatics.get_all_actors_of_class(w,unreal.TripoElevator)[0]
r=unreal.TripoRuntimeSubsystem.get_runtime(p)
out=Path(unreal.Paths.project_dir()).resolve().parent/'tools/evaluation/elevator-rework/split-controls-test.json'
checks=[];phase=0;at=time.monotonic()
def check(name,ok):
 checks.append(dict(name=name,passed=bool(ok)))
 if not ok:raise AssertionError(name)
def next_step():
 global phase,at
 phase+=1;at=time.monotonic()
def finish(error=None):
 unreal.unregister_slate_post_tick_callback(callback)
 pc.set_ignore_look_input(False)
 out.write_text(json.dumps(dict(checks=checks,error=error),indent=2),encoding='utf-8')
def tick(dt):
 try:
  elapsed=time.monotonic()-at
  if elapsed>20:raise AssertionError('Timeout '+str(phase))
  if phase==0:
   e.restore_floor(0);p.set_actor_location(unreal.Vector(-1720,565,150),False,True)
   p.character_movement.set_movement_mode(unreal.MovementMode.MOVE_WALKING,0)
   pc.set_ignore_look_input(True);next_step()
  elif phase==1 and elapsed>.5:
   check('Separate open target',e.door_control!=e.cabin_control)
   check('Open from closed cabin',e.door_control.try_interact(p));next_step()
  elif phase==2 and elapsed>1.2:
   check('Open does not move cabin',e.current_floor==0 and e.phase==unreal.TripoElevatorPhase.DOCKED and e.door_alpha>.99)
   check('Travel command accepted',e.cabin_control.try_interact(p))
   check('Open cancels pending departure',e.door_control.try_interact(p) and e.phase==unreal.TripoElevatorPhase.DOCKED)
   next_step()
  elif phase==3 and elapsed>e.auto_close_delay+2:
   check('Doors can close while docked',e.door_alpha==0)
   check('Reopen after timeout without moving',e.door_control.try_interact(p));next_step()
  elif phase==4 and elapsed>1.2:
   check('Reopened fully',e.door_alpha>.99)
   check('Explicit travel starts departure',e.cabin_control.try_interact(p));next_step()
  elif phase==5 and e.phase==unreal.TripoElevatorPhase.MOVING:
   check('Opening forbidden during motion',not e.door_control.try_interact(p));next_step()
  elif phase==6 and e.current_floor==1 and e.door_alpha>.99:
   next_step()
  elif phase==7 and elapsed>4:
   check('Arrival remains open after four seconds',e.door_alpha>.99)
   check('Arrival open button extends hold',e.door_control.try_interact(p))
   finish()
 except Exception as ex:finish(str(ex))
callback=unreal.register_slate_post_tick_callback(tick)
