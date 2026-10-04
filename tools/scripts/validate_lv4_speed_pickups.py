import unreal,time,json
from pathlib import Path
w=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
p=unreal.GameplayStatics.get_player_character(w,0)
r=unreal.TripoRuntimeSubsystem.get_runtime(p)
balls=unreal.GameplayStatics.get_all_actors_of_class(w,unreal.TripoSpeedPickup)
fast=next(b for b in balls if b.multiplier>1); slow=next(b for b in balls if b.multiplier<1)
checks=[]; phase=0;at=time.monotonic();old=p.get_actor_location()
output=Path(unreal.Paths.project_dir()).resolve().parent/'tools/evaluation/speed-pickups/test.json'
def check(name,ok):
 checks.append(dict(name=name,passed=bool(ok)))
 if not ok:raise AssertionError(name)
def step():
 global phase,at
 phase+=1;at=time.monotonic()
def finish(error=None):
 unreal.unregister_slate_post_tick_callback(callback)
 r.set_pause_reason(unreal.TripoPauseReason.MENU,False)
 p.set_actor_location(old,False,True)
 p.character_movement.set_movement_mode(unreal.MovementMode.MOVE_WALKING,0)
 output.write_text(json.dumps(dict(checks=checks,error=error),indent=2),encoding='utf-8')
def tick(dt):
 try:
  t=time.monotonic()-at
  if t>15:raise AssertionError('Timeout '+str(phase))
  if phase==0:
   p.character_movement.set_movement_mode(unreal.MovementMode.MOVE_NONE,0)
   p.set_actor_location(fast.get_actor_location(),False,True);step()
  elif phase==1 and t>.5:
   check('Touch collects green',fast.get_editor_property('collected'))
   check('Green hidden',fast.visual_actor.get_editor_property('hidden'))
   check('Speed increases',abs(p.get_speed_buff_multiplier()-1.5)<.01)
   p.set_actor_location(slow.get_actor_location(),False,True);step()
  elif phase==2 and t>.5:
   check('Touch collects blue',slow.get_editor_property('collected'))
   check('Blue hidden',slow.visual_actor.get_editor_property('hidden'))
   check('Blue replaces green',abs(p.get_speed_buff_multiplier()-.65)<.01)
   r.set_pause_reason(unreal.TripoPauseReason.MENU,True);step()
  elif phase==3 and t>6.5:
   check('Pause preserves duration',abs(p.get_speed_buff_multiplier()-.65)<.01)
   r.set_pause_reason(unreal.TripoPauseReason.MENU,False);step()
  elif phase==4 and t>6.2:
   check('Effect expires',p.get_speed_buff_multiplier()==1.)
   check('Speed restored',abs(p.character_movement.max_walk_speed-p.get_editor_property('walk_speed'))<.1)
   check('No repeat pickup while standing',slow.get_editor_property('collected'))
   finish()
 except Exception as e:finish(str(e))
callback=unreal.register_slate_post_tick_callback(tick)
