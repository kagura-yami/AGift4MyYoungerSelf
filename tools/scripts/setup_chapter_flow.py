"""Add explicit chapter exits at existing end-of-level gift markers; preserve authored gates."""
import unreal, shutil
from pathlib import Path
root=Path(unreal.Paths.project_dir()).resolve()
ls=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
asys=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
for package,gift,event,hint in [('/Game/Maps/HomeVertical/Maps/L_Home_Vertical_02','Story.Home.GiftFound','Story.Home.Exit','E / 前往学校'),('/Game/Maps/School/School_v2','Story.School.GiftFound','Story.School.ScreenPortal','E / 前往第三关')]:
 source=root/'Content'/ (package.removeprefix('/Game/')+'.umap')
 backup=root.parent/'tools/evaluation/chapter-flow-backup'/source.name
 backup.parent.mkdir(parents=True,exist_ok=True)
 if not backup.exists(): shutil.copy2(source,backup)
 ls.load_level(package)
 actors=asys.get_all_level_actors()
 marker=next(a for a in actors if isinstance(a,unreal.TripoStoryTrigger) and str(a.event_id)==gift)
 label='ChapterExit_'+event.split('.')[1]
 exits=[a for a in actors if a.get_actor_label()==label]
 a=exits[0] if exits else asys.spawn_actor_from_class(unreal.TripoStoryTrigger,marker.get_actor_location())
 a.set_actor_label(label)
 a.set_editor_property('catalog',marker.catalog)
 a.set_editor_property('event_id',event)
 a.set_editor_property('automatic',False)
 a.set_editor_property('hint',hint)
 a.get_editor_property('label').set_text(hint)
 print('EXIT',package,a.get_actor_location(),event)
 ls.save_current_level()
