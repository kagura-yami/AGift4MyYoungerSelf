import bpy, math, sys, json, bmesh
from mathutils import Vector, Quaternion, Matrix
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
OUT=ROOT/'tools/artifacts/gift-film';OUT.mkdir(parents=True,exist_ok=True)
bpy.ops.object.select_all(action='SELECT');bpy.ops.object.delete(use_global=False)
s=bpy.context.scene;s.render.engine='BLENDER_WORKBENCH';s.render.resolution_x=1280;s.render.resolution_y=720;s.render.resolution_percentage=100
s.render.fps=24;s.frame_start=1;s.frame_end=672
s.render.image_settings.file_format='PNG';s.render.film_transparent=False
s.display.shading.light='STUDIO';s.display.shading.studiolight_rotate_z=.4
s.display.shading.color_type='MATERIAL';s.display.shading.show_shadows=True;s.display.shading.show_cavity=True
s.display.shading.cavity_type='BOTH';s.display.shading.curvature_ridge_factor=1.3;s.display.shading.show_specular_highlight=True
s.display.shading.background_type='WORLD';s.world.color=(.055,.09,.10)
s.view_settings.view_transform='Standard'
def mat(name,c):
 m=bpy.data.materials.new(name);m.diffuse_color=(*c,1);return m
cream=mat('warm paper',(.82,.76,.61));teal=mat('midnight teal',(.045,.15,.18));gold=mat('gift gold',(.95,.48,.14));pink=mat('ribbon',(.92,.29,.32));ink=mat('ink',(.025,.052,.075));white=mat('screen letters',(.9,.94,.85));green=mat('route mint',(.22,.66,.51))
def cube(n,loc,scale,m,bevel=.04):
 bpy.ops.mesh.primitive_cube_add(size=1,location=loc);o=bpy.context.object;o.name=n;o.scale=scale;bpy.ops.object.transform_apply(location=False,rotation=False,scale=True);o.data.materials.append(m)
 if bevel:mod=o.modifiers.new('soft edges','BEVEL');mod.width=bevel;mod.segments=2;o.modifiers.new('normals','WEIGHTED_NORMAL')
 return o
def key(o,f,loc=None,rot=None,scale=None):
 if loc is not None:o.location=loc;o.keyframe_insert('location',frame=f)
 if rot is not None:o.rotation_euler=rot;o.keyframe_insert('rotation_euler',frame=f)
 if scale is not None:o.scale=scale;o.keyframe_insert('scale',frame=f)
font=bpy.data.fonts.load('C:/Windows/Fonts/msyh.ttc')
def text(n,body,loc,size,m,rot=(math.pi/2,0,0)):
 c=bpy.data.curves.new(n,'FONT');c.body=body;c.font=font;c.size=size;c.align_x='CENTER';c.extrude=.001
 o=bpy.data.objects.new(n,c);s.collection.objects.link(o);o.location=loc;o.rotation_euler=rot;c.materials.append(m);return o
cube('stage',(0,0,-.12),(22,18,.2),cream)
cube('backdrop',(0,5,3),(22,.2,6),teal)
cube('work desk',(-3,0,1.2),(3,1.3,.12),teal)
for x in [-4.2,-1.8]:cube('desk leg',(x,0,.6),(.12,.8,1.2),ink)
cube('monitor',(-3,.32,2.05),(1.9,.16,1.1),ink)
cube('screen',(-3,.225,2.05),(1.7,.025,.91),teal)
cube('monitor stand',(-3,.32,1.47),(.15,.2,.4),ink)
cube('keyboard',(-3,-.38,1.29),(1.25,.35,.05),cream)
for i in range(11):cube('keys',(-3.55+i*.11,-.4,1.325),(.075,.17,.02),ink,.005)
text('screen title','A GIFT FOR',(-3,.19,2.28),.14,white)
text('screen second line','MY YOUNGER SELF',(-3,.19,2.04),.09,white)
text('screen build','BUILD  01  /  PLAY',(-3,.19,1.81),.065,green)
cube('notebook',(-4,-.2,1.3),(.4,.5,.03),gold)
text('wall caption','MAKE SOMETHING WORTH GIVING',(-3,4.85,3.4),.26,cream)
palette=[(.594,.747,(.93,.71,.55)),(.594,.705,(.83,.56,.43)),(.631,.753,(.3,.19,.14)),(.631,.729,(.97,.92,.82)),(.260,.360,(.33,.21,.15)),(.245,.864,(.84,.57,.19)),(.602,.322,(.19,.28,.36)),(.162,.063,(.93,.85,.71)),(.237,.283,(.29,.21,.16)),(.257,.110,(.92,.85,.72)),(.199,.022,(.15,.12,.11)),(.811,.561,(.96,.81,.43)),(.729,.561,(.93,.72,.24))]
def character(name,loc,scale,adult=False):
 before=set(bpy.data.objects);bpy.ops.import_scene.fbx(filepath=str(ROOT/'game/Content/Models/juese/SK_MiniCharacter_Son_01.FBX'))
 imported=set(bpy.data.objects)-before
 bpy.ops.object.empty_add();root=bpy.context.object;root.name=name
 for o in imported:
  if o.parent not in imported:o.parent=root
 arm=next(o for o in imported if o.type=='ARMATURE');mesh=next(o for o in imported if o.type=='MESH')
 mats=[mat(name+str(i),(.14,.33,.38) if adult and i==5 else col) for i,(_,_,col) in enumerate(palette)]
 mesh.data.materials.clear()
 for m in mats:mesh.data.materials.append(m)
 uv=mesh.data.uv_layers.active.data
 for poly in mesh.data.polygons:
  v=sum((uv[j].uv for j in poly.loop_indices),Vector((0,0)))/len(poly.loop_indices)
  poly.material_index=min(range(len(palette)),key=lambda i:(v.x-palette[i][0])**2+(v.y-palette[i][1])**2)
 root.location=loc;root.scale=(scale,)*3
 arm.animation_data_clear()
 for b in arm.pose.bones:
  b.location=(0,0,0);b.rotation_mode='QUATERNION';b.rotation_quaternion=(1,0,0,0);b.scale=(1,1,1)
 bpy.context.view_layer.update()
 # Simple hand-keyed blocking on the imported skin, preserving its rig.
 for side,sign in [('l',1),('r',-1)]:
  bone=arm.pose.bones['upperarm_'+side];bone.rotation_mode='QUATERNION'
  shoulder=arm.matrix_world @ bone.bone.head_local
  direction=(arm.matrix_world.to_3x3() @ (bone.bone.tail_local-bone.bone.head_local)).normalized()
  target=Vector((sign*.1,0,-1)).normalized()
  q=direction.rotation_difference(target)
  for f in [1,72,144,240,336,420,504,576,672]:
   s.frame_set(f)
   delta=Matrix.Translation(shoulder) @ (Quaternion(Vector((1,0,0)),.055*math.sin(f*.04)) @ q).to_matrix().to_4x4() @ Matrix.Translation(-shoulder)
   for b in [bone]+list(bone.children_recursive):
    b.rotation_mode='QUATERNION'
    b.matrix=arm.matrix_world.inverted() @ delta @ arm.matrix_world @ b.bone.matrix_local
    b.keyframe_insert('rotation_quaternion',frame=f);b.keyframe_insert('location',frame=f)
    bpy.context.view_layer.update()
 head=arm.pose.bones['head'];head.rotation_mode='XYZ'
 for f,z in [(1,0),(168,.08),(336,-.08),(456,.10),(576,-.04),(672,0)]:head.rotation_euler.z=z;head.keyframe_insert('rotation_euler',frame=f)
 # Rigid low-poly sleeve controllers keep the small hands readable in this film.
 arm_groups={g.index for g in mesh.vertex_groups if any(t in g.name for t in ['upperarm','lowerarm','hand','thumb','ring','index','middle','pinky'])}
 remove={v.index for v in mesh.data.vertices if sum(g.weight for g in v.groups if g.group in arm_groups)>.35}
 bm=bmesh.new();bm.from_mesh(mesh.data);bm.verts.ensure_lookup_table();bmesh.ops.delete(bm,geom=[v for v in bm.verts if v.index in remove],context='VERTS');bm.to_mesh(mesh.data);bm.free()
 for sign in [-1,1]:
  sleeve=cube(name+' sleeve',(0,0,0),(.19,.19,.55),mats[5],.025);sleeve.parent=root
  fore=cube(name+' forearm',(0,0,0),(.16,.16,.42),mats[5],.02);fore.parent=root
  hand=cube(name+' hand',(0,0,0),(.17,.20,.12),mats[0],.02);hand.parent=root
  for f in [1,72,144,240,312,408,480,552,672]:
   shoulder=Vector((sign*.32,0,1.17))
   offering=adult and f>=408
   typing=adult and f<313
   elbow=Vector((sign*.36,-(.32 if typing or offering else .03),.87))
   palm=Vector((sign*.28,-(.68 if typing or offering else .07),(.96 if typing else 1.06) if typing or offering else .58))
   palm.z+=.025*math.sin(f*.09+sign)
   for ob,a,b in [(sleeve,shoulder,elbow),(fore,elbow,palm)]:
    ob.location=(a+b)*.5;ob.rotation_euler=(b-a).to_track_quat('Z','Y').to_euler();ob.scale.z=(b-a).length/(.55 if ob==sleeve else .42)
    ob.keyframe_insert('location',frame=f);ob.keyframe_insert('rotation_euler',frame=f);ob.keyframe_insert('scale',frame=f)
   key(hand,f,palm)
 s.frame_set(1)
 return root,arm,mesh
adult,aa,am=character('The maker',(-3,-1.25,0),1.08,True)
adult.rotation_euler.z=math.pi
child,ca,cm=character('My younger self',(2.4,-.1,0),.72)
child.rotation_euler.z=-math.pi/2
key(adult,1,(-3,-1.25,0),rot=(0,0,math.pi));key(adult,312,(-3,-1.25,0),rot=(0,0,math.pi))
key(adult,408,(-.8,0,0),rot=(0,0,math.pi/2));key(adult,672,(-.8,0,0),rot=(0,0,math.pi/2))
# A small traversable world takes shape between the two versions of the player.
for i,(x,y,z) in enumerate([(-.1,0,.25),(.5,.4,.4),(1,.25,.65),(1.55,-.15,.35)]):
 o=cube('a different route '+str(i),(x,y,z),(.42,.42,.15),green if i%2 else teal)
 key(o,1,scale=(.001,)*3);key(o,180+i*12,scale=(.001,)*3);key(o,215+i*12,scale=(1,)*3)
bpy.ops.object.empty_add();gift=bpy.context.object;gift.name='The game as a gift'
for n,loc,sz,m in [('box',(0,0,0),(.65,.65,.55),gold),('ribbon A',(0,0,.01),(.11,.67,.57),pink),('ribbon B',(0,0,.01),(.67,.11,.57),pink)]:o=cube(n,loc,sz,m);o.parent=gift
lid=cube('opening lid',(0,0,.32),(.72,.72,.1),gold);lid.parent=gift
for x in [-.14,.14]:
 o=cube('bow',(x,0,.44),(.28,.12,.1),pink);o.parent=lid;o.location=(x,0,.14);o.rotation_euler.y=x*2
key(gift,1,(-3,.17,2.03),scale=(.001,)*3);key(gift,160,(-3,.17,2.03),scale=(.001,)*3)
key(gift,240,(.7,0,1.4),scale=(1,)*3);key(gift,360,(.7,0,1.4));key(gift,480,(1.7,-.1,1.05));key(gift,672,(1.7,-.1,1.05))
key(lid,1,(0,0,.32));key(lid,492,(0,0,.32));key(lid,552,(0,0,.85),rot=(0,.25,0));key(lid,672,(0,0,.85),rot=(0,.25,0))
for i in range(14):
 a=i*2.399;o=cube('possibility '+str(i),(1.7,-.1,1.1),(.055,)*3,green if i%2 else gold,.005)
 key(o,1,scale=(.001,)*3);key(o,500,scale=(.001,)*3);key(o,540,(1.7+math.cos(a)*.7,-.1+math.sin(a)*.7,1.5+i*.055),scale=(1,)*3);key(o,672,(1.7+math.cos(a)*1.2,-.1+math.sin(a)*1.2,2+i*.055),scale=(.3,)*3)
bpy.ops.object.camera_add();cam=bpy.context.object;s.camera=cam;cam.data.type='ORTHO'
def camera(f,pos,target,lens):
 cam.location=pos;cam.rotation_euler=(Vector(target)-cam.location).to_track_quat('-Z','Y').to_euler();cam.data.ortho_scale=lens
 cam.keyframe_insert('location',frame=f);cam.keyframe_insert('rotation_euler',frame=f);cam.data.keyframe_insert('ortho_scale',frame=f)
camera(1,(-6,-6,4),(-3,0,1.3),5.7);camera(168,(-5.5,-5.3,3.6),(-3,0,1.4),5)
camera(169,(4,-6,5),(.3,0,1),5.5);camera(336,(3.5,-5,4),(.6,0,1.2),4.8)
camera(337,(4,-8,3.8),(.4,0,1.1),7.7);camera(504,(3,-7,3.3),(.5,0,1.1),6.7)
camera(505,(4,-6,3.6),(1.5,0,1.2),5.2);camera(672,(4,-6,3.9),(1.3,0,1.35),6)
# Hold the cuts; interpolate each shot, never travel across a cut.
for action in bpy.data.actions:
 for layer in action.layers:
  for strip in layer.strips:
   for slot in action.slots:
    bag=strip.channelbag(slot)
    if bag:
     for fc in bag.fcurves:
      for k in fc.keyframe_points:
       k.interpolation='CONSTANT' if int(k.co.x) in [168,336,504] and action==cam.animation_data.action else 'LINEAR'
probe={}
for f in [1,408,576]:
 s.frame_set(f);bpy.context.view_layer.update();dg=bpy.context.evaluated_depsgraph_get();ev=am.evaluated_get(dg)
 probe[f]=[list(ev.matrix_world @ ev.data.vertices[i].co) for i in [0,100,200]]
(OUT/'motion-probes.json').write_text(json.dumps(probe,indent=2))
bpy.ops.wm.save_as_mainfile(filepath=str(OUT/'GiftForMyYoungerSelf.blend'))
if '--preview' in sys.argv:
 for f in [72,280,450,600]:s.frame_set(f);s.render.filepath=str(OUT/f'preview-{f}.png');bpy.ops.render.render(write_still=True)
else:
 s.render.filepath=str(OUT/'frames/frame-');bpy.ops.render.render(animation=True)
