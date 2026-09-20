"""Validate authored data and all /Game package dependencies before cook."""
import json
from pathlib import Path
import unreal

def run(report_path):
    registry=unreal.AssetRegistryHelpers.get_asset_registry()
    validator=unreal.get_editor_subsystem(unreal.EditorValidatorSubsystem)
    assets=registry.get_assets_by_path('/Game',recursive=True)
    checked=[]
    for data in assets:
        package=str(data.package_name)
        if package.startswith('/Game/Data/'):
            result,errors,warnings=validator.is_asset_valid(data,unreal.DataValidationUsecase.MANUAL)
            checked.append({'package':package,'result':str(result),'errors':[str(x) for x in errors],'warnings':[str(x) for x in warnings]})
            assert result==unreal.DataValidationResult.VALID,package+' failed native validation'
        deps=registry.get_dependencies(data.package_name,unreal.AssetRegistryDependencyOptions(include_soft_package_references=True,include_hard_package_references=True))
        for dep in deps:
            name=str(dep)
            if name.startswith('/Game/'):
                assert unreal.EditorAssetLibrary.does_asset_exist(name),'Missing dependency '+name
    report={'ok':True,'packages':len(assets),'data':checked}
    Path(report_path).write_text(json.dumps(report,ensure_ascii=False,indent=2))
    return report
