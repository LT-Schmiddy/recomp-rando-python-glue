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
import yaml

from pathlib import Path

from RecompSoloServer import start_server, is_port_in_use

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

    recomp_data.item_names = sorted(world.item_names)
    recomp_data.location_names = sorted(world.location_names)

    # debug
    # print(json.dumps(recomp_data.options, indent=4))
    # with recomp_data.mod_data_path.joinpath("output.json").open('w') as f:
    #     f.write(json.dumps(recomp_data.options, indent=4))

def populate_yaml_header(output_file):
    yaml.dump({"name": "Player"}, output_file) # defaulting solo generated yamls to "Player"
    yaml.dump({"game": recomp_data.game_name}, output_file)
    yaml.dump({"requires": {"version": "0.6.7"}}, output_file) # expected archipelago version(?)

def clear_players_folder():
    for yaml_file in recomp_data.mod_data_path.joinpath('Archipelago', 'local', 'Players').glob("*.yaml"):
        yaml_file.unlink(missing_ok=True)

def generate():
    # grabs yaml from mod_data/<mod_id>/Archipelago/local/Players
    try:
        erargs, seed = generate_main()
        multiworld = ERMain(erargs, seed)
        return True
    except Exception as e:
        print(e)
        return False

BASE_SERVER_PORT = 38281
SERVER_PORT_RANGE = 5

def host(multidata = ""):
    # find open port in range to start the solo server on
    # TODO: add possible overrides?
    server_port = 0
    for port_attempt in range(SERVER_PORT_RANGE): # range of 5 above deafult server port (any more is absurd)
        port = BASE_SERVER_PORT + port_attempt
        # print(f"checking if {port} is available: {is_port_in_use(port)}")
        if not is_port_in_use(port):
            server_port = port
            break
    
    if not server_port:
        print(f"No open port found in range [{BASE_SERVER_PORT} - {BASE_SERVER_PORT + SERVER_PORT_RANGE}].")
        recomp_data.error_msg = f"No open port found in range [{BASE_SERVER_PORT} - {BASE_SERVER_PORT + SERVER_PORT_RANGE}]."
        return False

    return start_server(multidata, server_port)