import unreal,shutil,time
from pathlib import Path
s=unreal.get_editor_subsystem(unreal.EditorActorSubsystem); actors=s.get_all_level_actors(); by={a.get_actor_label():a for a in actors}
out=Path(unreal.Paths.project_dir()).resolve().parent/'tools/evaluation/window-exit';out.mkdir(exist_ok=True)
shutil.copy2(Path(unreal.Paths.project_content_dir())/'Maps/lv4.umap',out/('lv4-before-'+str(int(time.time()))+'.umap'))
for label,pos,entry in [('LV4_CompanyWindowEntry',(1006,275,610),True),('LV4_CompanyWindowExit',(1006,102,610),False)]:
 a=by.get(label) or s.spawn_actor_from_class(unreal.TripoTeleportPoint,unreal.Vector(*pos))
 a.set_actor_label(label);a.set_folder_path('LV4_Mechanisms/CompanyJump')
 a.set_actor_location_and_rotation(unreal.Vector(*pos),unreal.Rotator(pitch=0,yaw=-90,roll=0),False,True)
 a.set_editor_property('link_id','LV4_CompanyWindow');a.set_editor_property('entry_enabled',entry)
 a.set_editor_property('require_forward_direction',True);a.set_editor_property('minimum_forward_dot',.7)
 a.set_editor_property('trigger_radius',100.);a.set_editor_property('destination_offset',unreal.Vector())
label='LV4_CompanyWindowHint'
a=by.get(label) or s.spawn_actor_from_class(unreal.TextRenderActor,unreal.Vector(940,240,690),unreal.Rotator(pitch=0,yaw=90,roll=0))
a.set_actor_label(label);a.set_folder_path('LV4_Mechanisms/CompanyJump')
a.text_render.set_text('SHIFT');a.text_render.set_world_size(16);a.text_render.set_horizontal_alignment(unreal.HorizTextAligment.EHTA_CENTER)
a.text_render.set_text_render_color(unreal.Color(110,220,240,255))
for a in actors:
 if a.get_actor_label()=='TEMP_WindowInspect':s.destroy_actor(a)
unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True,True)
print('Window entry and safe outside landing saved')

