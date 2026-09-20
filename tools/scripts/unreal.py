"""Native MCP client fallback when Codex has not reloaded project MCP configuration."""
import argparse
import asyncio
import json
from pathlib import Path
from datetime import timedelta
from mcp import ClientSession
from mcp.client.streamable_http import streamablehttp_client


async def run(args):
    if args.request:
        request = json.loads(Path(args.request).read_text(encoding='utf-8-sig'))
    else:
        code = Path(args.file).read_text(encoding='utf-8-sig') if args.file else args.code
        request = {'action':'execute','run':'sync','transaction':False,
                   'commands':[{'kind':'python','mode':'eval' if args.eval else 'exec','code':code}]}
    async with streamablehttp_client(args.url, timeout=timedelta(seconds=120)) as streams:
        async with ClientSession(streams[0],streams[1]) as session:
            await session.initialize()
            response = await session.call_tool('unreal',request)
            payload = response.model_dump(mode='json',exclude_none=True)
            if args.report:
                path = Path(args.report)
                path.parent.mkdir(parents=True,exist_ok=True)
                path.write_text(json.dumps(payload,ensure_ascii=False,indent=2),encoding='utf-8')
            failed = bool(response.isError)
            for item in response.content:
                if item.type == 'text':
                    print(item.text)
                    try:
                        data = json.loads(item.text)
                        failed = failed or data.get('ok') is False or data.get('success') is False
                    except json.JSONDecodeError:
                        pass
            if failed != args.expect_error:
                return 1
            return 0


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    source = parser.add_mutually_exclusive_group(required=True)
    source.add_argument('--code'); source.add_argument('--file'); source.add_argument('--request')
    parser.add_argument('--eval',action='store_true')
    parser.add_argument('--expect-error',action='store_true')
    parser.add_argument('--report')
    parser.add_argument('--url',default='http://127.0.0.1:18777/mcp')
    raise SystemExit(asyncio.run(run(parser.parse_args())))
