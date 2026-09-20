"""Exercise actual MCP transports and preserve full responses as evidence."""
import argparse
import asyncio
import base64
import json
import os
from datetime import timedelta
from pathlib import Path
from mcp import ClientSession, StdioServerParameters
from mcp.client.stdio import stdio_client
from mcp.client.streamable_http import streamablehttp_client

ROOT=Path(__file__).resolve().parents[2]
EVAL=ROOT/'tools/evaluation'

async def run(args):
    env=os.environ.copy()
    env.update({'MCP_UNREAL_PROJECT':str(EVAL/'Probe/Probe.uproject'), 'UNREAL_VERSION':'5.7', 'UE_PROJECT_PATH':str(EVAL/'Probe/Probe.uproject'), 'UE_EDITOR_PATH':r'C:\Apps\UE\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe'})
    python=str(EVAL/'venv/Scripts/python.exe')
    configs={
      'remi':(str(EVAL/'bin/mcp-unreal.exe'),[]),
      'ronildo':(python,[str(EVAL/'repos/ronildo/server/unreal_mcp_server.py')]),
      'api':(str(EVAL/'venv/Scripts/unreal-api-mcp.exe'),[]),
      'api-offline':(python,[str(ROOT/'tools/scripts/api_offline.py')]),
      'chir':('node',[str(EVAL/'repos/chir/dist/cli.js')]),
    }
    async def session_work(read,write):
        async with ClientSession(read,write,read_timeout_seconds=timedelta(seconds=args.timeout)) as session:
            await session.initialize()
            if args.call:
                data=json.loads(Path(args.args_file).read_text(encoding='utf-8-sig')) if args.args_file else json.loads(args.args)
                result=await session.call_tool(args.call,data)
            else:
                result=await session.list_tools()
            payload=result.model_dump(mode='json',exclude_none=True)
            out=EVAL/'reports'/f'{args.label}.json'
            out.write_text(json.dumps(payload,ensure_ascii=False,indent=2),encoding='utf-8')
            if args.call:
                for i,item in enumerate(payload.get('content',[])):
                    if item.get('type')=='image':
                        path=out.with_name(out.stem+f'-{i}.png')
                        path.write_bytes(base64.b64decode(item['data']))
                        print('IMAGE',path)
                    elif item.get('type')=='text': print(item['text'][:args.print_limit])
                print('isError:',payload.get('isError',False))
            else:
                print(json.dumps([{'name':t['name'],'inputSchema':t['inputSchema']} for t in payload['tools']],indent=2)[:args.print_limit])
            print('REPORT',out)
            await asyncio.sleep(0.5)
    if args.server=='avatar':
        async with streamablehttp_client('http://127.0.0.1:18777/mcp',timeout=timedelta(seconds=args.timeout)) as streams:
            await session_work(streams[0],streams[1])
    else:
        command,argv=configs[args.server]
        with (EVAL/'reports'/f'{args.label}.stderr.log').open('w',encoding='utf-8') as errors:
            async with stdio_client(StdioServerParameters(command=command,args=argv,env=env,cwd=str(EVAL)),errlog=errors) as streams:
                await session_work(*streams)

if __name__=='__main__':
    p=argparse.ArgumentParser()
    p.add_argument('--server',required=True,choices=['remi','avatar','ronildo','api','api-offline','chir'])
    p.add_argument('--call'); p.add_argument('--args',default='{}'); p.add_argument('--args-file')
    p.add_argument('--label',required=True); p.add_argument('--timeout',type=int,default=90)
    p.add_argument('--print-limit',type=int,default=5000)
    asyncio.run(run(p.parse_args()))
