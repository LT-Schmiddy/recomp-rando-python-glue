# this module exists to store variables that can be used in other python code easily

ctx = None
game_name = None
recomp_mod_id = None
mod_data_path = None
queued_scouts = set()
queued_locations = set() # unsure if this will actually get used
last_location_sent = 0
error_msg = "" # TODO: move error messages out of ctx into here
game_data = dict() # meant for easy global storage of data

# solo
server_ctx = None
solo_seeds = list() # stores a list of all seed files in the output directory
solo_yamls = list() # stores a list of all yamls in the "solo_yamls" directory
options = dict()
solo_yaml_options = dict() # could probably be done better
last_generated_yaml_location = None
item_names = set()
location_names = set()
is_solo = False