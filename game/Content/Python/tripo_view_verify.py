"""View-led locomotion checks. Run fresh PIE at normal foreground frame rate."""
import json
import time
from pathlib import Path
import unreal
_handle = None

def run(report_path):
    global _handle
    assert _handle is None
    world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
    player = unreal.GameplayStatics.get_player_character(world,0)
    camera = unreal.GameplayStatics.get_player_camera_manager(world,0)
    assert player
    report = Path(report_path)
    report.parent.mkdir(parents=True,exist_ok=True)
    report.write_text(json.dumps({'ok':None,'status':'running'}))
    state = {'phase':0,'at':time.monotonic(),'checks':[],'samples':[]}
    def close(a,b): return abs((a-b+180)%360-180)<1
    def check(ok,name):
        assert ok,name
        state['checks'].append(name)
    def advance(): state.update(phase=state['phase']+1,at=time.monotonic())
    def reset():
        player.reset_practice_position()
        player.set_actor_location(unreal.Vector(-500,-500,90),False,False)
    def tick(dt):
        age = time.monotonic()-state['at']
        phase = state['phase']
        try:
            if phase == 0 and age>.4:
                player.look_for_test(135.)
                advance()
            elif phase == 1 and age>.5:
                check(close(player.get_camera_yaw(),135),'View is not clamped to old 35-degree range')
                check(close(camera.get_camera_rotation().yaw,135),'Rendered camera follows view controller')
                check(close(player.get_actor_rotation().yaw,135),'Idle body follows the view')
                player.set_actor_rotation(unreal.Rotator(yaw=-90),False)
                check(close(player.get_camera_yaw(),135),'External body rotation cannot rotate view')
                player.look_for_test(-170.)
                advance()
            elif phase == 2 and age>.4:
                check(close(camera.get_camera_rotation().yaw,-170),'View wraps across 180 degrees')
                reset()
                advance()
            elif phase == 3 and age>.4:
                state['start'] = player.get_actor_location()
                player.drive_for_test(1.5,1.,0.,False)
                advance()
            elif phase == 4 and age>.35:
                pos = player.get_actor_location()
                check(pos.x>state['start'].x+40,'Held W moves along initial view')
                state['turn'] = pos
                player.look_for_test(90.)
                advance()
            elif phase == 5 and age>.4:
                pos = player.get_actor_location()
                check(pos.y>state['turn'].y+40,'Turning view redirects held W without release')
                check(close(player.get_camera_yaw(),90),'Movement does not feed rotation back into view')
                state['samples'].append({'body':player.get_actor_rotation().yaw,'view':player.get_camera_yaw(),'x':pos.x,'y':pos.y})
                reset()
                advance()
            elif phase == 6 and age>.4:
                state['start'] = player.get_actor_location()
                player.drive_for_test(.4,0.,1.,False)
                advance()
            elif phase == 7 and age>.5:
                pos=player.get_actor_location()
                check(pos.y>state['start'].y+40 and abs(pos.x-state['start'].x)<5,'D strafes right relative to view')
                check(close(player.get_camera_yaw(),0) and close(player.get_actor_rotation().yaw,0),'Strafing keeps body and camera facing forward')
                reset()
                advance()
            elif phase == 8 and age>.4:
                state['start'] = player.get_actor_location()
                player.drive_for_test(.4,-1.,0.,False)
                advance()
            elif phase == 9 and age>.5:
                check(player.get_actor_location().x<state['start'].x-40,'S moves backward relative to view')
                check(close(player.get_camera_yaw(),0) and close(player.get_actor_rotation().yaw,0),'Backward movement does not flip camera or body')
                reset()
                advance()
            elif phase == 10 and age>.4:
                player.drive_for_test(.1,0.,0.,True)
                advance()
            elif phase == 11 and age>.12:
                check(player.character_movement.is_falling(),'Jump is airborne before view turn')
                player.look_for_test(90.)
                advance()
            elif phase == 12 and age>.12:
                check(player.character_movement.is_falling(),'Observe view turn during jump')
                check(close(camera.get_camera_rotation().yaw,90) and close(player.get_movement_basis_yaw(),90),'Airborne view and new input remain view-led')
                finish(True,'')
        except Exception as error:
            finish(False,str(error))
    def finish(ok,error):
        global _handle
        unreal.unregister_slate_post_tick_callback(_handle)
        _handle=None
        report.write_text(json.dumps({'ok':ok,'error':error,'checks':state['checks'],'samples':state['samples']},indent=2))
        player.reset_practice_position()
    reset()
    _handle=unreal.register_slate_post_tick_callback(tick)
    return 'scheduled'
