import unreal,shutil
from pathlib import Path
root=Path(unreal.Paths.project_dir()).resolve()
src=root/'Content/Maps/HomeVertical/Maps/L_Home_Vertical_02.umap'
bak=root.parent/'tools/evaluation/home-lighting-backup.umap'
if not bak.exists(): shutil.copy2(src,bak)
es=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
asys=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
assert 'L_Home_Vertical_02' in es.get_editor_world().get_name()
for name,pos,power,radius in [('TVWall_SoftFill',(-1630,0,440),150,720),('TVWall_PathFill',(-1150,0,340),65,620)]:
 a=next((a for a in asys.get_all_level_actors() if a.get_actor_label()==name),None)
 if not a:a=asys.spawn_actor_from_class(unreal.PointLight,unreal.Vector(*pos))
 a.set_actor_label(name);a.set_folder_path('Home/00_Atmosphere/TVWallReadability')
 c=a.light_component;c.set_mobility(unreal.ComponentMobility.MOVABLE)
 c.set_editor_property('intensity_units',unreal.LightUnits.LUMENS)
 c.set_intensity(power);c.set_editor_property('attenuation_radius',radius)
 c.set_editor_property('source_radius',130);c.set_editor_property('soft_source_radius',110)
 c.set_light_color(unreal.LinearColor(.72,.84,1.0));c.set_cast_shadows(name!='TVWall_PathFill')
es.set_level_viewport_camera_info(unreal.Vector(-700,470,230),unreal.Rotator(pitch=0,yaw=-145,roll=0))
unreal.SystemLibrary.execute_console_command(es.get_editor_world(),'Shot SHOWUI filename=C:/Dev/Projects/UE/Tripothon/tools/evaluation/home-lighting-after.png')

unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
