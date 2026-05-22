import worlds
from worlds.AutoWorld import AutoWorldRegister
from Generate import main as generate_main
from Main import main as ERMain

worlds.network_data_package = DataPackage = {
    "games": {world_name: world.get_data_package_data() for world_name, world in AutoWorldRegister.world_types.items()},
}

def main():
    erargs, seed = generate_main()
    multiworld = ERMain(erargs, seed)