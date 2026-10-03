import unreal,time,json
from pathlib import Path
w=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world();p=unreal.GameplayStatics.get_player_character(w,0);n=unreal.GameplayStatics.get_all_actors_of_class(w,unreal.TripoChaser)[0];e=unreal.GameplayStatics.get_all_actors_of_class(w,unreal.TripoElevator)[0]
r=unreal.TripoRuntimeSubsystem.get_runtime(p);ws=unreal.TripoWorldSubsystem.get(p)
checks=[];phase=0;at=time.monotonic();marker=None
out=Path(unreal.Paths.project_dir()).resolve().parent/'tools/evaluation/lv4-extra-pie.json'
def check(name,value):
 checks.append(dict(name=name,passed=bool(value)))
 if not value:raise AssertionError(name)
def place(a,xyz,yaw=0):
 a.set_actor_location_and_rotation(unreal.Vector(*xyz),unreal.Rotator(pitch=0,yaw=yaw,roll=0),False,True);a.character_movement.stop_movement_immediately()
def advance():
 global phase,at
 phase+=1;at=time.monotonic()
def done(error=None):
 unreal.unregister_slate_post_tick_callback(cb);out.write_text(json.dumps(dict(passed=error is None,error=error,checks=checks),indent=2),encoding='utf-8')
n.set_editor_property('sight_radius',0);n.reset_after_restore();place(p,(-600,0,92));marker=n.get_actor_location()
def tick(dt):
 global marker,phase,at
 elapsed=time.monotonic()-at
 try:
  if phase==0 and elapsed>.55:
   check('Patrol waits at initial waypoint',n.get_actor_location().distance(marker)<10);advance()
  elif phase==1 and elapsed>1:
   check('Patrol resumes after one-second dwell',n.get_actor_location().distance(marker)>30)
   place(n,(-800,-800,90));advance()
  elif phase==2 and n.patrol_index==2:
   place(n,(600,-800,90));phase=21;at=time.monotonic()
  elif phase==21 and elapsed>1.5:
   check('Ping-pong reverses at last waypoint',n.patrol_index==1 and n.get_actor_location().x<550)
   n.reset_after_restore();n.set_editor_property('sight_radius',900);n.character_movement.disable_movement();place(n,(-700,-700,90),90);place(p,(-700,-200,92));phase=3;at=time.monotonic()
  elif phase==3 and elapsed>.4:
   check('Wall occludes front-facing target',n.state==unreal.TripoChaseState.WANDERING and n.aggro==0)
   place(p,(-600,0,92));check('Recovery checkpoint prepared',ws.set_checkpoint(p,p.get_actor_transform(),False));marker=r.get_epoch()
   n.set_editor_property('spawn_grace_seconds',0);place(p,(-500,-800,92));place(n,(-570,-800,90),0);n.start_chase(p);advance()
  elif phase==4 and elapsed>.5:
   check('Actual catch invokes checkpoint recovery',r.get_epoch()>marker and p.get_actor_location().distance(unreal.Vector(-600,0,90))<10)
   check('Catch recovery resets NPC',n.aggro==0 and n.get_actor_location().distance(unreal.Vector(-1900,-800,90))<10)
   n.stop_chase();e.restore_floor(1);place(p,(-270,0,92));advance()
  elif phase==5 and elapsed>.3:
   h=p.set_actor_location(unreal.Vector(0,0,92),True,False)
   check('Absent cabin landing blocks entry into shaft',p.get_actor_location().x<-180)
   check('Remote button cannot interact through wall/range',not e.can_interact(p))
   done()
  elif elapsed>10:raise TimeoutError(str(phase))
 except Exception as ex:done(str(ex))
cb=unreal.register_slate_post_tick_callback(tick)
