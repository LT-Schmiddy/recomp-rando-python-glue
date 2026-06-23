# this module exists to store variables that can be used in other python code easily

ctx = None
game_name = None
mod_data_path = None
queued_scouts = set()
queued_locations = set() # unsure if this will actually get used
last_location_sent = 0
error_msg = "" # TODO: move error messages out of ctx into here

# solo
server_ctx = None
options = dict()
solo_seeds = list() # stores a list of all seed files in the output directory
is_solo = False