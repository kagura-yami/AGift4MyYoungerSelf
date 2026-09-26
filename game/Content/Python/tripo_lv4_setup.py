"""Make lv4 a playable map: global GameMode already spawns ATripoCharacter from any
PlayerStart, so this script only has to guarantee a sane spawn point exists and
optionally place a static reference copy of the graybox prototype.

Run inside the UE editor Python console with lv4 open:

    import tripo_lv4_setup as s
    s.report()      # 看一下现状
    s.build()       # 放 PlayerStart（+ 可选的静态参考模型）
    s.clean()       # 撤掉本脚本加的东西
    s.dry_run()     # 只看坐标是不是悬空/埋墙
"""
import unreal

MAP_PATH = '/Game/Maps/lv4'
TAG = 'Lv4Setup'

# 参考模型：C++ 构造函数里 GrayboxBody 缩放在胶囊中心、ForwardMarker 在前方。
BODY_SCALE = (0.62, 0.62, 1.5)
BODY_Z_OFFSET = 0.0
NOSE_OFFSET = (43.0, 0.0, 48.0)
NOSE_SCALE = (0.3, 0.3, 0.65)


def _editor():
    return unreal.get_editor_subsystem(unreal.EditorActorSubsystem)


def _world():
    return unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()


def _assert_right_map():
    world = _world()
    assert world and MAP_PATH in world.get_path_name(), \
        '先打开 lv4 再跑（当前：%s）' % (world.get_path_name() if world else 'None')
    return world


def _ours():
    return [a for a in _editor().get_all_level_actors() if TAG in a.get_actor_tags()]


def report():
    """不修改任何东西，只汇报 lv4 里跟"能不能跑起来"有关的现状。"""
    _assert_right_map()
    editor = _editor()
    actors = editor.get_all_level_actors()

    starts = [a for a in actors if isinstance(a, unreal.PlayerStart)]
    chars = [a for a in actors if isinstance(a, unreal.TripoCharacter)]
    meshes = [a for a in actors if isinstance(a, unreal.StaticMeshActor)]
    zones = [a for a in actors if isinstance(a, unreal.TripoZone)]

    game_mode = unreal.GameplayStatics.get_game_mode(_world())
    default_pawn = None
    try:
        cdo = unreal.get_default_object(unreal.TripoGameMode)
        default_pawn = cdo.get_editor_property('default_pawn_class')
    except Exception:
        pass

    bounds = None
    if meshes:
        box = unreal.MathLibrary.make_box(unreal.Vector(0, 0, 0), unreal.Vector(0, 0, 0))
        pass

    return {
        'map': _world().get_path_name(),
        'total_actors': len(actors),
        'player_starts': [(a.get_actor_label(), tuple(a.get_actor_location())) for a in starts],
        'tripo_characters': [a.get_actor_label() for a in chars],
        'static_meshes': len(meshes),
        'tripo_zones': [(a.get_actor_label(), str(a.get_editor_property('kind'))) for a in zones],
        'default_pawn_class': str(default_pawn) if default_pawn else '(读不到)',
        'our_actors': [a.get_actor_label() for a in _ours()],
    }


def dry_run(start_xyz=None):
    """不做任何修改，检查打算放的 PlayerStart 有没有悬空或埋在几何体里。"""
    _assert_right_map()
    xyz = start_xyz or _suggest_start()
    editor = _editor()
    actors = editor.get_all_level_actors()
    meshes = [a for a in actors if isinstance(a, unreal.StaticMeshActor)]

    probe_from = unreal.Vector(xyz[0], xyz[1], xyz[2] + 300.0)
    probe_to = unreal.Vector(xyz[0], xyz[1], xyz[2] - 2000.0)
    hit = unreal.SystemLibrary.line_trace_single(
        _world(), probe_from, probe_to,
        unreal.TraceTypeQuery.TRACE_TYPE_QUERY1, False, [],
        unreal.DrawDebugTrace.NONE, True)

    # 胶囊在候选点能不能塞下（r=34, h=88，留点余量）
    r, h = 40.0, 96.0
    overlap = unreal.SystemLibrary.box_overlap_actors(
        _world(),
        unreal.Vector(xyz[0], xyz[1], xyz[2]),
        unreal.Vector(r, r, h),
        unreal.Rotator(0, 0, 0), [], unreal.DrawDebugTrace.NONE, True)

    return {
        'candidate': xyz,
        'ground_hit': bool(hit),
        'ground_z': hit.location.z if hit else None,
        'ground_actor': hit.actor.get_actor_label() if hit else None,
        'drop_to_ground': round(xyz[2] - hit.location.z, 1) if hit else None,
        'blocking_actors': [a.get_actor_label() for a in (overlap or []) if a not in meshes[:0]] if overlap else [],
        'overlap_count': len(overlap) if overlap else 0,
    }


def _suggest_start():
    """没有 PlayerStart 时，猜一个落点：取所有静态网格的包围盒中心。"""
    assert _assert_right_map()
    meshes = [a for a in _editor().get_all_level_actors() if isinstance(a, unreal.StaticMeshActor)]
    if not meshes:
        return (0.0, 0.0, 120.0)
    lo = [1e9, 1e9, 1e9]
    hi = [-1e9, -1e9, -1e9]
    for m in meshes:
        origin, extent = m.get_actor_bounds(False)
        for i in range(3):
            lo[i] = min(lo[i], origin[i] - extent[i])
            hi[i] = max(hi[i], origin[i] + extent[i])
    return (round((lo[0] + hi[0]) * 0.5, 1),
            round((lo[1] + hi[1]) * 0.5, 1),
            round(hi[2] + 120.0, 1))


def build(start_xyz=None, add_reference_copy=True, snap_to_ground=True):
    """放 PlayerStart（必要时），并在旁边放一个静态参考小人。

    add_reference_copy：摆一个不会动、不参与游戏的灰色球+锥，只为了在编辑器里
    看得见"角色站上去大概占多大位置"。真正的操控角色由 GameMode 在 PIE 时生成。
    """
    world = _assert_right_map()
    editor = _editor()
    clean()

    existing = [a for a in editor.get_all_level_actors() if isinstance(a, unreal.PlayerStart)]
    created = []

    if existing:
        start = existing[0]
        note = '沿用已有的 PlayerStart: ' + start.get_actor_label()
    else:
        xyz = start_xyz or _suggest_start()
        if snap_to_ground:
            hit = unreal.SystemLibrary.line_trace_single(
                world,
                unreal.Vector(xyz[0], xyz[1], xyz[2] + 300.0),
                unreal.Vector(xyz[0], xyz[1], xyz[2] - 2000.0),
                unreal.TraceTypeQuery.TRACE_TYPE_QUERY1, False, [],
                unreal.DrawDebugTrace.NONE, True)
            if hit:
                xyz = (xyz[0], xyz[1], hit.location.z + 92.0)
        start = editor.spawn_actor_from_class(unreal.PlayerStart, unreal.Vector(*xyz))
        start.set_actor_label('Lv4_PlayerStart')
        start.set_actor_rotation(unreal.Rotator(0.0, 0.0, 0.0), False)
        created.append(start)
        note = '新建 PlayerStart @ %s' % (tuple(round(v, 1) for v in xyz),)

    for a in created:
        a.set_editor_property('tags', [TAG])

    if add_reference_copy:
        loc = start.get_actor_location()
        rot = start.get_actor_rotation()

        base = editor.spawn_actor_from_class(
            unreal.StaticMeshActor,
            unreal.Vector(loc.x + 160.0, loc.y, loc.z - 4.0))
        base.set_actor_label('Lv4_Ref_GrayboxBody')
        base.set_actor_scale3d(unreal.Vector(*BODY_SCALE))
        base.static_mesh_component.set_static_mesh(
            unreal.load_asset('/Engine/BasicShapes/Sphere.Sphere'))
        base.static_mesh_component.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
        base.set_editor_property('tags', [TAG])

        nose = editor.spawn_actor_from_class(
            unreal.StaticMeshActor,
            unreal.Vector(loc.x + 160.0 + NOSE_OFFSET[0],
                          loc.y + NOSE_OFFSET[1],
                          loc.z - 4.0 + NOSE_OFFSET[2]))
        nose.set_actor_label('Lv4_Ref_ForwardMarker')
        nose.set_actor_rotation(unreal.Rotator(-90.0, 0.0, 0.0), False)
        nose.set_actor_scale3d(unreal.Vector(*NOSE_SCALE))
        nose.static_mesh_component.set_static_mesh(
            unreal.load_asset('/Engine/BasicShapes/Cone.Cone'))
        nose.static_mesh_component.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
        nose.set_editor_property('tags', [TAG])

    saved = unreal.EditorLoadingAndSavingUtils.save_map(world, MAP_PATH)
    return {
        'note': note,
        'player_start': start.get_actor_label(),
        'start_location': tuple(round(v, 1) for v in start.get_actor_location()),
        'added': [a.get_actor_label() for a in created] + (['GrayboxBody', 'ForwardMarker'] if add_reference_copy else []),
        'saved': bool(saved),
        'next': 'PIE 直接按 Play，GameMode 会在这个 PlayerStart 上生成 ATripoCharacter',
    }


def clean():
    """撤掉本脚本加过的所有东西。"""
    _assert_right_map()
    editor = _editor()
    removed = []
    for a in _ours():
        removed.append(a.get_actor_label())
        editor.destroy_actor(a)
    return removed


def check():
    """跑完全套：报告 + 干跑 + 构建，返回一份可读的 dict。"""
    r = report()
    d = dry_run()
    b = build()
    return {'before': r, 'dry_run': d, 'build': b, 'after': report()}
