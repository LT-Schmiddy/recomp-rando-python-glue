import os
from pathlib import Path

root_dir = Path(__file__).parent
pyyaml_src_dir = root_dir.joinpath("./libs/pyyaml/lib/yaml")
typing_extensions_src_dir = root_dir.joinpath("./libs/typing_extensions/src")
websockets_src_dir = root_dir.joinpath("./libs/websockets/src/websockets")


include_python_files: dict[Path, Path] = {
    # including single file:
    Path("schema.py"): root_dir.joinpath("./libs/schema/schema.py")
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
