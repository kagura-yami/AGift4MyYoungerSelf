"""Cross-frame checks in LV4 PIE; no save-game or editor asset changes."""
import unreal, time, json
from pathlib import Path
w = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
assert w and 'lv4' in w.get_name().lower()
p = unreal.GameplayStatics.get_player_character(w, 0)
assert p and p.is_player_controlled()
t = unreal.GameplayStatics.get_all_actors_of_class(w, unreal.TripoChaseTrigger)[0]
zone = next(z for z in unreal.GameplayStatics.get_all_actors_of_class(w, unreal.TripoChaseHideZone) if z.get_actor_label() == 'LV4_ChaseEndElevator_SafeZone')
for n in unreal.GameplayStatics.get_all_actors_of_class(w, unreal.TripoChaser): n.stop_chase()
if not t.active_chaser: assert t.activate_chase(p)
n = t.active_chaser
mesh = n.get_component_by_class(unreal.SkeletalMeshComponent)
anim = mesh.get_anim_instance()
checks, samples = [], []
phase, at = 0, time.monotonic()
out = Path(unreal.Paths.project_dir()).resolve().parent / 'tools/evaluation/grandma-chase-validation.json'

def check(name, ok):
    checks.append(dict(name=name, passed=bool(ok)))
    assert ok, name

def place(a, xyz, yaw=0):
    a.set_actor_location_and_rotation(unreal.Vector(*xyz), unreal.Rotator(yaw=yaw), False, True)
    a.character_movement.stop_movement_immediately()

def advance():
    global phase, at
    phase += 1
    at = time.monotonic()

def finish(error=None):
    unreal.unregister_slate_post_tick_callback(cb)
    n.stop_chase()
    out.write_text(json.dumps(dict(passed=error is None, error=error, checks=checks, samples=samples), indent=2), encoding='utf-8')
    print('GRANDMA_VALIDATION', error or 'PASS')

check('Spawned enemy uses Grandma mesh', mesh.skeletal_mesh_asset.get_name() == 'SK_MiniCharacter_Grandma_01')
check('Dedicated textured material assigned', mesh.get_material(0).get_name() == 'MI_Grandma_Chase')
check('Native locomotion instance initialized', isinstance(anim, unreal.TripoLocomotionAnimInstance))
n.stop_chase()
place(n, (1956, -1750, 610), -90)
place(p, (1956, -2190, 610), 90)
n.set_editor_property('catch_hold_seconds', 100)

def tick(dt):
    global marker, idle_bone
    elapsed = time.monotonic() - at
    try:
        if phase == 0 and elapsed > .4:
            check('Stationary enemy enters idle animation', anim.motion_state == 0)
            idle_bone = mesh.get_socket_transform('head', unreal.RelativeTransformSpace.RTS_ACTOR).translation
            marker = n.get_actor_location()
            check('Pursuit starts against controlled player', n.start_chase(p))
            advance()
        elif phase == 1:
            samples.append(dict(speed=anim.ground_speed, state=anim.motion_state))
            if elapsed > .65:
                check('Navigation moves enemy', n.get_actor_location().distance(marker) > 25)
                check('Movement selects moving animation', any(s['state'] == 1 and s['speed'] > 12 for s in samples))
                check('Animated bones change pose', mesh.get_socket_transform('head', unreal.RelativeTransformSpace.RTS_ACTOR).translation.distance(idle_bone) > .01)
                place(p, (1956, -2300, 635))
                p.set_actor_location(unreal.Vector(2280, -2300, 635), True, True)
                check('Player capsule sweeps through elevator doorway', abs(p.get_actor_location().x - 2280) < 1)
                check('Entered cabin is in safe zone', zone.contains_point(p.get_actor_location()))
                place(n, (2050, -2300, 635), 0)
                n.start_chase(p)
                advance()
        elif phase == 2 and elapsed > .2:
            check('Cabin clears aggro immediately', n.aggro == 0)
            check('Cabin ends chasing and searching', n.state == unreal.TripoChaseState.WANDERING)
            n.set_actor_location(unreal.Vector(2280, -2300, 635), True, True)
            check('Safe zone blocks enemy entry', n.get_actor_location().x < 2130)
            advance()
        elif phase == 3 and elapsed > 1:
            check('Visible player inside cabin cannot rebuild aggro', n.aggro == 0)
            check('Cabin floor supports player', 600 < p.get_actor_location().z < 650)
            p.set_actor_location(unreal.Vector(2600, -2300, 635), True, True)
            check('Cabin back wall still blocks player', p.get_actor_location().x < 2450)
            place(p, (2280, -2300, 635))
            p.set_actor_location(unreal.Vector(1956, -2300, 635), True, True)
            check('Player can leave cabin through doorway', abs(p.get_actor_location().x - 1956) < 1)
            finish()
        elif elapsed > 8: raise TimeoutError(str(phase))
    except Exception as e: finish(str(e))

cb = unreal.register_slate_post_tick_callback(tick)
