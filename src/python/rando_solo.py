import worlds
from worlds.AutoWorld import AutoWorldRegister, World
# from worlds.mm_recomp import MMRWorld # temp, apworld needs to be loaded for auto world discovery
from Generate import main as generate_main
from Main import main as ERMain
from Options import get_option_groups, Choice, Toggle, DefaultOnToggle, Range, OptionList, NumericOption

# apworld needs to be loaded before this module is imported for auto world discovery
worlds.network_data_package = DataPackage = {
    "games": {world_name: world.get_data_package_data() for world_name, world in AutoWorldRegister.world_types.items()},
}

import json

from pathlib import Path

from RecompSoloServer import start_server

import recomp_data

def populate_options():
    # initially from generate_yaml_templates in Options.py
    world = AutoWorldRegister.world_types[recomp_data.game_name]

    # unused, maybe could be utilized in the future?
    presets = world.web.options_presets.copy()
    presets.update({"": {}})

    option_groups = get_option_groups(world)
    for group_name, option_group in option_groups.items():
        recomp_data.options[group_name] = dict()
        for option_id, option_class in option_group.items():
            recomp_data.options[group_name][option_id] = dict()
            cur_option = recomp_data.options[group_name][option_id]
            
            try:
                cur_option["name"] = option_class.display_name
            except:
                cur_option["name"] = option_id # fallback if display name is not found
                pass
            
            cur_option["description"] = option_class.__doc__
            if option_class.__doc__ == None:
                cur_option["description"] = "" # safety blank string
            
            cur_option["type"] = option_class.__bases__[0].__name__
            if issubclass(option_class, Choice):
                cur_option["choices"] = option_class.options
            if issubclass(option_class, Range):
                cur_option["range_start"] = option_class.range_start
                cur_option["range_end"] = option_class.range_end
            if issubclass(option_class, NumericOption): # toggle/other
                cur_option["default"] = option_class.default

    # debug
    # print(json.dumps(recomp_data.options, indent=4))
    # with recomp_data.mod_data_path.joinpath("output.json").open('w') as f:
    #     f.write(json.dumps(recomp_data.options, indent=4))

# generate() and solo() are temporary WIP as things are being worked out
def generate():
    # grabs yaml from mod_data/<mod_id>/Archipelago/local/Players
    # needs a yaml to manually be placed there for now as solo menu doesn't do so yet
    erargs, seed = generate_main()
    multiworld = ERMain(erargs, seed)

def host():
    start_server()