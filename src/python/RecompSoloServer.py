# import NetUtils
import Utils
# from Utils import version_tuple, restricted_loads, Version, async_start, get_intended_text
# from NetUtils import Endpoint, ClientStatus, NetworkItem, decode, encode, NetworkPlayer, Permission, NetworkSlot, \
#     SlotType, LocationStore, MultiData, Hint, HintStatus
# from BaseClasses import ItemClassification

from MultiServer import Context, server, console, auto_shutdown, server_per_message_deflate_factory

# unsure if this/above is needed for the actual implementation, leaving just in case
# class RecompServerContext(Context):
# 	# def load(self, multidatapath: str, use_embedded_server_options: bool = False):

from MultiServer import main as server_main
from MultiServer import parse_args

import recomp_data
from rando_async_controller import AsyncLoopThread

from pathlib import Path

async_thread = AsyncLoopThread()
async_thread.start()

def start_server():
	global async_thread
	
	# grab latest generated seed for testing
	output_folder = recomp_data.mod_data_path.joinpath("Archipelago", "local", "output")
	output_files = output_folder.glob("*.zip")
	latest_file = max(output_files, key=lambda item: item.stat().st_ctime)

	args = parse_args()
	# args.multidata = str(generated_data)
	args.multidata = str(latest_file)
	print(str(latest_file))
	args.host = "localhost"
	async_thread.enqueue(server_main(args))