import asyncio
import time
import typing
import json
from pathlib import Path
from copy import deepcopy

import Utils
import websockets
import functools
from NetUtils import decode, encode, JSONtoTextParser, JSONMessagePart, NetworkItem, NetworkPlayer, ClientStatus
from CommonClient import CommonContext, server_loop

import recomp_data

# APCONNECT

# yes this still saves as "apconnect.txt" for the bit, even though a json would probably be better
def save_ap_connect(address, player_name, password):
    ap_connect = Path(recomp_data.mod_data_path, "apconnect.txt")
    ap_connect.write_text(f"{address}\n{player_name}\n{password}")

def get_ap_connect():
    ap_connect = Path(recomp_data.mod_data_path, "apconnect.txt")
    
    if not ap_connect.exists(): # first time running the randomizer
        ap_connect.write_text("archipelago.gg:38281\nPlayer\n")

    connection_info = ap_connect.read_text().splitlines()
    
    if len(connection_info) < 3:
        connection_info.append("") # empty password
    
    return connection_info

# SAVES

def additional_data_exists(key):
    return key in recomp_data.ctx.additional_data

def set_additional_data(key, data): # if games need to save additional data (per slot for now)
    ctx = recomp_data.ctx
    ctx.additional_data[key] = data

def get_additional_data(key):
    return recomp_data.ctx.additional_data[key]

def save_current_state(slot = 0):
    ctx = recomp_data.ctx
    data_path = Path(recomp_data.mod_data_path).joinpath("save_data")
    data_path.mkdir(parents=True, exist_ok=True)
    file_name = f"multi_{ctx.seed_name}_{ctx.player_names[ctx.slot]}.json" # TODO: account for solo
    save_data = deepcopy(ctx.save_data) # dumb python referencing bs

    # fill blank slots with dummy data
    while len(save_data) <= slot:
        blank_save_data = {
            "checked_locations": set(),
            "received_items": [],
            "additional_data": dict()
        }
        save_data.append(blank_save_data)

    save_data[slot] = {
        "checked_locations": deepcopy(ctx.local_checked),
        "received_items": [item._asdict() for item in ctx.local_received],
        "additional_data": deepcopy(ctx.additional_data)
    }

    with open(data_path.joinpath(file_name), "w") as f:
        f.write(json.dumps(save_data, default=list, indent=4)) # dumb thing but i don't like how lists look in json lol
    ctx.save_data = save_data

async def load_saved_state_from_slot(slot = 0):
    try:
        ctx = recomp_data.ctx
        saved_data = ctx.save_data[slot]
        
        checked_locations = set(saved_data["checked_locations"])
        received_items = [NetworkItem(item["item"], item["location"], item["player"], item["flags"])
                          for item in saved_data["received_items"]]
        additional_data = saved_data["additional_data"]

        ctx.local_checked |= checked_locations
        ctx.local_received = received_items
        ctx.additional_data = additional_data

        ctx.locations_checked |= ctx.local_checked
        await ctx.send_msgs([{"cmd": "LocationChecks", "locations": list(ctx.locations_checked)}]) # send locations checked offline
    except FileNotFoundError:
        pass
    except Exception as e:
        print("failed", e.__class__.__name__, e)
        pass

def load_saved_state():
    try:
        ctx = recomp_data.ctx
        data_path = Path(recomp_data.mod_data_path).joinpath("save_data")
        file_name = f"multi_{ctx.seed_name}_{ctx.player_names[ctx.slot]}.json" # TODO: account for solo

        with open(data_path.joinpath(file_name), "r") as f:
            saved_data = json.load(f)
        
        ctx.save_data = saved_data
    except FileNotFoundError:
        pass
    except Exception as e:
        print("failed", e.__class__.__name__, e)
        pass