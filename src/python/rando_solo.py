import worlds
from worlds.AutoWorld import AutoWorldRegister, World
# from worlds.mm_recomp import MMRWorld # temp, apworld needs to be loaded for auto world discovery
from Generate import main as generate_main
from Main import main as ERMain

worlds.network_data_package = DataPackage = {
    "games": {world_name: world.get_data_package_data() for world_name, world in AutoWorldRegister.world_types.items()},
}

def main():
    erargs, seed = generate_main()
    multiworld = ERMain(erargs, seed)