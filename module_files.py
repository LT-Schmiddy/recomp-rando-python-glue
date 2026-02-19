import os
from pathlib import Path

root_dir = Path(__file__).parent
pyyaml_src_dir = root_dir.joinpath("./libs/pyyaml/lib/yaml")
typing_extensions_src_dir = root_dir.joinpath("./libs/typing_extensions/src")
websockets_src_dir = root_dir.joinpath("./libs/websockets/src/websockets")
archipelago_src_dir = root_dir.joinpath("./libs/Archipelago")

nrm_archipelago_dir = Path("archipelago")

include_python_files: dict[Path, Path] = {
    # Including single files
    # Modules
    Path("schema.py"): root_dir.joinpath("./libs/schema/schema.py"),

    # Archipelago Base Files
    nrm_archipelago_dir.joinpath("BaseClasses.py"): archipelago_src_dir.joinpath("./BaseClasses.py"),
    nrm_archipelago_dir.joinpath("Generate.py"): archipelago_src_dir.joinpath("./Generate.py"),
    nrm_archipelago_dir.joinpath("Fill.py"): archipelago_src_dir.joinpath("./Fill.py"),
    nrm_archipelago_dir.joinpath("Main.py"): archipelago_src_dir.joinpath("./Main.py"),
    # Path("ModuleUpdate.py"): root_dir.joinpath("./src/python/ModuleUpdate.py"), # dummy `ModuleUpdate.py`
    nrm_archipelago_dir.joinpath("MultiServer.py"): archipelago_src_dir.joinpath("./MultiServer.py"),
    nrm_archipelago_dir.joinpath("NetUtils.py"): archipelago_src_dir.joinpath("./NetUtils.py"),
    nrm_archipelago_dir.joinpath("Options.py"): archipelago_src_dir.joinpath("./Options.py"),
    nrm_archipelago_dir.joinpath("Utils.py"): archipelago_src_dir.joinpath("./Utils.py"),
    nrm_archipelago_dir.joinpath("settings.py"): archipelago_src_dir.joinpath("./settings.py"),
    nrm_archipelago_dir.joinpath("requirements.txt"): archipelago_src_dir.joinpath("./requirements.txt"), # ?
    # Path("CustomGenerate.py"): root_dir.joinpath("./src/python/CustomGenerate.py"), # a custom `Generate.py` for custom json output

    # Archipelago World Files
    nrm_archipelago_dir.joinpath("worlds/__init__.py"): archipelago_src_dir.joinpath("./worlds/__init__.py"),
    # Path("worlds/__init__.py"): root_dir.joinpath("./src/python/worlds_init.py"), # a custom `__init__.py` meant to add loading zip archives
    nrm_archipelago_dir.joinpath("worlds/AutoSNIClient.py"): archipelago_src_dir.joinpath("./worlds/AutoSNIClient.py"),
    nrm_archipelago_dir.joinpath("worlds/AutoWorld.py"): archipelago_src_dir.joinpath("./worlds/AutoWorld.py"),
    nrm_archipelago_dir.joinpath("worlds/Files.py"): archipelago_src_dir.joinpath("./worlds/Files.py"),
    nrm_archipelago_dir.joinpath("worlds/LauncherComponents.py"): archipelago_src_dir.joinpath("./worlds/LauncherComponents.py"),
    nrm_archipelago_dir.joinpath("worlds/alttp/EntranceRandomizer.py"): archipelago_src_dir.joinpath("./worlds/alttp/EntranceRandomizer.py"),
    nrm_archipelago_dir.joinpath("worlds/alttp/Text.py"): archipelago_src_dir.joinpath("./worlds/alttp/Text.py"),
    nrm_archipelago_dir.joinpath("worlds/alttp/__init__.py"): archipelago_src_dir.joinpath("./worlds/alttp/__init__.py"),
    # Path("worlds/__init__.py"): root_dir.joinpath("./src/python/worlds_init.py"), # dummy `__init__.py`
}

def populate_file_injection(injections: dict[Path, Path], inject_root: Path, search_dir: Path):
    for inject_path, file_path in [(inject_root.joinpath(i), search_dir.joinpath(i)) for i in os.listdir(search_dir)]:
        if file_path.is_file() and file_path.suffix == ".py":
            injections[inject_path] = file_path
        elif file_path.is_dir():
            populate_file_injection(injections, inject_path, file_path)

# Including entire directories            
populate_file_injection(include_python_files, Path("yaml"), pyyaml_src_dir)
populate_file_injection(include_python_files, Path("typing_extensions"), typing_extensions_src_dir)
populate_file_injection(include_python_files, Path("websockets"), websockets_src_dir)

populate_file_injection(include_python_files, Path("worlds/generic"), archipelago_src_dir.joinpath("./worlds/generic"))
