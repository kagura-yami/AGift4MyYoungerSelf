"""Build a configurable continuous graybox, not a claim of final art/original-map restoration."""
import unreal
from tripo_story_data import EVENTS


def build():
    editor = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    world = unreal.EditorLoadingAndSavingUtils.new_blank_map(False)
    cube = unreal.load_asset('/Engine/BasicShapes/Cube.Cube')

    def spawn(cls, label, xyz, scale=None):
        a = editor.spawn_actor_from_class(cls, unreal.Vector(*xyz))
        a.set_actor_label(label)
        if scale: a.set_actor_scale3d(unreal.Vector(*scale))
        return a

    def block(label, xyz, scale, tags=None):
        a = spawn(unreal.StaticMeshActor, label, xyz, scale)
        a.static_mesh_component.set_static_mesh(cube)
        if tags: a.set_editor_property('tags', tags)
        return a

    def asset(name, cls):
        obj = unreal.load_asset('/Game/Data/'+name)
        if obj: return obj
        factory = unreal.DataAssetFactory()
        factory.set_editor_property('data_asset_class', cls)
        return unreal.AssetToolsHelpers.get_asset_tools().create_asset(name,'/Game/Data',cls,factory)

    def zone(label, kind, x, hint, extent=(90,420,100)):
        a = spawn(unreal.TripoZone,label,(x,0,0))
        a.set_editor_property('kind',kind)
        a.set_editor_property('hint',hint)
        a.volume.set_box_extent(unreal.Vector(*extent))
        return a

    catalog = asset('DA_Story',unreal.TripoStoryCatalog)
    rows, floors, locations, challenges = [], [0]*8, {}, []
    cursor = 500
    previous = None
    chapter_challenges = {}
    challenge_after = {'Story.Home.TimerBrief':'Home','Story.School.GrantSlow':'School',
                       'Story.Minecraft.RouteBrief':'Minecraft','Story.Office.FinalRouteBrief':'Office'}
    exit_events = {'Story.Home.Exit':'Home','Story.School.ScreenPortal':'School',
                   'Story.Minecraft.Exit':'Minecraft','Story.Office.Exit':'Office'}

    for event_id,title,text,grant in EVENTS:
        if grant >= 0: floors[grant] = max(floors[grant],1)
        requirements = chapter_challenges.get(exit_events.get(event_id,''),[])
        row = unreal.TripoStoryEvent(id=event_id,title=title,text=text,grant_floors=list(floors),
                                    required_events=[previous] if previous else [],required_challenges=requirements)
        rows.append(row)
        a = spawn(unreal.TripoStoryTrigger,event_id,(cursor,0,0))
        a.set_editor_property('catalog',catalog); a.set_editor_property('event_id',event_id)
        a.set_editor_property('hint','E / '+title)
        locations[event_id] = cursor
        zone('Safe_'+event_id,unreal.TripoZoneKind.SAFE,cursor,'SAFE / SAVE')
        zone('Checkpoint_'+event_id,unreal.TripoZoneKind.CHECKPOINT,cursor-100,'CHECKPOINT')
        if event_id == 'Story.Minecraft.GrantStepStone':
            area = zone('StonePractice',unreal.TripoZoneKind.BUILD_ALLOWED,cursor+220,'Q / BUILD PRACTICE',(350,420,800))
        if event_id == 'Story.Minecraft.ExchangeIntro':
            npc=zone('Postman',unreal.TripoZoneKind.NPC,cursor,'E / EXCHANGE')
            npc.set_editor_property('protected_levels',list(floors))
        if event_id == 'Story.School.GrantWallJump':
            block('MarkedWall',(cursor+220,200,230),(.3,3,4.6),['TripoWallJump'])
        if event_id in ('Story.School.GrantSlow','Story.Office.GrantRewind'):
            p=spawn(unreal.TripoMechanism,'TimePlatform_'+event_id,(cursor+170,220,80),(2.5,2.5,.3))
            p.set_editor_property('kind',unreal.TripoMechanismKind.PLATFORM)
            p.set_editor_property('travel',unreal.Vector(0,0,250))
        if event_id == 'Story.Minecraft.GrantEcho':
            plate=spawn(unreal.TripoMechanism,'EchoPlate',(cursor+140,-220,15),(2,2,.2))
            plate.set_editor_property('allow_echo',True)
            door=spawn(unreal.TripoMechanism,'EchoPracticeDoor',(cursor+260,-220,130),(.25,2,2.6))
            door.set_editor_property('kind',unreal.TripoMechanismKind.GATE); door.set_editor_property('inputs',[plate])
        if event_id in exit_events:
            gate=spawn(unreal.TripoMechanism,'ChapterExit_'+exit_events[event_id],(cursor+160,0,300),(.3,10,6))
            gate.set_editor_property('kind',unreal.TripoMechanismKind.GATE)
            gate.set_editor_property('required_events',[event_id]); gate.set_editor_property('required_challenges',requirements)
        previous=event_id
        cursor += 450
        if event_id in challenge_after:
            chapter = challenge_after[event_id]
            chapter_challenges[chapter] = []
            for n in (1,2):
                cid = chapter+'.Route'+str(n)
                d = asset('DA_'+chapter+str(n),unreal.TripoChallengeDefinition)
                d.set_editor_property('challenge_id',cid); d.set_editor_property('budget',18.+n*4)
                d.set_editor_property('required_levels',list(floors))
                enum_values=[unreal.TripoAbility.DASH,unreal.TripoAbility.UP_DASH,unreal.TripoAbility.WALL_JUMP,
                             unreal.TripoAbility.STEP_STONE,unreal.TripoAbility.SLOW,unreal.TripoAbility.REWIND,
                             unreal.TripoAbility.ECHO,unreal.TripoAbility.BONUS_TIME]
                d.set_editor_property('rewards',[unreal.TripoRewardOption(ability=enum_values[i],weight=1.) for i in range(8) if floors[i]>0])
                unreal.EditorAssetLibrary.save_loaded_asset(d)
                start=zone('Start_'+cid,unreal.TripoZoneKind.START,cursor,'START / '+cid)
                start.set_editor_property('challenge',d)
                for offset in (400,900):
                    block('Jump_'+cid+'_'+str(offset),(cursor+offset,0,38),(.65,8,.76))
                    danger=zone('Hazard_'+cid+'_'+str(offset),unreal.TripoZoneKind.HAZARD,cursor+offset,'JUMP',(38,400,35))
                finish=zone('Finish_'+cid,unreal.TripoZoneKind.FINISH,cursor+1450,'FINISH / '+cid)
                finish.set_editor_property('challenge_id',cid)
                zone('SafeFinish_'+cid,unreal.TripoZoneKind.SAFE,cursor+1450,'SAFE')
                chapter_challenges[chapter].append(cid); challenges.append(cid)
                cursor += 1850

    catalog.set_editor_property('events',rows)
    unreal.EditorAssetLibrary.save_loaded_asset(catalog)
    block('TutorialFloor',(cursor/2,0,-30),((cursor+1000)/100,10,.5))
    spawn(unreal.PlayerStart,'TutorialStart',(0,0,100))
    # These are clearly replaceable layout markers, with no collision or gameplay permission.
    origin=locations['Story.Minecraft.Enter']
    for n,kind in enumerate([unreal.TripoMemoryKind.SHARED_BASE,unreal.TripoMemoryKind.SHARED_ROUTE,unreal.TripoMemoryKind.MESSAGE_AND_PHOTO]):
        marker=spawn(unreal.TripoMemoryLandmark,'MemoryReference_'+str(n),(origin+n*300,380,70))
        marker.set_editor_property('kind',kind)
        marker.set_editor_property('reference',['共同据点 · 原图参考待提供','共同路线 · 原图参考待提供','留言/合影 · 原始资料待提供'][n])
    light=spawn(unreal.DirectionalLight,'Sun',(0,0,1500))
    light.set_actor_rotation(unreal.Rotator(pitch=-50,yaw=-30,roll=0),False)
    light.light_component.set_mobility(unreal.ComponentMobility.MOVABLE)
    light.light_component.set_editor_property('intensity',4.)
    sky=spawn(unreal.SkyLight,'Sky',(0,0,800)); sky.light_component.set_mobility(unreal.ComponentMobility.MOVABLE)
    assert unreal.EditorLoadingAndSavingUtils.save_map(world,'/Game/Maps/L_Tutorial')
    return {'map':world.get_path_name(),'events':len(rows),'challenges':challenges,'route_length_cm':cursor,'locations':locations}
