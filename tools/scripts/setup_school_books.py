"""Run inside Unreal after building. Preserve authored books as disabled references."""
import unreal
import math,struct,wave
from pathlib import Path
s=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
actors=s.get_all_level_actors()
def label(name): return next(a for a in actors if a.get_actor_label()==name)
def spawn(cls,name,loc,rot=unreal.Rotator()):
    a=next((a for a in actors if a.get_actor_label()==name),None)
    if not a:a=s.spawn_actor_from_class(cls,loc,rot)
    a.set_actor_location(loc,False,True);a.set_actor_rotation(rot,True)
    a.set_actor_label(name);a.set_folder_path('Gameplay/BooksPuzzle')
    return a
p=spawn(unreal.TripoSchoolBooks,'School_ManualBooks',unreal.Vector(25600,3335,6480))
books=[]
for index,oldname in enumerate(['BP_BookGameplay3','BP_BookGameplay2','BP_BookGameplay']):
    old=label(oldname); mesh=old.get_component_by_class(unreal.StaticMeshComponent)
    a=spawn(unreal.TripoSchoolBookItem,'School_Book_'+str(index),old.get_actor_location(),old.get_actor_rotation())
    a.mesh.set_static_mesh(mesh.static_mesh)
    for i in range(mesh.get_num_materials()):a.mesh.set_material(i,mesh.get_material(i))
    a.set_actor_transform(mesh.get_world_transform(),False,True)
    a.set_editor_property('puzzle',p); a.set_editor_property('subject',index)
    b=mesh.static_mesh.get_bounding_box()
    a.interaction.set_editor_property('relative_location',(b.min+b.max)*.5)
    a.interaction.set_box_extent((b.max-b.min)*.5+unreal.Vector(5,5,8))
    # Translucent overlay preserves the cover while indicating focus.
    a.interaction.set_editor_property('highlight_material',unreal.load_asset('/Game/Materials/BookDecal/M_BookFocus'))
    old.set_actor_hidden_in_game(True);old.set_actor_enable_collision(False)
    books.append(a)
p.set_editor_property('books',books)
p.set_editor_property('locked_door',label('BP_TheLockedDoor2'))
p.set_editor_property('icons',[unreal.load_asset('/Game/Materials/BookDecal/'+n) for n in ['Chinese_Book','Math_Book','English_Book']])
p.set_editor_property('notes',[unreal.load_asset('/Game/Maps/School/Blueprint/LastRoomGameplay/Sounds/BlackKey'+str(i)) for i in [1,2,3]])
# Short original synthesized classroom bell, no external audio dependency.
wavpath=Path(unreal.Paths.project_dir())/'Content/Audio/SchoolBooksBell.wav'
wavpath.parent.mkdir(parents=True,exist_ok=True)
with wave.open(str(wavpath),'wb') as out:
    out.setnchannels(1);out.setsampwidth(2);out.setframerate(44100)
    samples=[]
    for n in range(66150):
        t=n/44100
        v=sum(math.sin(2*math.pi*freq*t)*gain*math.exp(-decay*t) for freq,gain,decay in [(880,.35,3),(1768,.13,4),(2347,.08,6)])*min(1,t/.004)
        samples.append(struct.pack('<h',int(v*32767)))
    out.writeframes(b''.join(samples))
task=unreal.AssetImportTask();task.filename=str(wavpath);task.destination_path='/Game/Audio';task.automated=True;task.replace_existing=True;task.save=True
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
p.set_editor_property('completion_sound',unreal.load_asset('/Game/Audio/SchoolBooksBell'))
cube=unreal.load_asset('/Engine/BasicShapes/Cube')
wood=label('wooden_desk_3d_model').get_component_by_class(unreal.StaticMeshComponent).get_material(0)
drawer=spawn(unreal.StaticMeshActor,'School_RewardDrawer',unreal.Vector(25615,3565,6275))
drawer.static_mesh_component.set_mobility(unreal.ComponentMobility.MOVABLE)
drawer.static_mesh_component.set_static_mesh(cube);drawer.static_mesh_component.set_material(0,wood)
drawer.set_actor_scale3d(unreal.Vector(.62,.48,.035))
front=spawn(unreal.StaticMeshActor,'School_RewardDrawerFront',unreal.Vector(25615,3588,6278))
front.static_mesh_component.set_mobility(unreal.ComponentMobility.MOVABLE)
front.static_mesh_component.set_static_mesh(cube);front.static_mesh_component.set_material(0,wood)
front.set_actor_scale3d(unreal.Vector(.65,.035,.14))
front.attach_to_actor(drawer,'',unreal.AttachmentRule.KEEP_WORLD,unreal.AttachmentRule.KEEP_WORLD,unreal.AttachmentRule.KEEP_WORLD,False)
for suffix,x in [('Left',25585),('Right',25645)]:
    side=spawn(unreal.StaticMeshActor,'School_Drawer'+suffix,unreal.Vector(x,3565,6279))
    side.static_mesh_component.set_mobility(unreal.ComponentMobility.MOVABLE)
    side.static_mesh_component.set_static_mesh(cube);side.static_mesh_component.set_material(0,wood)
    side.set_actor_scale3d(unreal.Vector(.025,.48,.09))
    side.attach_to_actor(drawer,'',unreal.AttachmentRule.KEEP_WORLD,unreal.AttachmentRule.KEEP_WORLD,unreal.AttachmentRule.KEEP_WORLD,False)
p.set_editor_property('drawer',drawer);p.set_editor_property('drawer_travel',unreal.Vector(0,62,0))
key=spawn(unreal.TripoSchoolBookItem,'School_BookRewardKey',unreal.Vector(25615,3578,6279))
key.mesh.set_static_mesh(unreal.load_asset('/Game/TripoModels/metal_key_3d_model/metal_key_3d_model'))
key.set_editor_property('subject',3);key.set_editor_property('puzzle',p)
bound=key.mesh.static_mesh.get_bounding_box();size=bound.max-bound.min
scale=24/max(size.x,size.y,size.z);key.set_actor_scale3d(unreal.Vector(scale,scale,scale))
key.set_actor_rotation(unreal.Rotator(roll=90,yaw=25),True)
center,extent=key.get_actor_bounds(False)
pos=key.get_actor_location();pos.z+=6278-(center.z-extent.z)
key.set_actor_location(pos,False,True)
key.interaction.set_editor_property('relative_location',(bound.min+bound.max)*.5)
key.interaction.set_box_extent((bound.max-bound.min)*.5+unreal.Vector(5/scale,5/scale,8/scale))
key.interaction.set_editor_property('highlight_material',None)
p.set_editor_property('reward_key',key)
for w,x in [(p.check_chinese,25457.7),(p.check_math,25573.4),(p.check_english,25700.3)]:
    w.set_world_location(unreal.Vector(x,3302.13,6380),False,False)
    w.set_world_rotation(unreal.Rotator(yaw=90),False,False)
p.instruction.set_world_location(unreal.Vector(25475,3302.13,6477),False,False)
p.instruction.set_world_rotation(unreal.Rotator(yaw=90),False,False)
assert unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
print('School manual book puzzle saved')


