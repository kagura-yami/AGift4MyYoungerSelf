import bpy, json
from collections import defaultdict
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.fbx(filepath='C:/Dev/Projects/UE/Tripothon/game/Content/Models/juese/SK_MiniCharacter_Son_01.FBX')
groups=defaultdict(list)
for obj in bpy.data.objects:
 if obj.type!='MESH':continue
 mesh=obj.data
 for poly in mesh.polygons:
  uv=mesh.uv_layers.active.data[poly.loop_start].uv
  key=(round(uv.x,3),round(uv.y,3))
  groups[key].extend([list(mesh.vertices[v].co) for v in poly.vertices])
rows=[]
for uv,pts in groups.items():
 rows.append(dict(uv=uv,n=len(pts),min=[min(p[i] for p in pts) for i in range(3)],max=[max(p[i] for p in pts) for i in range(3)]))
open('C:/Dev/Projects/UE/Tripothon/tools/evaluation/son-uv.json','w').write(json.dumps(rows,indent=2))
