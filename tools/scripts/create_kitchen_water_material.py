import unreal
mel=unreal.MaterialEditingLibrary
path='/Game/Materials/Water/M_KitchenWater_Surface'
m=unreal.load_asset(path)
if not m: m=unreal.AssetToolsHelpers.get_asset_tools().create_asset('M_KitchenWater_Surface','/Game/Materials/Water',unreal.Material,unreal.MaterialFactoryNew())
# Custom water output nodes need explicit removal before rebuilding.
for i in range(100):
    old=unreal.find_object(m,'MaterialExpressionSingleLayerWaterMaterialOutput_'+str(i))
    if old: mel.delete_material_expression(m,old)
mel.delete_all_material_expressions(m)
m.set_editor_property('shading_model',unreal.MaterialShadingModel.MSM_SINGLE_LAYER_WATER)
m.set_editor_property('two_sided',True)
m.set_editor_property('tangent_space_normal',False)
def expr(cls):return mel.create_material_expression(m,cls)
def scalar(v,prop=None):
    n=expr(unreal.MaterialExpressionConstant); n.set_editor_property('r',v)
    if prop: assert mel.connect_material_property(n,'',prop)
    return n
def color(v,prop=None):
    n=expr(unreal.MaterialExpressionConstant3Vector);n.set_editor_property('constant',unreal.LinearColor(*v,1))
    if prop: assert mel.connect_material_property(n,'',prop)
    return n
color((.012,.025,.026),unreal.MaterialProperty.MP_BASE_COLOR)
scalar(.22,unreal.MaterialProperty.MP_ROUGHNESS);scalar(.35,unreal.MaterialProperty.MP_SPECULAR);scalar(.12,unreal.MaterialProperty.MP_OPACITY)
p=expr(unreal.MaterialExpressionWorldPosition)
t=expr(unreal.MaterialExpressionScalarParameter);t.set_editor_property('parameter_name','WaterTime')
n=expr(unreal.MaterialExpressionCustom)
names=['P','T']+['Ripple'+str(i) for i in range(8)]
inputs=[]
for name in names:
    pin=unreal.CustomInput();pin.set_editor_property('input_name',name);inputs.append(pin)
n.set_editor_property('inputs',inputs);n.set_editor_property('output_type',unreal.CustomMaterialOutputType.CMOT_FLOAT4)
code="""float2 q=P.xy;
float a=q.x*.046+q.y*.024+T*.7;
float b=q.x*-.021+q.y*.057-T*.91;
float c=q.x*.21+q.y*.18+T*1.4;
float2 slope=float2(.035*cos(a)-.02*cos(b)+.008*cos(c),.02*cos(a)+.035*cos(b)+.006*cos(c));
float crest=0;
"""
for i in range(8):
    code+="""{
float4 r=Ripple%d;
float age=T-r.z;
float2 delta=q-r.xy;
float d=length(delta);
float front=d-70-age*95;
float envelope=exp(-front*front/1100)*saturate(age*8)*saturate(1-age/3.5)*r.w;
float wave=sin(front*.22)*envelope*.65;
slope+=delta/max(d,1)*wave;
crest+=pow(saturate(cos(front*.22)),10)*envelope;
}
""" % i
code+='return float4(normalize(float3(slope,1)),saturate(crest));'
n.set_editor_property('code',code)
assert mel.connect_material_expressions(p,'',n,'P');assert mel.connect_material_expressions(t,'',n,'T')
for i in range(8):
    v=expr(unreal.MaterialExpressionVectorParameter)
    v.set_editor_property('parameter_name','Ripple'+str(i))
    v.set_editor_property('default_value',unreal.LinearColor(0,0,-100,0))
    assert mel.connect_material_expressions(v,'RGBA',n,'Ripple'+str(i))
mask=expr(unreal.MaterialExpressionComponentMask)
mask.set_editor_property('r',True);mask.set_editor_property('g',True);mask.set_editor_property('b',True);mask.set_editor_property('a',False)
assert mel.connect_material_expressions(n,'',mask,'')
assert mel.connect_material_property(mask,'',unreal.MaterialProperty.MP_NORMAL)
shade=expr(unreal.MaterialExpressionCustom)
pin=unreal.CustomInput();pin.set_editor_property('input_name','Wave');shade.set_editor_property('inputs',[pin])
shade.set_editor_property('output_type',unreal.CustomMaterialOutputType.CMOT_FLOAT3)
shade.set_editor_property('code','return lerp(float3(.012,.025,.026),float3(.12,.20,.20),Wave.a*.7);')
assert mel.connect_material_expressions(n,'',shade,'Wave')
assert mel.connect_material_property(shade,'',unreal.MaterialProperty.MP_BASE_COLOR)

w=expr(unreal.MaterialExpressionSingleLayerWaterMaterialOutput)
print('water inputs',mel.get_material_expression_input_names(w))
for name,value in [('ScatteringCoefficients',(.001,.0025,.003)),('AbsorptionCoefficients',(.022,.009,.006)),('ColorScaleBehindWater',(1,1,1))]:
    assert mel.connect_material_expressions(color(value),'',w,name),name
assert mel.connect_material_expressions(scalar(.25),'',w,'PhaseG')
mel.recompile_material(m); unreal.EditorAssetLibrary.save_loaded_asset(m)
print('WATER_MATERIAL_READY')


