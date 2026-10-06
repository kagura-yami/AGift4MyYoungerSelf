"""Run with tools/.venv Python while the editor is outside PIE."""
import asyncio
import json
from datetime import timedelta
from mcp import ClientSession
from mcp.client.streamable_http import streamablehttp_client

BP = '/Game/Blueprint/Door/BP_CanOpenDoor'

async def main():
    async with streamablehttp_client('http://127.0.0.1:18777/mcp', timeout=timedelta(seconds=120)) as streams:
        async with ClientSession(streams[0], streams[1]) as session:
            await session.initialize()
            async def run(command):
                result = await session.call_tool('unreal', dict(action='execute', run='sync', commands=[command]))
                data = json.loads(next(c.text for c in result.content if c.type == 'text'))
                assert data['ok'], data
                return data['data']['results'][0]
            async def graph(op, **kw):
                return (await run(dict(kind='blueprint_graph', operation=op, blueprint_path=BP, **kw)))['value']
            nodes = (await graph('inspect'))['nodes']
            overlap = next(n for n in nodes if n['class'].endswith('K2Node_ComponentBoundEvent'))
            do_once = next(n for n in nodes if n['title'] == 'Do Once')
            event = await graph('add_custom_event', event_name='ManualOpenDoor', x=-600, y=500)
            # Look up the new node through inspect so the script is safe to rerun.
            nodes = (await graph('inspect'))['nodes']
            event = next(n for n in nodes if n['class'].endswith('K2Node_CustomEvent') and 'ManualOpenDoor' in n['title'])
            await graph('disconnect', from_node_id=overlap['id'], from_pin_name='then', from_pin_direction='output')
            await graph('connect', from_node_id=event['id'], from_pin_name='then', from_pin_direction='output', to_node_id=do_once['id'], to_pin_name='execute', to_pin_direction='input')
            await graph('compile')
            await run(dict(kind='python', mode='exec', code="""
import unreal
bp=unreal.load_asset('/Game/Blueprint/Door/BP_CanOpenDoor')
s=unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
f=unreal.SubobjectDataBlueprintFunctionLibrary
handles=s.k2_gather_subobject_data_for_blueprint(bp)
objects=[(h,f.get_object_for_blueprint(f.get_data(h),bp)) for h in handles]
door_handle,door=next((h,o) for h,o in objects if isinstance(o,unreal.StaticMeshComponent) and o.get_name().startswith('rusted_green_door'))
focus=next((o for h,o in objects if isinstance(o,unreal.TripoInteractionTarget)),None)
if focus is None:
    handle,reason=s.add_new_subobject(unreal.AddNewSubobjectParams(parent_handle=door_handle,new_class=unreal.TripoInteractionTarget,blueprint_context=bp))
    assert not str(reason), reason
    s.rename_subobject(handle,'ManualDoorInteraction')
    focus=f.get_object_for_blueprint(f.get_data(handle),bp)
focus.set_editor_property('interaction_event','ManualOpenDoor')
focus.set_editor_property('single_use',False)
focus.set_editor_property('bToggleDoorTimeline',True)
focus.set_editor_property('prompt','开门')
focus.set_editor_property('highlight_mesh',door)
bounds=door.static_mesh.get_bounding_box()
centre=(bounds.min+bounds.max)*.5
extent=(bounds.max-bounds.min)*.5
focus.set_editor_property('relative_location',centre)
focus.set_box_extent(extent+unreal.Vector(2,2,2))
unreal.BlueprintEditorLibrary.compile_blueprint(bp)
"""))
            print(await graph('compile', save=True))

asyncio.run(main())
