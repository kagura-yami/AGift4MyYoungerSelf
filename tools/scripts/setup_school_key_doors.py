"""Keep the authored door timeline, replacing proximity opening with E + a key."""
import asyncio,json
from datetime import timedelta
from mcp import ClientSession
from mcp.client.streamable_http import streamablehttp_client
BP='/Game/Maps/School/Blueprint/BP_TheLockedDoor'
async def main():
 async with streamablehttp_client('http://127.0.0.1:18777/mcp',timeout=timedelta(seconds=120)) as streams:
  async with ClientSession(streams[0],streams[1]) as session:
   await session.initialize()
   async def run(command):
    r=await session.call_tool('unreal',dict(action='execute',run='sync',transaction=False,commands=[command]))
    d=json.loads(next(c.text for c in r.content if c.type=='text'));assert d['ok'],d
    return d['data']['results'][0]
   async def graph(op,**kw):return (await run(dict(kind='blueprint_graph',operation=op,blueprint_path=BP,**kw)))['value']
   for n in (await graph('inspect'))['nodes']:
    if n['class'].endswith('K2Node_ComponentBoundEvent'):
     await graph('disconnect',from_node_id=n['id'],from_pin_name='then',from_pin_direction='output')
   await graph('compile')
   await run(dict(kind='python',mode='exec',code="""
import unreal
bp=unreal.load_asset('/Game/Maps/School/Blueprint/BP_TheLockedDoor')
s=unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem);f=unreal.SubobjectDataBlueprintFunctionLibrary
objects=[(h,f.get_object_for_blueprint(f.get_data(h),bp)) for h in s.k2_gather_subobject_data_for_blueprint(bp)]
door_handle,door=next((h,o) for h,o in objects if isinstance(o,unreal.StaticMeshComponent) and o.get_name().startswith('rusted_green_door'))
focus=next((o for h,o in objects if isinstance(o,unreal.TripoInteractionTarget)),None)
if not focus:
 h,reason=s.add_new_subobject(unreal.AddNewSubobjectParams(parent_handle=door_handle,new_class=unreal.TripoInteractionTarget,blueprint_context=bp));assert not str(reason),reason
 s.rename_subobject(h,'ManualKeyDoorInteraction');focus=f.get_object_for_blueprint(f.get_data(h),bp)
focus.set_editor_property('bToggleDoorTimeline',True);focus.set_editor_property('bUseSchoolKey',True)
focus.set_editor_property('single_use',False);focus.set_editor_property('prompt','需要对应钥匙');focus.set_editor_property('highlight_mesh',door)
focus.set_editor_property('reach',300)
b=door.static_mesh.get_bounding_box();focus.set_editor_property('relative_location',(b.min+b.max)*.5);focus.set_box_extent((b.max-b.min)*.5+unreal.Vector(3,3,3))
for h,o in objects:
 if isinstance(o,unreal.TextRenderComponent):o.set_hidden_in_game(True)
 if isinstance(o,unreal.BoxComponent) and not isinstance(o,unreal.TripoInteractionTarget):o.set_editor_property('generate_overlap_events',False)
unreal.BlueprintEditorLibrary.compile_blueprint(bp)
"""))
   print(await graph('compile',save=True))
asyncio.run(main())

