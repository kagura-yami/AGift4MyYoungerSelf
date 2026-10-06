import unreal, json
from pathlib import Path
out=Path(unreal.Paths.project_dir()).resolve().parent/'tools/evaluation/level-cleanup'
out.mkdir(parents=True,exist_ok=True)
def inspect(path):
    ls=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    assert ls.load_level(path)
    rows=[]
    for a in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors():
        loc=a.get_actor_location(); center,extent=a.get_actor_bounds(False)
        rows.append(dict(name=a.get_name(),label=a.get_actor_label(),cls=a.get_class().get_name(),folder=str(a.get_folder_path()),loc=[loc.x,loc.y,loc.z],center=[center.x,center.y,center.z],extent=[extent.x,extent.y,extent.z]))
    (out/(path.rsplit('/',1)[1]+'.json')).write_text(json.dumps(rows,ensure_ascii=False,indent=2),encoding='utf-8')
for p in ['/Game/Maps/HomeVertical/Maps/L_Home_Vertical_02','/Game/Maps/School/School_v2']:
    inspect(p)
ar=unreal.AssetRegistryHelpers.get_asset_registry()
ar.search_all_assets(True)
maps=[str(a.package_name) for a in ar.get_assets_by_path('/Game',True) if str(a.asset_class_path.asset_name)=='World']
refs={p:[str(r) for r in ar.get_referencers(p,unreal.AssetRegistryDependencyOptions(True,True,False,False))] for p in maps}
(out/'map-references.json').write_text(json.dumps(refs,indent=2),encoding='utf-8')
