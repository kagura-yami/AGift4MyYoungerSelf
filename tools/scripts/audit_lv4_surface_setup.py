"""Read-only inventory of visible geometry for material/normal diagnostics."""
import unreal,json
from pathlib import Path
rows=[];duplicates={}
for a in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors():
 for c in a.get_components_by_class(unreal.MeshComponent):
  if not c.is_visible():continue
  mesh=c.static_mesh if isinstance(c,unreal.StaticMeshComponent) else (c.skeletal_mesh_asset if isinstance(c,unreal.SkeletalMeshComponent) else None)
  if not mesh:continue
  materials=[]
  for i in range(c.get_num_materials()):
   m=c.get_material(i)
   materials.append(m.get_path_name() if m else None)
  scale=c.get_world_scale()
  pos=c.get_world_location()
  row=dict(actor=a.get_actor_label(),component=c.get_name(),asset=mesh.get_path_name(),location=[pos.x,pos.y,pos.z],scale=[scale.x,scale.y,scale.z],mirrored=scale.x*scale.y*scale.z<0,materials=materials)
  rows.append(row)
out=Path('C:/Dev/Projects/UE/Tripothon/tools/evaluation/surface-audit');out.mkdir(exist_ok=True)
(out/'inventory.json').write_text(json.dumps(rows,ensure_ascii=False,indent=2),encoding='utf-8')
print('Visible components:',len(rows),'Unique meshes:',len(set(r['asset'] for r in rows)),'Mirrored:',sum(r['mirrored'] for r in rows),'Missing materials:',sum(None in r['materials'] for r in rows))
