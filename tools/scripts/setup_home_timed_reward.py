import unreal
from pathlib import Path
es=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
aa=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
assert es.get_editor_world().get_name()=='L_Home_Vertical_02'
by={a.get_actor_label():a for a in aa.get_all_level_actors()}
old=by['Finish_Home.Route1']
for a in list(aa.get_all_level_actors()):
    if a.get_actor_label() in ['Finish_Home.Route1_Gifts','Home_Route1_Reward_1','Home_Route1_Reward_2']: aa.destroy_actor(a)
finish=aa.spawn_actor_from_class(unreal.TripoGiftFinish,old.get_actor_location())
finish.set_actor_label('Finish_Home.Route1_Gifts'); finish.set_folder_path('Gameplay/TimedReward')
finish.set_editor_property('challenge_id','Home.Route1'); finish.set_editor_property('hint','限时奖励检查点')
finish.volume.set_box_extent(old.volume.get_scaled_box_extent())
boxes=[]
# Place inside the dry kitchen landing, clear of the doorway and fridge.
for i,y in enumerate([300,410]):
    start=unreal.Vector(4620,y,3430); end=unreal.Vector(4620,y,3000)
    hit=unreal.SystemLibrary.line_trace_single(es.get_editor_world(),start,end,unreal.TraceTypeQuery.TRACE_TYPE_QUERY1,True,[],unreal.DrawDebugTrace.NONE,True).to_tuple()
    assert hit[0], 'Missing floor'
    print('FLOOR',hit[5],hit[9].get_actor_label())
    g=aa.spawn_actor_from_class(unreal.TripoGiftBox,hit[5]+unreal.Vector(0,0,.5))
    g.set_actor_label('Home_Route1_Reward_'+str(i+1)); g.set_folder_path('Gameplay/TimedReward')
    g.set_editor_property('gift_id','Home.Route1.Reward.'+str(i+1))
    for c,path in [(g.box_mesh,'/Game/Blueprint/Gift/SM_HomeGiftBody'),(g.lid_mesh,'/Game/Blueprint/Gift/SM_HomeGiftLid')]:
        c.set_static_mesh(unreal.load_asset(path)); c.set_relative_location(unreal.Vector(),False,True); c.set_relative_scale3d(unreal.Vector(1,1,1))
    g.prompt.set_hidden_in_game(True)
    f=g.get_component_by_class(unreal.TripoInteractionTarget); f.set_relative_location(unreal.Vector(0,0,25),False,True); f.set_box_extent(unreal.Vector(25,26,25))
    g.set_editor_property('bEnabled',False); g.set_actor_hidden_in_game(True); g.set_actor_enable_collision(False)
    boxes.append(g)
finish.set_editor_property('gift_boxes',boxes)
assert aa.destroy_actor(old)
assert unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
print('READY',[(g.get_actor_label(),g.get_actor_location()) for g in boxes])


