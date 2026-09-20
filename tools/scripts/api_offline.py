"""Evaluate the API MCP against the exact locally indexed engine; no update/download."""
from pathlib import Path
from unreal_api_mcp import db, server

path = Path(__file__).resolve().parents[1] / 'evaluation/reports/unreal_docs_5.7.db'
if not path.is_file():
    raise SystemExit('Build the local UE 5.7 index before starting the offline API MCP.')
server._conn = db.get_connection(path)
server.main()
