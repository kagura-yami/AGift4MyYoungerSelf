import unreal

folder='/Game/Story/Ending'
assets=unreal.AssetToolsHelpers.get_asset_tools()
def asset(name,cls,factory):
    return unreal.load_asset(folder+'/'+name) or assets.create_asset(name,folder,cls,factory)
source=asset('MS_GiftStorybook',unreal.FileMediaSource,unreal.FileMediaSourceFactoryNew())
source.set_file_path(unreal.Paths.project_content_dir()+'Movies/GiftForMyYoungerSelf_Storybook.mp4')
texture=asset('MT_GiftStorybook',unreal.MediaTexture,unreal.MediaTextureFactoryNew())
texture.set_editor_property('new_style_output',True)
material=unreal.load_asset(folder+'/M_GiftStorybook_UI')
if not material:
    material=assets.create_asset('M_GiftStorybook_UI',folder,unreal.Material,unreal.MaterialFactoryNew())
    material.set_editor_property('material_domain',unreal.MaterialDomain.MD_UI)
    sample=unreal.MaterialEditingLibrary.create_material_expression(material,unreal.MaterialExpressionTextureSampleParameter2D,-300,0)
    sample.set_editor_property('parameter_name','MovieTexture')
    sample.set_editor_property('texture',texture)
    sample.set_editor_property('sampler_type',unreal.MaterialSamplerType.SAMPLERTYPE_COLOR)
    unreal.MaterialEditingLibrary.connect_material_property(sample,'RGB',unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    unreal.MaterialEditingLibrary.recompile_material(material)
for obj in [source,texture,material]:
    unreal.EditorAssetLibrary.save_loaded_asset(obj,False)
bp=unreal.load_asset('/Game/Blueprint/Lv4/BP_ThirdFloorSuitNPC')
cdo=unreal.get_default_object(bp.generated_class())
cdo.modify()
cdo.set_editor_property('ending_movie',source)
cdo.set_editor_property('ending_movie_material',material)
unreal.EditorAssetLibrary.save_loaded_asset(bp,False)
aa=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
for npc in aa.get_all_level_actors():
    if npc.get_actor_label()=='LV4_ThirdFloorSuitNPC':
        npc.modify()
        npc.set_editor_property('ending_movie',source)
        npc.set_editor_property('ending_movie_material',material)
        print('BOUND',npc.get_actor_label(),source.get_editor_property('file_path'))
unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
