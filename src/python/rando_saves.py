import asyncio
import time
import typing
import json
from pathlib import Path

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

def save_current_state(slot = 0):
    ctx = recomp_data.ctx
    data_path = Path(recomp_data.mod_data_path).joinpath("save_data")
    data_path.mkdir(parents=True, exist_ok=True)
    file_name = f"multi_{ctx.seed_name}_{ctx.player_names[ctx.slot]}.json" # TODO: account for solo
    
    # fill blank slots with dummy data
    while len(ctx.save_data) <= slot:
        blank_save_data = {
            "checked_locations": set(),
            "received_items": []
        }
        ctx.save_data.append(blank_save_data)

    ctx.save_data[slot] = {
        "checked_locations": ctx.local_checked,
        "received_items": [item._asdict() for item in ctx.local_received]
    }

    with open(data_path.joinpath(file_name), "w") as f:
        f.write(json.dumps(ctx.save_data, default=list, indent=4)) # dumb thing but i don't like how lists look in json lol

async def load_saved_state_from_slot(slot = 0):
    try:
        ctx = recomp_data.ctx
        saved_data = ctx.save_data[slot]
        
        checked_locations = set(saved_data["checked_locations"])
        received_items = [NetworkItem(item["item"], item["location"], item["player"], item["flags"])
                          for item in saved_data["received_items"]]

        ctx.local_checked |= checked_locations
        ctx.local_received = received_items
        await ctx.send_msgs([{"cmd": "LocationChecks", "locations": list(ctx.locations_checked)}]) # send locations checked offline
    except Exception as e:
        print("failed", e)
        pass

def load_saved_state():
    try:
        ctx = recomp_data.ctx
        data_path = Path(recomp_data.mod_data_path).joinpath("save_data")
        file_name = f"multi_{ctx.seed_name}_{ctx.player_names[ctx.slot]}.json" # TODO: account for solo

        with open(data_path.joinpath(file_name), "r") as f:
            saved_data = json.load(f)
        
        ctx.save_data = saved_data
    except Exception as e:
        print("failed", e)
        pass