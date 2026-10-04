import unreal, json
from pathlib import Path
mesh=unreal.load_asset('/Game/Models/lv4/SM_Bld_Elevator_01')
md=mesh.get_static_mesh_description(0)
rows=[]; parent=list(range(md.get_polygon_count())); seen={}
def find(i):
 while parent[i]!=i:parent[i]=parent[parent[i]];i=parent[i]
 return i
for i in range(md.get_polygon_count()):
 pts=[md.get_vertex_position(v) for v in md.get_polygon_vertices(unreal.PolygonID(i))];rows.append(pts)
 for p in pts:
  key=(round(p.x,2),round(p.y,2),round(p.z,2))
  if key in seen:parent[find(i)]=find(seen[key])
  else:seen[key]=i
groups={}
for i,pts in enumerate(rows):groups.setdefault(find(i),[]).append(i)
report=[]
for g,ids in groups.items():
 pts=[p for i in ids for p in rows[i]]
 report.append(dict(id=g,count=len(ids),minimum=[min(getattr(p,k) for p in pts) for k in 'xyz'],maximum=[max(getattr(p,k) for p in pts) for k in 'xyz']))
Path('C:/Dev/Projects/UE/Tripothon/tools/evaluation/elevator-rework/islands.json').write_text(json.dumps(report,indent=2))
print('islands',len(report))
