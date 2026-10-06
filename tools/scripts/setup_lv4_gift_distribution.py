"""Use existing desk-gift presentation for fixed/random rewards and lobby safety."""
import unreal,shutil,time
from pathlib import Path
es=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
ls=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
aa=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
assert es.get_editor_world().get_name()=='lv4' and not es.get_game_world()
assert ls.save_current_level()
root=Path(unreal.Paths.project_dir()).resolve().parent
out=root/'tools/evaluation/gift-distribution';out.mkdir(parents=True,exist_ok=True)
shutil.copy2(root/'game/Content/Maps/lv4.umap',out/('lv4-'+time.strftime('%Y%m%d-%H%M%S')+'.umap'))
by={a.get_actor_label():a for a in aa.get_all_level_actors()}
source=by['LV4_OfficeDesk_DashGift']
def presentation(g):
 g.modify()
 for dst,src in [(g.box_mesh,source.box_mesh),(g.lid_mesh,source.lid_mesh)]:
  dst.set_static_mesh(src.static_mesh);dst.set_relative_transform(src.get_relative_transform(),False,True)
  dst.set_editor_property('override_materials',list(src.get_editor_property('override_materials')))
 for prop in ['interaction_distance','lid_lift','open_seconds']:g.set_editor_property(prop,source.get_editor_property(prop))
 g.prompt.set_hidden_in_game(True)
 dst=g.get_component_by_class(unreal.TripoInteractionTarget)
 src=source.get_component_by_class(unreal.TripoInteractionTarget)
 dst.set_relative_transform(src.get_relative_transform(),False,True);dst.set_box_extent(src.get_unscaled_box_extent())
def option(ability):
 r=unreal.TripoRewardOption();r.ability=ability;r.weight=1;return r
lift=by['LV4_Gift_ElevatorRoof_Random'];presentation(lift)
lift.set_editor_property('rewards',[option(unreal.TripoAbility.UP_DASH)])
random=list(unreal.get_default_object(unreal.TripoGiftBox).rewards)
spots=[('OfficeCorner',(440,-5,520)),('GroundDesk',(205,25,60)),('Vending',(0,995,60)),('Lounge',(1260,-1380,561.3))]
for key,xyz in spots:
 label='LV4_RandomGift_'+key
 g=by.get(label) or aa.spawn_actor_from_class(source.get_class(),unreal.Vector(*xyz))
 presentation(g);g.set_actor_label(label);g.set_folder_path('Gameplay/Gifts')
 hit=unreal.SystemLibrary.line_trace_single(es.get_editor_world(),unreal.Vector(xyz[0],xyz[1],xyz[2]+35),unreal.Vector(xyz[0],xyz[1],xyz[2]-15),unreal.TraceTypeQuery.TRACE_TYPE_QUERY1,True,[g],unreal.DrawDebugTrace.NONE,True)
 assert hit,key
 g.set_actor_location(hit.to_tuple()[5]+unreal.Vector(0,0,.3),False,True)
 g.set_editor_property('gift_id','LV4.Random.'+key);g.set_editor_property('rewards',random)
 print('GIFT',key,str(g.get_actor_location()),hit.to_tuple()[9].get_actor_label())
e=by['SM_Bld_Elevator_3'];e.modify()
e.static_mesh_component.set_static_mesh(unreal.load_asset('/Game/Blueprint/Lv4/Collision/SM_ChaseEndElevator_Open'))
e.static_mesh_component.set_collision_profile_name('BlockAll')
# The copied cabin overlaps the existing long corridor wall collider.
# Preserve that wall except for the cabin doorway (including a solid header).
wall=by['LV4_Solid_SkeletalMeshActor_93'];wall.modify()
wall.static_mesh_component.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
center,extent=wall.get_actor_bounds(False)
ymin,ymax=center.y-extent.y,center.y+extent.y
zmin,zmax=center.z-extent.z,center.z+extent.z
for key,lo,hi,bottom,top in [('South',ymin,-805,zmin,zmax),('North',-535,ymax,zmin,zmax),('Header',-805,-535,925,zmax)]:
 label='LV4_CopyElevator_Wall_'+key
 a=by.get(label) or aa.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector())
 a.modify();a.set_actor_label(label);a.set_folder_path('LV4_Mechanisms/WallCollision')
 a.static_mesh_component.set_static_mesh(unreal.load_asset('/Engine/BasicShapes/Cube'))
 a.set_actor_location(unreal.Vector(center.x,(lo+hi)/2,(bottom+top)/2),False,True)
 a.set_actor_scale3d(unreal.Vector(extent.x*2/100,(hi-lo)/100,(top-bottom)/100))
 a.static_mesh_component.set_collision_profile_name('BlockAll')
 a.set_actor_hidden_in_game(True);a.static_mesh_component.set_visibility(False)
name='LV4_SecondFloorLobby_ClearAggro'
zone=by.get(name) or aa.spawn_actor_from_class(unreal.TripoChaseHideZone,unreal.Vector(-525,620,725))
zone.modify();zone.set_actor_label(name);zone.set_folder_path('Gameplay/OfficeChase')
zone.set_actor_location(unreal.Vector(-525,620,725),False,True)
zone.volume.set_box_extent(unreal.Vector(1075,760,205))
zone.set_editor_property('aggro_reduction_per_second',0)
zone.label.set_hidden_in_game(True);zone.label.set_visibility(False)
# Deliberately do not register this as an ExitZone: lobby clears aggro, safe rooms despawn.
by['LV4_OfficeDoor_ChaseTrigger'].build_navigation_for_level()
assert ls.save_current_level()
