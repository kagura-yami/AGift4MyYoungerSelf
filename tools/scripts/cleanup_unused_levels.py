"""Prune reviewed chapter regions; retain a restorable backup and actor manifest."""
import unreal, json, shutil
from pathlib import Path
ROOT=Path(unreal.Paths.project_dir()).resolve()
OUT=ROOT.parent/'tools/evaluation/level-cleanup'
HOME='/Game/Maps/HomeVertical/Maps/L_Home_Vertical_02'
SCHOOL='/Game/Maps/School/School_v2'
UNUSED=['/Game/Maps/L_LogicLab','/Game/Maps/L_Tutorial','/Game/Maps/L_ChaseWhitebox','/Game/Maps/L_Lv4MechanismWhitebox','/Game/Maps/HomeVertical/Maps/L_Home_Vertical_01','/Game/Maps/School/School']
GLOBAL={'WorldSettings','LevelScriptActor','LightmassImportanceVolume','NavMeshBoundsVolume','RecastNavMesh','SkyLight','DirectionalLight','ExponentialHeightFog','PostProcessVolume'}
def keep(row,home):
    n,f,c=row['name'],row['folder'],row['cls']
    x,y,z=row['loc']
    if c in GLOBAL or c=='PlayerStart': return True
    if home:
        if n=='BP_NotCanOpenDoor_C_5' or f.startswith(('Home/01_BirthBedroom','Home/02_FoldedCorridor','Home/00_Atmosphere','Gameplay/BedroomShelfRoute','Gameplay/ChapterExit')): return True
        # Shared doorway wall closes the end of the surviving corridor.
        if n in ['StaticMeshActor_292','StaticMeshActor_293','StaticMeshActor_294']: return True
        if f.startswith(('Home/03','Home/04','Home/07','Home/阳台')) or f=='Home': return False
        return row['center'][0] <= 2512 and row['center'][2] < 2600
    if f=='Base' or f=='Gameplay/ChapterExit' or n=='BP_Classroom_Array_C_13022': return True
    # Upper floors are separate array blueprints, not part of the starting classroom.
    if c=='BP_Classroom_Array_C': return False
    if n=='TripoZone_2': return False # Old corridor checkpoint beyond the new exit.
    if n in ['StaticMeshActor_1','BP_CanOpenDoor_C_1']: return True
    return -560<=x<=530 and -450<=y<=340 and -50<=z<=430

def process(path,apply):
    ls=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    aa=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    assert ls.load_level(path)
    records=json.loads((OUT/(path.rsplit('/',1)[1]+'.json')).read_text(encoding='utf-8'))
    retained={r['name'] for r in records if keep(r,path==HOME)}
    actors=list(aa.get_all_level_actors())
    # Preserve attached objects belonging to retained structures, and their ancestors.
    changed=True
    while changed:
        changed=False
        for a in actors:
            parent=a.get_attach_parent_actor()
            if parent and (a.get_name() in retained or parent.get_name() in retained):
                for obj in [a,parent]:
                    if obj.get_name() not in retained:
                        retained.add(obj.get_name());changed=True
    removed=[r for r in records if r['name'] not in retained]
    (OUT/(path.rsplit('/',1)[1]+'.removal.json')).write_text(json.dumps(removed,ensure_ascii=False,indent=2),encoding='utf-8')
    if apply:
        for a in actors:
            if unreal.SystemLibrary.is_valid(a) and not a.get_name().startswith('DESTROYED_') and a.get_name() not in retained:
                assert aa.destroy_actor(a), a.get_name()
        assert ls.save_current_level()
    return dict(map=path,before=len(records),after=len(retained),removed=len(removed))

def run(apply=False):
    OUT.mkdir(parents=True,exist_ok=True)
    for path in [HOME,SCHOOL]+UNUSED:
        src=ROOT/'Content'/(path.removeprefix('/Game/')+'.umap')
        dst=OUT/'backup'/src.relative_to(ROOT)
        if src.exists() and not dst.exists(): dst.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(src,dst)
    report=[process(p,apply) for p in [HOME,SCHOOL]]
    (OUT/('result.json' if apply else 'plan.json')).write_text(json.dumps(report,indent=2),encoding='utf-8')
    print(report)

run(globals().get('APPLY_CLEANUP',False))
