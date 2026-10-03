"""Create the LV4 mechanics sandbox once. Never overwrite a designer's existing level."""
import unreal
MAP='/Game/Maps/L_Lv4MechanismWhitebox'
assert not unreal.EditorAssetLibrary.does_asset_exist(MAP), 'Map exists; inspect instead of regenerating'
assert not unreal.EditorLoadingAndSavingUtils.get_dirty_map_packages(), 'Save the current level first'
levels=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
actors=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
assert levels.new_level(MAP)
cube=unreal.load_asset('/Engine/BasicShapes/Cube')
def spawn(cls,name,xyz,yaw=0):
 a=actors.spawn_actor_from_class(cls,unreal.Vector(*xyz),unreal.Rotator(pitch=0,yaw=yaw,roll=0));a.set_actor_label(name);return a
def block(name,xyz,size):
 a=spawn(unreal.StaticMeshActor,name,xyz);a.static_mesh_component.set_static_mesh(cube);a.static_mesh_component.set_collision_profile_name('BlockAll');a.set_actor_scale3d(unreal.Vector(*(v/100 for v in size)));return a
def label(name,xyz,text,yaw=180):
 a=spawn(unreal.TextRenderActor,name,xyz,yaw);c=a.get_component_by_class(unreal.TextRenderComponent);c.set_text(text);c.set_world_size(28);c.set_text_render_color(unreal.Color(20,45,35,255));return a
# West of shaft: a large ground arena. Upper landing joins via a wide ramp for return testing.
block('Ground',(-1280,0,-25),(2240,2800,50))
block('Ground_North',(520,800,-25),(1360,1200,50))
block('Ground_South',(520,-800,-25),(1360,1200,50))
block('Ground_Back',(700,0,-25),(1000,400,50))
block('UpperLanding',(-700,0,575),(1000,1000,50))
block('UpperLeftRail',(-700,-510,680),(1000,20,160))
block('UpperRightRail',(-700,510,680),(1000,20,160))
# Shaft walls: doorway points to -X; cabin floor at X=0.
block('Shaft_Back',(190,0,440),(40,400,920))
block('Shaft_Left',(0,-190,440),(380,40,920))
block('Shaft_Right',(0,190,440),(380,40,920))
block('LandingJamb_L',(-180,-130,440),(40,60,920))
block('LandingJamb_R',(-180,130,440),(40,60,920))
block('LandingLintel_0',(-180,0,430),(40,200,340))
lift=spawn(unreal.TripoElevator,'LV4_Elevator', (0,0,0))
label('ElevatorInstructions',(-270,0,280),'ELEVATOR\nApproach: open / Inside: E\nC: echo | P: pause')
label('UpperInstructions',(-300,0,880),'UPPER STOP\nInside cabin: E to return')
label('CabinButtonHint',(98,75,145),'[ E ]',180).attach_to_component(lift.cabin,'',unreal.AttachmentRule.KEEP_WORLD,unreal.AttachmentRule.KEEP_WORLD,unreal.AttachmentRule.KEEP_WORLD,False)
spawn(unreal.PlayerStart,'LV4_Start',(-600,0,100))
safe=spawn(unreal.TripoZone,'Checkpoint_Start',(-600,0,110))
safe.set_editor_property('kind',unreal.TripoZoneKind.SAFE)
safe.volume.set_box_extent(unreal.Vector(200,220,140))
cp=spawn(unreal.TripoZone,'Checkpoint_Recovery',(-600,0,110));cp.set_editor_property('kind',unreal.TripoZoneKind.CHECKPOINT);cp.set_editor_property('safe_offset',unreal.Vector(0,0,-18));cp.set_editor_property('hint','CHECKPOINT');cp.volume.set_box_extent(unreal.Vector(150,150,130))
light=spawn(unreal.PointLight,'CabinLight',(0,0,240));light.point_light_component.set_mobility(unreal.ComponentMobility.MOVABLE);light.point_light_component.set_editor_property('intensity',35);light.point_light_component.set_editor_property('attenuation_radius',400);light.point_light_component.set_editor_property('cast_shadows',False);light.attach_to_component(lift.cabin,'',unreal.AttachmentRule.KEEP_WORLD,unreal.AttachmentRule.KEEP_WORLD,unreal.AttachmentRule.KEEP_WORLD,False)
# Patrol circuit south of the elevator; hiding pockets on either side.
route=[(-1900,-800,90),(-800,-800,90),(600,-800,90)]
points=[spawn(unreal.TargetPoint,'Patrol_'+str(i),p) for i,p in enumerate(route)]
npc=spawn(unreal.TripoChaser,'LV4_Patrol',route[0]);npc.set_editor_property('patrol_points',points);npc.set_editor_property('start_patrolling',True);npc.set_editor_property('sight_radius',900)
for i,(x,y) in enumerate([(-1300,-1170),(500,-1170)]):
 z=spawn(unreal.TripoChaseHideZone,'Hide_'+str(i),(x,y,120));z.volume.set_box_extent(unreal.Vector(180,150,140));z.label.set_relative_rotation(unreal.Rotator(pitch=0,yaw=180,roll=0),False,True)
 m=block('HideMarker_'+str(i),(x,y,2),(350,290,4));mat=unreal.load_asset('/Game/Materials/Whitebox/M_ChaseHide');m.static_mesh_component.set_material(0,mat)
 block('HideBack_'+str(i),(x,y-160,160),(400,20,320))
 block('HideLeft_'+str(i),(x-190,y,160),(20,300,320))
 block('HideRight_'+str(i),(x+190,y,160),(20,300,320))
block('SightOccluder',(-700,-430,150),(300,35,300))
label('PatrolInstructions',(-900,-280,240),'PATROL AREA\nFront: chase / Behind: sneak\nGreen rooms: SAFE, aggro -45/s')
nav=spawn(unreal.NavMeshBoundsVolume,'LV4_Navigation',(-600,0,300));nav.set_actor_scale3d(unreal.Vector(20,16,6))
builder=spawn(unreal.TripoChaseTrigger,'NavigationBuilder',(0,0,-1000));builder.set_editor_property('enabled',False)
for name,yaw,power in [('Sun',-35,4),('Fill',150,1.5)]:
 a=spawn(unreal.DirectionalLight,name,(0,0,1200),yaw);a.set_actor_rotation(unreal.Rotator(pitch=-55,yaw=yaw,roll=0),False);a.light_component.set_editor_property('intensity',power);a.light_component.set_editor_property('forward_shading_priority',1 if name=='Sun' else 0);a.light_component.set_editor_property('cast_shadows',name=='Sun')
spawn(unreal.SkyLight,'SkyLight',(-600,0,1200))
# Reusable BP classes for designers; native defaults preserve legacy chase behavior.
for name,cls in [('BP_Lv4Elevator',unreal.TripoElevator),('BP_Lv4Patrol',unreal.TripoChaser),('BP_Lv4HideZone',unreal.TripoChaseHideZone)]:
 path='/Game/Blueprint/Lv4/'+name
 if not unreal.EditorAssetLibrary.does_asset_exist(path):
  factory=unreal.BlueprintFactory();factory.set_editor_property('parent_class',cls)
  bp=unreal.AssetToolsHelpers.get_asset_tools().create_asset(name,'/Game/Blueprint/Lv4',unreal.Blueprint,factory)
  unreal.EditorAssetLibrary.save_loaded_asset(bp,False)
builder.build_navigation_for_level()
assert levels.save_current_level()
print('CREATED',MAP)
