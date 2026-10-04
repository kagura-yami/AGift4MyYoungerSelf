import unreal,time,json
from pathlib import Path
w=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world();p=unreal.GameplayStatics.get_player_character(w,0);pc=unreal.GameplayStatics.get_player_controller(w,0)
e=unreal.GameplayStatics.get_all_actors_of_class(w,unreal.TripoElevator)[0];r=unreal.TripoRuntimeSubsystem.get_runtime(p)
frames=[a for a in unreal.GameplayStatics.get_all_actors_of_class(w,unreal.StaticMeshActor) if a.get_actor_label().startswith('LV4_FixedFrame_')];before=[a.get_actor_location() for a in frames]
checks=[];phase=0;at=time.monotonic();positions=[]
out=Path(unreal.Paths.project_dir()).resolve().parent/'tools/evaluation/elevator-rework/journey.json'
def check(n,b):
 checks.append(dict(name=n,passed=bool(b)))
 if not b:raise AssertionError(n)
def step():
 global phase,at
 phase+=1;at=time.monotonic()
def place(x,y,z,walking=False):
 p.set_actor_location(unreal.Vector(x,y,z),False,True);p.character_movement.stop_movement_immediately();p.character_movement.set_movement_mode(unreal.MovementMode.MOVE_WALKING if walking else unreal.MovementMode.MOVE_NONE,0)
def finish(error=None):
 unreal.unregister_slate_post_tick_callback(cb);r.set_pause_reason(unreal.TripoPauseReason.MENU,False)
 out.write_text(json.dumps(dict(checks=checks,error=error,positions=positions),indent=2),encoding='utf8')
def tick(dt):
 global phase,at,pause_z
 try:
  elapsed=time.monotonic()-at
  if elapsed>22:raise AssertionError('Timeout '+str(phase))
  if phase==0:
   pc.set_ignore_look_input(True);e.restore_floor(0);place(-1430,817,150);pc.set_control_rotation(unreal.Rotator(pitch=3,yaw=180,roll=0));step()
  elif phase==1 and elapsed>.6:
   check('Ground focused button',p.get_focused_target()==e.landing_controls[0]);check('Ground call',e.landing_controls[0].try_interact(p));step()
  elif phase==2 and elapsed>1:
   check('Fully open',e.door_alpha>.99);place(-1720,565,150,True);pc.set_control_rotation(unreal.Rotator(pitch=0,yaw=0,roll=0));step()
  elif phase==3 and elapsed>.6:
   check('Cabin floor aligned',abs(p.get_actor_location().z-148)<5);check('Inside button works',e.cabin_control.try_interact(p));step()
  elif phase==4 and e.phase==unreal.TripoElevatorPhase.MOVING:
   positions.append(dict(cabin=e.cabin.get_world_location().z,player=p.get_actor_location().z))
   check('Cabin doors closed during travel',e.door_alpha==0)
   if e.cabin.get_world_location().z>220:
    check('Player carried smoothly',abs(p.get_actor_location().z-e.cabin.get_world_location().z-90)<8)
    check('Landing frames stay fixed',all(a.get_actor_location().distance(v)<.01 for a,v in zip(frames,before)))
    pause_z=e.cabin.get_world_location().z;r.set_pause_reason(unreal.TripoPauseReason.MENU,True);step()
  elif phase==5 and elapsed>.4:
   check('Pause freezes cabin',abs(e.cabin.get_world_location().z-pause_z)<.01);r.set_pause_reason(unreal.TripoPauseReason.MENU,False);step()
  elif phase==6 and e.current_floor==1 and e.door_alpha>.99:
   check('Upper floor aligned',abs(p.get_actor_location().z-613.619)<8)
   place(-1430,826,614);pc.set_control_rotation(unreal.Rotator(pitch=-6.6,yaw=180,roll=0));step()
  elif phase==7 and elapsed>e.auto_close_delay+1.5:
   check('Upper landing closes after exit',e.door_alpha==0)
   check('Upper down button focused',p.get_focused_target()==e.landing_controls[1]);check('Upper call accepted',e.landing_controls[1].try_interact(p));step()
  elif phase==8 and elapsed>1:
   check('Upper door opens on call',e.door_alpha>.99);place(-1720,565,614,True);step()
  elif phase==9 and elapsed>.5:
   check('Return ride accepted',e.cabin_control.try_interact(p));step()
  elif phase==10 and e.current_floor==0 and e.phase==unreal.TripoElevatorPhase.DOCKED:
   check('Return ride carries player',abs(p.get_actor_location().z-150)<8)
   check('Frames unchanged after round trip',all(a.get_actor_location().distance(v)<.01 for a,v in zip(frames,before)))
   finish()
 except Exception as ex:finish(str(ex))
cb=unreal.register_slate_post_tick_callback(tick)
