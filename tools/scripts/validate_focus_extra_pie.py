import unreal,time,json
from pathlib import Path
w=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
assert w and w.get_name().endswith('lv4')
p=unreal.GameplayStatics.get_player_character(w,0);pc=unreal.GameplayStatics.get_player_controller(w,0)
e=unreal.GameplayStatics.get_all_actors_of_class(w,unreal.TripoElevator)[0]
checks=[];phase=0;at=time.monotonic()
def check(name,ok):
 checks.append(dict(name=name,passed=bool(ok)))
 if not ok:raise AssertionError(name)
def step():
 global phase,at
 phase+=1;at=time.monotonic()
def finish(error=None):
 unreal.unregister_slate_post_tick_callback(cb)
 (Path(unreal.Paths.project_dir()).resolve().parent/'tools/evaluation/focus-extra.json').write_text(json.dumps(dict(checks=checks,error=error),indent=2),encoding='utf8')
def place(x,y,z):
 p.set_actor_location(unreal.Vector(x,y,z),False,True);p.character_movement.stop_movement_immediately();p.character_movement.disable_movement()
def tick(dt):
 global phase,at
 try:
  elapsed=time.monotonic()-at
  if elapsed>15:raise AssertionError('Timeout')
  if phase==0:
   pc.set_ignore_look_input(True);place(-50,-70,250);pc.set_control_rotation(unreal.Rotator(pitch=0,yaw=90,roll=0));step()
  elif phase==1 and elapsed>.3:
   camera=pc.get_player_view_point()[0];check('Camera stays on player side of wall',camera.y>-151)
   pc.set_control_rotation(unreal.Rotator(pitch=0,yaw=-90,roll=0));p.set_actor_rotation(unreal.Rotator(pitch=0,yaw=-90,roll=0),True)
   p.character_movement.set_movement_mode(unreal.MovementMode.MOVE_FALLING,0);p.abilities.grant_level_floor(unreal.TripoAbility.DASH,3)
   result=p.abilities.try_activate(unreal.TripoAbility.DASH,None);check('Normal dash activates',result[0]==unreal.TripoAbilityFailure.NONE);step()
  elif phase==2 and elapsed>.3:
   check('Normal dash cannot pass wall',p.get_actor_location().y>-151)
   place(-1430,817,180);e.restore_floor(0);pc.set_control_rotation(unreal.Rotator(pitch=-9.22,yaw=180,roll=0));step()
  elif phase==3 and elapsed>.3:
   check('Reticle selects the up control '+str(p.get_actor_location())+' '+str(pc.get_player_view_point()),p.get_focused_target()==e.landing_controls[0]);check('Selected control has glow overlay',e.landing_controls[0].highlight_mesh.get_overlay_material() is not None)
   pc.set_control_rotation(unreal.Rotator(pitch=-13.5,yaw=180,roll=0));step()
  elif phase==4 and elapsed>.3:
   check('Inactive down button has no interaction',p.get_focused_target() is None);check('Looking away removes glow',e.landing_controls[0].highlight_mesh.get_overlay_material() is None)
   portals=unreal.GameplayStatics.get_all_actors_of_class(w,unreal.TripoTeleportPoint)
   entry=next(a for a in portals if str(a.link_id)=='LV4_UpperElevatorDoor' and a.entry_enabled)
   exit=next(a for a in portals if str(a.link_id)=='LV4_UpperElevatorDoor' and not a.entry_enabled)
   e.restore_floor(1);place(-1485,565,610);p.set_actor_rotation(unreal.Rotator(pitch=0,yaw=180,roll=0),True)
   old=exit.get_actor_location();exit.set_actor_location(unreal.Vector(-50,-150,250),False,True)
   check('Obstructed destination refused',not entry.try_teleport(p));exit.set_actor_location(old,False,True)
   place(-1300,565,610);check('Outside special point cannot transfer',not entry.try_teleport(p))
   e.restore_floor(0);place(-1430,817,180);pc.set_control_rotation(unreal.Rotator(pitch=-9.22,yaw=180,roll=0));finish()
 except Exception as error:finish(str(error))
cb=unreal.register_slate_post_tick_callback(tick)
