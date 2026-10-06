import unreal,time,json
from pathlib import Path
w=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world();p=unreal.GameplayStatics.get_player_character(w,0)
entry=next(a for a in unreal.GameplayStatics.get_all_actors_of_class(w,unreal.TripoTeleportPoint) if a.get_actor_label()=='LV4_CompanyWindowEntry')
p.abilities.grant_level_floor(unreal.TripoAbility.DASH,2) # Test-only unlock in PIE; chapter progression grants it normally.
checks=[];phase=0;at=time.monotonic();path=[]
out=Path(unreal.Paths.project_dir()).resolve().parent/'tools/evaluation/window-exit/test.json'
def check(name,ok):
 checks.append(dict(name=name,passed=bool(ok)))
 if not ok:raise AssertionError(name)
def step():
 global phase,at
 phase+=1;at=time.monotonic()
def finish(error=None):
 unreal.unregister_slate_post_tick_callback(cb)
 out.write_text(json.dumps(dict(checks=checks,error=error,path=path),indent=2))
def tick(dt):
 try:
  t=time.monotonic()-at;l=p.get_actor_location();path.append([phase,round(l.x,1),round(l.y,1),round(l.z,1)])
  if t>8:raise AssertionError('timeout '+str(phase))
  if phase==0:
   p.set_actor_location(unreal.Vector(1006,275,610),False,True);p.look_for_test(90);p.set_actor_rotation(unreal.Rotator(yaw=90),False)
   check('Facing inside does not teleport',not entry.try_teleport(p));p.look_for_test(-90);p.set_actor_rotation(unreal.Rotator(yaw=-90),False);step()
  elif phase==1 and t>.3:
   result=p.abilities.try_activate(unreal.TripoAbility.DASH,None)
   check('Forward ability reaches platform',abs(p.get_actor_location().y-102)<2);step()
  elif phase==2 and t>1:
   if p.abilities.get_cooldown_remaining(unreal.TripoAbility.DASH)>0:return
   check('Stable outside landing',p.character_movement.is_moving_on_ground() and abs(l.z-610)<4)
   p.look_for_test(-68);p.drive_for_test(2,1,0,True);step()
  elif phase==3 and t>.22:
   result=p.abilities.try_activate(unreal.TripoAbility.DASH,None);check('Air dash activates',result[0]==unreal.TripoAbilityFailure.NONE);step()
  elif phase==4 and t>1.7:
   check('Lands in opposite company',l.y< -510 and l.z>640 and p.character_movement.is_moving_on_ground());finish()
 except Exception as e:finish(str(e))
cb=unreal.register_slate_post_tick_callback(tick)



