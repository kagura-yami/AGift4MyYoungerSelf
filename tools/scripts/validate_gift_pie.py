"""Run in a fresh L_ChaseWhitebox PIE instance. Never writes the user's save slots."""
import json
import time
from pathlib import Path
import unreal

world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
assert world and 'ChaseWhitebox' in world.get_name()
player=unreal.GameplayStatics.get_player_character(world,0)
pc=unreal.GameplayStatics.get_player_controller(world,0)
progress=unreal.TripoProgressSubsystem.get(player)
runtime=unreal.TripoRuntimeSubsystem.get_runtime(player)
boxes={str(a.get_editor_property('gift_id')):a for a in unreal.GameplayStatics.get_all_actors_of_class(world,unreal.TripoGiftBox)}
random_box=boxes['Whitebox.Random']
stone_box=boxes['Whitebox.Stone']
empty_box=boxes['Whitebox.Empty']
for trigger in unreal.GameplayStatics.get_all_actors_of_class(world,unreal.TripoChaseTrigger):
    trigger.set_editor_property('enabled',False)
checks=[]
phase=0
started=time.monotonic()
key=unreal.Key()
key.set_editor_property('key_name','E')
report=Path(unreal.Paths.project_dir()).resolve().parent/'tools/evaluation/gift-pie.json'
ABILITIES=[unreal.TripoAbility.DASH,unreal.TripoAbility.UP_DASH,unreal.TripoAbility.WALL_JUMP,unreal.TripoAbility.STEP_STONE,unreal.TripoAbility.SLOW,unreal.TripoAbility.REWIND,unreal.TripoAbility.ECHO,unreal.TripoAbility.BONUS_TIME]
def levels(): return [player.abilities.get_level(a) for a in ABILITIES]
baseline=levels()

def check(name,result):
    checks.append({'name':name,'passed':bool(result)})
    if not result: raise AssertionError(name)

def place(x,y,z=90):
    player.set_actor_location(unreal.Vector(x,y,z),False,True)
    player.character_movement.stop_movement_immediately()

def finish(error=None):
    pc.key_for_test(key,False)
    runtime.set_pause_reason(unreal.TripoPauseReason.MENU,False)
    unreal.unregister_slate_post_tick_callback(callback)
    report.write_text(json.dumps({'passed':error is None,'error':error,'checks':checks},ensure_ascii=False,indent=2),encoding='utf-8')
    unreal.log('GIFT_PIE_RESULT '+str(report)+' success='+str(error is None))

place(-160,-450)

def tick(dt):
    global phase,started,baseline,echo
    elapsed=time.monotonic()-started
    try:
        if phase==0 and elapsed>.4:
            check('Remote box cannot be opened',not stone_box.try_open(player))
            check('Nearby box is reachable',random_box.is_in_reach(player))
            runtime.set_pause_reason(unreal.TripoPauseReason.MENU,True)
            check('Pause prevents rewards',not random_box.try_open(player))
            runtime.set_pause_reason(unreal.TripoPauseReason.MENU,False)
            pc.key_for_test(key,True)
            phase=1;started=time.monotonic()
        elif phase==1 and elapsed>.25:
            pc.key_for_test(key,False)
            check('E input actually opens the nearest box',random_box.opened)
            now=levels()
            check('Exactly one ability gains one level',sum(now)-sum(baseline)==1 and sum(a!=b for a,b in zip(now,baseline))==1)
            check('Receipt recorded with a stable map key',progress.has_claimed_gift(random_box.get_receipt_key()))
            check('Receipt matches the awarded ability',now[random_box.receipt.ability_index]==random_box.receipt.granted_level)
            check('Repeated interaction cannot award again',not random_box.try_open(player) and levels()==now)
            check('Reward feedback names the gift', '收到礼物' in progress.get_message())
            phase=2;started=time.monotonic()
        elif phase==2 and elapsed>.8:
            check('Lid animates upward',random_box.lid_mesh.get_editor_property('relative_location').z>160)
            check('Receipt menu pauses gameplay',pc.get_hud().is_gift_receipt_open() and runtime.is_action_paused())
            pc.get_hud().handle_action('gift.confirm')
            check('Confirm resumes gameplay',not runtime.is_action_paused())
            # A nearby wall blocks interaction even with sufficient distance.
            empty_box.set_actor_location(unreal.Vector(1200,0,0),False,True)
            place(1000,0)
            check('Wall blocks gift interaction',not empty_box.try_open(player))
            empty_box.set_actor_location(unreal.Vector(0,650,0),False,True)
            place(-160,-900)
            saved_id=stone_box.get_editor_property('gift_id')
            stone_box.set_editor_property('gift_id','None')
            check('Missing stable ID cannot award',not stone_box.try_open(player))
            stone_box.set_editor_property('gift_id',saved_id)
            bad=unreal.TripoRewardOption()
            bad.set_editor_property('weight',-1)
            original=list(stone_box.get_editor_property('rewards'))
            stone_box.set_editor_property('rewards',[bad])
            check('Malformed pool does not consume the box',not stone_box.try_open(player) and not stone_box.opened)
            stone_box.set_editor_property('rewards',original)
            player.abilities.grant_level_floor(unreal.TripoAbility.ECHO,1)
            check('Echo can be created for reward-sharing test',pc.create_echo())
            echo=next(b for b in pc.get_echo_bodies() if b!=player)
            baseline=levels()
            check('Configured stone gift awards successfully',stone_box.try_open(player))
            check('Configured pool upgrades the expected ability',player.abilities.get_level(unreal.TripoAbility.STEP_STONE)==baseline[3]+1)
            check('Existing clone receives the same upgrade',echo.abilities.get_level(unreal.TripoAbility.STEP_STONE)==baseline[3]+1)
            phase=4;started=time.monotonic()
        elif phase==4 and elapsed>1:
            pc.get_hud().handle_action('gift.confirm')
            check('Switching clone cannot duplicate receipt',pc.possess_echo_body(echo) and not stone_box.try_open(echo))
            pc.possess_echo_body(player)
            pc.reclaim_echo(echo)
            place(-160,650)
            empty_box.set_editor_property('gift_id','Whitebox.Random')
            check('Duplicate gift ID shares the existing receipt',not empty_box.try_open(player))
            empty_box.set_editor_property('gift_id','Whitebox.Empty')
            empty_box.set_editor_property('rewards',list(random_box.get_editor_property('rewards')))
            for ability in ABILITIES: player.abilities.grant_level_floor(ability,3)
            check('All-maxed pool still completes safely',empty_box.try_open(player))
            check('All-maxed pool grants a keepsake, not level four',empty_box.receipt.ability_index==-1 and levels()==[3]*8)
            phase=5;started=time.monotonic()
        elif phase==5 and elapsed>1:
            pc.get_hud().handle_action('gift.confirm')
            empty_box.set_editor_property('gift_id','Whitebox.TestEmpty')
            empty_box.set_editor_property('rewards',[])
            check('Explicitly empty pool also settles safely',empty_box.try_open(player) and empty_box.receipt.ability_index==-1)
            phase=6;started=time.monotonic()
        elif phase==6 and elapsed>1:
            pc.get_hud().handle_action('gift.confirm')
            check('Checkpoint recovery succeeds',unreal.TripoWorldSubsystem.get(player).restore_player(player))
            phase=3;started=time.monotonic()
        elif phase==3 and elapsed>.3:
            check('Recovery retains claimed gift ledger',progress.has_claimed_gift(random_box.get_receipt_key()))
            place(-160,-450)
            check('Recovery does not reopen claimed gifts',random_box.opened and not random_box.try_open(player))
            finish()
    except Exception as exc:
        finish(str(exc))

callback=unreal.register_slate_post_tick_callback(tick)
print('GIFT_PIE_STARTED')
